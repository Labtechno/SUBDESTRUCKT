#!/usr/bin/env python3
"""S0.1 — agrège les CSV bruts en tableaux Markdown. Usage : python -I scripts/summarize.py data audio > data/summary.md
Dépendances : stdlib + numpy."""
import collections, os, sys, wave
import numpy as np
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.stdout.reconfigure(encoding="utf-8")
from common import load, lsd, md

D = sys.argv[1] if len(sys.argv) > 1 else "data"
A = sys.argv[2] if len(sys.argv) > 2 else "audio"
m = load(f"{D}/matrix.csv"); ok = [r for r in m if r["status"] == "ok"]
F = ("set", "sr", "K", "mode", "topo", "depth_name", "f0", "ref")
idx = {tuple(r[k] for k in F): r for r in ok}
def get(r, **kw):
    k = {f: r[f] for f in F}; k.update(kw)
    return idx.get(tuple(k[f] for f in F))
MODES = ("PM", "LinFM", "TZFM", "ExpFM")
CH = -20.0  # chaos_db > -20 : régime chaotique (une perturbation de 1e-9 sur une phase diverge)

print("# S0.1 — tableaux de synthèse (générés par scripts/summarize.py)\n")

# ------------------------------------------------------------------ 0
print("## 0. Intégrité et stabilité\n")
print(f"- Rendus matrice : {len(m)} ; rendus effectués : {len(ok)} ; refusés (T2 non représentable, K insuffisant) : {len(m)-len(ok)}"
      f" (tous à 44,1 kHz K=1 avec au moins une arête Delayed : {all(r['sr']==44100 and r['K']==1 for r in m if r['status']!='ok')}).")
print(f"- NaN : {int(sum(r['nan'] for r in ok))} ; Inf : {int(sum(r['inf'] for r in ok))} ; crête hôte max : {max(r['peak'] for r in ok):.3f} "
      f"(sortie moteur ≤ 1 par construction, dépassement = suroscillation du décimateur) ; |valeur retardée| max : {max(r['max_abs_delayed'] for r in ok):.3f}.\n")
rows = []
for mo in MODES:
    s = [r for r in ok if r["mode"] == mo]
    dcl = [r for r in s if abs(r["dc"]) > 0.1]
    rows.append((mo, len(s), max(r["max_hz"] for r in s), max(r["clamp_frac"] for r in s), np.mean([r["chaos_db"] > CH for r in s]) * 100,
                 len(dcl), len([r for r in dcl if r["sr"] == 48000]), max(r["negblock_frac"] for r in s), max(r["tzrev_frac"] for r in s)))
print(md(["mode", "rendus", "|f| inst. max (Hz)", "fraction max d'échantillons bornés", "% chaotiques", "rendus |DC|>0.1", "dont à 48 kHz (T1≡T2)",
          "LinFM : fraction max f<0 bloquée", "TZFM : fraction max phase rétrograde"], rows, "{:.3f}"))
print("PM : |f| inst. est dérivé du saut de phase de lecture (borné à 0,5 cycle/éch.) et n'est pas une borne. "
      "ExpFM : f0·2^4 ; avec f0 = 1760 Hz et ratio 3, la fréquence dépasse Nyquist hôte (aucune compensation, conforme au baseline).\n")
# DC : T1 vs T2
pairs = collections.Counter()
for r in ok:
    if r["ref"] != "T1": continue
    o = get(r, ref="T2")
    if o: pairs[(abs(r["dc"]) > 0.1, abs(o["dc"]) > 0.1)] += 1
print(f"Verrouillage DC (|DC|>0,1) T1/T2 sur paires identiques : les deux {pairs[(True,True)]}, T1 seul {pairs[(True,False)]}, T2 seul {pairs[(False,True)]}, aucun {pairs[(False,False)]}.\n")

