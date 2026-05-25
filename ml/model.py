"""MeshGraphNet-style surrogate for transient heat diffusion.

Encode-process-decode graph network (Pfaff et al., 2021) implemented in plain
PyTorch. The processor runs L message-passing blocks with residual updates;
message aggregation uses scatter-add (index_add_), so torch_geometric is not
needed. The model predicts the normalized per-node increment delta = u^{t+1} -
u^t; rollout integrates these increments from the initial condition.
"""

import torch
import torch.nn as nn


def mlp(in_dim, hidden, out_dim, num_layers=2, layernorm=True):
    layers = [nn.Linear(in_dim, hidden), nn.SiLU()]
    for _ in range(num_layers - 1):
        layers += [nn.Linear(hidden, hidden), nn.SiLU()]
    layers += [nn.Linear(hidden, out_dim)]
    if layernorm:
        layers += [nn.LayerNorm(out_dim)]
    return nn.Sequential(*layers)


class GraphNetBlock(nn.Module):
    """One message-passing step with residual edge and node updates."""

    def __init__(self, hidden):
        super().__init__()
        self.edge_mlp = mlp(3 * hidden, hidden, hidden)
        self.node_mlp = mlp(2 * hidden, hidden, hidden)

    def forward(self, node, edge, edge_index):
        s, r = edge_index[0], edge_index[1]
        edge_in = torch.cat([edge, node[s], node[r]], dim=1)
        edge = edge + self.edge_mlp(edge_in)

        agg = torch.zeros_like(node)
        agg.index_add_(0, r, edge)               # sum incoming messages
        node = node + self.node_mlp(torch.cat([node, agg], dim=1))
        return node, edge


class MeshGraphNet(nn.Module):
    def __init__(self, node_in, edge_in, hidden=64, num_blocks=5):
        super().__init__()
        self.node_encoder = mlp(node_in, hidden, hidden)
        self.edge_encoder = mlp(edge_in, hidden, hidden)
        self.blocks = nn.ModuleList(
            [GraphNetBlock(hidden) for _ in range(num_blocks)])
        self.decoder = mlp(hidden, hidden, 1, layernorm=False)

    def forward(self, x, edge_index, edge_attr):
        node = self.node_encoder(x)
        edge = self.edge_encoder(edge_attr)
        for block in self.blocks:
            node, edge = block(node, edge, edge_index)
        return self.decoder(node)                # (num_nodes, 1) normalized delta


@torch.no_grad()
def rollout(model, ds, sid):
    """Autoregressively predict the full trajectory of sample `sid`.

    Starts from the true initial condition (frame 0) and repeatedly applies the
    learned increment. Returns a (F, N) tensor of predicted temperatures.
    """
    model.eval()
    u = ds.frames[sid, 0].clone()
    params = ds.params[sid:sid + 1]
    pred = torch.empty(ds.F, ds.N, device=ds.device)
    pred[0] = u
    for t in range(1, ds.F):
        x, edge_index, edge_attr = ds.build_inputs(u.unsqueeze(0), params)
        delta_norm = model(x, edge_index, edge_attr).squeeze(1)
        u = u + delta_norm * ds.d_std + ds.d_mean
        pred[t] = u
    return pred
