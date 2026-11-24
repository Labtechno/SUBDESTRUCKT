#!/usr/bin/env python3
"""S0 — réponse fréquentielle du délai fractionnaire causal T2 (Lagrange cubique, y[n-d0..n-d0-3])."""
import numpy as np, math
def weights(D):
    d0 = min(max(int(math.floor(D)) - 1, 1), 32)
    w = []
    for k in range(4):
        wk = 1.0
        for m in range(4):
            if m != k: wk *= (D - (d0 + m)) / ((d0 + k) - (d0 + m))
        w.append(wk)
    return d0, w
print("| sr | K | D (éch. moteur) | gain @1k | @5k | @10k | @20k (dB) | erreur de retard @10k (%) |")
print("|---|---|---|---|---|---|---|---|")
for sr in (44100, 48000, 88200, 96000):
    for K in (1, 2, 4):
        fsE = sr * K; D = max(fsE / 48000, 1.0)
        if abs(D - round(D)) < 1e-9: continue
        d0, w = weights(D)
        out = []
        for f in (1000, 5000, 10000, 20000):
            om = 2 * math.pi * f / fsE
            H = sum(w[k] * np.exp(-1j * om * (d0 + k)) for k in range(4))
            out.append((20 * math.log10(abs(H)), -np.angle(H) / om))
        derr = (out[2][1] - D) / D * 100
        print(f"| {sr} | {K} | {D:.4f} | " + " | ".join(f"{o[0]:+.3f}" for o in out) + f" | {derr:+.2f} |")
