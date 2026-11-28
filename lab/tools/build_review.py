"""ARCH-UI-04-R2-B1 — fusion des mesures PT-02 par plateforme et revue des libellés P1 (reproductible, C02).

Entrées : docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-PROPOSALS.csv ; docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/data/<plateforme>/
typography-measurements.csv (produits par le Component Lab : `sbd_component_lab --all`).
Sorties : B1-COMPONENT-LAB/TYPOGRAPHY-MEASUREMENTS.csv (toutes plateformes) et B1-COMPONENT-LAB/P1-NAMING-REVIEW.md.
Aucun libellé n'est déclaré validé : la revue prépare l'arbitrage du propriétaire (OD-02, validation par priorité).

Usage : py -3.11 lab/tools/build_review.py
"""
import collections
import csv
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[2]
R2 = ROOT / 'docs' / 'ARCH-UI-04-R2'
B1 = R2 / 'B1-COMPONENT-LAB'
PLATFORMS = [p for p in ('macos', 'windows') if (B1 / 'data' / p / 'typography-measurements.csv').exists()]


def rows(path):
    with open(path, encoding='utf-8-sig', newline='') as f:
        return list(csv.DictReader(f))


props = rows(R2 / 'R2-CONTROL-LABELS-PROPOSALS.csv')
meas = {p: rows(B1 / 'data' / p / 'typography-measurements.csv') for p in PLATFORMS}
for p in PLATFORMS:
    assert len(meas[p]) == len(props), (p, len(meas[p]), len(props))

# 1. fusion (ordre : plateformes puis lignes du CSV de propositions)
with open(B1 / 'TYPOGRAPHY-MEASUREMENTS.csv', 'w', encoding='utf-8-sig', newline='') as f:
    w = None
    for p in PLATFORMS:
        for r in meas[p]:
            if w is None:
                w = csv.DictWriter(f, fieldnames=list(r.keys()))
                w.writeheader()
            w.writerow(r)

# 2. revue P1
P1 = [k for k, r in enumerate(props) if r['Priorité'] == 'P1']
GROUPS = [
    ('Distinctions critiques', lambda ref: ref in {'wt.z', 'cross.master', 'cross.fwd', 'cross.rev', 'perf.balance', 'cross.audio.amount',
                                                  'cross.audio.mode', 'out.gain', 'out.limiter', 'out.ceiling', 'bar.peak', 'units.peak'}),
    ('Autonomie : ARM, DRONE, AUTHORIZE, états', lambda ref: ref.endswith('.arm') or ref in {'bar.authorize', 'bar.drone', 'eng.state.drone'}
     or ref.startswith('perf.auth') or ref.startswith('state.')),
    ('Gel et capture', lambda ref: ref.endswith('freeze') or 'capture' in ref),
    ('Modes de feedback', lambda ref: ref.endswith('fb_mode')),
    ('Position WT et transformations Z', lambda ref: ref in {'wt.x', 'wt.y'} or ref.startswith('wt.z') or ref.startswith('sound.wtz')),
    ('Macros', lambda ref: ref.startswith('macro')),
    ('Core et PERFORMANCE', lambda ref: ref.startswith('perf.')),
    ('Barre globale', lambda ref: ref.startswith('bar.')),
    ('ENGINEERING : groupes, vues, ports', lambda ref: ref.startswith('eng.')),
    ('Réseau CROSS et FM', lambda ref: ref.startswith('cross.') or ref.startswith('fm.')),
    ('Autres P1 (priorité reprise de Codex)', lambda ref: True),
]


def group_of(ref):
    base = ref.split('#')[0]
    for name, f in GROUPS:
        if f(base):
            return name
    return GROUPS[-1][0]


def m(p, k, col):
    return meas[p][k][col] if p in meas else ''


def signal(k):
    v = {p: m(p, k, 'Verdict 100 %') for p in PLATFORMS}
    s = []
    for p, x in v.items():
        if x not in ('OK', ''):
            s.append(f'{p}: {x}')
    if any(m(p, k, 'Repère critique') == 'oui' for p in PLATFORMS):
        s.append('critique (12 px)')
    return '; '.join(s)


def esc(s):
    return s.replace('|', '\\|')


by_group = collections.OrderedDict((g[0], []) for g in GROUPS)
for k in P1:
    by_group[group_of(props[k]['Référence'])].append(k)

