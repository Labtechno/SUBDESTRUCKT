# SUBDESTRUCKT — S0.1 — recette perceptive finale : feuille d'écoute ODT, instructions, archive ZIP, intégrité.
# Usage : py -3 -I scripts/make_final_listening.py <render_dir> <out_dir>
#   <render_dir> : sortie de `s01_cli final` (12 WAV C{1,2,3,5}_{REF,X,Y}.wav)
#   <out_dir>    : reçoit package/ (ODT + instructions), SUBDESTRUCKT_S01_FINAL_LISTENING.zip et INTEGRITY.md
# Bibliothèque standard + numpy. L'archive ne contient que les 12 WAV, la feuille ODT et les instructions (pas de clé).
import hashlib
import os
import re
import struct
import sys
import zipfile
from xml.sax.saxutils import escape

import numpy as np

CASES = [
    ("C1", "Feedback FM modéré", "OP1 en auto-modulation vers OP2, PM 1,2 rad, 110 Hz, OP1 en dent de scie"),
    ("C2", "Feedback FM élevé", "OP1 auto-modulé + cycle OP1 ↔ OP2, FM exponentielle 1,5 octave, 110 Hz"),
    ("C3", "Interaction WT/FM modérée", "Wavetable (dent de scie) ↔ OP1, PM 1,0 rad, 110 Hz"),
    ("C5", "Matrice FM dense", "Matrice 4×4 opérateurs, PM 1,0 rad, 110 Hz"),
]
ROLES = ["REF", "X", "Y"]
ZIP_NAME = "SUBDESTRUCKT_S01_FINAL_LISTENING.zip"
ODT_NAME = "FEUILLE_ECOUTE_S01_FINALE.odt"
TXT_NAME = "INSTRUCTIONS_ECOUTE.txt"
FIXED_TIME = (2026, 11, 25, 12, 0, 0)

# ------------------------------------------------------------------------------------------------ ODT
NS = ('xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" '
      'xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" '
      'xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" '
      'xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" '
      'xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" '
      'xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" '
      'xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" '
      'xmlns:dc="http://purl.org/dc/elements/1.1/"')

STYLES = f'''<?xml version="1.0" encoding="UTF-8"?>
<office:document-styles {NS} office:version="1.3">
 <office:font-face-decls>
  <style:font-face style:name="Liberation Sans" svg:font-family="'Liberation Sans'" style:font-family-generic="swiss"/>
 </office:font-face-decls>
 <office:styles>
  <style:default-style style:family="paragraph">
   <style:paragraph-properties fo:margin-top="0cm" fo:margin-bottom="0.15cm"/>
   <style:text-properties style:font-name="Liberation Sans" fo:font-size="10.5pt" fo:language="fr" fo:country="FR"/>
  </style:default-style>
  <style:default-style style:family="table-cell">
   <style:table-cell-properties fo:padding="0.12cm"/>
  </style:default-style>
  <style:style style:name="Standard" style:family="paragraph" style:class="text"/>
  <style:style style:name="Title" style:family="paragraph" style:parent-style-name="Standard" style:class="chapter">
   <style:paragraph-properties fo:margin-bottom="0.1cm"/>
   <style:text-properties fo:font-size="17pt" fo:font-weight="bold"/>
  </style:style>
  <style:style style:name="Subtitle" style:family="paragraph" style:parent-style-name="Standard" style:class="chapter">
   <style:paragraph-properties fo:margin-bottom="0.4cm"/>
   <style:text-properties fo:font-size="11pt" fo:color="#444444"/>
  </style:style>
  <style:style style:name="Heading_20_1" style:display-name="Heading 1" style:family="paragraph" style:parent-style-name="Standard" style:default-outline-level="1" style:class="text">
   <style:paragraph-properties fo:margin-top="0.5cm" fo:margin-bottom="0.15cm" fo:keep-with-next="always"/>
   <style:text-properties fo:font-size="13.5pt" fo:font-weight="bold"/>
  </style:style>
  <style:style style:name="Small" style:family="paragraph" style:parent-style-name="Standard">
   <style:paragraph-properties fo:keep-with-next="always"/>
   <style:text-properties fo:font-size="9pt" fo:color="#444444"/>
  </style:style>
 </office:styles>
 <office:automatic-styles>
  <style:page-layout style:name="pm1">
   <style:page-layout-properties fo:page-width="21cm" fo:page-height="29.7cm" style:print-orientation="portrait"
     fo:margin-top="1.8cm" fo:margin-bottom="1.8cm" fo:margin-left="2cm" fo:margin-right="2cm"/>
  </style:page-layout>
 </office:automatic-styles>
 <office:master-styles>
  <style:master-page style:name="Standard" style:page-layout-name="pm1"/>
 </office:master-styles>
</office:document-styles>
'''


