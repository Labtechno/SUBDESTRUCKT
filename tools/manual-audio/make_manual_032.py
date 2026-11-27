#!/usr/bin/env python3
"""SUBDESTRUCKT — édition 0.3.2 du manuel (ARCH-G4.3, DSP FINAL LOCK, recette physique en attente).
Copie l'édition source 0.3.1 (manual/src/0.3.1, inchangée) vers manual/src/0.3.2 et n'y modifie QUE les comportements
effectivement changés par ARCH-G4.3 : TRANSPORT AUTHORIZATION (autorisation par la lecture, l'enregistrement et le
rendu hors temps réel ; Panic ; Drone), D2 LEGACY / REGENERATIVE, catalogue Z de huit transformations (Shape), protection
d'extrême niveau adoptée. Chaque substitution est vérifiée (échec si le texte d'origine a disparu). Les exemples audio
sont rendus séparément par render_examples_g4 au commit source de la build (28 exemples identiques à 0.3.1, 12 ajoutés).
Usage (racine du dépôt) : python tools/manual-audio/make_manual_032.py"""
import json
import pathlib
import shutil
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SRC = ROOT / "manual/src/0.3.1"
DST = ROOT / "manual/src/0.3.2"


def fail(msg):
    print("ECHEC make_manual_032 : " + msg)
    sys.exit(1)


def edit(rel, pairs):
    p = DST / rel
    s = p.read_text(encoding="utf-8")
    for pair in pairs:
        old, new = pair[0], pair[1]
        count = pair[2] if len(pair) > 2 else 1
        if s.count(old) != count:
            fail(f"{rel} : texte attendu {count} fois, trouvé {s.count(old)} : {old[:80]}")
        s = s.replace(old, new)
    p.write_text(s, encoding="utf-8", newline="\n")


if DST.exists():
    shutil.rmtree(DST)
shutil.copytree(SRC, DST, ignore=shutil.ignore_patterns("audio"))
(DST / "audio").mkdir()

TA = ("<strong>TRANSPORT AUTHORIZATION</strong> : un projet ouvert à l'arrêt garde ses états autonomes (filtre "
      "auto-oscillant ou gel modal, gel D2/S2, N2, Drone restauré) <strong>suspendus</strong> ; dès que vous lancez la "
      "<strong>lecture</strong> (ou l'enregistrement) de la DAW, ils sont autorisés automatiquement, sans ouvrir "
      "l'éditeur. Un <strong>export hors temps réel</strong> en lecture les autorise aussi. Le bouton "
      "<strong>Autoriser</strong> reste disponible pour les hôtes sans transport exploitable. L'autorisation n'est "
      "jamais enregistrée.")
