# SUBDESTRUCKT — S0.1 — analyse du benchmark matriciel et application des seuils fixés dans bench/THRESHOLDS.md.
# Usage : python3 -I scripts/bench_report.py <dossier_résultats> [titre_machine]
# Lit bench_matrix.csv, bench_reps.csv, bench_isolated.csv et, s'il existe, machine.txt ; écrit bench_report.md
# et bench_verdict.json dans le même dossier. Bibliothèque standard uniquement.
import csv
import json
import os
import statistics as st
import sys

sys.stdout.reconfigure(encoding="utf-8")
D = sys.argv[1]
TITLE = sys.argv[2] if len(sys.argv) > 2 else "machine de mesure"

# ---- seuils (identiques à bench/THRESHOLDS.md ; ne pas modifier après mesure)
TH = {
    "S1_peak": 2.0,
    "S2a_rep_load": 2.0, "S2b_stress_load": 5.0,
    "S3a_p99_budget": 20.0, "S3b_p99_budget_b32": 30.0,
    "S4_median": 1.10, "S4_p90": 1.25,
    "S5_ratio": 3.0, "S5_add_rep": 1.0, "S5_add_stress": 2.5,
    "S6_latency_ms": 1.0,
}


def q(vals, p):
    s = sorted(vals)
    return s[min(len(s) - 1, int(p * len(s)))]


rows = list(csv.DictReader(open(os.path.join(D, "bench_matrix.csv"), encoding="utf-8")))
for r in rows:
    for k in ("sr", "K", "block", "nroutes", "ndelayed", "latency_host"):
        r[k] = int(float(r[k]))
    for k in ("latency_ms", "D_eng", "delay_us"):
        r[k] = float(r[k])
    if r["status"] == "ok":
        for k in ("mean_ns", "p50_ns", "p95_ns", "p99_ns", "max_ns", "rep_mean_min_ns", "rep_mean_max_ns", "load_mean_pct",
                  "load_p99_pct", "load_max_pct", "ns_per_host_sample", "peak", "budget_ns"):
            r[k] = float(r[k])
        for k in ("rt_allocs", "nan", "inf"):
            r[k] = int(r[k])
ok = [r for r in rows if r["status"] == "ok"]
main = [r for r in ok if r["frac"] != "thiran1"]  # variante principale (Lagrange3 si fractionnaire)


def policy_k(ref, sr):
    return 1 if ref == "T1" else (2 if sr == 44100 else 1)


def is_policy(r):
    return r["K"] == policy_k(r["ref"], r["sr"])


def key(r, *f):
    return tuple(r[x] for x in f)


verdict = {}
lines = []
w = lines.append

# ---- S0 validité causale
rej = [r for r in rows if r["status"] == "rejected"]
exp_rej = {(m, t, b) for m in ("PM", "LinFM", "TZFM", "ExpFM") for t in ("self_feedback", "wt_op1", "dense4x4", "wtfm_dense_2cycles")
           for b in (32, 64, 128, 256, 512)}
got_rej = {(r["mode"], r["topo"], r["block"]) for r in rej if r["sr"] == 44100 and r["K"] == 1 and r["ref"] == "T2"}
s0 = got_rej == exp_rej and len(rej) == len(exp_rej)
verdict["S0"] = s0

# ---- S1 stabilité
nan = sum(r["nan"] + r["inf"] for r in ok)
allocs = sum(r["rt_allocs"] for r in ok)
peak = max(r["peak"] for r in ok)
s1 = nan == 0 and allocs == 0 and peak <= TH["S1_peak"]
verdict["S1"] = s1


# ---- S2 / S3 par référence (politique K)
def s23(ref):
    pol = [r for r in main if r["ref"] == ref and is_policy(r)]
    rep = [r["load_mean_pct"] for r in pol if r["topo_class"] == "representative" and r["block"] >= 64]
    strs = [r["load_mean_pct"] for r in pol if r["topo_class"] == "stress" and r["block"] >= 64]
    p99a = [r["load_p99_pct"] for r in pol if r["block"] >= 64]
    p99b = [r["load_p99_pct"] for r in pol if r["block"] == 32]
    return {
        "S2a": (max(rep), max(rep) <= TH["S2a_rep_load"]),
        "S2b": (max(strs), max(strs) <= TH["S2b_stress_load"]),
        "S3a": (max(p99a), max(p99a) <= TH["S3a_p99_budget"]),
        "S3b": (max(p99b), max(p99b) <= TH["S3b_p99_budget_b32"]),
    }


