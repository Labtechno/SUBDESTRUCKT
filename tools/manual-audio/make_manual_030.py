#!/usr/bin/env python3
"""SUBDESTRUCKT — contenu éditorial du manuel 0.3.0 (ARCH-G4.1).
Reprend le générateur 0.2.0 (make_manual_020.py) et n'en modifie que ce qui a changé dans la build ARCH-G4.1 :
chaque substitution est vérifiée (échec si le texte d'origine a disparu). Ne documente que ce qui existe ; les
limitations restent explicites. Exécuter depuis la racine du dépôt, après le rendu audio (render_examples_g4)."""
import pathlib

SRC = pathlib.Path(__file__).with_name("make_manual_020.py").read_text(encoding="utf-8")
subs = []


def sub(old, new, count=1):
    subs.append((old, new, count))


# ---------------------------------------------------------------- cadre
sub('D = "manual/src/0.2.0"', 'D = "manual/src/0.3.0"')
sub("Bienvenue dans SUBDESTRUCKT 0.2.0", "Bienvenue dans SUBDESTRUCKT 0.3.0")
sub("Cette édition décrit la build de développement intégré ARCH-G4.", "Cette édition décrit la build de développement intégré ARCH-G4.1.")
sub("La diffusion publique du binaire est <strong>bloquée</strong> (licence JUCE non arbitrée, pas de signature Developer ID ni de notarisation).</li>",
    "Aucun binaire n'est distribué : la licence JUCE n'est pas arbitrée et il n'y a ni signature Developer ID ni notarisation. "
    "La recette se fait en <strong>compilant vous-même</strong> le plugin sur votre Mac (chapitre 2).</li>")
sub("<h2>Ce qui est nouveau depuis 0.1.0-b5</h2><ul>",
    "<h2>Ce qui est nouveau dans 0.3.0</h2><ul>"
    "<li><strong>Balance WT/FM</strong> en Performance : de la wavetable seule au FM seul, sans toucher aux modulations ni au CROSS.</li>"
    "<li><strong>Morph spectral</strong> entre formes d'onde (en plus de Linear et Stepped).</li>"
    "<li><strong>Éditeur de formes</strong> (dessin), <strong>générateur procédural</strong> et <strong>bibliothèque</strong> de banques (.sdwt).</li>"
    "<li><strong>Enveloppes</strong> : déclenchement Voice / Note / Gate et synchronisation au tempo.</li>"
    "<li><strong>Captures</strong> du Glitch D2 et du Freeze S2, enregistrées avec le preset ou le projet.</li>"
    "<li><strong>N1</strong> : suréchantillonnage interne selon la Quality (repliement fortement réduit en Normal et High).</li>"
    "<li><strong>Protection d'extrême niveau</strong> (proposition SPEC-HR-01) au-delà de +24 dBFS, signalée par « ELP ».</li>"
    "<li>Corrections : temps d'attaque/relâchement des suiveurs d'enveloppe, modulation d'Output Gain et du plafond du limiteur.</li></ul>"
    "<h2>Rappel : nouveautés de 0.2.0 par rapport à 0.1.0-b5</h2><ul>")
sub("<p>Les projets enregistrés avec 0.1.0-b5 s'ouvrent dans 0.2.0 et sonnent à l'identique (vérifié bit à bit par le test REG-01). L'inverse n'est pas vrai : 0.1.0-b5 refuse un état enregistré par 0.2.0.</p>",
    "<p>Les projets 0.1.0-b5 s'ouvrent dans 0.3.0 et sonnent à l'identique (vérifié bit à bit, REG-01). Les projets 0.2.0 s'ouvrent ; "
    "les nouveaux réglages prennent des valeurs neutres. <strong>Trois différences audibles sont volontaires</strong> : N1 est suréchantillonné en "
    "Quality Normal et High (choisissez Eco pour retrouver le N1 de 0.2.0) ; les suiveurs d'enveloppe appliquent désormais leurs temps ; "
    "une modulation d'Output Gain ou du plafond du limiteur agit désormais. Les versions antérieures refusent un état 0.3.0.</p>")

