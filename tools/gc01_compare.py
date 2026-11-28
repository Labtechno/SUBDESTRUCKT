"""ARCH-UI-04-R2-B2 — GC-01 : comparaison référence / courant d'un run alterné (r2b2-evidence.yml), seuils GS définitifs
(B2-00-ADDENDUM.md §4). Lit gc01-summary-ref-*.csv et gc01-summary-cur-*.csv d'un dossier de run, apparie les séries,
calcule les rapports courant / référence (médiane des deux exécutions de chaque côté) et les confronte aux seuils
COMPARATIFS. Les seuils absolus (GS-02 11,1 ms, GS-06 16,7 ms) exigent la machine physique M3 : jamais jugés ici.
Un résultat de VM est comparatif et NON certifiant.

Plancher de bruit (CE-B2-04) : --aa=<dossier d'un run A/A> (référence et courant au rendu identique). Pour chaque seuil, si
l'écart relatif observé en A/A sur la même série atteint la marge du seuil (ex. 5 % pour GS-04), le verdict est
NON CONCLUANT (bruit VM) : ni PASS ni ÉCHEC, mesure physique requise.

Usage : py -3.11 tools/gc01_compare.py <dossier du run> [<csv de sortie>] [--aa=<dossier A/A>]
"""
import csv, glob, pathlib, statistics, sys

aa_dir = next((pathlib.Path(a.split('=', 1)[1]) for a in sys.argv[1:] if a.startswith('--aa=')), None)
sys.argv = [a for a in sys.argv if not a.startswith('--aa=')]
d = pathlib.Path(sys.argv[1])


def load(prefix, base=None):
    runs = []
    for f in sorted(glob.glob(str((base or d) / f'gc01-summary-{prefix}-*.csv'))):
        runs.append({(r['metric'], r['scenario']): r for r in csv.DictReader(open(f, encoding='utf-8'), delimiter=';')})
    return runs


ref, cur = load('ref'), load('cur')
assert ref and cur, 'run non alterne (ref-* et cur-* attendus)'
aa_ref, aa_cur = (load('ref', aa_dir), load('cur', aa_dir)) if aa_dir else ([], [])


def med(runs, key, field):
    v = [float(R[key][field]) for R in runs if key in R]
    return statistics.median(v) if v else None


rows = []
keys = [k for k in ref[0] if all(k in R for R in ref + cur)]
for k in keys:
    m, s = k
    for field in ('median', 'p95', 'p99'):
        a, b = med(ref, k, field), med(cur, k, field)
        ratio = (b / a) if a not in (None, 0.0) else None
        rows.append([m, s, field, a, b, ratio])


def worst(metric, field, scen_filter=lambda s: True):
    r = [x for x in rows if x[0] == metric and x[2] == field and scen_filter(x[1]) and x[5] is not None]
    return max(r, key=lambda x: x[5]) if r else None


verdicts = []
w = worst('GP-01', 'median')
verdicts.append(('GS-01', 'GP-01 mediane rendu complet <= 1,5 x ref (pire vue x zoom)', w, w and w[5] <= 1.5))
w = worst('GP-02', 'p95', lambda s: s.endswith('total'))
verdicts.append(('GS-02 (comparatif)', 'GP-02 p95 rafraichissement <= 1,5 x ref ; partie absolue 11,1 ms : M3 physique', w, w and w[5] <= 1.5))
rest = [x for x in rows if x[0] == 'GP-03' and x[1] == 'repos-zones-par-rafraichissement' and x[2] == 'median']
verdicts.append(('GS-03', 'zones invalidees au repos (ref 0.4.1 : 1 = ScopeView) ; ne doit pas augmenter', rest[0] if rest else None,
                 bool(rest) and rest[0][4] <= rest[0][3]))
w = worst('GP-04', 'p99', lambda s: s.endswith('-bloc'))
dep = [x for x in rows if x[0] == 'GP-04' and x[1].endswith('depassements-traitement') and x[2] == 'median']
dep_ok = all(x[4] <= max(x[3], 2.0) for x in dep)  # plancher de bruit de la VM : 0 a 2 blocs observes sur la reference
verdicts.append(('GS-04', 'GP-04 p99 bloc <= 1,05 x ref ; depassements de traitement non superieurs au plancher de la reference', w,
                 w and w[5] <= 1.05 and dep_ok))