edit("chapters/01-01.html", [
    ("<h1>Bienvenue dans SUBDESTRUCKT 0.3.1</h1>", "<h1>Bienvenue dans SUBDESTRUCKT 0.3.2</h1>"),
    ("Cette édition décrit la build de développement intégré ARCH-G4.1.</p>",
     "Cette édition décrit la build de développement intégré ARCH-G4.3 (DSP final, recette physique en attente).</p>"),
    ("<h2>Ce qui est nouveau dans 0.3.1</h2>",
     "<h2>Ce qui est nouveau dans 0.3.2</h2><ul><li>" + TA + "</li>"
     "<li><strong>PANIC</strong> retire toujours l'autorisation ; elle ne revient qu'après un arrêt puis une nouvelle "
     "lecture (ou Autoriser).</li>"
     "<li><strong>D2 Glitch Buffer</strong> : mode de Feedback <strong>Regenerative</strong> (le Feedback s'entend même "
     "à Position 0) par défaut dans les nouvelles instances ; les projets et presets antérieurs restent en "
     "<strong>Legacy</strong> (son historique).</li>"
     "<li><strong>Wavetable Z</strong> : catalogue complet de huit transformations (Harmonic Tilt, Mask, Focus, Shift, "
     "Phase Scatter ajoutées) et un réglage <strong>Shape</strong> par emplacement.</li>"
     "<li>Les 28 exemples audio de 0.3.1 sont identiques ; 12 exemples nouveaux (Z et D2).</li></ul>"
     "<h2>Rappel : nouveautés de 0.3.1</h2>"),
    ("Les projets 0.3.0 s'ouvrent dans 0.3.1 (même format d'état) avec les mêmes réglages et captures ; s'ils contiennent "
     "un état autonome armé ou une capture, celui-ci attend <strong>Autoriser</strong>.</p>",
     "Les projets 0.3.0 s'ouvrent dans 0.3.1 (même format d'état) avec les mêmes réglages et captures ; s'ils contiennent "
     "un état autonome armé ou une capture, celui-ci attend <strong>Autoriser</strong>.</p><p>0.3.2 écrit le "
     "<strong>format d'état 4</strong> : les projets 0.3.x s'ouvrent avec leur son historique (D2 en Legacy) ; un projet "
     "enregistré par 0.3.2 est refusé par les versions antérieures.</p>"),
])
edit("chapters/04-04.html", [
    ("Un état autonome peut jouer sans note : filtre auto-oscillant ou gel modal, gel D2 ou S2, N2. À l'ouverture d'un "
     "projet, dans une copie d'instance, au chargement d'un preset avec capture, et après PANIC, ces états sont suspendus : "
     "leurs réglages et captures restent enregistrés tels quels, mais rien ne joue. Cliquez <strong>Autoriser</strong> pour "
     "les activer dans cette instance. L'autorisation n'est jamais enregistrée dans le projet : elle se redemande à chaque "
     "ouverture.</p>",
     "Un état autonome peut jouer sans note : filtre auto-oscillant ou gel modal, gel D2 ou S2, N2, Drone. À l'ouverture "
     "d'un projet, dans une copie d'instance et au chargement d'un preset avec capture, ces états sont suspendus : leurs "
     "réglages et captures restent enregistrés tels quels, mais rien ne joue. <strong>Lancer la lecture</strong> (ou "
     "l'enregistrement) de la DAW les autorise ; un export hors temps réel aussi. Un projet chargé pendant que la DAW lit "
     "déjà attend un arrêt puis une nouvelle lecture. <strong>Autoriser</strong> les active immédiatement (secours pour "
     "les hôtes sans transport). Mettre en marche un armement ou le Drone d'un clic dans l'éditeur vaut aussi "
     "autorisation. Après PANIC, l'autorisation est retirée jusqu'au prochain arrêt suivi d'une lecture, ou Autoriser. "
     "L'autorisation n'est jamais enregistrée ; l'onglet Engineering › Diagnostics affiche son état et les derniers "
     "événements de transport.</p>"),
])
edit("chapters/06-06.html", [
    ("<h2>Z : trois transformations</h2><ul><li><strong>Harmonic Fold</strong>",
     "<h2>Z : huit transformations</h2><ul><li><strong>Harmonic Fold</strong>"),
    ("<li><strong>Spectral Mutation</strong> : petites variations aléatoires d'un cycle à l'autre (reproductibles).</li></ul>",
     "<li><strong>Spectral Mutation</strong> : petites variations aléatoires d'un cycle à l'autre (reproductibles).</li>"
     "<li><strong>Harmonic Tilt</strong> : penche le spectre de la forme, du sombre et massif (Shape −1) au brillant et "
     "abrasif (Shape +1). Agit sur la forme avant la FM, le CROSS et le repli : ce n'est pas un filtre de sortie. Sans "
     "compensation de gain, un réglage très brillant peut devenir extrêmement fort (la protection d'extrême niveau "
     "s'engage) : gardez Gain Compensation activé.</li>"
     "<li><strong>Harmonic Mask</strong> : creuse un peigne régulier dans la série harmonique (Shape −1 : harmoniques "
     "paires, son creux ; 0 : une sur trois ; +1 : une sur huit).</li>"
     "<li><strong>Harmonic Focus</strong> : concentre l'énergie autour d'un rang (Shape place le centre, la profondeur "
     "resserre la zone) : formants, cavités, résonances métalliques.</li>"
     "<li><strong>Harmonic Shift</strong> : déplace le contenu vers des rangs plus hauts ou plus bas sans changer la "
     "hauteur de la note (Shape fixe le sens et l'étendue) ; la fondamentale est conservée si Keep Fundamental est "
     "activé.</li>"
     "<li><strong>Phase Scatter</strong> : change les relations de phase entre harmoniques sans toucher au spectre "
     "d'amplitude ; inaudible seul, il change fortement la réponse au repli, à la FM et au CROSS (Shape choisit le "
     "motif, identique d'une instance à l'autre).</li></ul>"),
    ("<p>Deux emplacements Z en série (ordre A→B ou B→A), chacun avec sa profondeur et sa courbe, sous le Z Master.</p>"
     "<div class=\"box warn\"><span class=\"tag\">Limitation</span><p>Le catalogue Z prévoit huit transformations. Cinq "
     "transformations historiques n'ont pas été retrouvées et ne sont pas implémentées : elles ne sont pas inventées.</p></div>",
     "<p>Deux emplacements Z en série (ordre A→B ou B→A), chacun avec sa profondeur, sa courbe et son Shape, sous le Z "
     "Master. Les cinq transformations harmoniques agissent sur la forme elle-même et s'appliquent avant Phase Warp, "
     "Spectral Mutation et Harmonic Fold, quel que soit l'ordre ; l'ordre A→B ou B→A compte entre deux transformations "
     "harmoniques.</p><div class=\"box warn\"><span class=\"tag\">Limitation</span><p>Les transformations harmoniques "
     "sont recalculées toutes les 128 échantillons, avec un fondu : une modulation de Z très rapide (par exemple par le "
     "CROSS à fréquence audio) les suit à cette cadence, alors que Fold, Warp et Mutation la suivent à chaque "
     "échantillon.</p></div>{{AB:e23_z_tilt_dark|e24_z_tilt_bright|Harmonic Tilt : sombre puis brillant}}"
     "{{AB:e25_z_mask_hollow|e26_z_mask_sweep|Harmonic Mask : creux, puis balayage du peigne}}"
     "{{AB:e27_z_focus_formant|e28_z_focus_metal|Harmonic Focus : formant mobile, puis cavité sur Metal Spectra}}"
     "{{AB:e29_z_shift_up|e30_z_shift_down|Harmonic Shift : vers le haut, puis vers le bas}}"
     "{{AB:e31_z_scatter_fold|e32_z_scatter_fm|Phase Scatter : avant un repli, puis via le CROSS vers OP1}}"),
])
edit("chapters/10-10.html", [
    ("<li><strong>D2 Glitch Buffer</strong> : boucles de fragments, lecture inversée, Freeze. Le Feedback n'est audible "
     "que si Position est supérieure à 0.</li>",
     "<li><strong>D2 Glitch Buffer</strong> : boucles de fragments, lecture inversée, Freeze. <strong>Feedback Mode</strong> "
     "(par emplacement) : <strong>Regenerative</strong> (défaut des nouvelles instances) régénère le fragment en cours "
     "de lecture, le Feedback s'entend donc aussi à Position 0 ; <strong>Legacy</strong> (projets et presets antérieurs) "
     "garde le comportement historique, où le Feedback n'est audible que si Position est supérieure à 0. Feedback 0 : "
     "les deux modes sont identiques ; le gel n'est jamais réécrit.</li>"),
    ("{{AB:e14_d1_crush|e15_s1_ring|Crusher puis ring modulator}}",
     "{{AB:e14_d1_crush|e15_s1_ring|Crusher puis ring modulator}}{{AB:e33_d2_legacy|e34_d2_regenerative|D2 à Position 0, "
     "Feedback 0,8 : Legacy puis Regenerative}}"),
])
edit("chapters/12-12.html", [
    ("et n'est restauré à l'ouverture d'un projet que si vous l'avez autorisé (Engineering › État). <strong>PANIC</strong> "
     "coupe en 5 ms, purge les états, désarme le Drone et suspend les états autonomes armés (réglages inchangés ; "
     "<strong>Autoriser</strong> les relance).</p>",
     "et sa configuration n'est restaurée à l'ouverture d'un projet que si vous l'avez permis (Engineering › État) ; même "
     "restauré, il attend l'autorisation (lecture de la DAW ou Autoriser) avant de maintenir une note. "
     "<strong>PANIC</strong> coupe en 5 ms, purge les états et les tampons de compensation de latence, désarme le Drone "
     "et retire toujours l'autorisation (réglages inchangés) : elle revient après un arrêt puis une lecture, ou avec "
     "<strong>Autoriser</strong>. Le message MIDI All Sound Off (CC120) agit comme PANIC ; All Notes Off (CC123) relâche "
     "seulement les notes.</p>"),
])
edit("chapters/17-17.html", [
    ("<li><strong>Projet 0.3.0 ou 0.3.1 ouvert dans une version antérieure à 0.3.0 :</strong> l'état est refusé (format "
     "plus récent) ; gardez une copie avant tout retour arrière.</li>",
     "<li><strong>Projet 0.3.0 ou 0.3.1 ouvert dans une version antérieure à 0.3.0 :</strong> l'état est refusé (format "
     "plus récent) ; gardez une copie avant tout retour arrière.</li><li><strong>Projet 0.3.2 ouvert dans 0.3.1 ou "
     "avant :</strong> refusé (format d'état 4).</li><li><strong>Le gel ou le Drone ne joue pas à l'ouverture :</strong> "
     "normal tant que la DAW est à l'arrêt ; lancez la lecture. S'il ne démarre toujours pas (projet ouvert pendant la "
     "lecture, hôte sans transport, export dans un hôte qui ne signale pas le rendu), arrêtez puis relancez la lecture, "
     "ou cliquez Autoriser, et notez l'hôte et la situation dans la recette (R-26 à R-28).</li><li><strong>Export sans "
     "les états autonomes :</strong> cliquez Autoriser avant l'export et signalez-le (R-28).</li>"),
])
pp = DST / "parameters.json"
pp.write_text(pp.read_text(encoding="utf-8").replace('"0.3.1 : tableau genere', '"0.3.2 : tableau genere', 1), encoding="utf-8", newline="\n")

