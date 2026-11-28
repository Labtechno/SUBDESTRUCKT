"""ARCH-UI-04-R2-B2 — chaîne de preuve visuelle CE-B2-03, étape 1 : planches du Component Lab rendues avec les peintres
partagés (ui/r2/) comparées pixel à pixel aux captures R1 HISTORIQUES publiées (références indépendantes et immuables,
docs/ARCH-UI-04-R2/B1-R1-FINAL-ARTISTIC-REFINEMENT/CAPTURES/<plateforme>/). Le bandeau d'en-tête (titre, plateforme, SHA
du code de rendu) est exclu ; tout le reste doit être identique. Les références ne sont jamais régénérées par cet outil.

Usage : py -3.11 lab/tools/compare_r1_refs.py <dossier des planches rendues> <plateforme> [<csv de sortie>] [--header=N]
Contrôle strict : rendre avec --sha <SHA de la référence> et comparer avec --header=0 (images entières). Dans les planches
composites 02 et 06c, la planche B1 incrustée garde son propre bandeau (SHA) jusqu'à la ligne 178 : avec un SHA différent,
ces lignes diffèrent légitimement.
Code de sortie 1 si une planche diffère ou manque.
"""
import csv, pathlib, sys

import numpy as np
from PIL import Image

ROOT = pathlib.Path(__file__).resolve().parents[2]
REF = ROOT / 'docs/ARCH-UI-04-R2/B1-R1-FINAL-ARTISTIC-REFINEMENT/CAPTURES'
HEADER = 132  # bandeau d'en-tête à l'échelle 2 (66 px logiques, comme la non-régression NEXUS / PLASMA du laboratoire)

args = [x for x in sys.argv[1:] if not x.startswith('--header=')]
for x in sys.argv[1:]:
    if x.startswith('--header='):
        HEADER = int(x.split('=')[1])  # 0 : images entières (rendu fait avec la même chaîne de SHA que la référence)
sys.argv = [sys.argv[0]] + args
out_dir, platform = pathlib.Path(sys.argv[1]), sys.argv[2]
rows, fail = [], 0
for ref in sorted((REF / platform).glob('*.png')):
    cur = out_dir / ref.name
    if not cur.exists():
        rows.append([ref.name, 'ABSENTE', '', '', ''])
        fail += 1
        continue
    a = np.asarray(Image.open(ref).convert('RGBA'), dtype=np.int16)
    b = np.asarray(Image.open(cur).convert('RGBA'), dtype=np.int16)
    if a.shape != b.shape:
        rows.append([ref.name, 'TAILLE', f'{a.shape[1]}x{a.shape[0]}', f'{b.shape[1]}x{b.shape[0]}', ''])
        fail += 1
        continue
    d = np.abs(a[HEADER:] - b[HEADER:]).max(axis=2)
    n = int((d > 0).sum())
    rows.append([ref.name, 'IDENTIQUE' if n == 0 else 'DIFFERENTE', str(n), str(d.size), str(int(d.max()))])
    fail += n > 0
for r in rows:
    print(f'{r[0]:42s} {r[1]:10s} pixels differents {r[2]:>9s} / {r[3]:>9s}  ecart max {r[4]}')
print(f'BILAN {platform} : {len(rows) - fail}/{len(rows)} planches identiques aux references R1 historiques ' + ('(images entieres)' if HEADER == 0 else f'(hors bandeau de {HEADER} lignes)'))
if len(sys.argv) > 3:
    with open(sys.argv[3], 'w', encoding='utf-8', newline='') as f:
        w = csv.writer(f)
        w.writerow(['planche', 'statut', 'pixels_differents', 'pixels_compares', 'ecart_max'])
        w.writerows(rows)
sys.exit(1 if fail else 0)
