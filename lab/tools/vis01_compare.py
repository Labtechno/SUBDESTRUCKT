"""ARCH-UI-04-R2-B2-03 — VIS-01, étape 2 de la chaîne de preuve visuelle CE-B2-03 : composant de PRODUCTION comparé à la
référence R1 HISTORIQUE publiée (docs/ARCH-UI-04-R2/B1-R1-FINAL-ARTISTIC-REFINEMENT/CAPTURES/<plateforme>/).

Entrée : le dossier rendu par `sbd_lab_compare --vis01 <dossier> [--sha <SHA de la référence>] --only <planche>` (planches
historiques reconstruites, composants R1 du laboratoire remplacés par les composants de production) et son manifeste
`vis01-cases.csv`. Les références ne sont jamais écrites.

Pour chaque image rendue :
  - CONTRÔLE : hors des bornes substituées (et hors des zones de production déclarées), l'image doit être identique à la
    référence ; sinon la comparaison de cette image est INVALIDE.
Pour chaque cas :
  - pixels différents dans les bornes, décomposés en « fenêtre de valeur » et « reste du composant » ;
  - IDENTIQUE : aucun pixel différent ;
  - TEXTE DE VALEUR : reste du composant identique ; seule la fenêtre de valeur diffère, et le texte de production
    diffère de celui de la référence (donnée non équivalente : la comparaison pixel à pixel de la fenêtre ne s'applique pas) ;
  - ETAT DU RENDU : le reste du composant diffère, MAIS le contrôle différentiel montre que le composant R1 du laboratoire
    lui-même, rendu à la même place dans les mêmes conditions (tous les autres cas substitués), présente au moins les
    mêmes pixels différents avec au moins le même écart. L'écart vient alors de l'état du moteur graphique (Direct2D met en
    cache la géométrie des tracés répétés), pas du composant de production. Exige --outil ;
  - DIFFERENTE : tout autre cas (à analyser, jamais déclaré conforme).

Usage : py -3.11 lab/tools/vis01_compare.py <dossier rendu> <plateforme> <dossier de sortie>
            [--outil=<sbd_lab_compare>] [--sha=<SHA de la référence>] [--b1=<captures B1 de la plateforme>]
Sorties : vis01-results.csv, vis01-controle.csv, vis01-bilan.txt, et une planche référence | production | écart (x8) par
cas non identique (dossier ecarts/). Code de sortie 1 si un contrôle est invalide ou si un cas est DIFFERENTE ou ERREUR.
"""
import csv, math, os, pathlib, subprocess, sys, tempfile

sys.stdout.reconfigure(encoding='utf-8')

import numpy as np
from PIL import Image

ROOT = pathlib.Path(__file__).resolve().parents[2]
REF = ROOT / 'docs/ARCH-UI-04-R2/B1-R1-FINAL-ARTISTIC-REFINEMENT/CAPTURES'
# zones de production déclarées (coordonnées logiques) : contenu qui évolue avec l'intégration B2-03, exclu du contrôle
PRODUCTION_ZONES = {'12-compare-041-r2.png': [(16, 76, 596, 880)]}  # plaque gauche du comparatif (composants de production)

opts = {a.split('=', 1)[0][2:]: a.split('=', 1)[1] for a in sys.argv[1:] if a.startswith('--') and '=' in a}
args = [a for a in sys.argv[1:] if not a.startswith('--')]
render, platform, out = pathlib.Path(args[0]), args[1], pathlib.Path(args[2])
out.mkdir(parents=True, exist_ok=True)
(out / 'ecarts').mkdir(exist_ok=True)
cases = list(csv.DictReader(open(render / 'vis01-cases.csv', encoding='utf-8')))


def px_rect(x, y, w, h, s):
    return math.floor(x * s), math.floor(y * s), math.ceil((x + w) * s), math.ceil((y + h) * s)


def load(p):
    return np.asarray(Image.open(p).convert('RGB'), dtype=np.int16)