sur = [x for x in rows if x[0] == 'GP-05' and x[1] == 'surcout-editeur' and x[2] == 'median']
half = [x for x in rows if x[0] == 'GP-05' and x[1] == 'croissance-seconde-moitie' and x[2] == 'median']
verdicts.append(('GS-05', 'surcout memoire editeur <= 1,25 x ref ; croissance seconde moitie non superieure a la ref + 1 MiB', sur[0] if sur else None,
                 bool(sur) and sur[0][5] is not None and sur[0][5] <= 1.25 and bool(half) and half[0][4] <= half[0][3] + 1.0))
w = worst('GP-07', 'p95')
verdicts.append(('GS-06 (comparatif)', 'GP-07 p95 interaction <= 1,5 x ref ; objectif 16,7 ms : M3 physique', w, w and w[5] <= 1.5))
w2 = worst('GP-04', 'p99', lambda s: s.startswith('editeur-ferme'))
verdicts.append(('GS-07', 'editeur ferme : p99 bloc <= 1,05 x ref', w2, w2 and w2[5] <= 1.05))

LIMITS = {'GS-01': 1.5, 'GS-02 (comparatif)': 1.5, 'GS-04': 1.05, 'GS-05': 1.25, 'GS-06 (comparatif)': 1.5, 'GS-07': 1.05}


def side_dispersion(x):  # dispersion interne : écart relatif max entre les exécutions d'un même binaire (référence, courant)
    k = (x[0], x[1])
    devs = []
    for runs in (ref, cur):
        v = [float(R[k][x[2]]) for R in runs if k in R]
        if len(v) >= 2 and min(v) > 0:
            devs.append(max(v) / min(v) - 1.0)
    return max(devs) if devs else 0.0


def aa_noise(x):  # écart relatif A/A : même série si elle existe, sinon famille (même mesure, même champ), hors séries d'information
    if not aa_ref:
        return side_dispersion(x)
    devs = [side_dispersion(x)]
    same = (x[0], x[1]) in aa_ref[0]
    for k in aa_ref[0]:
        if k[0] != x[0] or not all(k in R for R in aa_ref + aa_cur) or k[1].endswith('-info') or 'depassements' in k[1]:
            continue
        if same and k[1] != x[1]:
            continue
        a, b = med(aa_ref, k, x[2]), med(aa_cur, k, x[2])
        if a and b is not None and a > 0:
            devs.append(abs(b / a - 1.0))
    return max(devs) if devs else None


print("Plancher de bruit = max(ecart A/A de la meme serie, dispersion interne des executions d'un meme binaire) (CE-B2-04)")
print(f'GC-01 run alterne {d.name} : {len(ref)} execution(s) reference, {len(cur)} execution(s) courant ; VM : comparatif, NON certifiant'
      + (f' ; plancher de bruit A/A : {aa_dir.name}' if aa_dir else ''))
for gid, desc, x, ok in verdicts:
    if x is None:
        print(f'{gid:20s} NON MESURE  {desc}')
        continue
    ratio = f'{x[5]:.3f}' if x[5] is not None else 'n/a'
    noise = aa_noise(x) if gid in LIMITS else None
    verdict = 'PASS' if ok else 'ECHEC'
    if noise is not None and noise >= LIMITS[gid] - 1.0 and not (x[5] is not None and x[5] > 1.0 + 2.0 * noise and not ok):
        verdict = 'NON CONCLUANT'  # la marge du seuil est dans le bruit A/A de la VM : mesure physique requise
    extra = f' bruit A/A {noise:.3f}' if noise is not None else ''
    print(f'{gid:20s} {verdict:14s} ref {x[3]:.3f} cur {x[4]:.3f} ratio {ratio}{extra}  [{x[0]} {x[1]} {x[2]}]  {desc}')
if len(sys.argv) > 2:
    with open(sys.argv[2], 'w', encoding='utf-8', newline='') as f:
        wr = csv.writer(f, delimiter=';')
        wr.writerow(['metric', 'scenario', 'field', 'ref', 'cur', 'ratio'])
        wr.writerows(rows)