# ---------------------------------------------------------------- installation : compilation locale
sub('ch(2, "ch-02", "Installation et désinstallation",\n   "SUBDESTRUCKT s\'installe avec un paquet d\'installation et se retire avec un paquet de désinstallation. Aucun autre plugin, projet ou preset n\'est touché.",\n   box("warn", "Distribution", "<p>Cette build n\'est pas publiée. L\'archive vous est remise par un canal privé décidé par la direction technique, avec son empreinte SHA-256. Ne l\'installez pas si l\'empreinte ne correspond pas.</p>")',
    'ch(2, "ch-02", "Obtenir, installer et désinstaller",\n   "Pour cette build, vous compilez SUBDESTRUCKT sur votre Mac : aucun binaire n\'est distribué. Le script fourni fait tout, gratuitement.",\n   box("warn", "Licence JUCE : votre choix", "<p>SUBDESTRUCKT utilise JUCE, disponible sous deux licences : AGPLv3, ou la licence JUCE 9 (niveau gratuit Starter sous un plafond de revenus). Le script ne choisit rien à votre place : il vous demande d\'indiquer la licence que <strong>vous</strong> avez retenue (étude : <code>docs/V1-ARCH-G4.1-LICENCE.md</code>).</p>")'
    ' + "<h2>Compiler sur votre Mac</h2><ol class=\\"steps\\"><li>Installez les outils gratuits : <code>xcode-select --install</code>, puis <code>brew install cmake ninja</code>.</li>'
    '<li>Récupérez les sources au commit indiqué par la direction technique.</li>'
    '<li>Lancez <code>tools/local-build/build_local_macos.sh --juce-licence=&lt;votre choix&gt; --install --pluginval</code>.</li>'
    '<li>Le script vérifie JUCE (SHA), compile, exécute les tests, signe le plugin localement et l\'installe dans <code>~/Library/Audio/Plug-Ins/VST3</code>. Le rapport est dans <code>local-build/out</code> (mesures de performance de <strong>votre</strong> Mac comprises).</li></ol>'
    '<p>Avec <code>--packages</code>, le script produit aussi, en local, les paquets Installer/Uninstaller et l\'archive décrits ci-dessous.</p>"')
sub("<h2>Mettre à jour depuis 0.1.0-b5</h2><p>Installez par-dessus : la version précédente est détectée, sauvegardée pendant l'opération et restaurée en cas d'échec.</p>",
    "<h2>Mettre à jour depuis 0.2.0</h2><p>Installez par-dessus : la version précédente est détectée, sauvegardée pendant l'opération et restaurée en cas d'échec. Avec <code>--install</code>, le bundle de votre dossier utilisateur est simplement remplacé.</p>")
sub("Vos presets (<code>~/Library/Application Support/Labtechno/SUBDESTRUCKT/Presets</code>) et votre MIDI Learn sont conservés.</p>",
    "Vos presets (<code>~/Library/Application Support/Labtechno/SUBDESTRUCKT/Presets</code>), vos banques (<code>…/Wavetables</code>) et votre MIDI Learn sont conservés. "
    "Installation par le script (<code>--install</code>) : supprimez <code>~/Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3</code>.</p>")

# ---------------------------------------------------------------- interface
sub("le mètre de sortie (en rouge au-delà de 0 dBFS)", "le mètre de sortie (en rouge au-delà de 0 dBFS ; « ELP n » si la protection d'extrême niveau s'est engagée)")
sub("<p>Le noyau jouable : X, Y, Z, CROSS Master, Output Gain,", "<p>Le noyau jouable : X, Y, Z, CROSS Master, <strong>WT/FM Balance</strong>, Output Gain,")
sub("Chaque chaîne n'affiche que les réglages de l'algorithme choisi, dans l'ordre réel (filtre avant ou après la destruction).</p>",
    "Chaque chaîne n'affiche que les réglages de l'algorithme choisi, dans l'ordre réel (filtre avant ou après la destruction). "
    "Le bouton <strong>Capturer gels</strong> (au-dessus du CROSS) enregistre le contenu des Glitch D2 et Freeze S2 actifs (chapitre 10).</p>")
