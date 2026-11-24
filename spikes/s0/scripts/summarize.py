#!/usr/bin/env python3
"""S0 — agrège les CSV bruts en tableaux comparatifs (Markdown). Usage : summarize.py data/ > summary.md"""
import sys
import numpy as np
import pandas as pd

d = sys.argv[1] if len(sys.argv) > 1 else "data"
m = pd.read_csv(f"{d}/matrix.csv")
dl = pd.read_csv(f"{d}/delay.csv")
tr = pd.read_csv(f"{d}/transition.csv")
pf = pd.read_csv(f"{d}/perf.csv")
bcols = [c for c in m.columns if c.startswith("b")][:23]  # 63 Hz .. 12.5 kHz
KEY = ["set", "cfg", "sr", "K", "depth", "f0", "rset"]


def md(df, floatfmt="{:.2f}"):
    df = df.copy()
    for c in df.columns:
        if df[c].dtype.kind == "f":
            df[c] = df[c].map(lambda v: "—" if pd.isna(v) else floatfmt.format(v))
    head = "| " + " | ".join(map(str, df.columns)) + " |\n|" + "|".join(["---"] * len(df.columns)) + "|\n"
    return head + "\n".join("| " + " | ".join(map(str, r)) + " |" for r in df.values) + "\n"


def lsd(a, b):
    return np.mean(np.abs(a[bcols].values - b[bcols].values), axis=1)


def pair(df, col, v1, v2, keys):
    a = df[df[col] == v1].set_index(keys)
    b = df[df[col] == v2].set_index(keys)
    idx = a.index.intersection(b.index)
    return a.loc[idx], b.loc[idx]


print("## 0. Intégrité numérique\n")
print(f"- Rendus analysés : {len(m)} ; NaN : {int(m.nan.sum())} ; Inf : {int(m.inf.sum())} ; pic max : {m.peak.max():.4f} ; |DC| max : {m.dc.abs().max():.4f}")
print(f"- T2 plafonné (D<1 éch. moteur) : {int(m.floored.sum())} rendus, tous à sr=44100 K=1 : {sorted(m[m.floored==1].sr.unique().tolist())} K={sorted(m[m.floored==1].K.unique().tolist())}\n")

print("## 1. Retard effectif mesuré (sonde petit signal, 997 Hz)\n")
dl["err_us"] = dl.measured_us - dl.expected_us
g = dl.groupby(["strat", "ref", "edge"]).agg(max_abs_err_us=("err_us", lambda x: x.abs().max()),
                                              measured_min_us=("measured_us", "min"), measured_max_us=("measured_us", "max")).reset_index()
print(md(g, "{:.4f}"))
print("Retard moyen par (strat, ref, sr, K) pour l'arête retardée, en µs :\n")
t = dl[dl.edge == "delayed_probe"].query("strat=='A'").pivot_table(index=["ref", "K"], columns="sr", values="measured_us").reset_index()
print(md(t, "{:.3f}"))

print("## 2. A vs B sur cascade acyclique (cfg 1) et autres topologies — distance spectrale (moy. |ΔdB| par bande 1/3 oct) et NMSE\n")
main = m[m.set == "main"]
rows = []
for cfg, grp in main.groupby("cfg"):
    for ref in ("T1", "T2"):
        for depth in (0.01, 1.0, 8.0):
            a, b = pair(grp[(grp.ref == ref) & (grp.depth == depth)], "strat", "A", "B", KEY)
            if len(a) == 0:
                continue
            rows.append(dict(cfg=cfg, name=a.cfgname.iloc[0], ref=ref, depth=depth,
                             LSD_A_vs_B=np.mean(np.abs(a[bcols].values - b[bcols].values)),
                             NMSE_B_vs_A_dB=b.nmse_vs_A_db.median(), ndel_A=a.ndelayed.iloc[0], ndel_B=b.ndelayed.iloc[0]))
r = pd.DataFrame(rows)
print(md(r[r.ref == "T1"], "{:.2f}"))

print("## 3. T1 vs T2 (même stratégie) — distance spectrale par sr (moyenne sur cfg, K, depth ≥ 1.0)\n")
rows = []
for st in ("A", "B"):
    for sr in (44100, 48000, 88200, 96000):
        sub = main[(main.strat == st) & (main.sr == sr) & (main.depth >= 1.0)]
        a, b = pair(sub[sub.ref == "T1"], "ref", "T1", "T1", KEY)
        t1 = sub[sub.ref == "T1"].set_index(KEY + ["K"]); t2 = sub[sub.ref == "T2"].set_index(KEY + ["K"])
        idx = t1.index.intersection(t2.index)
        rows.append(dict(strat=st, sr=sr, LSD_T1_vs_T2=np.mean(np.abs(t1.loc[idx][bcols].values - t2.loc[idx][bcols].values))))