def spaces(t):  # ODF fusionne les espaces consécutifs : les encoder en <text:s/>
    return re.sub(r" {2,}", lambda m: f' <text:s text:c="{len(m.group(0)) - 1}"/>', t)


def p(txt, style="Standard"):
    return f'<text:p text:style-name="{style}">{spaces(escape(txt))}</text:p>'


def p_bold(label, rest="", style="Standard"):
    return f'<text:p text:style-name="{style}"><text:span text:style-name="B">{escape(label)}</text:span>{escape(rest)}</text:p>'


def cell(content, style="TC"):
    return f'<table:table-cell table:style-name="{style}" office:value-type="string">{content}</table:table-cell>'


def form_table(name, rows, col_a="5.2cm", col_b="11.8cm"):
    out = [f'<table:table table:name="{name}" table:style-name="T">',
           '<table:table-column table:style-name="TA"/><table:table-column table:style-name="TB"/>']
    for label, answer, tall in rows:
        out.append('<table:table-row table:style-name="RT">' if tall else '<table:table-row>')
        out.append(cell(p_bold(label), "TCL"))
        out.append(cell(p(answer) if answer else p("")))
        out.append('</table:table-row>')
    out.append('</table:table>')
    return "".join(out)


BOX = "☐"


def content_xml():
    auto = '''<office:automatic-styles>
  <style:style style:name="T" style:family="table"><style:table-properties style:width="17cm" table:align="left" fo:margin-bottom="0.2cm" style:may-break-between-rows="false" fo:keep-with-next="auto"/></style:style>
  <style:style style:name="TA" style:family="table-column"><style:table-column-properties style:column-width="5.2cm"/></style:style>
  <style:style style:name="TB" style:family="table-column"><style:table-column-properties style:column-width="11.8cm"/></style:style>
  <style:style style:name="RT" style:family="table-row"><style:table-row-properties style:min-row-height="3.2cm"/></style:style>
  <style:style style:name="TC" style:family="table-cell"><style:table-cell-properties fo:padding="0.12cm" fo:border="0.5pt solid #000000"/></style:style>
  <style:style style:name="TCL" style:family="table-cell"><style:table-cell-properties fo:padding="0.12cm" fo:border="0.5pt solid #000000" fo:background-color="#eeeeee"/></style:style>
  <style:style style:name="B" style:family="text"><style:text-properties fo:font-weight="bold"/></style:style>
  <style:style style:name="PB" style:family="paragraph" style:parent-style-name="Heading_20_1"><style:paragraph-properties fo:break-before="page"/></style:style>
 </office:automatic-styles>'''
    body = [p("SUBDESTRUCKT — S0.1 — Recette perceptive finale", "Title"),
            p("Feuille d'écoute à l'aveugle — 4 cas × (REF, X, Y)", "Subtitle"),
            form_table("Auditeur", [
                ("Auditeur·rice", "", False),
                ("Date", "", False),
                ("Système d'écoute", f"{BOX} casque   {BOX} enceintes — modèle : ", False),
                ("Interface / lecteur", "Fréquence de sortie réglée à 48 kHz : " + f"{BOX} oui   {BOX} non   {BOX} je ne sais pas", False),
            ]),
            p("Mode d'emploi", "Heading_20_1"),
            p("Pour chaque cas, écouter REF, puis X et Y, autant de fois que nécessaire, en revenant à REF. "
              "REF est la référence 48 kHz. X et Y sont deux variantes à comparer à REF ; leur attribution est "
              "aléatoire et indépendante d'un cas à l'autre."),
            p(f"Pour répondre, remplacer {BOX} par ☒ (ou écrire un X à côté de la case). Une seule case par ligne. "
              "« incertain » et « aucune différence » sont des réponses valables : ne pas forcer un choix."),
            p("Les définitions des rubriques et la procédure complète figurent dans INSTRUCTIONS_ECOUTE.txt.", "Small")]
    for i, (cid, title, desc) in enumerate(CASES):
        body.append(p(f"{cid} — {title}", "Heading_20_1"))
        body.append(p(desc, "Small"))
        body.append(p(f"Fichiers : {cid}_REF.wav · {cid}_X.wav · {cid}_Y.wav", "Small"))
        body.append(form_table(f"{cid}", [
            ("Différence audible entre X et Y", f"{BOX} oui      {BOX} non      {BOX} incertain", False),
            ("Rendu le plus proche de REF", f"{BOX} X      {BOX} Y      {BOX} aucune différence", False),
            ("Préférence artistique", f"{BOX} X      {BOX} Y      {BOX} aucune", False),
            ("Observations libres", "", True),
        ]))
    body.append(p("Synthèse (facultatif)", "PB"))
    body.append(form_table("Synthese", [
        ("Remarques générales", "", True),
        ("Conditions d'écoute particulières (fatigue, bruit, nombre d'écoutes…)", "", True),
    ]))
    body.append(p("La clé X/Y n'est pas dans l'archive. Elle est conservée séparément et ne sera consultée "
                  "qu'après réception de cette feuille remplie.", "Small"))
    return (f'<?xml version="1.0" encoding="UTF-8"?>\n<office:document-content {NS} office:version="1.3">\n'
            f' {auto}\n <office:body><office:text>' + "".join(body) + '</office:text></office:body>\n</office:document-content>\n')


