#!/usr/bin/env python3
"""SUBDESTRUCKT — édition 0.3.1 du manuel (ARCH-G4.2).
Copie l'édition source 0.3.0 (manual/src/0.3.0, inchangée) vers manual/src/0.3.1 et n'y modifie QUE les comportements
effectivement changés par ARCH-G4.2 : autorisation des états autonomes (restauration, duplication, preset, Panic,
bouton Autoriser), politique MIDI Learn, compilation locale sécurisée, protection d'extrême niveau (marge). Chaque
substitution est vérifiée (échec si le texte d'origine a disparu). Les exemples audio sont rendus séparément par
render_examples_g4 au commit source de la build (contrôle bloquant de manual/build.py).
Usage (racine du dépôt) : python tools/manual-audio/make_manual_031.py"""
import json
import pathlib
import shutil
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SRC = ROOT / "manual/src/0.3.0"
DST = ROOT / "manual/src/0.3.1"


def fail(msg):
    sys.exit("make_manual_031 : " + msg)


def edit(rel, pairs):
    p = DST / rel
    s = p.read_text(encoding="utf-8")
    for pair in pairs:
        old, new = pair[0], pair[1]
        n = pair[2] if len(pair) > 2 else 1  # nombre d'occurrences attendu (toutes remplacées)
        if s.count(old) != n:
            fail(f"{rel} : texte d'origine introuvable ou ambigu ({s.count(old)} au lieu de {n}) : {old[:90]!r}")
        s = s.replace(old, new)
    p.write_text(s, encoding="utf-8", newline="\n")


if DST.exists():
    shutil.rmtree(DST)
# audio : rendu au commit de la build par render_examples_g4 (non copié depuis 0.3.0)
shutil.copytree(SRC, DST, ignore=shutil.ignore_patterns("audio"))
(DST / "audio").mkdir()

# ---------------------------------------------------------------- chapitre 1 : nouveautés
edit("chapters/01-01.html", [
    ("<h2>Ce qui est nouveau dans 0.3.0</h2>",
     "<h2>Ce qui est nouveau dans 0.3.1</h2><ul>"
     "<li><strong>États autonomes protégés à l'ouverture</strong> : un projet, une copie d'instance ou un preset qui "
     "contient un armement (filtre auto-oscillant ou gel modal, gel D2/S2, N2) ou une capture de gel arrive "
     "<strong>suspendu</strong> : rien ne joue tout seul. Le bouton <strong>Autoriser</strong> de la barre du haut "
     "active ces états pour cette instance. Vos réglages et vos captures sont conservés à l'identique.</li>"
     "<li><strong>PANIC</strong> suspend aussi les états autonomes armés (un nouvel Autoriser les relance).</li>"
     "<li><strong>MIDI Learn</strong> : une commande MIDI pour un réglage, réglages d'armement et messages MIDI "
     "réservés refusés, plusieurs instances ne s'effacent plus mutuellement leurs affectations.</li>"
     "<li><strong>Compilation locale sécurisée</strong> : le script s'arrête au premier échec, exige pluginval, "
     "produit toujours la livraison complète et n'installe plus rien lui-même.</li>"
     "<li>Sinon, le son est celui de 0.3.0 (mêmes exemples audio, rendus au nouveau commit).</li></ul>"
     "<h2>Rappel : nouveautés de 0.3.0</h2>"),
])

