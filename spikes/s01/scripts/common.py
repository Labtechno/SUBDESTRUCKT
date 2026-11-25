"""S0.1 — utilitaires d'agrégation (stdlib + numpy uniquement)."""
import csv, math
import numpy as np

def load(path):
    with open(path, newline='', encoding='utf-8') as f:
        rows = list(csv.DictReader(f))
    for r in rows:
        for k, v in list(r.items()):
            try:
                r[k] = float(v) if v not in ('', None) and k not in ('hash',) else v
            except ValueError:
                pass
    return rows

def bands(r, n=23):
    keys = [k for k in r.keys() if k.startswith('b') and k[1:].isdigit()][:n]
    return np.array([r[k] for k in keys], dtype=float)

def lsd(a, b):
    return float(np.mean(np.abs(bands(a) - bands(b))))

def md(header, rows, fmt="{:.2f}"):
    out = "| " + " | ".join(header) + " |\n|" + "|".join(["---"] * len(header)) + "|\n"
    for r in rows:
        cells = []
        for v in r:
            if isinstance(v, float):
                cells.append("—" if math.isnan(v) else fmt.format(v))
            else:
                cells.append(str(v))
        out += "| " + " | ".join(cells) + " |\n"
    return out