sub("<p>Les outils de précision : réseaux FM/CROSS, matrice de modulation, banques, Randomizer, état et presets, diagnostics.</p>",
    "<p>Les outils de précision : réseaux FM/CROSS, matrice de modulation, banques (avec l'éditeur de formes), Randomizer, état et presets, diagnostics.</p>"
    '" + fig("assets/ui_eng_banks.png", "Engineering Banques", "Engineering › Banques : éditeur de formes, générateur, bibliothèque. " + CAPNOTE) + "')

# ---------------------------------------------------------------- wavetable
sub("Le passage entre formes est progressif (Linear) ou par paliers (Stepped).</p>",
    "Le passage entre formes est progressif (Linear), par paliers (Stepped) ou <strong>spectral</strong> (Spectral : les harmoniques glissent d'une forme à l'autre au lieu d'un fondu des ondes ; deux formes opposées ne s'annulent plus au milieu).</p>"
    "{{AB:e18_morph_linear|e19_morph_spectral|Balayage de X : Linear puis Spectral (banque Metal Spectra)}}")
sub("La banque importée est enregistrée dans votre projet et dans vos presets.</p>",
    "La banque importée est enregistrée dans votre projet et dans vos presets.</p>"
    "<h2>Créer vos banques</h2><p>Engineering › Banques réunit trois outils. L'<strong>éditeur</strong> : choisissez une forme, dessinez-la à la souris, puis « Appliquer la forme ». "
    "Le <strong>générateur</strong> crée une banque complète (Harmonic Tilt, Pulse Width, Random Spectra, Formant Sweep, Fold Sweep ; grille 16×1, 16×4 ou 64×16 ; graine ; intensité). "
    "La <strong>bibliothèque</strong> enregistre la banque active en fichier <code>.sdwt</code> et recharge vos fichiers. Chaque opération crée une nouvelle banque et s'annule par Undo ; la banque jouée n'est jamais modifiée en place.</p>")
sub("<p><strong>Clean</strong> lit des versions limitées en bande",
    "<p>Morph spectral : le spectre de chaque forme est préparé au chargement de la banque ; la forme intermédiaire est recalculée toutes les 128 échantillons, avec un fondu, limitée en bande. Une banque <code>.sdwt</code> contient la grille, les données et leur empreinte SHA-256 : un fichier altéré est refusé.</p><p><strong>Clean</strong> lit des versions limitées en bande")

# ---------------------------------------------------------------- destruction
sub("<li><strong>D2 Glitch Buffer</strong> : boucles de fragments, lecture inversée, Freeze.</li>",
    "<li><strong>D2 Glitch Buffer</strong> : boucles de fragments, lecture inversée, Freeze. Le Feedback n'est audible que si Position est supérieure à 0.</li>")
sub("<p>Les gains de sortie s'additionnent : avec plusieurs processeurs poussés à +24 dB, la garde numérique (+30 dBFS) peut s'engager. Gardez une marge, ou activez le limiteur musical (onglet VOIX).</p>",
    "<p>Les gains de sortie s'additionnent. Au-delà de +24 dBFS, la protection d'extrême niveau (ELP, chapitre 12) sature doucement le signal et l'affiche ; en dessous, elle ne fait rien. Gardez une marge, ou activez le limiteur musical (onglet VOIX).</p>"
    "<h2>Captures Glitch et Freeze</h2><p>Un gel D2 ou S2 dépend de ce que vous avez joué. <strong>Capturer gels</strong> enregistre son contenu avec le preset ou le projet ; à l'ouverture, il est restauré dès que l'emplacement exécute D2 ou S2. "
    "Un projet enregistré pendant un gel armé capture automatiquement ce gel. La lecture reprend au début du fragment : la position exacte de lecture n'est pas restaurée.</p>")
