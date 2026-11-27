#!/usr/bin/env python3
"""SUBDESTRUCKT — édition 0.3.3 du manuel (ARCH-G4.3-R1, correction DSP finale).
Copie l'édition source 0.3.2 (manual/src/0.3.2, inchangée) vers manual/src/0.3.3 et n'y modifie QUE les comportements
changés par ARCH-G4.3-R1 : Harmonic Tilt à énergie constante et crête bornée, continuité de Harmonic Shift, recette
complémentaire (écoute A/B des captures R2 entre fréquences, niveau de Tilt). Substitutions vérifiées. Les exemples
audio sont rendus par render_examples_g4 au commit de la build (seuls les exemples Tilt et Shift changent).
Usage (racine du dépôt) : python tools/manual-audio/make_manual_033.py"""
import json
import pathlib
import shutil
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SRC = ROOT / "manual/src/0.3.2"
DST = ROOT / "manual/src/0.3.3"


def fail(msg):
    print("ECHEC make_manual_033 : " + msg)
    sys.exit(1)


def edit(rel, pairs):
    p = DST / rel
    s = p.read_text(encoding="utf-8")
    for old, new in pairs:
        if s.count(old) != 1:
            fail(f"{rel} : texte attendu 1 fois, trouvé {s.count(old)} : {old[:80]}")
        s = s.replace(old, new)
    p.write_text(s, encoding="utf-8", newline="\n")


if DST.exists():
    shutil.rmtree(DST)
shutil.copytree(SRC, DST, ignore=shutil.ignore_patterns("audio"))
(DST / "audio").mkdir()

edit("chapters/01-01.html", [
    ("<h1>Bienvenue dans SUBDESTRUCKT 0.3.2</h1>", "<h1>Bienvenue dans SUBDESTRUCKT 0.3.3</h1>"),
    ("Cette édition décrit la build de développement intégré ARCH-G4.3 (DSP final, recette physique en attente).</p>",
     "Cette édition décrit la build corrective ARCH-G4.3-R1 (DSP final, recette physique en attente).</p>"),
    ("<h2>Ce qui est nouveau dans 0.3.2</h2>",
     "<h2>Ce qui est nouveau dans 0.3.3</h2><ul><li><strong>Harmonic Tilt</strong> redistribue l'énergie au lieu de "
     "l'amplifier : le niveau reste constant quand vous assombrissez ou éclaircissez (même sans Gain Compensation), et "
     "la crête de la forme ne peut monter que de 6 dB au plus par emplacement Tilt (réduction de gain douce, jamais "
     "d'écrêtage). Le caractère sombre ou brillant est conservé ; le niveau des sons utilisant Tilt change par rapport "
     "à 0.3.2.</li><li><strong>Harmonic Shift</strong> : même balayage ; les harmoniques entrent et sortent du spectre "
     "progressivement et la phase évolue sans saut (plus de clic quand la profondeur traverse certaines valeurs).</li>"
     "<li>Phase Scatter, D2, captures et autorisation par le transport : inchangés.</li></ul>"
     "<h2>Rappel : nouveautés de 0.3.2</h2>"),
])
edit("chapters/06-06.html", [
    ("Agit sur la forme avant la FM, le CROSS et le repli : ce n'est pas un filtre de sortie. Sans compensation de gain, "
     "un réglage très brillant peut devenir extrêmement fort (la protection d'extrême niveau s'engage) : gardez Gain "
     "Compensation activé.</li>",
     "Agit sur la forme avant la FM, le CROSS et le repli : ce n'est pas un filtre de sortie. Tilt <strong>redistribue</strong> "
     "l'énergie entre les harmoniques lues : le niveau reste constant, avec ou sans Gain Compensation, et la crête de la "
     "forme ne monte pas de plus de 6 dB par emplacement Tilt (au-delà, une réduction de gain douce s'applique).</li>"),
    ("déplace le contenu vers des rangs plus hauts ou plus bas sans changer la hauteur de la note (Shape fixe le sens et "
     "l'étendue) ; la fondamentale est conservée si Keep Fundamental est activé.</li>",
     "déplace le contenu vers des rangs plus hauts ou plus bas sans changer la hauteur de la note (Shape fixe le sens et "
     "l'étendue) ; la fondamentale est conservée si Keep Fundamental est activé. Le déplacement est continu : une "
     "harmonique qui entre ou sort du spectre le fait progressivement. Sur une forme aux partiels clairsemés, l'écart "
     "avec la forme d'origine n'augmente pas toujours avec la profondeur : des partiels déplacés retombent par moments "
     "sur les positions d'origine (comportement voulu du balayage).</li>"),
])
pp = DST / "parameters.json"
pp.write_text(pp.read_text(encoding="utf-8").replace('"0.3.2 : tableau genere', '"0.3.3 : tableau genere', 1), encoding="utf-8", newline="\n")