r23 = {ref: s23(ref) for ref in ("T1", "T2")}

# ---- S4 coût T2/T1 à K identique
idx = {key(r, "sr", "K", "ref", "mode", "topo", "block"): r for r in main}
ratios, ratio_rows = [], []
for r in main:
    if r["ref"] != "T2":
        continue
    t1 = idx.get((r["sr"], r["K"], "T1", r["mode"], r["topo"], r["block"]))
    if t1:
        ratios.append(r["mean_ns"] / t1["mean_ns"])
        ratio_rows.append((r, r["mean_ns"] / t1["mean_ns"]))
s4_med, s4_p90 = st.median(ratios), q(ratios, 0.9)
s4 = s4_med <= TH["S4_median"] and s4_p90 <= TH["S4_p90"]
verdict["S4"] = s4
thiran = []
for r in ok:
    if r["frac"] == "thiran1":
        t1 = idx.get((r["sr"], r["K"], "T1", r["mode"], r["topo"], r["block"]))
        if t1:
            thiran.append(r["mean_ns"] / t1["mean_ns"])

# ---- S5 surcoût politique 2x à 44.1 kHz
s5r = {"representative": [], "stress": []}
s5a = {"representative": [], "stress": []}
pol_rows = []
for r in main:
    if r["sr"] == 44100 and r["ref"] == "T2" and r["K"] == 2 and r["block"] >= 64:
        t1 = idx.get((44100, 1, "T1", r["mode"], r["topo"], r["block"]))
        if t1:
            s5r[r["topo_class"]].append(r["mean_ns"] / t1["mean_ns"])
            s5a[r["topo_class"]].append(r["load_mean_pct"] - t1["load_mean_pct"])
            pol_rows.append((r, t1))
s5_ratio = st.median(s5r["representative"] + s5r["stress"])
s5_add_rep, s5_add_str = st.median(s5a["representative"]), st.median(s5a["stress"])
s5 = s5_ratio <= TH["S5_ratio"] and s5_add_rep <= TH["S5_add_rep"] and s5_add_str <= TH["S5_add_stress"]
verdict["S5"] = s5

# ---- S6 latence
lat_t2 = {r["latency_ms"] for r in main if r["sr"] == 44100 and r["ref"] == "T2" and r["K"] == 2}
lat_t1 = {r["latency_ms"] for r in main if r["sr"] == 44100 and r["ref"] == "T1" and r["K"] == 1}
s6_add = max(lat_t2) - min(lat_t1)
s6 = s6_add <= TH["S6_latency_ms"]
verdict["S6"] = s6

t1_ok = all(v[1] for v in r23["T1"].values())
t2_ok = all(v[1] for v in r23["T2"].values())
if not (s0 and s1) or not t1_ok:
    decision = "NO-GO"
elif t2_ok and s4 and s5 and s6:
    decision = "GO-B-T2"
else:
    decision = "GO-B-T1"

# ---- dispersion entre répétitions
disp = [(r["rep_mean_max_ns"] - r["rep_mean_min_ns"]) / r["p50_ns"] * 100 for r in ok if r["p50_ns"] > 0]

# ================================================================ rapport
P = lambda b: "PASSE" if b else "ÉCHEC"
w(f"# S0.1 — benchmark T1/T2 — {TITLE}\n")
w("Généré par `scripts/bench_report.py` à partir des CSV bruts. Les seuils sont ceux de `bench/THRESHOLDS.md`, fixés avant la mesure.\n")
w("**Machine virtuelle : ces résultats ne certifient pas le temps réel sur un Mac M1 physique.**\n")
mt = os.path.join(D, "machine.txt")
if os.path.exists(mt):
    w("## Machine\n\n```\n" + open(mt, encoding="utf-8", errors="replace").read().strip() + "\n```\n")