# ---------------------------------------------------------------- chapitre 2 : compilation et installation
edit("chapters/02-02.html", [
    ("<li>Lancez <code>tools/local-build/build_local_macos.sh --juce-licence=&lt;votre choix&gt; --install --pluginval</code>.</li>"
     "<li>Le script vérifie JUCE (SHA), compile, exécute les tests, signe le plugin localement et l'installe dans "
     "<code>~/Library/Audio/Plug-Ins/VST3</code>. Le rapport est dans <code>local-build/out</code> (mesures de "
     "performance de <strong>votre</strong> Mac comprises).</li></ol><p>Avec <code>--packages</code>, le script "
     "produit aussi, en local, les paquets Installer/Uninstaller et l'archive décrits ci-dessous.</p>",
     "<li>Validez ou retirez vos modifications locales : le script refuse un dépôt modifié (compilation non "
     "reproductible).</li>"
     "<li>Lancez <code>tools/local-build/build_local_macos.sh --juce-licence=&lt;votre choix&gt;</code> "
     "(option <code>--commit=&lt;sha&gt;</code> pour un autre commit).</li>"
     "<li>Le script exporte les sources exactes du commit, vérifie JUCE et pluginval (SHA), compile, exécute les "
     "tests, contrôle l'identité du plugin, lance pluginval en rigueur 10 et assemble la livraison complète dans "
     "<code>local-build/delivery-…</code>. Il s'arrête au premier échec et ne produit alors aucune livraison. Le "
     "rapport (mesures de performance de <strong>votre</strong> Mac comprises) est dans <code>local-build/out-…</code>.</li>"
     "<li>Il n'installe rien : ouvrez vous-même <code>SUBDESTRUCKT_Installer.pkg</code> (ci-dessous). Il signale "
     "une copie de SUBDESTRUCKT présente à la fois dans votre dossier VST3 utilisateur et dans le dossier système, "
     "sans y toucher.</li></ol>"),
    ("Avec <code>--install</code>, le bundle de votre dossier utilisateur est simplement remplacé.</p>",
     "Une ancienne copie installée par le script 0.3.0 (option <code>--install</code>, supprimée en 0.3.1) se trouve "
     "dans <code>~/Library/Audio/Plug-Ins/VST3</code> : retirez-la avant la recette pour éviter un doublon.</p>"),
    ("Installation par le script (<code>--install</code>) : supprimez <code>~/Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3</code>.</p>",
     "Ancienne installation par le script 0.3.0 (<code>--install</code>) : supprimez "
     "<code>~/Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3</code>.</p>"),
])

# ---------------------------------------------------------------- chapitre 4 : barre persistante, Inspector
edit("chapters/04-04.html", [
    ("Quality, Drone, zoom (75 à 150 %), Inspector, PANIC.</p>",
     "Quality, <strong>Autoriser</strong> (visible seulement quand des états autonomes sont suspendus), Drone, zoom "
     "(75 à 150 %), Inspector, PANIC.</p><div class=\"box fact\"><span class=\"tag\">États autonomes suspendus</span>"
     "<p>Un état autonome peut jouer sans note : filtre auto-oscillant ou gel modal, gel D2 ou S2, N2. À l'ouverture "
     "d'un projet, dans une copie d'instance, au chargement d'un preset avec capture, et après PANIC, ces états sont "
     "suspendus : leurs réglages et captures restent enregistrés tels quels, mais rien ne joue. Cliquez "
     "<strong>Autoriser</strong> pour les activer dans cette instance. L'autorisation n'est jamais enregistrée dans "
     "le projet : elle se redemande à chaque ouverture.</p></div>"),
    ("Vous pouvez y ajouter une modulation, activer le MIDI Learn ou le verrouiller contre le Randomizer.",
     "Vous pouvez y ajouter une modulation, activer le MIDI Learn (puis bougez la commande de votre contrôleur ; une "
     "commande pilote un seul réglage et un réglage une seule commande ; les armements autonomes, les réglages non "
     "automatisables et les messages MIDI réservés — Bank Select, Data Entry/RPN/NRPN, sustain, messages de mode — "
     "sont refusés) ou le verrouiller contre le Randomizer."),
    ("Capture produite par le test automatisé de l'éditeur",
     "Capture de l'édition 0.3.0, identique en 0.3.1 hors état suspendu, produite par le test automatisé de l'éditeur", 4),
])

# ---------------------------------------------------------------- chapitre 10 : captures
edit("chapters/10-10.html", [
    ("à l'ouverture, il est restauré dès que l'emplacement exécute D2 ou S2.",
     "à l'ouverture, il est conservé et restitué après <strong>Autoriser</strong> (barre du haut), dès que "
     "l'emplacement exécute D2 ou S2. Un gel S2 restitué continue ensuite de s'estomper selon son réglage Decay."),
])

# ---------------------------------------------------------------- chapitre 12 : Drone et PANIC
edit("chapters/12-12.html", [
    ("<strong>PANIC</strong> coupe en 5 ms, purge les états et désarme le Drone.</p>",
     "<strong>PANIC</strong> coupe en 5 ms, purge les états, désarme le Drone et suspend les états autonomes armés "
     "(réglages inchangés ; <strong>Autoriser</strong> les relance).</p>"),
])