# ---------------------------------------------------------------- recette
tp = DST / "recette/tests.json"
t = json.loads(tp.read_text(encoding="utf-8"))
t["campaignId"] = "SUBDESTRUCKT-ARCH-G43-RECETTE-PHYSIQUE"
t["source"] = "manual/src/0.3.2/recette/tests.json"
by = {x["id"]: x for x in t["tests"]}
for x in t["tests"]:
    x["prerequisites"] = [p.replace("SUBDESTRUCKT 0.3.1 compilé et installé", "SUBDESTRUCKT 0.3.2 compilé et installé") for p in x["prerequisites"]]
by["R-02"]["official"]["expected"] = by["R-02"]["expected"] = by["R-02"]["expected"].replace("version 0.3.1", "version 0.3.2")
r = by["R-13"]
r["official"]["expected"] = r["expected"] = ("Réglages identiques ; états autonomes et Drone restauré suspendus à l'arrêt ; "
                                             "autorisés au lancement de la lecture, sans Autoriser.")
r = by["R-16"]
r["steps"] = r["steps"] + ["Pendant la lecture, avec un gel armé : PANIC ; laissez la lecture continuer 5 s (rien ne revient) ; arrêtez puis relancez la lecture (le gel revient)."]
r["official"]["expected"] = r["expected"] = ("PANIC coupe en moins d'une seconde, désarme le Drone et retire l'autorisation ; "
                                             "rien ne revient pendant la même lecture ; un arrêt puis une lecture la rétablit.")