sub('adv("Approfondir : repliement de N1", "<p>N1 utilise une méthode anti-repliement (ADAA). Avec un repli extrême, le repliement reste élevé ; Quality High double le suréchantillonnage du moteur et le réduit (mesures dans le rapport ARCH-G4).</p>")',
    'adv("Approfondir : repliement de N1", "<p>N1 combine une méthode anti-repliement (ADAA) et un suréchantillonnage interne : Eco ×1 (N1 de 0.2.0), Normal ×2, High ×4. Mesure (3,1 kHz, drive 24, repli 1) : énergie repliée −2,5 dB en Eco, −38,5 dB en Normal, −54,1 dB en High. La latence ajoutée (27 ou 41 échantillons) est annoncée à la DAW et le signal sec est aligné.</p>") + "{{AB:e20_n1_eco|e21_n1_high|N1 extrême sur un sinus aigu : Quality Eco puis High}}"')

# ---------------------------------------------------------------- modulation
sub('box("warn", "Limitation", "<p>Les enveloppes ne se synchronisent pas encore au tempo : la règle n\'est pas encore arbitrée.</p>"))',
    'box("fact", "Déclenchement et tempo (règles provisoires)", "<p>Chaque enveloppe a un <strong>Trigger</strong> : <strong>Voice</strong> (défaut, règle de la voix : relance en Mono, pas en Legato), <strong>Note</strong> (relance à chaque nouvelle note, même en Legato), <strong>Gate</strong> (relance seulement quand aucune note n\'était tenue). La relance part du niveau courant : pas de saut. '
    '<strong>Tempo Sync</strong> lit Delay, Attack, Hold, Decay et Release en noires (1 = une noire) au tempo de la DAW (120 bpm sans transport) ; un changement de tempo est suivi au bloc suivant. Ces règles sont provisoires (non arbitrées).</p>")'
    ' + "<p>Les suiveurs d\'enveloppe appliquent leurs temps Attack/Release (corrigé en 0.3.0). Un suiveur ne peut pas moduler un réglage situé avant le point qu\'il mesure (la liaison reste affichée, inactive, avec sa raison).</p>")')

# ---------------------------------------------------------------- sortie, quality
sub("<p><strong>Quality</strong> (préférence du projet, pas du preset) : Normal reproduit 0.1.0-b5 ; High double le suréchantillonnage (plus de CPU, plus de latence à 44,1 kHz) ; Eco est identique à Normal pour l'oscillation. La latence annoncée à la DAW dépend aussi de S2.</p>",
    "<p><strong>Quality</strong> (préférence du projet, pas du preset) : Normal reproduit l'oscillation de 0.1.0-b5 ; High double le suréchantillonnage (plus de CPU, plus de latence à 44,1 kHz) ; Eco est identique à Normal pour l'oscillation mais garde N1 sans suréchantillonnage interne (moins de CPU, plus de repliement). La latence annoncée à la DAW dépend aussi de S2 et de N1.</p>"
    "<p><strong>Protection d'extrême niveau (ELP)</strong> : au-delà de +24 dBFS, une saturation douce ramène le signal vers +30 dBFS au plus ; en dessous, elle n'agit pas du tout. Chaque engagement est compté (Diagnostics) et signalé par « ELP » à côté du mètre. C'est une <em>proposition</em> (SPEC-HR-01) en attente d'arbitrage ; elle ne remplace pas le limiteur musical.</p>")

# ---------------------------------------------------------------- presets
sub("Un preset ne contient ni la Quality ni le Drone.</p>",
    "Un preset ne contient ni la Quality ni le Drone. <strong>Un preset ne peut jamais armer un comportement autonome</strong> : au chargement, les cases Autonomous Armed sont décochées (la barre le signale) ; réarmez-les vous-même. Un projet DAW, lui, est restauré tel que vous l'avez enregistré. Les captures D2/S2 et une banque utilisateur sont enregistrées dans le preset.</p>")

# ---------------------------------------------------------------- diagnostics, audio, recette, limitations
sub("Les compteurs de sûreté (non-finis, garde +30 dBFS, borne PM, limiteur, Panic) restent normalement à zéro.</p>",
    "Les compteurs de sûreté (non-finis, garde +30 dBFS, borne PM, limiteur, protection +24 dBFS, Panic) restent normalement à zéro.</p>")
sub("Dix exercices progressifs, réalisables avec les commandes de cette build. Chacun part de Init.", "Treize exercices progressifs, réalisables avec les commandes de cette build. Chacun part de Init.")
sub("<li><strong>Format :</strong> WAV 24 bits, mono (stéréo mixée), 48 kHz, Quality Normal.</li>",
    "<li><strong>Format :</strong> WAV 24 bits, mono (stéréo mixée), 48 kHz, Quality Normal (sauf indication : la paire N1 compare Eco et High).</li>")
