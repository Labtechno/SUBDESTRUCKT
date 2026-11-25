# SUBDESTRUCKT — figures « mesurées » du manuel : forme d'onde et spectre calculés à partir des WAV rendus par le moteur
# réel (render_examples). Usage : python -I make_figures.py <dossier_wav> <dossier_svg>
import json
import os
import struct
import sys

import numpy as np

SRC, OUT = sys.argv[1], sys.argv[2]
os.makedirs(OUT, exist_ok=True)
FS = 48000
INK, DIM, ACC, GRID = "#e8e6e1", "#9aa0a6", "#ff7a2f", "#3a3f45"


def read(path):
    d = open(path, "rb").read()
    pcm = d[44:]
    b = np.frombuffer(pcm, dtype=np.uint8).reshape(-1, 3).astype(np.int32)
    v = b[:, 0] | (b[:, 1] << 8) | (b[:, 2] << 16)
    v = np.where(v >= 1 << 23, v - (1 << 24), v)
    return v / 8388608.0


def svg(w, h, body, title, desc):
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" role="img" aria-labelledby="t d">'
            f'<title id="t">{title}</title><desc id="d">{desc}</desc>'
            f'<rect width="{w}" height="{h}" fill="#17191c"/>{body}</svg>')


def waveform(x, f0, label):
    W, H = 640, 200
    start = int(1.0 * FS)
    n = int(round(3 * FS / f0))  # trois périodes
    seg = x[start:start + n]
    m = max(1e-9, np.max(np.abs(seg)))
    pts = " ".join(f"{20 + i * (W - 40) / (n - 1):.1f},{H / 2 - seg[i] / m * (H / 2 - 24):.1f}" for i in range(n))
    body = (f'<line x1="20" y1="{H/2}" x2="{W-20}" y2="{H/2}" stroke="{GRID}"/>'
            f'<polyline fill="none" stroke="{ACC}" stroke-width="2" points="{pts}"/>'
            f'<text x="20" y="18" fill="{DIM}" font-family="monospace" font-size="12">Forme d\'onde mesurée — 3 périodes — {label}</text>'
            f'<text x="{W-20}" y="{H-8}" text-anchor="end" fill="{DIM}" font-family="monospace" font-size="11">moteur réel 0.1.0-b5, 48 kHz</text>')
    return svg(W, H, body, f"Forme d'onde : {label}", f"Trois périodes du signal rendu par le moteur SUBDESTRUCKT 0.1.0-b5 ({label}).")


def spectrum(x, label, fmax=8000):
    W, H = 640, 240
    seg = x[int(0.5 * FS): int(0.5 * FS) + 32768]
    win = np.blackman(len(seg))
    X = np.abs(np.fft.rfft(seg * win)) * 2 / win.sum()
    f = np.fft.rfftfreq(len(seg), 1 / FS)
    db = 20 * np.log10(X / X.max() + 1e-9)
    sel = f <= fmax
    f, db = f[sel], np.maximum(db[sel], -80)
    xs = 40 + f / fmax * (W - 60)
    ys = 20 + (-db) / 80 * (H - 50)
    pts = " ".join(f"{a:.1f},{b:.1f}" for a, b in zip(xs[::2], ys[::2]))
    grid = "".join(f'<line x1="40" y1="{20 + k/80*(H-50):.1f}" x2="{W-20}" y2="{20 + k/80*(H-50):.1f}" stroke="{GRID}"/>'
                   f'<text x="34" y="{24 + k/80*(H-50):.1f}" text-anchor="end" fill="{DIM}" font-family="monospace" font-size="10">-{k}</text>'
                   for k in (0, 20, 40, 60, 80))
    ticks = "".join(f'<text x="{40 + k/fmax*(W-60):.1f}" y="{H-14}" text-anchor="middle" fill="{DIM}" font-family="monospace" font-size="10">{k//1000} kHz</text>'
                    for k in range(0, fmax + 1, 2000))
    body = (grid + ticks + f'<polyline fill="none" stroke="{ACC}" stroke-width="1.2" points="{pts}"/>'
            f'<text x="40" y="14" fill="{DIM}" font-family="monospace" font-size="12">Spectre mesuré (dB, 0 = raie la plus forte) — {label}</text>')
    return svg(W, H, body, f"Spectre : {label}", f"Spectre de fréquences du rendu du moteur réel ({label}), de 0 à {fmax//1000} kHz.")


meta = json.load(open(os.path.join(SRC, "examples.json"), encoding="utf-8"))
f0 = {57: 220.0, 45: 110.0}
made = []
for e in meta["examples"]:
    x = read(os.path.join(SRC, e["file"]))
    note_f = 440.0 * 2 ** ((e["note"] - 69) / 12)
    label = e["id"].split("_", 1)[1].replace("_", " ")
    for kind, content in (("onde", waveform(x, note_f, label)), ("spectre", spectrum(x, label))):
        name = f"mesure_{kind}_{e['id']}.svg"
        open(os.path.join(OUT, name), "w", encoding="utf-8").write(content)
        made.append(name)
print(len(made), "figures")