def ref_view(planche, c, shape):
    ox, oy = int(c['origine_x']), int(c['origine_y'])
    return refs[planche][oy:oy + shape[0], ox:ox + shape[1]]


def case_diff(ref, cur, c):
    """carte d'écart dans les bornes du cas, et masque de la fenêtre de valeur (élargi d'un pixel)"""
    s = float(c['echelle'])
    x, y, w, h = int(c['x']), int(c['y']), int(c['w']), int(c['h'])
    x0, y0, x1, y1 = px_rect(x, y, w, h, s)
    dc = np.abs(ref[y0:y1, x0:x1] - cur[y0:y1, x0:x1]).max(axis=2)
    vmask = np.zeros_like(dc, dtype=bool)
    vx, vy, vw, vh = (float(c[k]) for k in ('valeur_x', 'valeur_y', 'valeur_w', 'valeur_h'))
    if vw > 0:
        a0, b0, a1, b1 = px_rect(x + vx, y + vy, vw, vh, s)
        vmask[max(0, b0 - y0 - 1):b1 - y0 + 1, max(0, a0 - x0 - 1):a1 - x0 + 1] = True
    return dc, vmask, (x0, y0, x1, y1)


def differential(c):
    """re-rend la planche en gardant le composant du laboratoire pour ce cas ; renvoie son écart hors fenêtre de valeur"""
    sheet = c['feuille']
    with tempfile.TemporaryDirectory() as tmp:
        cmd = [opts['outil'], '--vis01', tmp, '--only', sheet]
        if sheet != '12' and 'sha' in opts:  # la planche 12 historique ne porte pas de SHA dans son bandeau
            cmd += ['--sha', opts['sha']]
        if 'b1' in opts:
            cmd += ['--b1', opts['b1']]
        subprocess.run(cmd, check=True, capture_output=True, env=dict(os.environ, SBD_VIS01_KEEP=f"{c['x']}:{c['y']}"))  # cas désigné par sa position
        lab = load(pathlib.Path(tmp) / c['image'])
    ref = ref_view(c['planche'], c, lab.shape)
    dc, vmask, _ = case_diff(ref, lab, c)
    dc[vmask] = 0
    return dc


refs = {}
results, controls, bad = [], [], 0
by_image = {}
for c in cases:
    by_image.setdefault(c['image'], []).append(c)