META = f'''<?xml version="1.0" encoding="UTF-8"?>
<office:document-meta {NS} office:version="1.3"><office:meta>
 <dc:title>SUBDESTRUCKT S0.1 — feuille d'écoute finale</dc:title>
 <dc:language>fr-FR</dc:language>
 <meta:generator>SUBDESTRUCKT spikes/s01/scripts/make_final_listening.py</meta:generator>
</office:meta></office:document-meta>
'''

MANIFEST = '''<?xml version="1.0" encoding="UTF-8"?>
<manifest:manifest xmlns:manifest="urn:oasis:names:tc:opendocument:xmlns:manifest:1.0" manifest:version="1.3">
 <manifest:file-entry manifest:full-path="/" manifest:version="1.3" manifest:media-type="application/vnd.oasis.opendocument.text"/>
 <manifest:file-entry manifest:full-path="content.xml" manifest:media-type="text/xml"/>
 <manifest:file-entry manifest:full-path="styles.xml" manifest:media-type="text/xml"/>
 <manifest:file-entry manifest:full-path="meta.xml" manifest:media-type="text/xml"/>
</manifest:manifest>
'''


def zinfo(name, compress):
    zi = zipfile.ZipInfo(name, FIXED_TIME)
    zi.compress_type = compress
    zi.external_attr = 0o644 << 16
    zi.create_system = 0
    return zi


def write_odt(path):
    with zipfile.ZipFile(path, "w") as z:
        z.writestr(zinfo("mimetype", zipfile.ZIP_STORED), "application/vnd.oasis.opendocument.text")
        z.writestr(zinfo("META-INF/manifest.xml", zipfile.ZIP_DEFLATED), MANIFEST)
        z.writestr(zinfo("content.xml", zipfile.ZIP_DEFLATED), content_xml())
        z.writestr(zinfo("styles.xml", zipfile.ZIP_DEFLATED), STYLES)
        z.writestr(zinfo("meta.xml", zipfile.ZIP_DEFLATED), META)


