# SUBDESTRUCKT — règle de décision v2 pour les FUTURS benchmarks (erratum S0.1, prospectif).
# Usage : python3 -I scripts/bench_decision_v2.py <dossier_run> [--candidates T1,T2]
#
# Défaut de la règle historique (bench/THRESHOLDS.md, v1) : « NO-GO si T1 ne tient pas S2/S3 » empêchait de retenir T2
# quand T2 passe et T1 échoue. La règle v2 évalue chaque candidat indépendamment :
#   - NO-GO global si S0 (validité causale) ou S1 (stabilité) échoue ;
#   - un candidat est ADMISSIBLE s'il tient tous les critères qui le concernent :
#       T1 : S2a, S2b, S3a, S3b (politique K de T1) ;
#       T2 : S2a, S2b, S3a, S3b (politique K de T2) + S4 (coût relatif à K égal) + S5 (surcoût 2x à 44,1 kHz) + S6 (latence) ;
#   - verdict : aucun admissible -> NO-GO ; un seul -> GO pour celui-ci ; les deux -> arbitrage (pas de choix automatique).
# Seuils : identiques aux seuils historiques (bench/THRESHOLDS.md), non modifiés.
# Appliquée aux données historiques, cette règle est une ILLUSTRATION : elle ne remplace pas le verdict historique (NO-GO).
import csv
import os
import statistics as st
import sys

sys.stdout.reconfigure(encoding="utf-8")
D = sys.argv[1]
cands = ["T1", "T2"]
if "--candidates" in sys.argv:
    cands = sys.argv[sys.argv.index("--candidates") + 1].split(",")
TH = {"S1_peak": 2.0, "S2a": 2.0, "S2b": 5.0, "S3a": 20.0, "S3b": 30.0, "S4_med": 1.10, "S4_p90": 1.25,
      "S5_ratio": 3.0, "S5_rep": 1.0, "S5_str": 2.5, "S6": 1.0}


def q(v, p):
    s = sorted(v)
    return s[min(len(s) - 1, int(p * len(s)))]


rows = list(csv.DictReader(open(os.path.join(D, "bench_matrix.csv"), encoding="utf-8")))
for r in rows:
    r["sr"], r["K"], r["block"] = int(float(r["sr"])), int(r["K"]), int(r["block"])
ok = [r for r in rows if r["status"] == "ok"]
main = [r for r in ok if r["frac"] != "thiran1"]
for r in ok:
    for k in ("mean_ns", "load_mean_pct", "load_p99_pct", "peak", "latency_ms"):
        r[k] = float(r[k])
pk = lambda ref, sr: 1 if ref == "T1" else (2 if sr == 44100 else 1)
pol = lambda r: r["K"] == pk(r["ref"], r["sr"])

rej = [r for r in rows if r["status"] == "rejected"]
S0 = len(rej) == 80 and all(r["sr"] == 44100 and r["K"] == 1 and r["ref"] == "T2" for r in rej)
S1 = all(int(r["nan"]) + int(r["inf"]) == 0 and int(r["rt_allocs"]) == 0 and r["peak"] <= TH["S1_peak"] for r in ok)


def own(ref):
    p = [r for r in main if r["ref"] == ref and pol(r)]
    return {
        "S2a": max(r["load_mean_pct"] for r in p if r["topo_class"] == "representative" and r["block"] >= 64) <= TH["S2a"],
        "S2b": max(r["load_mean_pct"] for r in p if r["topo_class"] == "stress" and r["block"] >= 64) <= TH["S2b"],
        "S3a": max(r["load_p99_pct"] for r in p if r["block"] >= 64) <= TH["S3a"],
        "S3b": max(r["load_p99_pct"] for r in p if r["block"] == 32) <= TH["S3b"],
    }


idx = {(r["sr"], r["K"], r["ref"], r["mode"], r["topo"], r["block"]): r for r in main}
ratios = [r["mean_ns"] / idx[(r["sr"], r["K"], "T1", r["mode"], r["topo"], r["block"])]["mean_ns"]
          for r in main if r["ref"] == "T2" and (r["sr"], r["K"], "T1", r["mode"], r["topo"], r["block"]) in idx]
S4 = st.median(ratios) <= TH["S4_med"] and q(ratios, 0.9) <= TH["S4_p90"]
s5r, s5a = {"representative": [], "stress": []}, {"representative": [], "stress": []}
for r in main:
    if r["sr"] == 44100 and r["ref"] == "T2" and r["K"] == 2 and r["block"] >= 64:
        t1 = idx.get((44100, 1, "T1", r["mode"], r["topo"], r["block"]))
        if t1:
            s5r[r["topo_class"]].append(r["mean_ns"] / t1["mean_ns"])
            s5a[r["topo_class"]].append(r["load_mean_pct"] - t1["load_mean_pct"])
S5 = (st.median(s5r["representative"] + s5r["stress"]) <= TH["S5_ratio"] and st.median(s5a["representative"]) <= TH["S5_rep"]
      and st.median(s5a["stress"]) <= TH["S5_str"])
lat2 = max(r["latency_ms"] for r in main if r["sr"] == 44100 and r["ref"] == "T2" and r["K"] == 2)
lat1 = min(r["latency_ms"] for r in main if r["sr"] == 44100 and r["ref"] == "T1" and r["K"] == 1)
S6 = lat2 - lat1 <= TH["S6"]

adm = {}
for c in cands:
    crit = own(c)
    if c == "T2":
        crit.update({"S4": S4, "S5": S5, "S6": S6})
    adm[c] = (all(crit.values()), crit)
    print(f"{c} : {'ADMISSIBLE' if adm[c][0] else 'non admissible'} — " + ", ".join(f"{k} {'ok' if v else 'ÉCHEC'}" for k, v in crit.items()))
print(f"S0 {'ok' if S0 else 'ÉCHEC'}, S1 {'ok' if S1 else 'ÉCHEC'}")
ok_c = [c for c in cands if adm[c][0]]
if not (S0 and S1) or not ok_c:
    v = "NO-GO"
elif len(ok_c) == 1:
    v = f"GO-B-{ok_c[0]}"
else:
    v = "ARBITRAGE (plusieurs candidats admissibles, pas de choix automatique)"
print(f"verdict règle v2 : {v}")
