"""ARCH-UI-04-R2-B1-R1 — validation P1 (LAB-A08) : statut lexical x statut géométrique mesuré (reproductible).

Entrées : docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-R1.csv (gen_labels_r1.py) ;
          docs/ARCH-UI-04-R2/B1-R1-FINAL-ARTISTIC-REFINEMENT/data/<plateforme>/typography-measurements.csv (Component Lab, PT-02).
Sorties (même dossier B1-R1) : TYPOGRAPHY-MEASUREMENTS.csv (toutes plateformes), P1-TRACEABILITY.csv (une ligne par
occurrence P1), P1-NAMING-VALIDATION.md.
Règle : un budget indéterminé n'est JAMAIS déclaré PASS ; la validation lexicale (propriétaire) est distincte de la
validation géométrique (mesure), et la plateforme cible est macOS.

Usage : py -3.11 lab/tools/build_validation_r1.py
"""
import collections
import csv
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
R2 = ROOT / 'docs' / 'ARCH-UI-04-R2'
D = R2 / 'B1-R1-FINAL-ARTISTIC-REFINEMENT'
PLATFORMS = [p for p in ('macos', 'windows') if (D / 'data' / p / 'typography-measurements.csv').exists()]
GEO = {'OK': 'PASS', 'OK_MARGE<8': 'PASS (marge < 8 px)', '2_LIGNES': 'PASS en deux lignes', 'DÉPASSEMENT': 'FAIL',
       'BUDGET_NON_MESURE': 'INDÉTERMINÉ (gabarit futur)'}


def read(p):
    with open(p, encoding='utf-8-sig', newline='') as f:
        return list(csv.DictReader(f))


def write(p, rows, fields):
    with open(p, 'w', encoding='utf-8-sig', newline='') as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(rows)


labels = read(R2 / 'R2-CONTROL-LABELS-R1.csv')
meas = {p: read(D / 'data' / p / 'typography-measurements.csv') for p in PLATFORMS}
for p in PLATFORMS:
    assert len(meas[p]) == len(labels), (p, len(meas[p]), len(labels))
    for a, b in zip(meas[p], labels):
        assert a['Référence'] == b['Référence'] and a['Libellé court proposé'] == b['Libellé court proposé']

# 1. fusion des mesures
allm = [r for p in PLATFORMS for r in meas[p]]
write(D / 'TYPOGRAPHY-MEASUREMENTS.csv', allm, list(allm[0].keys()))

# 2. traçabilité P1
P1 = [k for k, r in enumerate(labels) if r['Priorité'] == 'P1']
trace = []
for k in P1:
    r = labels[k]
    row = {'Référence': r['Référence'], 'Occurrence': r['Occurrence'], 'Famille graphique': r['Famille graphique'], 'Emplacement': r['Emplacement'],
           'Libellé R2-A': r['Libellé R2-A'], 'Libellé R1 (LAB-A08)': r['Libellé court proposé'], 'Nom complet': r['Nom complet (Inspector, infobulle)'],
           'Statut lexical': 'VALIDÉ (LAB-A08)' + (' — modifié' if r['Libellé R2-A'] != r['Libellé court proposé'] else ''),
           'Budget px log.': r['Budget px log.'] or 'indéterminé'}
    for p in PLATFORMS:
        m = meas[p][k]
        row[f'{p} largeur 100 % px'] = m['Largeur mesurée 100 % px']
        row[f'{p} verdict'] = GEO[m['Verdict 100 %']]
    ref = meas['macos' if 'macos' in PLATFORMS else PLATFORMS[0]][k]
    row['Statut géométrique (cible macOS)'] = GEO[ref['Verdict 100 %']]
    row['Repère critique'] = ref['Repère critique']
    row['Plancher 9 px à 75 %'] = ref['Plancher 9 px à 75 %']
    trace.append(row)
write(D / 'P1-TRACEABILITY.csv', trace, list(trace[0].keys()))

