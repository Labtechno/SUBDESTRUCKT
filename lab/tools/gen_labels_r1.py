"""ARCH-UI-04-R2-B1-R1 — révision R1 de la nomenclature (arbitrage LAB-A08), reproductible.

Entrée : docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-PROPOSALS.csv (révision R2-A, inchangée, régénérée par gen_labels.py).
Sortie : docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-R1.csv = R2-A + règles LAB-A08 appliquées de façon déterministe :
  - XAUDIO AMT -> AMOUNT et XAUDIO MODE -> MODE, sous l'en-tête de section CROSS AUDIO (nouvelle ligne de titre) ;
  - KEEP F0 -> KEEP FUND ; onglet BUS / XAUDIO -> BUS / CROSS AUDIO (XAUDIO abandonné) ; gain général = OUT GAIN ;
  - statut lexical : P1 = « Lexique validé (LAB-A08) » ; P2 et P3 = validation progressive OD-02 (inchangée) ;
  - statut géométrique : NON porté ici (il dépend des mesures PT-02, voir build_validation_r1.py).
Les largeurs « estimées » restent des estimations (0,525 x corps) ; seules les mesures PT-02 font foi.

Usage : py -3.11 lab/tools/gen_labels_r1.py            (réécrit le CSV R1)
        py -3.11 lab/tools/gen_labels_r1.py --check    (identité octet pour octet, code 1 sinon)
"""
import collections
import csv
import io
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SRC = ROOT / 'docs' / 'ARCH-UI-04-R2' / 'R2-CONTROL-LABELS-PROPOSALS.csv'
OUT = ROOT / 'docs' / 'ARCH-UI-04-R2' / 'R2-CONTROL-LABELS-R1.csv'
K = 0.525  # modèle d'ESTIMATION de R2-A (conservé pour traçabilité ; les mesures PT-02 font foi)

LABEL_RULES = {  # (référence) -> (ancien, nouveau, justification)
    'cross.audio.amount': ('XAUDIO AMT', 'AMOUNT', "LAB-A08 : AMOUNT sous l'en-tête de section CROSS AUDIO (XAUDIO abandonné)"),
    'cross.audio.mode': ('XAUDIO MODE', 'MODE', "LAB-A08 : MODE sous l'en-tête de section CROSS AUDIO (XAUDIO abandonné)"),
    'wt.z_fund': ('KEEP F0', 'KEEP FUND', 'LAB-A08 : KEEP FUND (abréviation validée)'),
    'sound.cross.tab.bus': ('BUS / XAUDIO', 'BUS / CROSS AUDIO', 'LAB-A08 : XAUDIO abandonné'),
}
NEW_ROWS = [  # nouvelles occurrences exigées par LAB-A08
    {'Référence': 'sound.cross.section.audio', 'Occurrence': 'Titre', 'Famille graphique': 'Titre', 'Emplacement': 'Sound Design / CROSS / BUS / AUDIO',
     'Libellé actuel': '—', 'Libellé court proposé': 'CROSS AUDIO', 'Nom complet (Inspector, infobulle)': 'Cross Audio section (post-chain)',
     'Corps px log.': '11', 'Justification': "LAB-A08 : en-tête de section requis pour AMOUNT et MODE", 'Priorité': 'P1',
     'Source mesure': 'Exigence LAB-A08 (nouvel en-tête ; gabarit défini en R2-B2 / R2-D)'},
]


def est(label, body):
    return round(len(label) * K * float(body), 1)


def main():
    with open(SRC, encoding='utf-8-sig', newline='') as f:
        rows = list(csv.DictReader(f))
    header = list(rows[0].keys())
    old_labels = [r['Libellé court proposé'] for r in rows]
    for r in rows:
        rule = LABEL_RULES.get(r['Référence'])
        if rule and r['Libellé court proposé'] == rule[0]:
            r['Libellé court proposé'] = rule[1]
            r['Justification'] = rule[2]
            r['Caractères'] = str(len(rule[1]))
            body = r['Corps px log.'] or '11'
            r['Largeur proposée estimée px log.'] = str(est(rule[1], body))
            if r['Budget px log.']:
                margin = round(float(r['Budget px log.']) - est(rule[1], body), 1)
                r['Marge estimée px'] = str(margin)
                r['Ajustement'] = 'OK (≥ 8 px)' if margin >= 8 else ('OK (marge < 8 px)' if margin >= 0 else 'DÉPASSEMENT — à revoir')
    for nr in NEW_ROWS:
        row = {h: '' for h in header}
        row.update(nr)
        row['Caractères'] = str(len(nr['Libellé court proposé']))
        row['Largeur proposée estimée px log.'] = str(est(nr['Libellé court proposé'], nr['Corps px log.']))
        rows.append(row)
        old_labels.append('—')
    # homonymies recalculées pour les libellés modifiés (même libellé, StableID différents, même emplacement)
    coll = collections.defaultdict(set)
    for r in rows:
        coll[(r['Emplacement'], r['Libellé court proposé'])].add(r['Référence'].split('#')[0])
    for r, old in zip(rows, old_labels):
        if r['Libellé court proposé'] != old and old != '—' or r['Référence'] in {n['Référence'] for n in NEW_ROWS}:
            n = len(coll[(r['Emplacement'], r['Libellé court proposé'])])
            if r['Référence'].startswith('cross.audio'):
                r["Risque d'ambiguïté"] = (f"moyen — {n} StableID partagent ce libellé dans l'emplacement mesuré (colonnes WT / CROSS / FM agrégées) ; "
                                           "désambiguïsé par l'en-tête de section CROSS AUDIO" if n > 1 else
                                           "faible — en-tête de section CROSS AUDIO (LAB-A08)")
            else:
                r["Risque d'ambiguïté"] = f"moyen — {n} StableID partagent ce libellé dans l'emplacement" if n > 1 else 'faible'
    # statuts
    out_rows = []
    for r, old in zip(rows, old_labels):
        changed = r['Libellé court proposé'] != old
        if r['Priorité'] == 'P1':
            r['Statut'] = 'Lexique validé (LAB-A08)' + (', libellé modifié ou ajouté' if changed else '') + ' ; géométrie : voir PT-02 R1'
        else:
            r['Statut'] = 'Proposé R2-A — validation progressive (OD-02)'
        r['Libellé R2-A'] = old
        r['Révision'] = 'R1 (LAB-A08)'
        out_rows.append(r)
    buf = io.StringIO(newline='')
    w = csv.DictWriter(buf, fieldnames=header + ['Libellé R2-A', 'Révision'])
    w.writeheader()
    w.writerows(out_rows)
    data = buf.getvalue().encode('utf-8-sig')
    if '--check' in sys.argv:
        same = OUT.read_bytes() == data
        print('CSV R1 identique octet pour octet' if same else 'CSV R1 DIFFÉRENT du fichier versionné')
        return 0 if same else 1
    OUT.write_bytes(data)
    changed = sum(1 for r, o in zip(out_rows, old_labels) if r['Libellé court proposé'] != o)
    p1 = sum(1 for r in out_rows if r['Priorité'] == 'P1')
    print(f'lignes {len(out_rows)} ; libellés modifiés ou ajoutés {changed} ; P1 {p1}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