sub("<code>SUBDESTRUCKT_ARCH_G4_RESULTS.json</code>", "<code>SUBDESTRUCKT_ARCH_G41_RESULTS.json</code>")
sub("<li><strong>Le plugin n'apparaît pas :</strong> relancez l'analyse des plugins VST3 de la DAW ; vérifiez la présence de <code>/Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3</code>.</li>",
    "<li><strong>Le plugin n'apparaît pas :</strong> relancez l'analyse des plugins VST3 de la DAW ; vérifiez la présence de <code>SUBDESTRUCKT.vst3</code> dans <code>/Library/Audio/Plug-Ins/VST3</code> (paquet) ou <code>~/Library/Audio/Plug-Ins/VST3</code> (script).</li>"
    "<li><strong>La compilation échoue :</strong> lisez <code>local-build/out/build.log</code> ; vérifiez Xcode Command Line Tools, cmake et ninja.</li>"
    "<li><strong>« ELP » s'affiche :</strong> des gains cumulés dépassent +24 dBFS ; baissez les gains de sortie des processeurs ou Output Gain.</li>")
sub("<li><strong>Son qui persiste :</strong> un emplacement armé (Autonomous Armed) ou le Drone ; décochez-les ou PANIC.</li>",
    "<li><strong>Son qui persiste :</strong> un emplacement armé (Autonomous Armed) ou le Drone ; décochez-les ou PANIC.</li>"
    "<li><strong>Un preset ne gèle plus comme avant :</strong> les armements d'un preset sont volontairement ignorés ; réarmez l'emplacement.</li>")
sub("<li><strong>Projet 0.2.0 ouvert dans 0.1.0-b5 :</strong> l'état est refusé (format plus récent) ; gardez une copie avant tout retour arrière.</li>",
    "<li><strong>Projet 0.3.0 ouvert dans une version antérieure :</strong> l'état est refusé (format plus récent) ; gardez une copie avant tout retour arrière.</li>")
sub("<li>Cinq transformations Z historiques non implémentées (catalogue non retrouvé).</li><li>Morph spectral entre formes, éditeur graphique de formes et générateur procédural dans l'interface : absents.</li>\"\n"
    "   \"<li>Enveloppes non synchronisables au tempo ; balance WT/FM de façade non exposée (décisions non arbitrées).</li>\"\n"
    "   \"<li>Repliement élevé de N1 en repli extrême (Quality High recommandé) ; garde +30 dBFS possible avec des gains cumulés extrêmes.</li>\"",
    "<li>Cinq transformations Z historiques non implémentées (catalogue non retrouvé).</li>"
    "<li>Règles de déclenchement et de synchronisation des enveloppes, protection ELP : provisoires (non arbitrées).</li>\"\n"
    "   \"<li>Captures D2/S2 : la position de lecture et le feedback en cours ne sont pas restaurés (pas de continuation exacte). D2 Feedback inaudible avec Position 0.</li>\"\n"
    "   \"<li>Repliement de N1 en Quality Eco identique à 0.2.0 (Normal ou High recommandés).</li>\"")
sub("Performances non certifiées sur M3/M4 ; graphisme provisoire",
    "Performances non certifiées sur M3/M4 (le script de compilation mesure votre machine) ; graphisme provisoire")