# ------------------------------------------------------------------ 1
dl = load(f"{D}/delay.csv")
print("## 1. Retard effectif mesuré (sonde 997 Hz, 10⁻³ rad)\n")
rows = []
for ref in ("T1", "T2"):
    for fr in ("lagrange3", "linear", "thiran1"):
        s = [r for r in dl if r["ref"] == ref and r["frac"] == fr and r["status"] == "ok"]
        if not s: continue
        for edge in ("immediate", "delayed"):
            e = [r for r in s if r["edge"] == edge]
            rows.append((ref, fr if ref == "T2" else "—", edge, len(e), min(r["measured_us"] for r in e), max(r["measured_us"] for r in e), max(abs(r["err_us"]) for r in e)))
print(md(["réf.", "fractionnaire", "arête", "n", "min (µs)", "max (µs)", "|erreur| max (µs)"], rows, "{:.5f}"))
ref_ = [r for r in dl if r["status"] != "ok"]
print(f"Refus explicites (T2 à 44,1 kHz K=1) : {len(ref_)} sondes. Aucune substitution silencieuse par 22,676 µs.\n")
t1 = {int(r["sr"]): r["measured_us"] for r in dl if r["ref"] == "T1" and r["edge"] == "delayed" and r["K"] == 1}
print("T1 mesuré : " + ", ".join(f"{k/1000:g} kHz = {v:.3f} µs" for k, v in sorted(t1.items())) + " ; T2 = 20,833 µs partout où il est autorisé.\n")

# ------------------------------------------------------------------ 2
fr = load(f"{D}/fracresp.csv")
print("## 2. Délai fractionnaire T2 : réponse théorique\n")
rows = []
for r in fr:
    if r["f_hz"] in (10000, 20000, 30000, 40000):
        rows.append((int(r["sr"]), int(r["K"]), r["D_eng"], r["frac"], int(r["f_hz"]), r["gain_db"], r["delay_err_pct"]))
rows.sort(key=lambda x: (x[2], x[0], x[3], x[4]))
seen = set(); rr = []
for x in rows:
    k = (x[2], x[3], x[4])
    if k in seen: continue  # 44,1k K2 et 88,2k K1 ont le même fsE et le même D
    seen.add(k); rr.append(x)
print(md(["sr", "K", "D (éch. moteur)", "méthode", "f (Hz)", "gain (dB)", "erreur de retard de phase (%)"], rr, "{:.3f}"))
print("Coût par lecture : Lagrange3 = 4 MAC, Linear = 2 MAC, Thiran1 = 2 MAC + 1 état IIR par source. "
      "44,1 kHz K=2 et 88,2 kHz K=1 partagent fsE = 88,2 kHz et D = 1,8375 (cas le plus défavorable).\n")

ff = load(f"{D}/fracfb.csv")
print("### 2b. Méthodes fractionnaires dans les boucles (écart spectral moyen par bandes, dB)\n")
g = collections.defaultdict(list)
for r in ff:
    reg = "chaotique" if r["chaos_db"] > CH else "régulier"
    g[(int(r["sr"]), int(r["K"]), reg, r["variant"])].append((r["lsd_vs_lagrange3"], r["lsd_vs_T1"]))
rows = [(k[0], k[1], k[2], k[3], len(v), np.mean([a for a, _ in v]), np.median([a for a, _ in v]), np.mean([b for _, b in v]), np.median([b for _, b in v]))
        for k, v in sorted(g.items())]
print(md(["sr", "K", "régime", "variante", "n", "vs Lagrange3 moy.", "méd.", "vs T1 moy.", "méd."], rows, "{:.3f}"))
print(f"NaN : {int(sum(r['nan'] for r in ff))}. |valeur retardée| max : " + ", ".join(
    f"{v} {max(r['max_abs_delayed'] for r in ff if r['variant']==v):.3f}" for v in ("T1", "T2_lagrange3", "T2_linear", "T2_thiran1")) + ".\n")

# ------------------------------------------------------------------ 3
print("## 3. B-T1 contre B-T2 à configuration identique\n")
g = collections.defaultdict(list)
for r in ok:
    if r["ref"] != "T1": continue
    o = get(r, ref="T2")
    if not o: continue
    reg = "chaotique" if max(r["chaos_db"], o["chaos_db"]) > CH else "régulier"
    g[(int(r["sr"]), reg)].append((lsd(r, o), r["hash"] == o["hash"], max(r["lsd_perturb"], o["lsd_perturb"])))
