"""ARCH-UI-04-R2-B2 — générateur reproductible du registre d'identifiants de composants et de la table des libellés.

Entrée (versionnée, lecture seule) : docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-R1.csv (nomenclature R1, LAB-A08).
Sorties :
  ui/r2/ComponentIds.inc  identifiants stables non paramétriques (OD-01, règles ID-01..ID-08) : action:<réf>, state:<réf>,
                          nav:<réf>. Les identifiants param:<StableID> se déduisent du registre (703) et ne sont pas listés.
                          Registre en AJOUT SEUL : un identifiant publié n'est jamais retiré ni renommé (--check le vérifie).
  ui/r2/Labels.inc        alias d'affichage : une entrée par occurrence du CSV (référence, occurrence, emplacement, libellé
                          court, nom complet, priorité, validé). « validé » = lexique validé (P1, LAB-A08) ; les P2 / P3 restent
                          des propositions (DB2-03) et ne sont jamais appliquées sans validation groupée.
Chaînes : octets non ASCII en échappements octaux (aucune dépendance à l'encodage du compilateur).

Usage : py -3.11 tools/gen_ui_labels.py           (réécrit les deux fichiers)
        py -3.11 tools/gen_ui_labels.py --check   (identité octet pour octet et ajout seul ; code 1 sinon)
"""
import csv, pathlib, re, sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
CSV = ROOT / 'docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-R1.csv'
IDS = ROOT / 'ui/r2/ComponentIds.inc'
LAB = ROOT / 'ui/r2/Labels.inc'
CHECK = '--check' in sys.argv


def c_str(s):
    out = []
    for b in s.encode('utf-8'):
        if b in (0x22, 0x5C):
            out.append('\\' + chr(b))
        elif 0x20 <= b < 0x7F:
            out.append(chr(b))
        else:
            out.append('\\%03o' % b)
    return '"' + ''.join(out) + '"'


rows = list(csv.DictReader(open(CSV, encoding='utf-8-sig')))
REF = next(k for k in rows[0] if k.lstrip('﻿').startswith('R'))

# Classification explicite des éléments hors registre (ID-02) : un identifiant n'est attribué qu'à un composant que l'on
# teste ou que l'on navigue. Les afficheurs sont des indicateurs (state:) ; les libellés statiques (intitulés d'axe ou de
# ligne, notes, légendes de commandes qui portent déjà param:<StableID>) n'ont pas d'identifiant ; tout le reste de la
# famille « Hors registre » est interactif (bouton, liste, champ, sélecteur) : action:. La famille « État » est le
# vocabulaire des textes d'état (OFF, ON, ARMED...), pas des composants : aucun identifiant.
STATE_REFS = {'bar.peak', 'perf.core.status', 'sound.wtz.order', 'eng.matrix.preview'}
STATIC_REFS = {'perf.core.x-axis', 'perf.core.y-axis', 'perf.auth.armed', 'perf.auth.exec', 'perf.auth.transport', 'perf.fm.index',
               'perf.fm.feedback', 'perf.balance.note', 'eng.matrix.self', 'eng.matrix.port'}


def nav_id(base):  # sound.cross.tab.bus -> nav:sound/cross/bus (schéma nav:<espace>/<groupe>/<outil>)
    return 'nav:' + base.replace('.tab.', '.').replace('.', '/')


ids = []
for r in rows:
    fam = r['Famille graphique']
    base = r[REF].split('#')[0]
    if fam == 'Hors registre':
        if base in STATIC_REFS:
            continue
        ident = ('state:' if base in STATE_REFS else 'action:') + base
    elif fam == 'Titre' and '.tab.' in base:
        ident = nav_id(base)
    else:
        continue
    if ident not in ids:
        ids.append(ident)
assert STATE_REFS | STATIC_REFS <= {r[REF].split('#')[0] for r in rows if r['Famille graphique'] == 'Hors registre'}
pat = re.compile(r'^(action|state|nav):[a-z0-9][a-z0-9._/-]*$')  # ASCII, indépendant du texte visible (ID-01)
assert all(pat.match(i) for i in ids), [i for i in ids if not pat.match(i)]

head = ('// GÉNÉRÉ par tools/gen_ui_labels.py depuis docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-R1.csv — ne pas modifier à la main.\n'
        '// Registre en ajout seul des identifiants de composants non paramétriques (OD-01, ID-02 / ID-03).\n')
ids_txt = head + ''.join(f'SBD_UI_COMPONENT_ID({c_str(i)})\n' for i in ids)

lab_txt = ('// GÉNÉRÉ par tools/gen_ui_labels.py depuis docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-R1.csv — ne pas modifier à la main.\n'
           '// SBD_UI_LABEL(référence, occurrence, option, emplacement, libellé court, nom complet, priorité, validé)\n'
           '// Suffixe #k du CSV : indice d\'option (0..n-1) pour les options de liste (famille SYNAPTIC TRACK), rang d\'occurrence\n'
           '// (>= 2) pour les variantes d\'emplacement. option = -1 : libellé de commande.\n')
for r in rows:
    ref = r[REF]
    base, suffix = (ref.split('#') + [''])[:2]
    is_option = r['Famille graphique'] == 'SYNAPTIC TRACK' and suffix != ''
    occ = 1 if (is_option or suffix == '') else int(suffix)
    opt = int(suffix) if is_option else -1
    validated = 1 if r['Priorité'] == 'P1' and r['Statut'].startswith('Lexique validé') else 0
    lab_txt += 'SBD_UI_LABEL({}, {}, {}, {}, {}, {}, {}, {})\n'.format(c_str(base), occ, opt, c_str(r['Emplacement']), c_str(r['Libellé court proposé']),
                                                                       c_str(r['Nom complet (Inspector, infobulle)']), c_str(r['Priorité']), validated)

if CHECK:
    ok = True
    for p, txt in [(IDS, ids_txt), (LAB, lab_txt)]:
        cur = p.read_text(encoding='utf-8') if p.exists() else ''
        if cur != txt:
            print(f'DIFFERENT : {p.relative_to(ROOT)}')
            ok = False
    old = re.findall(r'SBD_UI_COMPONENT_ID\("([^"]*)"\)', IDS.read_text(encoding='utf-8')) if IDS.exists() else []
    missing = [i for i in old if i not in ids]
    if missing:
        print('IDENTIFIANTS RETIRES (interdit, ajout seul) :', missing)
        ok = False
    print('identifiants et libelles identiques octet pour octet' if ok else 'ECHEC')
    sys.exit(0 if ok else 1)
IDS.write_text(ids_txt, encoding='utf-8', newline='\n')
LAB.write_text(lab_txt, encoding='utf-8', newline='\n')
print(f'{len(ids)} identifiants ({sum(i.startswith("action:") for i in ids)} action, {sum(i.startswith("state:") for i in ids)} state, '
      f'{sum(i.startswith("nav:") for i in ids)} nav) ; {len(rows)} libelles ({lab_txt.count(", 1)")} valides)')
