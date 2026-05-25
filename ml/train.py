"""Train the MeshGraphNet surrogate on a transient-heat dataset.

Learns the one-step map u^t -> u^{t+1} (as a normalized increment) and reports
both the next-step error and the full autoregressive rollout error against the
FEM ground truth on held-out samples.

Example:
    python ml/train.py --data ml/data/box_small --epochs 30
"""

import argparse
import os
import random

import torch

from dataset import HeatDataset
from model import MeshGraphNet, rollout


def split_samples(num_samples, val_frac, seed):
    ids = list(range(num_samples))
    random.Random(seed).shuffle(ids)
    n_val = max(1, int(round(num_samples * val_frac)))
    return ids[n_val:], ids[:n_val]   # train, val


@torch.no_grad()
def next_step_rmse(model, ds, sample_ids, chunk=8):
    model.eval()
    se, cnt = 0.0, 0
    for s in sample_ids:
        params1 = ds.params[s:s + 1]
        for a in range(0, ds.F - 1, chunk):   # chunk frames to cap batch size
            b = min(a + chunk, ds.F - 1)
            u_t = ds.frames[s, a:b]
            u_tp = ds.frames[s, a + 1:b + 1]
            x, ei, ea = ds.build_inputs(u_t, params1.expand(u_t.shape[0], 2))
            delta = model(x, ei, ea).squeeze(1).reshape(u_t.shape)
            pred = u_t + delta * ds.d_std + ds.d_mean
            se += ((pred - u_tp) ** 2).sum().item()
            cnt += u_tp.numel()
    return (se / cnt) ** 0.5


@torch.no_grad()
def rollout_rmse(model, ds, sample_ids):
    total, final = 0.0, 0.0
    for s in sample_ids:
        pred = rollout(model, ds, s)
        true = ds.frames[s]
        total += ((pred - true) ** 2).mean().item() ** 0.5
        final += ((pred[-1] - true[-1]) ** 2).mean().item() ** 0.5
    n = len(sample_ids)
    return total / n, final / n


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--data", default="ml/data/box_small")
    ap.add_argument("--out", default="ml/checkpoints/model.pt")
    ap.add_argument("--epochs", type=int, default=30)
    ap.add_argument("--batch", type=int, default=8)
    ap.add_argument("--hidden", type=int, default=64)
    ap.add_argument("--blocks", type=int, default=5)
    ap.add_argument("--lr", type=float, default=1e-3)
    ap.add_argument("--val-frac", type=float, default=0.25)
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--threads", type=int, default=0)
    args = ap.parse_args()

    if args.threads > 0:
        torch.set_num_threads(args.threads)
    torch.manual_seed(args.seed)

    ds = HeatDataset(args.data)
    train_ids, val_ids = split_samples(ds.S, args.val_frac, args.seed)
    ds.compute_stats(train_ids)
    print(f"dataset: {ds.S} samples (train {len(train_ids)}, val {len(val_ids)}), "
          f"N={ds.N} nodes, E={ds.edge_index.shape[1]} directed edges, "
          f"F={ds.F} frames, dim={ds.dim}")

    transitions = [(s, t) for s in train_ids for t in range(ds.F - 1)]

    model = MeshGraphNet(ds.node_in_dim, ds.edge_in_dim, args.hidden, args.blocks)
    n_params = sum(p.numel() for p in model.parameters())
    print(f"model: MeshGraphNet hidden={args.hidden} blocks={args.blocks} "
          f"({n_params:,} params)")

    opt = torch.optim.Adam(model.parameters(), lr=args.lr)
    loss_fn = torch.nn.MSELoss()

    for ep in range(1, args.epochs + 1):
        model.train()
        random.Random(args.seed + ep).shuffle(transitions)
        running, nb = 0.0, 0
        for i in range(0, len(transitions), args.batch):
            batch = transitions[i:i + args.batch]
            s_idx = torch.tensor([b[0] for b in batch], device=ds.device)
            t_idx = torch.tensor([b[1] for b in batch], device=ds.device)
            u_t = ds.frames[s_idx, t_idx]
            u_tp = ds.frames[s_idx, t_idx + 1]
            params = ds.params[s_idx]

            x, ei, ea = ds.build_inputs(u_t, params)
            target = ((u_tp - u_t - ds.d_mean) / ds.d_std).reshape(-1, 1)
            pred = model(x, ei, ea)
            loss = loss_fn(pred, target)

            opt.zero_grad()
            loss.backward()
            opt.step()
            running += loss.item()
            nb += 1

        if ep % 5 == 0 or ep == 1 or ep == args.epochs:
            ns = next_step_rmse(model, ds, val_ids)
            print(f"epoch {ep:3d}  train_loss {running / nb:.4f}  "
                  f"val_next_step_rmse {ns:.4e}")

    roll_mean, roll_final = rollout_rmse(model, ds, val_ids)
    print(f"\nHeld-out rollout RMSE (full trajectory): {roll_mean:.4e}")
    print(f"Held-out rollout RMSE (final frame):     {roll_final:.4e}")
    # Reference scale: typical temperature spread in the data.
    spread = (ds.frames.max() - ds.frames.min()).item()
    print(f"(temperature range in dataset ~ {spread:.3f}; "
          f"relative final-frame error ~ {roll_final / spread * 100:.2f}%)")

    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    torch.save({
        "model": model.state_dict(),
        "stats": ds.stats_dict(),
        "config": {"hidden": args.hidden, "blocks": args.blocks,
                   "node_in": ds.node_in_dim, "edge_in": ds.edge_in_dim},
        "data_root": args.data,
        "val_ids": val_ids,
    }, args.out)
    print(f"saved checkpoint -> {args.out}")


if __name__ == "__main__":
    main()