ABBR = ['XAUDIO', 'INHARM', 'PH MODE', 'S&H', 'EXC LVL', 'ST PHASE', 'MOD TYPE', 'DETECT', 'TRANS', 'SYM', 'CTRL', 'REGEN', 'FDBK', 'LVL',
        'GAIN COMP', 'KEEP F0', 'X EDGE', 'Y EDGE', 'XFADE', 'DIR']


def width_cols(k):
    return ' | '.join(m(p, k, 'Largeur mesurée 100 % px') or '—' for p in PLATFORMS)


lines = []
A = lines.append
A('# ARCH-UI-04-R2-B1 — revue des libellés P1')
A('')
A('Validation progressive par priorité (OD-02, option B). **Aucun libellé n\'est validé par ce document** : le propriétaire '
  'arbitre groupe par groupe. Source : `../R2-CONTROL-LABELS-PROPOSALS.csv` (ligne P1 = priorité P1). Mesures : PT-02 '
  '(`TYPOGRAPHY-MEASUREMENTS.csv`), largeurs en px logiques à 100 %, police d\'interface de chaque plateforme. Ce fichier '
  'est généré par `lab/tools/build_review.py`.')
A('')
A(f'Plateformes mesurées : {", ".join(PLATFORMS)}. Occurrences P1 : **{len(P1)}** '
  f'({sum(1 for k in P1 if props[k]["Occurrence"].startswith("Paramètre"))} canoniques, '
  f'{sum(1 for k in P1 if props[k]["Occurrence"].startswith("Variante"))} variantes, '
  f'{sum(1 for k in P1 if props[k]["Occurrence"].startswith("Option"))} options, '
  f'{sum(1 for k in P1 if not props[k]["Occurrence"][:4] in ("Para", "Vari", "Opti"))} hors registre).')
A('')
A('## 1. Synthèse par groupe')
A('')
A('| Groupe | Occurrences | Libellés distincts | Signalements |')
A('|---|---|---|---|')
for g, ks in by_group.items():
    if not ks:
        continue
    anchor = re.sub(r"[^\w\- ]", "", g.lower()).replace(" ", "-")  # algorithme d'ancre GitHub (lettres accentuées conservées)
    A(f'| [{g}](#{anchor}) | {len(ks)} | '
      f'{len(set(props[k]["Libellé court proposé"] for k in ks))} | {sum(1 for k in ks if signal(k))} |')
A('')
A('## 2. Libellés critiques à arbitrer en priorité')
A('')
A('| Fonction | Libellé proposé | Nom complet (Inspector, infobulle) | Note |')
A('|---|---|---|---|')
for ref, note in [('wt.z', 'homonymie « Z Master » levée'), ('cross.master', 'axe Z du Core : « Z · CROSS LVL » (deux lignes sur PLASMA V)'),
                  ('cross.fwd', 'axe X du Core : « X · WT→FM »'), ('cross.rev', 'axe Y du Core : « Y · FM→WT »'),
                  ('perf.balance', 'balance audio pré-chaînes'), ('cross.audio.amount', 'post-chaînes'),
                  ('out.gain', '**recommandation de l\'architecte : OUT GAIN** (et non GAIN, réservé au gain de sortie des étages)'),
                  ('bar.peak', 'mesure en dBFS, jamais dB'), ('bar.authorize', 'action d\'instance'), ('sound.chain.capture', 'action ; même terme partout'),
                  ('state.render', 'rendu hors ligne (0.4.1)')]:
    r = next(r for r in props if r['Référence'] == ref)
    A(f'| `{ref}` | **{esc(r["Libellé court proposé"])}** | {esc(r["Nom complet (Inspector, infobulle)"])} | {note} |')
A('')
A('## 3. Analyses transversales')
A('')
# homonymies : même libellé, StableID différents, même emplacement (P1 uniquement)
hom = collections.defaultdict(set)
for k in P1:
    r = props[k]
    if r['Occurrence'].startswith('Variante'):
        hom[(r['Emplacement'], r['Libellé court proposé'])].add(r['Référence'])
hom = {k: v for k, v in hom.items() if len(v) > 1}
A(f'**Homonymies** (même libellé, StableID différents, même emplacement mesuré) : {len(hom)} groupe(s). Toutes sont '
  'désambiguïsées par l\'en-tête de plaque ou de cellule ; à confirmer visuellement.')