r = by["R-21"]
r["steps"] = ["Suivez l'exercice 13.", "Enregistrez aussi le projet pendant le gel armé ; fermez et rouvrez.",
              "À la réouverture, DAW à l'arrêt : silence ; lancez la lecture : le fragment capturé est rejoué sans Autoriser."]
r["official"]["expected"] = r["expected"] = ("Après réouverture, rien ne joue à l'arrêt ; la lecture restitue le fragment capturé ; "
                                             "le preset arrive désarmé.")
r["pass"] = "Silence à l'arrêt ; fragment restitué au lancement de la lecture ; preset désarmé."
r["fail"] = "Son à l'arrêt sans action, fragment perdu ou non restitué à la lecture, preset armé."
r = by["R-25"]
r["steps"] = ["Créez un gel D2 armé qui tourne (exercice 13).", "Pendant la lecture, dupliquez la piste.",
              "Copie : silence (bouton Autoriser visible) tant que la lecture continue ; original : le gel continue.",
              "Arrêtez puis relancez la lecture : la copie joue aussi.",
              "Enregistrez, fermez, rouvrez : silence à l'arrêt, gels restitués à la lecture."]
r["official"]["procedure"] = "Dupliquer pendant la lecture une piste contenant un gel armé ; arrêter puis relancer ; rouvrir le projet"
r["official"]["expected"] = r["expected"] = ("Copie créée pendant la lecture : suspendue jusqu'à un arrêt puis une lecture ; "
                                             "original inchangé ; projet rouvert : silence à l'arrêt, restitution à la lecture.")