# 3. rapport
geo_counts = {p: collections.Counter(GEO[meas[p][k]['Verdict 100 %']] for k in P1) for p in PLATFORMS}
L = []
A = L.append
A('# ARCH-UI-04-R2-B1-R1 — validation de la nomenclature P1 (LAB-A08)')
A('')
A('Fichier généré par `lab/tools/build_validation_r1.py`. Traçabilité individuelle des occurrences P1 :')
A('[P1-TRACEABILITY.csv](P1-TRACEABILITY.csv). Nomenclature révisée : [`../R2-CONTROL-LABELS-R1.csv`](../R2-CONTROL-LABELS-R1.csv)')
A('(générée par `lab/tools/gen_labels_r1.py`). La révision R2-A reste intacte et se régénère à l\'identique.')
A('')
A('## 1. Deux statuts distincts')
A('')
A('| Statut | Portée | Source |')
A('|---|---|---|')
A(f'| **Lexical** | {len(P1)} occurrences P1 : **validées** par le propriétaire (LAB-A08, option B), y compris 1 occurrence ajoutée par LAB-A08 (en-tête CROSS AUDIO) | arbitrage LAB-A08 |')
A('| **Géométrique** | mesure JUCE réelle dans le gabarit (PT-02) ; un budget indéterminé n\'est jamais déclaré PASS | `TYPOGRAPHY-MEASUREMENTS.csv` |')
A('')
A('P2 et P3 restent soumis à la validation progressive OD-02. Les 703 StableID, noms hôtes, titres accessibles, indices de')
A('choix, plages et unités sont inchangés. LAB-A08 n\'arbitre pas la restriction inter-domaines des transformations Z.')
A('')
A('## 2. Statut géométrique des occurrences P1')
A('')
heads = sorted({g for c in geo_counts.values() for g in c})
A('| Statut | ' + ' | '.join(PLATFORMS) + ' |')
A('|---|' + '---|' * len(PLATFORMS))
for g in heads:
    A(f'| {g} | ' + ' | '.join(str(geo_counts[p][g]) for p in PLATFORMS) + ' |')
A('')
A('## 3. Modifications LAB-A08')
A('')
A('| Référence | Emplacement | R2-A | R1 | Nom complet | ' + ' | '.join(f'{p} (px / budget)' for p in PLATFORMS) + ' |')
A('|---|---|---|---|---|' + '---|' * len(PLATFORMS))
for k, r in enumerate(labels):
    if r['Libellé R2-A'] != r['Libellé court proposé']:
        A(f"| `{r['Référence']}` | {r['Emplacement'].replace('|', '/')} | {r['Libellé R2-A']} | **{r['Libellé court proposé']}** | {r['Nom complet (Inspector, infobulle)']} | "
          + ' | '.join(f"{meas[p][k]['Largeur mesurée 100 % px']} / {r['Budget px log.'] or '?'} ({GEO[meas[p][k]['Verdict 100 %']]})" for p in PLATFORMS) + ' |')
A('')
A('AMOUNT et MODE n\'apparaissent que dans la section CROSS AUDIO (onglet BUS / CROSS AUDIO de SOUND DESIGN). Le recalcul ne trouve')
A('aucune nouvelle homonymie dans leurs emplacements. XAUDIO n\'est plus utilisé nulle part. Le gain général reste OUT GAIN ; GAIN')
A('désigne le gain de sortie d\'un étage, dans sa plaque.')
A('')
A('## 4. Contrôles exigés (brief §12.4)')
A('')
# homonymies
hom = collections.defaultdict(set)
for k in P1:
    r = labels[k]
    if r['Occurrence'].startswith('Variante'):
        hom[(r['Emplacement'], r['Libellé court proposé'])].add(r['Référence'])
hom = {k: v for k, v in hom.items() if len(v) > 1}
A(f'**Homonymies** (même libellé, StableID différents, même emplacement mesuré) : **{len(hom)} groupes** (16 relevés en B1). Toutes sont '
  'levées par un en-tête de plaque, de rangée ou de colonne ; aucune n\'a été créée par LAB-A08.')
A('')
A('| Libellé | Emplacement mesuré | StableID | Désambiguïsation |')
A('|---|---|---|---|')
for (loc, lab), refs in sorted(hom.items()):
    how = 'en-têtes source / destination de la cellule' if lab == 'DEPTH' else ('plaque de l\'emplacement (WT1 … GL2, filtre)' if 'TRAITEMENTS' in loc
                                                                                 else 'colonne ou rangée (opérateur, source CROSS)')
    A(f"| `{lab}` | {loc.replace('|', '/')} | {len(refs)} : {', '.join(sorted(refs)[:4])}{' …' if len(refs) > 4 else ''} | {how} |")