w(f"Configurations : {len(rows)} ({len(ok)} mesurées, {len(rej)} rejetées). Blocs chronométrés : {sum(int(r['blocks']) for r in ok)}.\n")
w(f"Dispersion entre répétitions ((max − min) / médiane des moyennes par répétition) : médiane {st.median(disp):.1f} %, P95 {q(disp, 0.95):.1f} %.\n")

w("## Seuils\n")
w("| id | critère | mesuré | seuil | résultat |\n|---|---|---|---|---|")
w(f"| S0 | T2 44,1 kHz / 1× rejeté (80 configurations), aucun autre rejet | {len(got_rej)} rejets attendus sur {len(rej)} | 80 / 80 | {P(s0)} |")
w(f"| S1 | NaN/Inf, allocations RT, crête | {nan} NaN/Inf, {allocs} allocations, crête {peak:.3f} | 0, 0, ≤ {TH['S1_peak']} | {P(s1)} |")
for ref in ("T1", "T2"):
    v = r23[ref]
    w(f"| S2a ({ref}) | charge moyenne max, représentatives, politique K, blocs ≥ 64 | {v['S2a'][0]:.3f} % | ≤ {TH['S2a_rep_load']} % | {P(v['S2a'][1])} |")
    w(f"| S2b ({ref}) | charge moyenne max, stress, politique K, blocs ≥ 64 | {v['S2b'][0]:.3f} % | ≤ {TH['S2b_stress_load']} % | {P(v['S2b'][1])} |")
    w(f"| S3a ({ref}) | P99 max / budget, politique K, blocs ≥ 64 | {v['S3a'][0]:.2f} % | ≤ {TH['S3a_p99_budget']} % | {P(v['S3a'][1])} |")
    w(f"| S3b ({ref}) | P99 max / budget, politique K, bloc 32 | {v['S3b'][0]:.2f} % | ≤ {TH['S3b_p99_budget_b32']} % | {P(v['S3b'][1])} |")
w(f"| S4 | T2/T1 à K identique ({len(ratios)} paires) | médiane {s4_med:.3f}, P90 {s4_p90:.3f} | ≤ {TH['S4_median']} / ≤ {TH['S4_p90']} | {P(s4)} |")
w(f"| S5 | politique 2× à 44,1 kHz (T2 K2 / T1 K1, blocs ≥ 64) | rapport médian {s5_ratio:.2f} ; +{s5_add_rep:.3f} pt (repr.), +{s5_add_str:.3f} pt (stress) | ≤ {TH['S5_ratio']} ; ≤ {TH['S5_add_rep']} ; ≤ {TH['S5_add_stress']} | {P(s5)} |")
w(f"| S6 | latence ajoutée à 44,1 kHz | +{s6_add:.3f} ms | ≤ {TH['S6_latency_ms']} ms | {P(s6)} |")
w(f"\n**Règle de décision appliquée mécaniquement : {decision}.** Cette décision technique ne remplace pas l'arbitrage de la direction technique.\n")

w("## Charge moyenne (% d'un cœur), bloc 128 — max sur les 4 modes\n")
topos = ["self_feedback", "wt_op1", "dense4x4", "wtfm_dense_2cycles"]
w("| fs | K | réf. | " + " | ".join(topos) + " |\n|---|---|---|" + "---|" * len(topos))
for sr in (44100, 48000, 88200, 96000):
    for K in (1, 2, 4):
        for ref in ("T1", "T2"):
            cells = []
            for t in topos:
                v = [r["load_mean_pct"] for r in main if r["sr"] == sr and r["K"] == K and r["ref"] == ref and r["topo"] == t and r["block"] == 128]
                cells.append(f"{max(v):.3f}" if v else "rejeté")
            pol = " ★" if K == policy_k(ref, sr) else ""
            w(f"| {sr/1000:g} | {K}{pol} | {ref} | " + " | ".join(cells) + " |")