# ---------------------------------------------------------------- chapitre 18 : limitations
edit("chapters/18-18.html", [
    ("MIDI Learn appliqué à la cadence de l'interface (≈ 33 ms).</li>",
     "MIDI Learn appliqué à la cadence de l'interface (≈ 33 ms).</li>"
     "<li>D2 : Feedback sans effet audible quand Position vaut 0 (comportement historique conservé ; une variante "
     "est à l'étude, non adoptée).</li>"
     "<li>Protection d'extrême niveau (au-delà de +24 dBFS) : toujours une proposition en attente de décision.</li>"),
])

# ---------------------------------------------------------------- titre, compatibilité, version
edit("chapters/01-01.html", [
    ("<h1>Bienvenue dans SUBDESTRUCKT 0.3.0</h1>", "<h1>Bienvenue dans SUBDESTRUCKT 0.3.1</h1>"),
    ("Les versions antérieures refusent un état 0.3.0.</p>",
     "Les versions antérieures refusent un état 0.3.0.</p><p>Les projets 0.3.0 s'ouvrent dans 0.3.1 (même format "
     "d'état) avec les mêmes réglages et captures ; s'ils contiennent un état autonome armé ou une capture, celui-ci "
     "attend <strong>Autoriser</strong>.</p>"),
])
edit("chapters/17-17.html", [
    ("<strong>Projet 0.3.0 ouvert dans une version antérieure :</strong>",
     "<strong>Projet 0.3.0 ou 0.3.1 ouvert dans une version antérieure à 0.3.0 :</strong>"),
])

# ---------------------------------------------------------------- glossaire : définitions héritées de 0.1.0-b5, fausses
# depuis 0.2.0 (défaut des éditions publiées 0.2.0 et 0.3.0, non modifiées) ; corrigées ici d'après les chapitres 5-7, 12
gp = DST / "glossary.json"
g = json.loads(gp.read_text(encoding="utf-8"))
terms = g if isinstance(g, list) else g["terms"]
fix = {
    "fm": "Famille de synthèse où un modulateur modifie un porteur. SUBDESTRUCKT 0.3.1 propose quatre modes : PM "
          "(modulation de phase, le son FM classique), Linear FM, Through-Zero FM et Exponential FM (chapitre 7).",
    "mono": "SUBDESTRUCKT 0.3.1 est monophonique : modes Mono ou Legato, priorité à la dernière, la plus basse ou la "
            "plus haute note (chapitre 12).",
    "oscillateur": "SUBDESTRUCKT 0.3.1 contient cinq oscillateurs : la wavetable (WT) et quatre opérateurs FM (OP1 à OP4).",
    "pm": "Mode de modulation par défaut des opérateurs de SUBDESTRUCKT 0.3.1 (avec Linear, Through-Zero et "
          "Exponential FM). Les profondeurs sont exprimées en radians (rad).",
    "wavetable": "La WT de SUBDESTRUCKT 0.3.1 lit une banque de formes en grille (jusqu'à 64 × 16) : X choisit la forme, "
                 "Y la table, Z applique une transformation calculée (chapitre 6).",
}
seen = set()
for term in terms:
    if term.get("id") in fix:
        term["long"] = fix[term["id"]]
        seen.add(term["id"])
if seen != set(fix):
    fail("glossaire : entrées introuvables " + str(set(fix) - seen))
gp.write_text(json.dumps(g, ensure_ascii=False, indent=1) + "\n", encoding="utf-8", newline="\n")
pp = DST / "parameters.json"
pp.write_text(pp.read_text(encoding="utf-8").replace('"0.3.0 : tableau genere', '"0.3.1 : tableau genere', 1), encoding="utf-8", newline="\n")

# ---------------------------------------------------------------- recette
tp = DST / "recette/tests.json"
t = json.loads(tp.read_text(encoding="utf-8"))
t["campaignId"] = "SUBDESTRUCKT-ARCH-G42-RECETTE-PHYSIQUE"
t["source"] = "manual/src/0.3.1/recette/tests.json"
by = {x["id"]: x for x in t["tests"]}
for x in t["tests"]:
    x["prerequisites"] = [p.replace("SUBDESTRUCKT 0.3.0 compilé et installé", "SUBDESTRUCKT 0.3.1 compilé et installé") for p in x["prerequisites"]]
r = by["R-01"]
r["official"]["procedure"] = "tools/local-build/build_local_macos.sh --juce-licence=<choix> (dépôt propre)"
r["official"]["expected"] = r["expected"] = ("Livraison produite (Installer.pkg, Uninstaller.pkg, README_FR, CHANGELOG, SHA256SUMS, ZIP) ; "
                                             "seuls des échecs connus documentés ; pluginval rigueur 10 PASS ; commit identique ; rien d'installé.")