print(md(pd.DataFrame(rows), "{:.2f}"))

print("## 4. Invariance au sample rate — LSD entre sr et la référence 48 kHz (K=4, depth ≥ 1.0, moyenne sur cfg)\n")
rows = []
for st in ("A", "B"):
    for ref in ("T1", "T2"):
        for K in (1, 2, 4):
            for sr in (44100, 88200, 96000):
                sub = main[(main.strat == st) & (main.ref == ref) & (main.K == K) & (main.depth >= 1.0)]
                x = sub[sub.sr == sr].set_index(["cfg", "depth"]); y = sub[sub.sr == 48000].set_index(["cfg", "depth"])
                idx = x.index.intersection(y.index)
                rows.append(dict(strat=st, ref=ref, K=K, sr=sr, LSD_vs_48k=np.mean(np.abs(x.loc[idx][bcols].values - y.loc[idx][bcols].values))))
r = pd.DataFrame(rows)
print(md(r.pivot_table(index=["strat", "ref", "K"], columns="sr", values="LSD_vs_48k").reset_index(), "{:.2f}"))

print("## 5. Invariance à l'oversampling — LSD (K vs K=4) et NMSE moyen, sr=48 kHz, depth ≥ 1.0\n")
rows = []
sub = main[(main.sr == 48000) & (main.depth >= 1.0)]
for st in ("A", "B"):
    for ref in ("T1", "T2"):
        for K in (1, 2):
            x = sub[(sub.strat == st) & (sub.ref == ref) & (sub.K == K)].set_index(["cfg", "depth"])
            y = sub[(sub.strat == st) & (sub.ref == ref) & (sub.K == 4)].set_index(["cfg", "depth"])
            idx = x.index.intersection(y.index)
            rows.append(dict(strat=st, ref=ref, K=K, LSD_vs_K4=np.mean(np.abs(x.loc[idx][bcols].values - y.loc[idx][bcols].values)),
                             median_NMSE_dB=x.nmse_vs_K4_db.median()))
print(md(pd.DataFrame(rows), "{:.2f}"))

print("## 6. Sensibilité (48 kHz, K=1) — LSD A vs B selon f0 et jeu de ratios (depth 1.0)\n")
sens = m[(m.set == "sens") & (m.depth == 1.0) & (m.ref == "T1")]
rows = []
for (f0, rs), g in sens.groupby(["f0", "rset"]):
    a, b = pair(g, "strat", "A", "B", ["cfg"])
    rows.append(dict(f0=f0, rset=rs, LSD_A_vs_B=np.mean(np.abs(a[bcols].values - b[bcols].values))))
print(md(pd.DataFrame(rows), "{:.2f}"))

print("## 7. Transitions topologiques\n")
print("click = max|Δy| autour de l'événement / RMS|Δy| précédent ; control = même mesure sans événement (max sur 6 instants). "
      "hard = route ajoutée/retirée à profondeur pleine ; ramped = profondeur en rampe 256 éch. (la transaction structurelle se fait à profondeur 0).\n")
tr["over_ctl"] = tr.click_ratio / tr.control_max
rows = []
for (sc, st), g in tr.groupby(["scenario", "strat"]):
    h = g[g["mode"] == "hard"]; r = g[g["mode"] == "ramped"]
    rows.append(dict(scenario=sc, strat=st, reclassified=int(g.reclassified.max()), causal=int(g.causal_after.min()),
                     hard_med=h.over_ctl.median(), hard_max=h.over_ctl.max(), ramped_med=r.over_ctl.median(), ramped_max=r.over_ctl.max(),
                     nan=int(g.nan.sum())))
print("Colonnes hard/ramped : click ÷ control (1.0 = indiscernable du signal normal).\n")
print(md(pd.DataFrame(rows), "{:.2f}"))

print("## 8. CPU (x86 Linux de développement — NON représentatif du M1), ns par échantillon moteur\n")
pv = pf.pivot_table(index=["cfgname", "strat", "ref"], columns="K", values="ns_per_engine_sample").reset_index()
print(md(pv, "{:.1f}"))
c = pf.groupby(["cfgname", "strat"]).compile_ns.mean().reset_index()
print("Temps de compilation du graphe (hors RT), ns :\n")
print(md(c.pivot_table(index="cfgname", columns="strat", values="compile_ns").reset_index(), "{:.0f}"))