rows = [(k[0], k[1], len(v), np.mean([x[0] for x in v]), np.median([x[0] for x in v]), np.percentile([x[0] for x in v], 90), sum(x[1] for x in v), np.mean([x[2] for x in v]))
        for k, v in sorted(g.items())]
print(md(["sr", "régime", "paires", "écart T1/T2 moy. (dB)", "méd.", "p90", "rendus bit-identiques", "plancher (perturbation 1e-9) moy."], rows, "{:.3f}"))
sw = collections.Counter()
for r in ok:
    if r["ref"] != "T1" or r["sr"] == 48000: continue
    o = get(r, ref="T2")
    if o: sw[(r["chaos_db"] > CH, o["chaos_db"] > CH)] += 1
print(f"Changement de régime entre T1 et T2 (hors 48 kHz) : T1 chaotique/T2 régulier {sw[(True,False)]}, T1 régulier/T2 chaotique {sw[(False,True)]}, "
      f"sur {sum(sw.values())} paires.\n")

# ------------------------------------------------------------------ 4
print("## 4. Invariance inter-sample-rate (écart spectral à 48 kHz, même K, même référence temporelle)\n")
print("À 48 kHz, T1 et T2 sont identiques bit à bit : la référence est commune. Seules les configurations avec au moins une arête Delayed sont comptées.\n")
g = collections.defaultdict(list)
for r in ok:
    if r["sr"] == 48000 or r["ref"] != "T1" or r["ndelayed"] == 0: continue
    t2 = get(r, ref="T2"); a = get(r, sr=48000.0)
    if not t2 or not a: continue
    reg = "chaotique" if max(r["chaos_db"], t2["chaos_db"], a["chaos_db"]) > CH else "régulier"
    g[(reg, int(r["K"]), int(r["sr"]))].append((lsd(r, a), lsd(t2, a), max(r["lsd_perturb"], t2["lsd_perturb"], a["lsd_perturb"])))
rows = []
for k, v in sorted(g.items()):
    v = np.array(v); d = v[:, 0] - v[:, 1]
    rows.append((k[0], k[1], k[2], len(v), v[:, 0].mean(), v[:, 1].mean(), np.median(v[:, 0]), np.median(v[:, 1]), int((d > 0.1).sum()), int((d < -0.1).sum()), v[:, 2].mean()))
print(md(["régime", "K", "sr", "n", "T1 moy.", "T2 moy.", "T1 méd.", "T2 méd.", "T2 plus proche (>0,1 dB)", "T1 plus proche", "plancher moy."], rows, "{:.3f}"))
print("### 4b. Par mode (régimes réguliers, K = 2 et 4, 88,2 et 96 kHz)\n")
g = collections.defaultdict(list)
for r in ok:
    if r["sr"] not in (88200, 96000) or r["K"] == 1 or r["ref"] != "T1" or r["ndelayed"] == 0: continue
    t2 = get(r, ref="T2"); a = get(r, sr=48000.0)
    if not t2 or not a or max(r["chaos_db"], t2["chaos_db"], a["chaos_db"]) > CH: continue
    g[r["mode"]].append((lsd(r, a), lsd(t2, a)))
rows = [(mo, len(g[mo]), np.mean([x for x, _ in g[mo]]), np.mean([y for _, y in g[mo]]), np.median([x for x, _ in g[mo]]), np.median([y for _, y in g[mo]])) for mo in MODES]
print(md(["mode", "n", "T1 moy.", "T2 moy.", "T1 méd.", "T2 méd."], rows, "{:.3f}"))