r["steps"] = ["git status : aucun changement ; git rev-parse HEAD : comparez au commit communiqué.",
              "Lancez le script avec votre choix de licence.",
              "Lisez le résumé (critères, échecs connus) et local-build/out-…/machine.txt ; vérifiez la présence du ZIP dans local-build/delivery-…."]
r["pass"] = "Commit identique, livraison complète, pluginval PASS, aucun échec non documenté."
r["fail"] = "Arrêt du script (code et message), échec non documenté, livraison absente."
r["onFail"] = "Joindre local-build/out-… (journaux) et le code de sortie."
by["R-02"]["official"]["expected"] = by["R-02"]["expected"] = by["R-02"]["expected"].replace("version 0.3.0", "version 0.3.1")
by["R-02"]["steps"] = [s for s in by["R-02"]["steps"]] + ["Si une copie existe dans ~/Library/Audio/Plug-Ins/VST3 (ancienne option --install), retirez-la d'abord."]
r = by["R-11"]
r["steps"] = r["steps"] + ["MIDI Learn : Inspector › MIDI Learn sur WT X, bougez un potentiomètre ; recommencez sur WT Y avec le même potentiomètre (remplacement) ; tentez la pédale de sustain (refusée)."]
r["official"]["expected"] = r["expected"] = "Une modulation créée agit ; l'expression MIDI agit ; MIDI Learn : affectation, remplacement, sustain refusé."
r = by["R-13"]
r["official"]["expected"] = r["expected"] = "Réglages identiques ; états autonomes suspendus jusqu'à Autoriser ; Drone non réactivé sans autorisation."
r = by["R-16"]
r["steps"] = r["steps"] + ["Avec un gel armé : PANIC, puis vérifiez le silence et le bouton Autoriser ; Autoriser."]
r["official"]["expected"] = r["expected"] = "PANIC coupe en moins d'une seconde, désarme le Drone et suspend les états autonomes ; Autoriser les relance."
r = by["R-21"]
r["steps"] = ["Suivez l'exercice 13.", "Enregistrez aussi le projet pendant le gel armé ; fermez et rouvrez.",
              "À la réouverture : silence et bouton Autoriser visible ; cliquez Autoriser."]
r["official"]["expected"] = r["expected"] = "Après réouverture, rien ne joue avant Autoriser ; après Autoriser, le fragment capturé est rejoué ; le preset arrive désarmé."
r["pass"] = "Suspendu à l'ouverture ; fragment restitué après Autoriser ; preset désarmé."
r["fail"] = "Son sans Autoriser, fragment perdu, preset armé."
new = {
    "id": "R-25", "title": "États autonomes : duplication et autorisation",
    "official": {"procedure": "Dupliquer une piste contenant un gel armé ; rouvrir le projet ; Autoriser",
                 "expected": "La copie et le projet rouvert sont suspendus (silence, Autoriser visible) ; l'original continue ; Autoriser active la copie."},
    "why": "Vérifier qu'aucune restauration n'active seule un comportement autonome.",
    "prerequisites": ["SUBDESTRUCKT 0.3.1 compilé et installé.", "Une DAW et un clavier MIDI."],
    "skills": "Dupliquer une piste dans la DAW.",
    "steps": ["Créez un gel D2 armé qui tourne (exercice 13).", "Dupliquez la piste.",
              "Copie : silence, bouton Autoriser visible ; original : le gel continue.",
              "Autoriser dans la copie : le gel est rejoué.", "Enregistrez, fermez, rouvrez : suspendu jusqu'à Autoriser."],
    "expected": "La copie et le projet rouvert sont suspendus (silence, Autoriser visible) ; l'original continue ; Autoriser active la copie.",
    "pass": "Aucune activation sans Autoriser.", "fail": "Un gel joue sans Autoriser.",
    "onFail": "Noter l'étape et la DAW.", "evidenceHint": "Description ou capture d'écran.",
}
t["tests"].append(new)
for g in t["groups"]:
    if g["id"] == "technique":
        g["tests"].append("R-25")
tp.write_text(json.dumps(t, ensure_ascii=False, indent=1) + "\n", encoding="utf-8", newline="\n")
print("make_manual_031 : édition source 0.3.1 écrite (%d tests de recette)" % len(t["tests"]))