# ------------------------------------------------------------------------------------------------ instructions
INSTRUCTIONS = """SUBDESTRUCKT — S0.1 — Recette perceptive finale — Instructions d'écoute
=====================================================================

1. Contenu de l'archive
-----------------------
- 12 fichiers WAV : C1, C2, C3 et C5, chacun en trois versions REF, X et Y.
- FEUILLE_ECOUTE_S01_FINALE.odt : la feuille à remplir (LibreOffice, Word ou Google Docs).
- INSTRUCTIONS_ECOUTE.txt : ce document.

Les quatre cas :
- C1 : feedback FM modéré (OP1 en auto-modulation vers OP2, PM 1,2 rad, 110 Hz).
- C2 : feedback FM élevé (OP1 auto-modulé + cycle OP1 <-> OP2, FM exponentielle 1,5 octave, 110 Hz).
- C3 : interaction WT/FM modérée (wavetable dent de scie <-> OP1, PM 1,0 rad, 110 Hz).
- C5 : matrice FM dense (4x4 opérateurs, PM 1,0 rad, 110 Hz).

2. Ce que contiennent les fichiers
----------------------------------
- REF : rendu à 48 kHz avec oversampling 2x. C'est la référence.
- X et Y : deux rendus du même patch à 96 kHz avec oversampling 2x, convertis ensuite à 48 kHz.
  Ils ne diffèrent que par la variante interne testée. Leur attribution à X ou Y a été tirée au sort,
  indépendamment pour chaque cas : X dans un cas n'a aucun lien avec X dans un autre cas.
- Les patches, profondeurs de modulation et états initiaux sont strictement ceux de la première campagne.
- Format commun : WAV PCM 24 bits, mono, 48 kHz, 3,000 s, fondus d'entrée et de sortie de 20 ms.
- Niveaux : chaque fichier est aligné à -20 dBFS RMS. Aucune saturation (crête la plus haute vers -11 dBFS).
- Conversion 96 -> 48 kHz : filtre sinc à fenêtre de Kaiser, 2049 coefficients, phase linéaire,
  appliqué identiquement à X et Y. Plat à +/-0,00000003 dB jusqu'à 21,5 kHz, atténuation d'au moins
  165 dB à partir de 22,5 kHz.

3. Préparation
--------------
- Écoute au casque de bonne qualité ou sur enceintes de monitoring, dans un endroit calme.
- Régler si possible la sortie audio du système à 48 kHz, 24 bits.
  Sous Windows : Paramètres son > périphérique > Propriétés > Format.
  Sur macOS : Configuration audio et MIDI.
- Désactiver tout traitement : améliorations audio Windows, égaliseur, spatialisation, normalisation
  du volume du lecteur.
- Régler le volume une seule fois, sur C1_REF.wav, à un niveau confortable et modéré, puis ne plus y toucher.
- Lecteur conseillé : un lecteur qui permet de basculer instantanément entre fichiers en boucle.
  Par exemple, Audacity avec les trois fichiers d'un cas importés sur trois pistes et le bouton Solo,
  ou le composant ABX de foobar2000.

4. Procédure, pour chaque cas, dans l'ordre C1, C2, C3, C5
----------------------------------------------------------
a) Écouter REF plusieurs fois.
b) Écouter X, puis Y, en revenant à REF aussi souvent que nécessaire.
c) Remplir les quatre rubriques de la feuille :
   - Différence audible entre X et Y : oui / non / incertain.
   - Rendu le plus proche de REF : X / Y / aucune différence.
   - Préférence artistique, indépendamment de REF : X / Y / aucune.
   - Observations libres : timbre, aigus, rugosité, battements, stabilité, attaque, etc.
d) Faire une courte pause avant le cas suivant.

Pour cocher une case, remplacer le symbole ☐ par ☒, ou écrire un X à côté.
« Incertain » et « aucune différence » sont des réponses valables : il vaut mieux les utiliser
que deviner.

5. Règles du test à l'aveugle
-----------------------------
- Répondre à l'oreille uniquement. Ne pas analyser les fichiers (spectrogramme, comparaison numérique)
  avant d'avoir rempli la feuille.
- La clé X/Y n'est pas dans l'archive. Ne pas la chercher avant d'avoir terminé.
- Les fichiers ont tous le même format, la même durée et la même taille : rien dans leurs propriétés
  n'indique l'attribution.

6. Retour
---------
Renvoyer la feuille ODT remplie. La direction technique ouvrira ensuite la clé pour l'arbitrage final.
"""


# ------------------------------------------------------------------------------------------------ WAV / intégrité
def read_wav24(path):
    with open(path, "rb") as f:
        data = f.read()
    assert data[:4] == b"RIFF" and data[8:12] == b"WAVE", "en-tête RIFF/WAVE"
    riff_size = struct.unpack_from("<I", data, 4)[0]
    assert riff_size + 8 == len(data), "taille RIFF"
    pos, fmt, pcm = 12, None, None
    while pos + 8 <= len(data):
        cid, size = data[pos:pos + 4], struct.unpack_from("<I", data, pos + 4)[0]
        if cid == b"fmt ":
            fmt = struct.unpack_from("<HHIIHH", data, pos + 8)
        elif cid == b"data":
            pcm = data[pos + 8:pos + 8 + size]
            assert len(pcm) == size, "chunk data tronqué"
        pos += 8 + size + (size & 1)
    assert fmt is not None and pcm is not None, "chunks fmt/data"
    b = np.frombuffer(pcm, dtype=np.uint8).reshape(-1, 3).astype(np.int32)
    v = b[:, 0] | (b[:, 1] << 8) | (b[:, 2] << 16)
    v = np.where(v >= 1 << 23, v - (1 << 24), v)
    return fmt, v.astype(np.float64) / 8388608.0, v


def db(x):
    return 20 * np.log10(max(x, 1e-30))