w("\n★ = politique K (K minimal causalement valide).\n")

w("## Taille de bloc — politique K, toutes topologies et modes\n")
w("| bloc | réf. | charge moy. max (%) | P99/budget max (%) | max/budget max (%) | ns/éch. hôte médian |\n|---|---|---|---|---|---|")
for b in (32, 64, 128, 256, 512):
    for ref in ("T1", "T2"):
        v = [r for r in main if r["block"] == b and r["ref"] == ref and is_policy(r)]
        w(f"| {b} | {ref} | {max(x['load_mean_pct'] for x in v):.3f} | {max(x['load_p99_pct'] for x in v):.2f} | "
          f"{max(x['load_max_pct'] for x in v):.1f} | {st.median(x['ns_per_host_sample'] for x in v):.1f} |")

w("\n## Coût relatif T2/T1 à oversampling identique (rapport des temps moyens, médiane [min ; max])\n")
w("| fs | K | médiane | min | max | retard T2 |\n|---|---|---|---|---|---|")
for sr in (44100, 48000, 88200, 96000):
    for K in (1, 2, 4):
        v = [x for (r, x) in ratio_rows if r["sr"] == sr and r["K"] == K]
        if v:
            rr = next(r for (r, x) in ratio_rows if r["sr"] == sr and r["K"] == K)
            w(f"| {sr/1000:g} | {K} | {st.median(v):.3f} | {min(v):.3f} | {max(v):.3f} | {rr['D_eng']:.4f} éch. ({rr['frac']}) |")
if thiran:
    w(f"\nVariante Thiran1 (bloc 128, retards fractionnaires) : T2/T1 médian {st.median(thiran):.3f} [{min(thiran):.3f} ; {max(thiran):.3f}].\n")

w("## Surcoût de la politique 2× permanente à 44,1 kHz (T2 K = 2 contre T1 K = 1)\n")
w("| topologie | bloc | T1 K1 charge (%) | T2 K2 charge (%) | rapport | points ajoutés |\n|---|---|---|---|---|---|")
for t in topos:
    for b in (64, 128, 512):
        v = [(r, t1) for (r, t1) in pol_rows if r["topo"] == t and r["block"] == b]
        if v:
            a = st.median(t1["load_mean_pct"] for (_, t1) in v)
            c = st.median(r["load_mean_pct"] for (r, _) in v)
            w(f"| {t} | {b} | {a:.3f} | {c:.3f} | {c / a:.2f} | +{c - a:.3f} |")
w("\n(médianes sur les 4 modes)\n")

w("## Latence algorithmique (décimateur ; le retard causal n'ajoute pas de latence)\n")
w("| fs | K | latence (éch. hôte) | latence (ms) |\n|---|---|---|---|")
seen = set()
for r in sorted(main, key=lambda r: (r["sr"], r["K"])):
    k = (r["sr"], r["K"])
    if k not in seen:
        seen.add(k)
        w(f"| {r['sr']/1000:g} | {r['K']} | {r['latency_host']} | {r['latency_ms']:.3f} |")

w("\n## Coûts isolés\n")
w("| mesure | nom | K | ns |\n|---|---|---|---|")
for r in csv.DictReader(open(os.path.join(D, "bench_isolated.csv"), encoding="utf-8")):
    w(f"| {r['kind']} | {r['name']} | {r['K']} | {float(r['ns_per_unit']):.3f} |")

open(os.path.join(D, "bench_report.md"), "w", encoding="utf-8").write("\n".join(lines) + "\n")
json.dump({"decision": decision, "criteria": {**verdict, **{f"{k}_{ref}": v[1] for ref in r23 for k, v in r23[ref].items()}},
           "values": {"S4_median": s4_med, "S4_p90": s4_p90, "S5_ratio": s5_ratio, "S5_add_rep": s5_add_rep,
                      "S5_add_stress": s5_add_str, "S6_add_ms": s6_add, "peak": peak}},
          open(os.path.join(D, "bench_verdict.json"), "w"), indent=2)
print("\n".join(lines))