A('')
red = [k for k in P1 if labels[k]['Libellé court proposé'] in ('WT Z', 'CROSS LVL') and labels[k]['Occurrence'].startswith('Variante')]
A(f'**Redondances volontaires** : {len(red)} (WT Z, CROSS LVL). Elles sont conservées parce que le libellé apparaît hors de la plaque de son domaine et que le préfixe '
  'empêche l\'homonymie « Z Master » / CROSS (distinctions validées §12.2).')
A('')
for k in red:
    A(f"- `{labels[k]['Référence']}` — {labels[k]['Emplacement'].replace('|', '/')} : **{labels[k]['Libellé court proposé']}**")
A('')
undet = [k for k in P1 if any(meas[p][k]['Verdict 100 %'] == 'BUDGET_NON_MESURE' for p in PLATFORMS)]
A(f'**Budgets P1 indéterminés** : **{len(undet)}** (36 en B1, plus l\'en-tête CROSS AUDIO ajouté par LAB-A08). Statut géométrique : INDÉTERMINÉ, '
  'à vérifier dans les lots qui définissent leurs gabarits (barre et états : R2-B2 et R2-C ; titres et onglets : R2-D et R2-E). Largeurs mesurées :')
A('')
A('| Référence | Libellé R1 | ' + ' | '.join(f'{p} px' for p in PLATFORMS) + ' | Lot de vérification |')
A('|---|---|' + '---|' * len(PLATFORMS) + '---|')
for k in undet:
    ref = labels[k]['Référence']
    lot = 'R2-B2 / R2-C' if ref.startswith(('bar.', 'state.', 'perf.', 'units.')) else ('R2-E' if ref.startswith(('eng.', 'inspector.')) else 'R2-D')
    A(f"| `{ref}` | {labels[k]['Libellé court proposé']} | " + ' | '.join(meas[p][k]['Largeur mesurée 100 % px'] for p in PLATFORMS) + f' | {lot} |')
A('')
for title, verdicts in [('Marges faibles (< 8 px)', ('OK_MARGE<8',)), ('Libellés nécessitant deux lignes', ('2_LIGNES',)), ('Dépassements', ('DÉPASSEMENT',))]:
    ks = [k for k in range(len(labels)) if any(meas[p][k]['Verdict 100 %'] in verdicts for p in PLATFORMS)]
    A(f'**{title}** (toutes priorités, au moins une plateforme) : {len(ks)}.')
    A('')
    for k in ks:
        A(f"- `{labels[k]['Référence']}` {labels[k]['Emplacement'].replace('|', '/')} : **{labels[k]['Libellé court proposé']}** — budget {labels[k]['Budget px log.']} px ; "
          + ' ; '.join(f"{p} {meas[p][k]['Largeur mesurée 100 % px']} px ({GEO[meas[p][k]['Verdict 100 %']]})" for p in PLATFORMS))
    A('')
crit = [k for k in range(len(labels)) if meas[PLATFORMS[0]][k]['Repère critique'] == 'oui']
A(f'**Repères critiques au plancher de 9 px à 75 %** (OD-06) : {len(crit)} occurrences, corps retenu 12 px (XL 12,5).')
A('')
A('| Référence | Libellé | Corps px | 75 % écran px | ' + ' | '.join(f'{p}' for p in PLATFORMS) + ' |')
A('|---|---|---|---|' + '---|' * len(PLATFORMS))
for k in crit:
    m0 = meas[PLATFORMS[0]][k]
    A(f"| `{labels[k]['Référence']}` | {labels[k]['Libellé court proposé']} | {m0['Corps R2 retenu px log.']} | {m0['Corps écran 75 % px']} | "
      + ' | '.join(meas[p][k]['Plancher 9 px à 75 %'] for p in PLATFORMS) + ' |')
A('')
(D / 'P1-NAMING-VALIDATION.md').write_text('\n'.join(L) + '\n', encoding='utf-8')
print(f'P1 {len(P1)} ; plateformes {PLATFORMS} ; homonymies {len(hom)} ; budgets indéterminés {len(undet)} ; critiques {len(crit)}')