# ---------------------------------------------------------------- glossaire
sub('    ("stableid", "StableID",',
    '    ("morph-spectral", "Morph spectral", "Passage d\'une forme à l\'autre par ses harmoniques.", "Les amplitudes des harmoniques sont interpolées et leurs phases moyennées ; deux formes opposées ne s\'annulent pas au milieu, contrairement au fondu linéaire.", "ch-06"),\n'
    '    ("sdwt", "Fichier .sdwt", "Fichier de banque de formes portable.", "Texte versionné : nom, grille, données et empreinte SHA-256 ; un fichier altéré est refusé.", "ch-06"),\n'
    '    ("capture", "Capture (D2/S2)", "Enregistrement du contenu d\'un gel Glitch ou Freeze.", "Sauvegardée avec le preset ou le projet et vérifiée par empreinte ; la lecture reprend au début du fragment.", "ch-10"),\n'
    '    ("elp", "ELP (protection d\'extrême niveau)", "Saturation douce au-delà de +24 dBFS.", "Proposition SPEC-HR-01 : inactive en dessous de +24 dBFS, asymptote +30 dBFS, chaque engagement compté.", "ch-12"),\n'
    '    ("trigger", "Trigger d\'enveloppe", "Ce qui relance une enveloppe : Voice, Note ou Gate.", "Voice suit la voix (Mono/Legato), Note relance à chaque note, Gate seulement à l\'ouverture du gate.", "ch-11"),\n'
    '    ("stableid", "StableID",')
sub('t["long"] = t["long"].replace("0.1.0-b5", "0.2.0")', 't["long"] = t["long"].replace("0.1.0-b5", "0.3.0")')

# ---------------------------------------------------------------- exercices
sub('       "Le son évolue lentement.", "Croire que le preset active le Drone : jamais.", "Ajoutez un LFO lent sur WT Y."),\n]',
    '       "Le son évolue lentement.", "Croire que le preset active le Drone : jamais.", "Ajoutez un LFO lent sur WT Y."),\n'
    '    ex(11, "Morph spectral", "Intermédiaire", "10 min", "Entendre la différence entre fondu et morph spectral.", ["morph spectral", "X"], "Init, OP1 Level 0.",\n'
    '       ["Banque Metal Spectra.", "WT X Morph : Linear ; balayez X.", "WT X Morph : Spectral ; balayez X."], "Metal Spectra · X 0 → 1", "En Spectral, les harmoniques glissent ; en Linear, deux spectres se superposent.",\n'
    '       "Écoutez le milieu de chaque transition.", "Banque à une seule forme : aucun morph possible.", "Spectral sur Y avec Harmonic Series.", ab=["e18_morph_linear", "e19_morph_spectral"]),\n'
    '    ex(12, "Dessiner une forme", "Débutant", "10 min", "Créer votre propre forme d\'onde.", ["éditeur", "banque", "Undo"], "Init, OP1 Level 0.",\n'
    '       ["Engineering › Banques : générateur Harmonic Tilt 16×4, Générer.", "Choisissez la forme 5, dessinez une rampe en escalier, Appliquer la forme.", "Jouez en balayant X, puis Undo."],\n'
    '       "Forme 5 dessinée", "Votre forme apparaît au passage de X ; Undo revient à la banque générée.", "La forme dessinée sonne plus brillante si elle a des angles vifs.",\n'
    '       "Dessiner sans appliquer : rien ne change.", "Enregistrez la banque (.sdwt) puis rechargez-la."),\n'
    '    ex(13, "Capturer un gel", "Avancé", "15 min", "Conserver un fragment Glitch dans un preset.", ["D2", "Freeze", "capture", "armement"], "Init.",\n'
    '       ["PROCESS › Global : Destruction 1 = D2, Mix 1, Autonomous Armed.", "Jouez, puis cochez Freeze pendant la note.", "Capturer gels ; sauvegardez un preset.", "Rechargez le preset, réarmez, jouez."],\n'
    '       "D2 · Freeze · capture", "Le fragment capturé revient sans rejouer la phrase d\'origine.", "Le gel recommence au début du fragment.",\n'
    '       "Oublier de réarmer : un preset n\'arme jamais.", "Même chose avec S2 Spectral Freeze."),\n]')

# ---------------------------------------------------------------- recette
sub('"prerequisites": prereq or ["SUBDESTRUCKT 0.2.0 installé (sauf R-01 et R-02).", "Une DAW et un clavier MIDI."]',
    '"prerequisites": prereq or ["SUBDESTRUCKT 0.3.0 compilé et installé (sauf R-01 et R-02).", "Une DAW et un clavier MIDI."]')