def true_peak(x, os_=4):
    n = len(x)
    X = np.fft.rfft(x)
    Y = np.zeros(n * os_ // 2 + 1, dtype=complex)
    Y[:len(X)] = X
    return np.max(np.abs(np.fft.irfft(Y, n * os_) * os_))


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    render_dir, out_dir = sys.argv[1], sys.argv[2]
    pkg = os.path.join(out_dir, "package")
    os.makedirs(pkg, exist_ok=True)
    odt_path, txt_path = os.path.join(pkg, ODT_NAME), os.path.join(pkg, TXT_NAME)
    write_odt(odt_path)
    with open(txt_path, "w", encoding="utf-8", newline="\r\n") as f:
        f.write(INSTRUCTIONS)

    wavs = [f"{c}_{r}.wav" for c, _, _ in CASES for r in ROLES]
    rows, ok = [], True
    for w in wavs:
        path = os.path.join(render_dir, w)
        fmt, x, raw = read_wav24(path)
        tag, ch, sr, brate, balign, bits = fmt
        checks = {
            "PCM": tag == 1, "mono": ch == 1, "48 kHz": sr == 48000, "24 bits": bits == 24,
            "byte rate": brate == 144000 and balign == 3, "3,000 s": len(x) == 144000,
            "pas de plein échelle": int(np.sum((raw >= 8388607) | (raw <= -8388608))) == 0,
            "fini": bool(np.all(np.isfinite(x))),
        }
        rms, peak, tp = db(np.sqrt(np.mean(x * x))), db(np.max(np.abs(x))), db(true_peak(x))
        checks["RMS -20 ±0,01"] = abs(rms + 20) <= 0.01
        checks["crête vraie < -1 dBFS"] = tp < -1.0
        good = all(checks.values())
        ok &= good
        rows.append((w, os.path.getsize(path), sr, bits, ch, len(x) / sr, rms, peak, tp, float(np.mean(x)),
                     sha256(path), "OK" if good else "ÉCHEC : " + ", ".join(k for k, v in checks.items() if not v)))

    zip_path = os.path.join(out_dir, ZIP_NAME)
    members = wavs + [ODT_NAME, TXT_NAME]
    with zipfile.ZipFile(zip_path, "w") as z:
        for m in members:
            src = os.path.join(pkg if m in (ODT_NAME, TXT_NAME) else render_dir, m)
            with open(src, "rb") as f:
                z.writestr(zinfo(m, zipfile.ZIP_DEFLATED), f.read(), compresslevel=9)
    # vérification de l'archive : CRC, liste exacte, contenu identique aux sources
    with zipfile.ZipFile(zip_path) as z:
        bad = z.testzip()
        names = z.namelist()
        same = all(hashlib.sha256(z.read(m)).hexdigest() ==
                   sha256(os.path.join(pkg if m in (ODT_NAME, TXT_NAME) else render_dir, m)) for m in members)
        odt_ok = zipfile.ZipFile(odt_path).namelist()[0] == "mimetype"
    zip_checks = {"CRC (testzip)": bad is None, "14 entrées exactes": sorted(names) == sorted(members),
                  "aucune clé": not any("KEY" in n.upper() for n in names), "contenu = sources": same,
                  "ODT : mimetype en tête": odt_ok}
    ok &= all(zip_checks.values())

    with open(os.path.join(out_dir, "INTEGRITY.md"), "w", encoding="utf-8", newline="\n") as f:
        f.write("# S0.1 — recette perceptive finale — intégrité\n\n"
                "Généré par `scripts/make_final_listening.py` (relecture indépendante des WAV écrits par `s01_cli final`).\n\n"
                "| fichier | octets | fs | bits | canaux | durée (s) | RMS (dBFS) | crête (dBFS) | crête vraie 4× (dBFS) | DC | SHA-256 | contrôle |\n"
                "|---|---|---|---|---|---|---|---|---|---|---|---|\n")
        for r in rows:
            f.write(f"| {r[0]} | {r[1]} | {r[2]} | {r[3]} | {r[4]} | {r[5]:.3f} | {r[6]:.3f} | {r[7]:.2f} | {r[8]:.2f} | {r[9]:.1e} | `{r[10]}` | {r[11]} |\n")
        f.write(f"\n## Archive `{ZIP_NAME}`\n\n- taille : {os.path.getsize(zip_path)} octets\n- SHA-256 : `{sha256(zip_path)}`\n"
                f"- entrées : {', '.join(names)}\n")
        for k, v in zip_checks.items():
            f.write(f"- {k} : {'OK' if v else 'ÉCHEC'}\n")
        f.write(f"\n- `{ODT_NAME}` : {os.path.getsize(odt_path)} octets, SHA-256 `{sha256(odt_path)}`\n"
                f"- `{TXT_NAME}` : {os.path.getsize(txt_path)} octets, SHA-256 `{sha256(txt_path)}`\n"
                f"\nRésultat global : {'OK' if ok else 'ÉCHEC'}\n")
    print(open(os.path.join(out_dir, "INTEGRITY.md"), encoding="utf-8").read())
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