A('')
for (loc, lab), refs in sorted(hom.items())[:40]:
    A(f'- `{esc(lab)}` — {esc(loc)} — {", ".join(sorted(refs))}')
A('')
red = [k for k in P1 if re.match(r'^(WT|FM|CROSS) ', props[k]['Libellé court proposé']) and props[k]['Référence'].split('.')[0].upper() in props[k]['Libellé court proposé'].split()[0]]
A(f'**Redondances potentielles** (préfixe de domaine conservé dans le libellé) : {len(red)}. Conservées volontairement '
  'quand le libellé apparaît hors de la plaque du domaine (WT Z, CROSS LVL) ; sinon à retirer.')
A('')
for k in red[:30]:
    A(f'- `{props[k]["Référence"]}` {esc(props[k]["Emplacement"])} : **{esc(props[k]["Libellé court proposé"])}**')
A('')
for title, verdicts in [('Dépassements', ('DÉPASSEMENT',)), ('Libellés nécessitant deux lignes', ('2_LIGNES',)), ('Marges insuffisantes (< 8 px)', ('OK_MARGE<8',))]:
    ks = [k for k in range(len(props)) if any(m(p, k, 'Verdict 100 %') in verdicts for p in PLATFORMS)]
    A(f'**{title}** (toutes priorités, au moins une plateforme) : {len(ks)}.')
    A('')
    for k in ks[:30]:
        A(f'- `{props[k]["Référence"]}` {esc(props[k]["Emplacement"])} : **{esc(props[k]["Libellé court proposé"])}** — budget '
          f'{props[k]["Budget px log."]} px ; ' + ' ; '.join(f'{p} {m(p, k, "Largeur mesurée 100 % px")} px ({m(p, k, "Verdict 100 %")})' for p in PLATFORMS))
    A('')
amb = collections.Counter()
for k in P1:
    lab = props[k]['Libellé court proposé']
    for a in ABBR:
        if re.search(r'(^|\W)' + re.escape(a) + r'($|\W)', lab):
            amb[a] += 1
A('**Abréviations à valider** (occurrences P1) : ' + ', '.join(f'{a} ({n})' for a, n in amb.most_common()) + '.')
A('')
nb = [k for k in P1 if any(m(p, k, 'Verdict 100 %') == 'BUDGET_NON_MESURE' for p in PLATFORMS)]
A(f'**Budgets non mesurés** (P1, éléments hors registre sans gabarit 0.4.1 relevé par Codex) : {len(nb)}. Leur budget sera fixé par '
  'la disposition R2-C ; les largeurs mesurées sont fournies.')
A('')
A('**Exceptions contextuelles** : `DEPTH` dans les cellules de réseau (route portée par les en-têtes) ; préfixes `X · `, `Y · `, '
  '`Z · ` des axes du Core ; préfixes `A` / `B` des slots Z ; `GAIN` (étages) face à `OUT GAIN` ; `BALANCE` avec la mention '
  '`PRE-CHAIN` ; `FM INDEX` / `FM FDBK` (libellés contextuels non arbitrés, OD-08).')
A('')
A('## 4. Détail par groupe')
A('')
plat_head = ' | '.join(f'{p} 100 % px' for p in PLATFORMS)
for g, ks in by_group.items():
    if not ks:
        continue
    A(f'### {g}')
    A('')
    A(f'| Référence | Occurrence | Emplacement | Actuel | Proposé | Nom complet | {plat_head} | Budget | Signalement |')
    A('|---|---|---|---|---|---|' + '---|' * len(PLATFORMS) + '---|---|')
    for k in ks:
        r = props[k]
        A(f'| `{esc(r["Référence"])}` | {esc(r["Occurrence"])} | {esc(r["Emplacement"])} | {esc(r["Libellé actuel"])} | '
          f'**{esc(r["Libellé court proposé"])}** | {esc(r["Nom complet (Inspector, infobulle)"])} | {width_cols(k)} | {r["Budget px log."] or "—"} | {signal(k)} |')
    A('')
(B1 / 'P1-NAMING-REVIEW.md').write_text('\n'.join(lines) + '\n', encoding='utf-8')
print(f'P1 {len(P1)} ; plateformes {PLATFORMS} ; fusion {sum(len(v) for v in meas.values())} lignes')