for image, cs in by_image.items():
    planche = cs[0]['planche']
    s = float(cs[0]['echelle'])
    if planche not in refs:
        refs[planche] = load(REF / platform / planche)
    cur = load(render / image)
    H, W = cur.shape[:2]
    ref = ref_view(planche, cs[0], cur.shape)
    if ref.shape != cur.shape:
        controls.append([image, planche, 'TAILLE', f'{ref.shape[1]}x{ref.shape[0]}', f'{W}x{H}'])
        bad += 1
        continue
    d = np.abs(ref - cur).max(axis=2)
    mask = np.zeros((H, W), dtype=bool)
    for c in cs:
        x0, y0, x1, y1 = px_rect(int(c['x']), int(c['y']), int(c['w']), int(c['h']), s)
        mask[y0:y1, x0:x1] = True
    for (zx, zy, zw, zh) in PRODUCTION_ZONES.get(planche, []):
        x0, y0, x1, y1 = px_rect(zx, zy, zw, zh, s)
        mask[y0:y1, x0:x1] = True
    nc = int((d[~mask] > 0).sum())
    controls.append([image, planche, 'IDENTIQUE' if nc == 0 else 'INVALIDE', str(nc), str(int((~mask).sum()))])
    bad += nc > 0
    for c in cs:
        dc, vmask, (x0, y0, x1, y1) = case_diff(ref, cur, c)
        n_all = int((dc > 0).sum())
        n_val = int((dc[vmask] > 0).sum())
        n_rest = n_all - n_val
        texts_differ = c['texte_reference'] != c['texte_production']
        note = ''
        if c['erreur']:
            status = 'ERREUR'
        elif n_all == 0:
            status = 'IDENTIQUE'
        elif n_rest == 0 and texts_differ:
            status = 'TEXTE DE VALEUR'
        elif 'outil' in opts:
            rest = dc.copy()
            rest[vmask] = 0
            lab = differential(c)
            inc = bool(np.all(lab[rest > 0] >= rest[rest > 0]))
            note = f'differentiel : composant du laboratoire {int((lab > 0).sum())} px (max {int(lab.max())}), production {n_rest} px (max {int(rest.max())})'
            status = ('ETAT DU RENDU' if inc and (n_val == 0 or texts_differ) else 'DIFFERENTE')
        else:
            status = 'DIFFERENTE'
        bad += status in ('DIFFERENTE', 'ERREUR')
        results.append([c['famille'], planche, image, c['echelle'], c['cas'], c['parametre'], c['gabarit'], str(dc.size), str(n_all), str(n_rest), str(n_val),
                        str(int(dc.max()) if dc.size else 0), c['texte_reference'], c['texte_production'], c.get('norme_reference', ''),
                        c.get('norme_production', ''), c.get('ajustement_ulps', ''), status, note])
        if status != 'IDENTIQUE':
            a = ref[y0:y1, x0:x1].astype(np.uint8)
            b = cur[y0:y1, x0:x1].astype(np.uint8)
            e = np.clip(dc * 8, 0, 255).astype(np.uint8)
            e = np.stack([e, e, e], axis=2)
            sep = np.full((a.shape[0], 4, 3), 255, np.uint8)
            img = Image.fromarray(np.concatenate([a, sep, b, sep, e], axis=1))
            z = max(1, round(2 / s))
            if z > 1:
                img = img.resize((img.width * z, img.height * z), Image.NEAREST)
            img.save(out / 'ecarts' / f"{pathlib.Path(image).stem}-{c['parametre']}-{c['gabarit']}-{c['x']}-{c['y']}.png")

with open(out / 'vis01-results.csv', 'w', encoding='utf-8', newline='') as f:
    w = csv.writer(f)
    w.writerow(['famille', 'planche', 'image', 'echelle', 'cas', 'parametre', 'gabarit', 'pixels', 'differents', 'differents_hors_valeur',
                'differents_fenetre_valeur', 'ecart_max', 'texte_reference', 'texte_production', 'norme_reference', 'norme_production',
                'ajustement_ulps', 'statut', 'note'])
    w.writerows(results)
with open(out / 'vis01-controle.csv', 'w', encoding='utf-8', newline='') as f:
    w = csv.writer(f)
    w.writerow(['image', 'planche', 'controle', 'pixels_differents_hors_substitution', 'pixels_controles'])
    w.writerows(controls)
lines = [f'VIS-01 {platform} : {len(results)} cas, {len(controls)} images']
for st in ('IDENTIQUE', 'TEXTE DE VALEUR', 'ETAT DU RENDU', 'DIFFERENTE', 'ERREUR'):
    lines.append(f'  {st:16s} {sum(r[17] == st for r in results)}')
lines.append(f"  controles valides {sum(c[2] == 'IDENTIQUE' for c in controls)}/{len(controls)}")
for r in results:
    lines.append(f'  {r[0]:8s} {r[1]:28s} x{float(r[3]):.2f} {r[5]:26s} {r[6]:5s} {r[17]:16s} hors valeur {r[9]:>6s}  fenetre {r[10]:>5s}  '
                 f'[{r[12]}] -> [{r[13]}]' + (f'  {r[18]}' if r[18] else ''))
for c in controls:
    lines.append(f'  controle {c[0]:22s} {c[2]} ({c[3]} px hors substitution)')
open(out / 'vis01-bilan.txt', 'w', encoding='utf-8').write('\n'.join(lines) + '\n')
print('\n'.join(lines))
sys.exit(1 if bad else 0)