sub('    rt(1, "Intégrité de l\'archive", "shasum -a 256 du ZIP ; shasum -a 256 -c SHA256SUMS.txt", "Empreinte identique à celle communiquée ; 4 lignes OK.",\n'
    '       "Vérifier que l\'archive est exactement celle construite et testée.", ["Calculez l\'empreinte du ZIP et comparez-la à celle communiquée.", "Dézippez, puis shasum -a 256 -c SHA256SUMS.txt."],\n'
    '       "Empreinte identique ET 4 lignes OK.", "Empreinte différente ou ligne FAILED.", "Ne pas installer ; signaler les deux empreintes.", "Texte des deux commandes.",\n'
    '       ["Le ZIP reçu par le canal privé et son empreinte.", "Terminal."], "Coller une commande dans le Terminal."),',
    '    rt(1, "Compilation locale", "tools/local-build/build_local_macos.sh --juce-licence=<choix> --pluginval", "Compilation réussie ; tests : 0 FAIL ; pluginval rigueur 10 PASS ; commit identique à celui communiqué.",\n'
    '       "Vérifier que le plugin construit sur votre Mac est celui du commit testé.", ["git rev-parse HEAD : comparez au commit communiqué.", "Lancez le script avec votre choix de licence et --pluginval.", "Lisez le résumé (critères PASS/FAIL) et local-build/out/machine.txt."],\n'
    '       "Commit identique, 0 FAIL, pluginval PASS.", "Erreur de compilation, FAIL, pluginval FAIL.", "Joindre local-build/out (journaux).", "Fichiers results.txt et machine.txt.",\n'
    '       ["Mac Apple Silicon, macOS 14+, Xcode Command Line Tools, cmake, ninja.", "Sources au commit communiqué.", "Terminal."], "Coller une commande dans le Terminal."),')
sub('"Installation réussie ; bundle présent ; version 0.2.0."', '"Installation réussie ; bundle présent ; version 0.3.0."')
sub('    rt(4, "Projet 0.1.0-b5", "Ouvrir un projet enregistré avec 0.1.0-b5", "Le son est identique ; aucun message d\'erreur.", "Vérifier la migration d\'état.",\n'
    '       ["Ouvrez un projet 0.1.0-b5 existant.", "Écoutez et comparez à votre souvenir ou à un rendu antérieur."]',
    '    rt(4, "Projets 0.1.0-b5 et 0.2.0", "Ouvrir un projet enregistré avec 0.1.0-b5 puis un projet 0.2.0 sans N1", "Le son est identique ; aucun message d\'erreur.", "Vérifier la migration d\'état.",\n'
    '       ["Ouvrez un projet 0.1.0-b5 existant.", "Ouvrez un projet 0.2.0 qui n\'utilise pas N1 (N1 change volontairement en Normal/High).", "Écoutez et comparez à un rendu antérieur."]')