r["pass"] = "Aucune activation pendant la lecture déjà en cours ; restitution après arrêt puis lecture."
r["fail"] = "La copie joue immédiatement, ou ne joue jamais après arrêt puis lecture."


def ta_test(tid, host, extra):
    steps = [
        "Préparez le projet de recette : piste 1 Drone + note tenue ; piste 2 gel D2 armé ; piste 3 gel S2 armé ; piste 4 "
        "Feedback D2 Regenerative ; piste 5 preset non autonome. Activez le journal : créez le fichier vide "
        "~/Library/Application Support/Labtechno/SUBDESTRUCKT/ta_diagnostics.on .",
        "1. Ouvrez le projet à l'arrêt : silence des pistes 1 à 4 (Engineering › Diagnostics : « retiree »).",
        "2. PLAY : les états autonomes jouent (source « transport »). 3. RECORD (sur une piste vide) : aucune rupture.",
        "4. STOP : ils continuent (autorisation conservée). 5. Boucle et 6. Seek pendant la lecture : aucune relance, aucun saut.",
        "7. PANIC pendant PLAY : silence ; la lecture continue sans retour du son. 8. STOP puis PLAY : retour.",
        "9. Chargez un preset pendant PLAY sur la piste 5 : aucun son autonome. 10. Dupliquez la piste 2 pendant PLAY : "
        "la copie reste muette jusqu'à STOP puis PLAY.",
        "11. Insérez une nouvelle instance pendant PLAY : aucun son sans note. 12. Fermez et rouvrez l'éditeur : aucun changement.",
        "13. Enregistrez, fermez, rouvrez le projet : étapes 1-2 identiques. 14. Plusieurs instances autonomes : chacune "
        "suit le transport, indépendamment.",
    ] + extra + ["Joignez ~/Library/Application Support/Labtechno/SUBDESTRUCKT/ta_log.txt (événements de transport et "
                 "d'autorisation) et notez la version exacte de " + host + "."]
    return {
        "id": tid, "title": "TRANSPORT AUTHORIZATION dans " + host,
        "official": {"procedure": "Protocole en 14 étapes (projet de recette à 5 pistes), " + host + " sur Mac Apple Silicon",
                     "expected": "Silence à l'arrêt à l'ouverture ; autorisation au lancement de la lecture ou de l'enregistrement ; "
                                 "conservée à l'arrêt ; PANIC prioritaire jusqu'à STOP puis PLAY ; aucune activation implicite."},
        "why": "Valider dans l'hôte réel que les comportements autonomes reviennent sans réautoriser chaque instance, et jamais par surprise.",
        "prerequisites": ["SUBDESTRUCKT 0.3.2 compilé et installé.", host + " (version exacte à relever).", "Un clavier MIDI."],
        "skills": "Utiliser le transport, dupliquer une piste, charger un preset dans " + host + ".",
        "steps": steps,
        "expected": "Silence à l'arrêt à l'ouverture ; autorisation au lancement de la lecture ou de l'enregistrement ; conservée à "
                    "l'arrêt ; PANIC prioritaire jusqu'à STOP puis PLAY ; aucune activation implicite.",
        "pass": "Les 14 étapes conformes ; le journal montre les transitions attendues.",
        "fail": "Une étape non conforme (son inattendu, son absent après PLAY, PANIC contourné).",
        "onFail": "Noter l'étape, l'hôte, sa version et joindre ta_log.txt.",
        "evidenceHint": "ta_log.txt, description, capture de l'onglet Diagnostics.",
    }