tp = DST / "recette/tests.json"
t = json.loads(tp.read_text(encoding="utf-8"))
t["campaignId"] = "SUBDESTRUCKT-ARCH-G431-RECETTE-PHYSIQUE"
t["source"] = "manual/src/0.3.3/recette/tests.json"
by = {x["id"]: x for x in t["tests"]}
for x in t["tests"]:
    x["prerequisites"] = [p.replace("SUBDESTRUCKT 0.3.2 compilé et installé", "SUBDESTRUCKT 0.3.3 compilé et installé") for p in x["prerequisites"]]
by["R-02"]["official"]["expected"] = by["R-02"]["expected"] = by["R-02"]["expected"].replace("version 0.3.2", "version 0.3.3")
r = by["R-29"]
r["steps"] = ["Écoutez les exemples e23 à e34 (comparez e23/e24 et e29/e30 avec l'édition 0.3.2 : niveau de Tilt et continuité de Shift).",
              "Sur WT, essayez chaque transformation avec Z Master, Depth et Shape ; automatisez Depth et Shape.",
              "Harmonic Tilt : balayez Shape de -1 à +1 avec Gain Compensation activé PUIS désactivé : le niveau doit rester du même ordre, sans saut ni saturation.",
              "Harmonic Shift : balayez la profondeur lentement sur Metal Spectra : aucun clic ; notez si la non-monotonie du timbre vous convient.",
              "Phase Scatter : avec Fold, N1 et le CROSS.", "D2 : Position 0, Feedback 0,5 à 1, Legacy puis Regenerative."]
r["official"]["expected"] = r["expected"] = ("Chaque transformation Z a un effet distinct ; Tilt sans saut de niveau (avec ou sans compensation) ; "
                                             "Shift sans clic ; Regenerative rend le Feedback audible à Position 0.")
t["tests"].append({
    "id": "R-30", "title": "Captures R2 entre fréquences d'échantillonnage : écoute A/B",
    "official": {"procedure": "Écouter docs/r1-audio/r2ab_*.wav (capture 48 kHz restituée à 44,1 / 48 / 88,2 / 96 kHz, même gain) ; "
                              "puis, dans la DAW, enregistrer un projet avec un gel S2 armé à 48 kHz et le rouvrir à 96 kHz",
                 "expected": "Même timbre et même niveau global ; à 88,2/96 kHz (et 44,1 kHz), éventuel léger flou des partiels graves (résolution du gel S2) jugé acceptable ou non."},
    "why": "La restitution entre fréquences conserve l'énergie mais la résolution du gel S2 change avec la fréquence : seul votre jugement d'écoute valide l'usage.",
    "prerequisites": ["SUBDESTRUCKT 0.3.3 compilé et installé.", "Casque ou moniteurs ; une DAW permettant de changer la fréquence du projet."],
    "skills": "Changer la fréquence d'échantillonnage d'un projet.",
    "steps": ["Écoutez la référence puis chaque restitution (même gain, aucune normalisation séparée).",
              "Dans la DAW : gel S2 armé, enregistrez à 48 kHz, rouvrez à 96 kHz puis 44,1 kHz ; lancez la lecture (autorisation par le transport).",
              "Notez toute différence de niveau, de hauteur des partiels graves ou de timbre."],
    "expected": "Même timbre et même niveau global ; à 88,2/96 kHz (et 44,1 kHz), éventuel léger flou des partiels graves (résolution du gel S2) jugé acceptable ou non.",
    "pass": "Différences jugées acceptables.", "fail": "Différence gênante (niveau, hauteur, timbre).",
    "onFail": "Noter les fréquences et la nature de la différence.", "evidenceHint": "Description d'écoute.",
})
for g in t["groups"]:
    if g["id"] == "musical":
        g["tests"].append("R-30")
tp.write_text(json.dumps(t, ensure_ascii=False, indent=1) + "\n", encoding="utf-8", newline="\n")
print("make_manual_033 : édition source 0.3.3 écrite (%d tests de recette)" % len(t["tests"]))
