"""Load a transient-heat dataset produced by tools/gen_heat_dataset.cpp.

The mesh / geometry is fixed across samples, so the graph (node coordinates and
edges) is shared and loaded once. Each sample contributes a temperature
trajectory (num_frames x num_nodes) plus its scalar parameters (alpha, kappa).

Everything is exposed as torch tensors; the only third-party dependency is
torch itself (message passing is implemented with index_add_, so neither
numpy nor torch_geometric is required).
"""

import csv
import json
import os

import torch


class HeatDataset:
    def __init__(self, root, device="cpu"):
        self.root = root
        self.device = torch.device(device)

        with open(os.path.join(root, "dataset.json")) as f:
            self.meta = json.load(f)
        self.dim = int(self.meta["dim"])
        self.N = int(self.meta["num_nodes"])
        self.F = int(self.meta["num_frames"])
        self.S = int(self.meta["num_samples"])

        self.coords = self._load_nodes()                  # (N, dim)
        self.edge_index = self._load_edges()              # (2, 2M) both directions
        self.edge_attr = self._edge_features()            # (2M, dim+1)
        self.frames, self.params = self._load_samples()   # (S,F,N), (S,2)

    # ---- loading -----------------------------------------------------------
    def _load_nodes(self):
        coords = []
        with open(os.path.join(self.root, "graph_nodes.csv")) as f:
            r = csv.reader(f)
            next(r)
            for row in r:
                coords.append([float(v) for v in row])
        return torch.tensor(coords, dtype=torch.float32, device=self.device)

    def _load_edges(self):
        src, dst = [], []
        with open(os.path.join(self.root, "graph_edges.csv")) as f:
            r = csv.reader(f)
            next(r)
            for a, b in r:
                a, b = int(a), int(b)
                src += [a, b]   # undirected -> add both directions
                dst += [b, a]
        return torch.tensor([src, dst], dtype=torch.long, device=self.device)

    def _edge_features(self):
        s, r = self.edge_index[0], self.edge_index[1]
        rel = self.coords[s] - self.coords[r]             # sender - receiver
        dist = rel.norm(dim=1, keepdim=True)
        return torch.cat([rel, dist], dim=1)

    def _load_samples(self):
        manifest = {}
        with open(os.path.join(self.root, "manifest.csv")) as f:
            for row in csv.DictReader(f):
                manifest[int(row["sample"])] = row

        frames = torch.empty(self.S, self.F, self.N, dtype=torch.float32)
        params = torch.empty(self.S, 2, dtype=torch.float32)
        for sid in range(self.S):
            traj = []
            path = os.path.join(self.root, f"sample_{sid:05d}", "frames.csv")
            with open(path) as f:
                for line in f:
                    if line.strip():
                        traj.append([float(v) for v in line.split(",")])
            frames[sid] = torch.tensor(traj, dtype=torch.float32)
            params[sid, 0] = float(manifest[sid]["alpha"])
            params[sid, 1] = float(manifest[sid]["kappa"])
        return frames.to(self.device), params.to(self.device)

    # ---- normalization -----------------------------------------------------
    def compute_stats(self, train_ids):
        """Compute normalization statistics from the training split only."""
        idx = torch.tensor(train_ids, device=self.device)
        fr = self.frames.index_select(0, idx)             # (T, F, N)

        u = fr.reshape(-1)
        self.u_mean, self.u_std = u.mean(), u.std().clamp_min(1e-8)

        delta = (fr[:, 1:] - fr[:, :-1]).reshape(-1)
        self.d_mean, self.d_std = delta.mean(), delta.std().clamp_min(1e-8)

        self.c_mean = self.coords.mean(0)
        self.c_std = self.coords.std(0).clamp_min(1e-8)

        p = self.params.index_select(0, idx)
        self.p_mean = p.mean(0)
        self.p_std = p.std(0).clamp_min(1e-8)

        self.e_mean = self.edge_attr.mean(0)
        self.e_std = self.edge_attr.std(0).clamp_min(1e-8)

        # Precompute the normalized static quantities (same for all samples).
        self.coords_norm = (self.coords - self.c_mean) / self.c_std
        self.edge_attr_norm = (self.edge_attr - self.e_mean) / self.e_std

    def stats_dict(self):
        keys = ["u_mean", "u_std", "d_mean", "d_std", "c_mean", "c_std",
                "p_mean", "p_std", "e_mean", "e_std"]
        return {k: getattr(self, k) for k in keys}

    def load_stats(self, d):
        for k, v in d.items():
            setattr(self, k, v.to(self.device))
        self.coords_norm = (self.coords - self.c_mean) / self.c_std
        self.edge_attr_norm = (self.edge_attr - self.e_mean) / self.e_std

    # ---- batching ----------------------------------------------------------
    def build_inputs(self, u_t, params):
        """Assemble batched GNN inputs for B graph instances sharing topology.

        u_t:    (B, N) current temperature field
        params: (B, 2) [alpha, kappa] per instance
        returns x (B*N, node_in), edge_index (2, B*E), edge_attr (B*E, edge_in)
        """
        B, N = u_t.shape
        u_feat = ((u_t - self.u_mean) / self.u_std).reshape(B * N, 1)
        coords_rep = self.coords_norm.repeat(B, 1)
        p_norm = (params - self.p_mean) / self.p_std
        p_rep = p_norm.repeat_interleave(N, dim=0)
        x = torch.cat([u_feat, coords_rep, p_rep], dim=1)

        E = self.edge_index.shape[1]
        offsets = (torch.arange(B, device=self.device) * N).repeat_interleave(E)
        edge_index = self.edge_index.repeat(1, B) + offsets.unsqueeze(0)
        edge_attr = self.edge_attr_norm.repeat(B, 1)
        return x, edge_index, edge_attr

    @property
    def node_in_dim(self):
        return 1 + self.dim + 2   # u, coords, [alpha, kappa]

    @property
    def edge_in_dim(self):
        return self.dim + 1       # relative position + distance
