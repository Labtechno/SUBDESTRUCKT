# SUBDESTRUCKT — S0.1 — erratum : dispersion entre répétitions recalculée à partir de bench_reps.csv.
# Usage : python3 -I scripts/dispersion_from_reps.py <dossier_run>
#
# Définition corrigée, pour chaque configuration mesurée :
#   dispersion = (max(moyennes par répétition) - min(moyennes par répétition)) / médiane(moyennes par répétition)
# Le script historique bench_report.py divisait par p50_ns (médiane des temps de BLOC, toutes répétitions confondues),
# ce qui mélange deux statistiques différentes. Ce script ne modifie aucune donnée et ne recalcule aucun verdict.
import csv
import os
import statistics as st
import sys

sys.stdout.reconfigure(encoding="utf-8")
D = sys.argv[1]
reps = {}
for r in csv.DictReader(open(os.path.join(D, "bench_reps.csv"), encoding="utf-8")):
    key = (r["sr"], r["K"], r["ref"], r["frac"], r["mode"], r["topo"], r["block"])
    reps.setdefault(key, []).append(float(r["mean_ns"]))

disp_new, disp_old, n_reps = [], [], set()
p50 = {}
for r in csv.DictReader(open(os.path.join(D, "bench_matrix.csv"), encoding="utf-8")):
    if r["status"] == "ok":
        p50[(r["sr"], r["K"], r["ref"], r["frac"], r["mode"], r["topo"], r["block"])] = float(r["p50_ns"])
for key, v in reps.items():
    n_reps.add(len(v))
    med = st.median(v)
    disp_new.append((max(v) - min(v)) / med * 100)
    if key in p50 and p50[key] > 0:
        disp_old.append((max(v) - min(v)) / p50[key] * 100)


def q(vals, p):
    s = sorted(vals)
    return s[min(len(s) - 1, int(p * len(s)))]


print(f"configurations : {len(reps)} ; répétitions par configuration : {sorted(n_reps)}")
print(f"dispersion corrigée (÷ médiane des moyennes par répétition) : médiane {st.median(disp_new):.1f} %, "
      f"P95 {q(disp_new, 0.95):.1f} %, max {max(disp_new):.1f} %")
print(f"dispersion historique (÷ p50 des blocs)                      : médiane {st.median(disp_old):.1f} %, "
      f"P95 {q(disp_old, 0.95):.1f} %")
cv = []
for v in reps.values():
    if len(v) > 1:
        cv.append(st.stdev(v) / st.mean(v) * 100)
print(f"coefficient de variation des moyennes par répétition : médiane {st.median(cv):.1f} %, P95 {q(cv, 0.95):.1f} %")