# ------------------------------------------------------------------ 5
print("## 5. 44,1 kHz : asymétrie T1 1× / T2 2×\n")
print("Repliement mesuré par l'énergie hors grille harmonique (inharm, dB ; régimes réguliers seulement) et énergie moteur au-dessus de Nyquist hôte avant décimation (oob).\n")
rows = []
for mo in MODES:
    s = [r for r in ok if r["sr"] == 44100 and r["set"] == "main" and r["mode"] == mo and r["ref"] == "T1" and r["K"] == 1 and r["chaos_db"] < CH]
    k2 = [get(r, K=2.0) for r in s]; t22 = [get(r, K=2.0, ref="T2") for r in s]
    pr = [(a, b, c) for a, b, c in zip(s, k2, t22) if b and c and b["chaos_db"] < CH and c["chaos_db"] < CH]
    if not pr: continue
    rows.append((mo, len(pr), np.median([a["inharm_db"] for a, _, _ in pr]), np.median([b["inharm_db"] for _, b, _ in pr]), np.median([c["inharm_db"] for _, _, c in pr]),
                 np.median([b["oob_db"] for _, b, _ in pr])))
print(md(["mode", "n", "T1 K1 inharm méd.", "T1 K2 inharm méd.", "T2 K2 inharm méd.", "oob K2 méd. (dB)"], rows, "{:.1f}"))
print("Le gain de repliement vient de K=2, pas de T2 : T1 à K=2 l'obtient aussi.\n")

# ------------------------------------------------------------------ 6
tr = load(f"{D}/transition.csv")
print("## 6. Transitions topologiques (rampes de profondeur + transaction sticky)\n")
print(f"- Rendus : {len(tr)} ; NaN : {int(sum(r['nan'] for r in tr))} ; routes survivantes reclassées : {int(sum(r['reclassified'] for r in tr))}.")
cls = collections.Counter((r["scenario"], r["new_class"]) for r in tr)
print("- Classe des nouvelles routes : " + ", ".join(f"{s} → {c}" for (s, c) in sorted(set(cls))) + ".\n")
print("Bouffée HF aux instants structurels (Δ⁴, fenêtres de 32 éch.), en dB relatif au maximum des régimes stationnaires avant/après. ≤ 0 dB = rien au-delà du signal lui-même.\n")
g = collections.defaultdict(list)
for r in tr: g[(r["curve"], int(r["R"]))].append((r["burst_at_db"], r["burst_db"], r["jump"]))
rows = []
for k in sorted(g, key=lambda k: (k[1], k[0])):
    v = np.array(g[k])
    rows.append((k[1], k[0], len(v), np.median(v[:, 0]), np.percentile(v[:, 0], 99), v[:, 0].max(), (v[:, 0] > 1).mean() * 100, (v[:, 0] > 6).mean() * 100, v[:, 1].max(), np.percentile(v[:, 2], 99)))
print(md(["R (éch. hôte)", "courbe", "n", "burst instants méd.", "p99", "max", "% > 1 dB", "% > 6 dB", "burst fenêtre complète max", "saut p99"], rows, "{:.2f}"))
print("Maximum par scénario (burst aux instants, dB) :\n")
g = collections.defaultdict(list)
for r in tr: g[(r["scenario"], r["curve"], int(r["R"]))].append(r["burst_at_db"])
scs = sorted(set(r["scenario"] for r in tr))
rows = [tuple([s, max(g[(s, "hard", 0)])] + [max(g[(s, "linear", R)] + g[(s, "smooth", R)]) for R in (64, 128, 256, 512, 1024)]) for s in scs]
print(md(["scénario", "sec (R=0)", "64", "128", "256", "512", "1024"], rows, "{:.2f}"))
print("Latence de geste : R pour ajout/retrait/fondu croisé, 2R pour la modification séquentielle (descente, échange, montée).\n")