t["tests"].append(ta_test("R-26", "Reason 14", []))
t["tests"].append(ta_test("R-27", "Ableton Live", []))
t["tests"].append({
    "id": "R-28", "title": "Exports audio avec états autonomes (Reason 14 et Ableton Live)",
    "official": {"procedure": "Exports sans lecture préalable, hors temps réel, bounce / rendu de piste / Freeze de piste, région décalée, comparaison lecture/export, annulation",
                 "expected": "Chaque export contient le Drone, les gels D2/S2 restitués et le Feedback ; lecture et export concordent selon le protocole ; aucune activation après annulation sans transport."},
    "why": "Un export peut démarrer dans une nouvelle instance, sans lecture ni éditeur : il doit quand même contenir la composition.",
    "prerequisites": ["SUBDESTRUCKT 0.3.2 compilé et installé.", "Reason 14 et Ableton Live (versions relevées).", "Projet de recette de R-26/R-27."],
    "skills": "Exporter l'audio d'un projet, d'une piste et d'une région ; comparer deux fichiers dans un éditeur audio.",
    "steps": [
        "15. Ouvrez le projet et exportez IMMÉDIATEMENT (sans lecture, sans Autoriser).",
        "16. Export hors temps réel (mode par défaut de l'hôte) ; puis en temps réel s'il est proposé.",
        "17. Bounce, rendu de piste et Freeze de piste (selon l'hôte).",
        "18. Export d'une région qui ne commence pas au début du projet.",
        "19. Comparez lecture enregistrée et export : présence et niveau des états autonomes (écart par bande, pas d'égalité "
        "bit à bit exigée : le gel S2 décroît et les phases dépendent du point de départ).",
        "20. Lancez un export, annulez-le, puis relancez la lecture : comportement normal (TA).",
        "Notez pour chaque export : hôte, mode, temps réel ou non, présence de chaque état autonome, et joignez ta_log.txt "
        "(l'événement « hors temps reel » et la source « rendu hors temps reel » attestent la détection).",
    ],
    "expected": "Chaque export contient le Drone, les gels D2/S2 restitués et le Feedback ; lecture et export concordent selon le protocole ; aucune activation après annulation sans transport.",
    "pass": "Tous les exports contiennent les états autonomes ; journal cohérent.",
    "fail": "Un export sans états autonomes, ou avec une activation inattendue.",
    "onFail": "Noter l'hôte, le mode d'export, joindre ta_log.txt et les fichiers exportés ; contournement : Autoriser avant l'export.",
    "evidenceHint": "Fichiers exportés, ta_log.txt, mesures comparatives.",
})
t["tests"].append({
    "id": "R-29", "title": "Écoute : transformations Z et D2 Regenerative",
    "official": {"procedure": "Exemples audio du chapitre 6 et du chapitre 10 ; puis réglages en direct",
                 "expected": "Chaque transformation Z a un effet distinct et progressif ; Regenerative rend le Feedback audible à Position 0 ; aucun clic à l'automation."},
    "why": "Juger musicalement les cinq nouvelles transformations et le nouveau mode D2.",
    "prerequisites": ["SUBDESTRUCKT 0.3.2 compilé et installé.", "Une DAW et un clavier MIDI."],
    "skills": "Régler un paramètre, automatiser un paramètre.",
    "steps": ["Écoutez les exemples e23 à e34.", "Sur WT, essayez chaque transformation avec Z Master, Depth et Shape ; automatisez Depth et Shape.",
              "Notez en particulier : niveau de Harmonic Tilt brillant avec et sans Gain Compensation ; progressivité de Harmonic Shift sur une banque complexe ; intérêt de Phase Scatter avec Fold, N1 et le CROSS.",
              "D2 : Position 0, Feedback 0,5 à 1, Legacy puis Regenerative."],
    "expected": "Chaque transformation Z a un effet distinct et progressif ; Regenerative rend le Feedback audible à Position 0 ; aucun clic à l'automation.",
    "pass": "Effets conformes, utilisables musicalement.",
    "fail": "Effet inaudible, brutal, ou niveau dangereux.",
    "onFail": "Noter le réglage exact (banque, X/Y, Z, Shape) et le comportement.",
    "evidenceHint": "Description, enregistrement court.",
})
for g in t["groups"]:
    if g["id"] == "technique":
        g["tests"] += ["R-26", "R-27", "R-28"]
    if g["id"] == "musical":
        g["tests"].append("R-29")
tp.write_text(json.dumps(t, ensure_ascii=False, indent=1) + "\n", encoding="utf-8", newline="\n")
print("make_manual_032 : édition source 0.3.2 écrite (%d tests de recette)" % len(t["tests"]))
