"""Evaluate a trained surrogate by autoregressive rollout against FEM truth.

Loads a checkpoint written by train.py, rolls out one or more samples from
their initial condition, and prints per-frame RMSE. Optionally writes the
predicted trajectory next to the ground truth for later inspection.

Example:
    python ml/rollout.py --ckpt ml/checkpoints/model.pt --sample 0
"""

import argparse
import os

import torch

from dataset import HeatDataset
from model import MeshGraphNet, rollout


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ckpt", default="ml/checkpoints/model.pt")
    ap.add_argument("--data", default=None,
                    help="Override dataset root (defaults to the trained one).")
    ap.add_argument("--sample", type=int, default=None,
                    help="Sample id to roll out (default: first held-out one).")
    ap.add_argument("--dump", default=None,
                    help="Optional path to save predicted vs true CSV.")
    args = ap.parse_args()

    ck = torch.load(args.ckpt, map_location="cpu", weights_only=False)
    ds = HeatDataset(args.data or ck["data_root"])
    ds.load_stats(ck["stats"])

    cfg = ck["config"]
    model = MeshGraphNet(cfg["node_in"], cfg["edge_in"], cfg["hidden"], cfg["blocks"])
    model.load_state_dict(ck["model"])

    sid = args.sample
    if sid is None:
        sid = ck.get("val_ids", [0])[0]

    pred = rollout(model, ds, sid)
    true = ds.frames[sid]

    per_frame = ((pred - true) ** 2).mean(dim=1).sqrt()
    print(f"sample {sid}: rollout over {ds.F} frames, {ds.N} nodes")
    print(f"  frame   1 RMSE: {per_frame[1].item():.4e}")
    print(f"  frame {ds.F // 2:3d} RMSE: {per_frame[ds.F // 2].item():.4e}")
    print(f"  frame {ds.F - 1:3d} RMSE: {per_frame[-1].item():.4e}")
    print(f"  trajectory mean RMSE: {per_frame.mean().item():.4e}")

    if args.dump:
        os.makedirs(os.path.dirname(args.dump) or ".", exist_ok=True)
        with open(args.dump, "w") as f:
            f.write("frame,node,pred,true\n")
            for t in range(ds.F):
                for i in range(ds.N):
                    f.write(f"{t},{i},{pred[t, i].item():.6g},{true[t, i].item():.6g}\n")
        print(f"  wrote {args.dump}")


if __name__ == "__main__":
    main()