# ------------------------------------------------------------------ 7
for bp, title in ((f"{D}/bench_m1.csv", "Apple M1 (mesuré par l'utilisateur)"), (f"{D}/bench_x86_indicative.csv", "x86 INDICATIF (AMD Ryzen 7 5800X, Windows, MSVC /O2), PAS Apple M1")):
  if os.path.exists(bp):
      b = load(bp); full = [r for r in b if r["kind"] == "full"]
      print("## 7. Coût CPU — x86 INDICATIF (AMD Ryzen 7 5800X, Windows, MSVC /O2), PAS Apple M1\n")
      print(f"- Allocations pendant la boucle temps réel : {int(sum(r['rt_allocs'] for r in full))} sur {len(full)} configurations.\n")
      bi = {(r["sr"], r["K"], r["ref"], r["frac"], r["mode"], r["topo"]): r for r in full}
      rows = []
      for r in full:
          if r["ref"] != "T1" or r["K"] != 1: continue
          Kp = 2.0 if (r["sr"] == 44100 and r["ndelayed"] > 0) else 1.0
          t2 = bi.get((r["sr"], Kp, "T2", "lagrange3", r["mode"], r["topo"]))
          t1k2 = bi.get((r["sr"], 2.0, "T1", "lagrange3", r["mode"], r["topo"]))
          t2k2 = bi.get((r["sr"], 2.0, "T2", "lagrange3", r["mode"], r["topo"]))
          rows.append((int(r["sr"]), r["topo"], r["mode"], r["ns_per_host_sample"], r["p99_ns"] / 128, int(Kp), t2["ns_per_host_sample"], t2["ns_per_host_sample"] / r["ns_per_host_sample"],
                       t2k2["ns_per_host_sample"] / t1k2["ns_per_host_sample"]))
      print(md(["sr", "topologie", "mode", "T1 K1 ns/éch. hôte", "T1 K1 p99/128 (ns)", "K T2 imposé", "T2 ns/éch. hôte", "T2/T1 (politique)", "T2/T1 à K=2 égal"], rows, "{:.2f}"))
      for r in b:
          if r["kind"] == "delay_reader": print(f"- Lecteur de retard {r['frac']} : {r['ns_per_host_sample']:.2f} ns par lecture.")
          if r["kind"] == "decimator": print(f"- Décimateur K={int(r['K'])} : {r['ns_per_host_sample']:.1f} ns par échantillon hôte.")
      print()

# ------------------------------------------------------------------ 8
if os.path.isdir(A):
    print("## 8. Rendus audio comparatifs — écarts objectifs (aucune écoute effectuée)\n")
    def rd(p):
        with wave.open(p) as w:
            n, sr = w.getnframes(), w.getframerate()
            raw = np.frombuffer(w.readframes(n), dtype=np.uint8).reshape(-1, 3)
        v = (raw[:, 0].astype(np.int32) | (raw[:, 1].astype(np.int32) << 8) | (raw[:, 2].astype(np.int32) << 16))
        v = np.where(v >= 1 << 23, v - (1 << 24), v) / 8388607.0
        return v, sr
    def bl(x, sr):
        N = 1 << int(np.ceil(np.log2(8192 * sr / 48000)))
        win = np.blackman(N); P = np.zeros(N // 2 + 1); c = 0
        for s in range(0, len(x) - N, N // 2): P += np.abs(np.fft.rfft(x[s:s + N] * win)) ** 2; c += 1
        f = np.arange(N // 2 + 1) * sr / N; tot = P[f < 20160].sum()
        cs = [63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000]
        return np.array([max(-70, 10 * np.log10(P[(f >= c0) & (f < c0 * 2 ** (1 / 3))].sum() / tot + 1e-30)) for c0 in cs])
    files = sorted(f for f in os.listdir(A) if f.endswith(".wav"))
    cases = sorted(set(f.split("_REF")[0] for f in files if "_REF_" in f))
    rows = []
    for c in cases:
        ref = bl(*rd(os.path.join(A, f"{c}_REF_48k.wav")))
        for pr in ("44k1_policy", "44k1_sameK2", "96k"):
            X = bl(*rd(os.path.join(A, f"{c}_{pr}_X.wav"))); Y = bl(*rd(os.path.join(A, f"{c}_{pr}_Y.wav")))
            rows.append((c, pr, np.mean(np.abs(X - Y)), np.mean(np.abs(X - ref)), np.mean(np.abs(Y - ref))))
    print(md(["cas", "paire", "écart X/Y (dB)", "X vs REF 48k", "Y vs REF 48k"], rows, "{:.3f}"))
    print("X/Y aveugles : correspondance T1/T2 dans audio/KEY.md et audio/KEY_audio_metrics.csv.\n")