sub('"Plugin de nouveau disponible ; projets rechargés.", "Vérifier la réinstallation.",\n'
    '       ["Réinstallez.", "Rouvrez le projet de R-13."], "Fonctionne.", "Échec.", "Noter le message.", "Capture."),\n]',
    '"Plugin de nouveau disponible ; projets rechargés.", "Vérifier la réinstallation.",\n'
    '       ["Réinstallez.", "Rouvrez le projet de R-13."], "Fonctionne.", "Échec.", "Noter le message.", "Capture."),\n'
    '    rt(19, "Balance WT/FM", "Performance : WT/FM Balance de -1 à +1", "-1 : WT seule ; 0 : son inchangé ; +1 : FM seul ; jamais plus fort qu\'au centre ; CROSS toujours actif.", "Vérifier la balance audio.",\n'
    '       ["Chargez Cross Metal.", "Balance à -1, 0, +1 en jouant.", "À +1, tournez CROSS Master : le FM reste modulé par la WT."], "Conforme.", "Saut de niveau, CROSS coupé, son plus fort qu\'au centre.", "Noter la position.", "Description."),\n'
    '    rt(20, "Enveloppes : Trigger et Sync", "MOD : ENV AUX1 vers WT X ; Trigger Voice/Note/Gate ; Tempo Sync", "Legato + Note : relance à chaque note ; Gate : relance seulement après relâchement total ; Sync suit le tempo.", "Vérifier les nouvelles règles.",\n'
    '       ["Voice Mode Legato, route ENV AUX1 → WT X.", "Trigger Voice puis Note : jouez legato.", "Trigger Gate.", "Tempo Sync, Attack 1 : changez le tempo de la DAW (60 puis 120)."], "Conforme, sans saut de niveau.", "Relance incorrecte, saut, tempo ignoré.", "Noter le réglage.", "Description."),\n'
    '    rt(21, "Captures D2/S2", "Capturer gels ; enregistrer le preset et le projet ; recharger", "Le fragment capturé est rejoué après rechargement ; le preset arrive désarmé.", "Vérifier les captures R2.",\n'
    '       ["Suivez l\'exercice 13.", "Enregistrez aussi le projet pendant le gel armé ; fermez et rouvrez."], "Fragment restauré ; preset désarmé ; projet restauré armé.", "Fragment perdu, preset armé.", "Noter l\'étape.", "Description."),\n'
    '    rt(22, "Éditeur et bibliothèque de banques", "Engineering › Banques", "Génération, dessin, Undo, enregistrement .sdwt et rechargement fonctionnent.", "Vérifier les banques utilisateur.",\n'
    '       ["Générez Random Spectra 16×4.", "Dessinez une forme, Appliquer, Undo, Redo.", "Enregistrez (.sdwt), chargez une autre banque, rechargez la vôtre.", "Enregistrez le projet, rouvrez-le sur une autre machine si possible."], "Conforme.", "Échec ou banque différente.", "Noter l\'étape.", "Fichier .sdwt."),\n'
    '    rt(23, "Morph spectral", "WT X Morph Spectral ; balayage de X", "Transition harmonique audible, sans clic.", "Vérifier le morph spectral.",\n'
    '       ["Banque Metal Spectra, OP1 Level 0.", "Comparez Linear et Spectral en balayant X (automation lente)."], "Différence audible, sans clic.", "Clic, silence.", "Noter la banque.", "Description."),\n'
    '    rt(24, "N1, Quality et ELP", "N1 drive 24 / repli 1, note aiguë ; Quality Eco/Normal/High ; gains extrêmes", "Repliement réduit en Normal et High ; latence compensée ; ELP signalé au-delà de +24 dBFS.", "Vérifier N1 et la protection.",\n'
    '       ["Global 1 : N1 drive 24, Fold 1 ; jouez une note aiguë.", "Comparez Eco, Normal, High.", "Ajoutez D1 et N2 avec Out +24 dB : observez « ELP » et Diagnostics."], "Conforme.", "Aucune différence, désalignement, son abîmé sans signalement.", "Noter la Quality.", "Description."),\n]')
sub('{"id": "musical", "title": "Validation musicale", "tests": ["R-05", "R-06", "R-07", "R-08", "R-09", "R-10", "R-11"]}',
    '{"id": "musical", "title": "Validation musicale", "tests": ["R-05", "R-06", "R-07", "R-08", "R-09", "R-10", "R-11", "R-19", "R-20", "R-21", "R-22", "R-23"]}')
sub('{"id": "technique", "title": "Validation technique", "tests": ["R-12", "R-13", "R-14", "R-15", "R-16"]}',
    '{"id": "technique", "title": "Validation technique", "tests": ["R-12", "R-13", "R-14", "R-15", "R-16", "R-24"]}')
sub('"campaignId": "SUBDESTRUCKT-ARCH-G4-RECETTE-PHYSIQUE", "source": "manual/src/0.2.0/recette/tests.json"',
    '"campaignId": "SUBDESTRUCKT-ARCH-G41-RECETTE-PHYSIQUE", "source": "manual/src/0.3.0/recette/tests.json"')
sub('{"note": "0.2.0 : tableau genere depuis source/core/ParamTable.inc"}', '{"note": "0.3.0 : tableau genere depuis source/core/ParamTable.inc"}')

out = SRC
for old, new, count in subs:
    n = out.count(old)
    if n < count:
        raise SystemExit(f"substitution introuvable : {old[:120]!r}")
    out = out.replace(old, new, count)
exec(compile(out, "make_manual_030 (derive de make_manual_020)", "exec"))
