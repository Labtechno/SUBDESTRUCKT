#!/usr/bin/env python3
"""SUBDESTRUCKT — contenu éditorial du manuel 0.2.0 (chapitres, glossaire, exercices, recette).
Ne documente que ce qui existe dans la build ARCH-G4 ; les limitations sont explicites. Exécuter depuis la racine."""
import json
import os
import shutil

D = "manual/src/0.2.0"
os.makedirs(D + "/chapters", exist_ok=True)


def ch(n, cid, title, lead, body):
    html = (f'<section class="chapter" id="{cid}">\n<header><p class="kicker">Chapitre {n}</p><h1>{title}</h1>\n'
            f'<p class="lead">{lead}</p></header>\n{body}\n</section>\n')
    slug = cid.replace("ch-", "")
    open(f"{D}/chapters/{n:02d}-{slug}.html", "w", encoding="utf-8", newline="\n").write(html)


def lvl(text):  # trois niveaux de lecture : découvrir (texte), comprendre (encadré), approfondir (details)
    return text


def adv(summary, body):
    return f'<details class="adv"><summary>{summary}</summary>{body}</details>'


def box(kind, tag, body):
    return f'<div class="box {kind}"><span class="tag">{tag}</span>{body}</div>'


def fig(src, alt, cap, label="Capture réelle", cls="meas"):
    return f'<figure><img class="zoomable" src="{src}" alt="{alt}"><figcaption><span class="figlabel {cls}">{label}</span>{cap}</figcaption></figure>'


CAPNOTE = ("Capture produite par le test automatisé de l'éditeur (rendu de développement Windows du même code d'interface ; "
           "sous macOS la typographie système diffère).")

ch(1, "ch-01", "Bienvenue dans SUBDESTRUCKT 0.2.0",
   "SUBDESTRUCKT est un instrument construit autour de l'interaction entre deux moteurs : une wavetable et un moteur FM. Cette édition décrit la build de développement intégré ARCH-G4.",
   box("proto", "Statut de cette build",
       "<ul><li>Build <strong>interne</strong> : ni beta, ni version publique. La diffusion publique du binaire est <strong>bloquée</strong> (licence JUCE non arbitrée, pas de signature Developer ID ni de notarisation).</li>"
       "<li>Les fonctions décrites ont été testées automatiquement (tests du cœur DSP, du wrapper VST3, hôte VST3 et pluginval sur macOS arm64 virtualisé). La recette sur un vrai Mac reste à faire (chapitre 17).</li>"
       "<li>Le graphisme de l'interface est <strong>provisoire</strong> : il sera finalisé après les tests d'ergonomie prévus.</li></ul>")
   + "<h2>Ce qui est nouveau depuis 0.1.0-b5</h2><ul>"
     "<li>Une wavetable complète : banques de formes en grille X×Y (jusqu'à 64×16), transformations Z, Character, synchronisation, import de vos propres formes (WAV).</li>"
     "<li>Quatre opérateurs FM (au lieu de deux) avec quatre modes de modulation : PM, Linear FM, Through-Zero FM, Exponential FM.</li>"
     "<li>Le CROSS complet : 55 liaisons possibles entre la wavetable, les opérateurs et un bus FM, plus le Cross Audio (Mix, Crossfade, Ring, AM).</li>"
     "<li>Trois filtres/résonateurs et six processeurs de destruction, avant ou après le filtre.</li>"
     "<li>Cinq enveloppes, quatre LFO, sources aléatoires et chaos, une matrice de 64 modulations, quatre macros.</li>"
     "<li>Presets, Undo/Redo, Randomize/Mutate/Evolve, Quality, Drone, limiteur musical, MIDI Learn.</li>"
     "<li>Une interface en trois espaces : Performance, Sound Design, Engineering.</li></ul>"
     "<h2>Comment lire ce manuel</h2><p>Chaque chapitre se lit à trois niveaux : le texte principal pour <strong>découvrir</strong>, les encadrés pour <strong>comprendre</strong>, les volets « Approfondir » pour les détails techniques. Les sons sont des rendus du moteur réel (chapitre 16). Les termes techniques renvoient au glossaire (chapitre 18).</p>"
     + box("fact", "Compatibilité", "<p>Les projets enregistrés avec 0.1.0-b5 s'ouvrent dans 0.2.0 et sonnent à l'identique (vérifié bit à bit par le test REG-01). L'inverse n'est pas vrai : 0.1.0-b5 refuse un état enregistré par 0.2.0.</p>"))

ch(2, "ch-02", "Installation et désinstallation",
   "SUBDESTRUCKT s'installe avec un paquet d'installation et se retire avec un paquet de désinstallation. Aucun autre plugin, projet ou preset n'est touché.",
   box("warn", "Distribution", "<p>Cette build n'est pas publiée. L'archive vous est remise par un canal privé décidé par la direction technique, avec son empreinte SHA-256. Ne l'installez pas si l'empreinte ne correspond pas.</p>")
   + "<h2>Contenu de l'archive</h2><ul><li><code>SUBDESTRUCKT_Installer.pkg</code> et <code>SUBDESTRUCKT_Uninstaller.pkg</code></li><li><code>README_FR.txt</code>, <code>CHANGELOG.txt</code>, <code>SHA256SUMS.txt</code></li></ul>"
     "<h2>Installer</h2><ol class=\"steps\"><li>Fermez toute DAW qui utilise SUBDESTRUCKT.</li><li>Vérifiez l'empreinte (chapitre 17, test R-01).</li>"
     "<li>Ouvrez l'Installer. Le paquet n'est pas signé : macOS le signale. Ouvrez Réglages Système › Confidentialité et sécurité, puis « Ouvrir quand même » pour ce paquet uniquement. Ne désactivez jamais Gatekeeper globalement.</li>"
     "<li>Saisissez votre mot de passe : le plugin est installé dans <code>/Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3</code>.</li><li>Relancez la DAW et l'analyse des plugins si nécessaire.</li></ol>"
     "<h2>Mettre à jour depuis 0.1.0-b5</h2><p>Installez par-dessus : la version précédente est détectée, sauvegardée pendant l'opération et restaurée en cas d'échec.</p>"
     "<h2>Désinstaller</h2><p>Ouvrez l'Uninstaller. Il ne retire que le bundle SUBDESTRUCKT identifié et son reçu. Vos presets (<code>~/Library/Application Support/Labtechno/SUBDESTRUCKT/Presets</code>) et votre MIDI Learn sont conservés.</p>")

ch(3, "ch-03", "Premières minutes",
   "Charger un son, le transformer en moins de trente secondes, et revenir en arrière si besoin.",
   '<ol class="steps"><li>Ouvrez SUBDESTRUCKT dans une piste instrument et jouez une note : vous entendez <strong>Init</strong>, le son de 0.1.0-b5.</li>'
   "<li>Avec les flèches <code>&lt;</code> et <code>&gt;</code> de la barre du haut, parcourez les presets d'usine : Cross Metal, FM Bell Bank, Doom Fold Comb, Glitch Lab, Spectral Formants, Ring Destroyer.</li>"
   "<li>Dans l'espace <strong>Performance</strong>, tournez <strong>WT X</strong> et <strong>WT Y</strong> (la forme de la wavetable), puis <strong>WT Z</strong> (sa transformation), puis <strong>CROSS Master</strong> (l'intensité de l'interaction entre les deux moteurs).</li>"
   "<li>Une erreur ? <strong>Undo</strong> annule le dernier geste complet (un mouvement de bouton = une étape).</li>"
   "<li>Un son bloqué ou trop fort ? <strong>PANIC</strong>, toujours visible en haut à droite, coupe en douceur et réinitialise.</li></ol>"
   "{{AUDIO:e01_init}}{{AB:e02_xy_basic|e03_xy_square|Déplacer X : du sinus au carré (banque Basic)}}")

ch(4, "ch-04", "L'interface",
   "Trois espaces de travail sur un même son : passer de l'un à l'autre, zoomer ou changer de moniteur ne modifie jamais le son.",
   "<h2>La barre persistante</h2><p>De gauche à droite : preset (un astérisque signale une modification non sauvegardée), Undo/Redo, les trois espaces, le mètre de sortie (en rouge au-delà de 0 dBFS), Quality, Drone, zoom (75 à 150 %), Inspector, PANIC.</p>"
   + fig("assets/ui_performance.png", "Espace Performance", "Espace Performance : X/Y/Z, CROSS, sortie, macros, commandes FM contextuelles et moniteur. " + CAPNOTE)
   + "<h2>Performance</h2><p>Le noyau jouable : X, Y, Z, CROSS Master, Output Gain, les macros M1 à M4 (renommables) et deux commandes FM contextuelles. <strong>FM INDEX</strong> pilote la profondeur de la première route FM qui alimente un opérateur audible ; <strong>FM FEEDBACK</strong> pilote l'auto-modulation de cet opérateur si elle existe. Le paramètre réellement piloté est affiché sous les commandes : ce ne sont pas des commandes cachées.</p>"
   + fig("assets/ui_sound_process.png", "Espace Sound Design", "Sound Design : WT à gauche, CROSS au centre, FM à droite ; en dessous les chaînes de traitement. " + CAPNOTE)
   + "<h2>Sound Design</h2><p>Les deux moteurs ont le même poids visuel, avec le CROSS au centre. En dessous, trois onglets : PROCESS (les trois chaînes de traitement), MOD (enveloppes, LFO, aléatoire) et VOIX (voix, accordage, sortie). Chaque chaîne n'affiche que les réglages de l'algorithme choisi, dans l'ordre réel (filtre avant ou après la destruction).</p>"
   + fig("assets/ui_eng_network.png", "Espace Engineering", "Engineering › Réseaux : les 55 routes, leur classe causale et leur identifiant. " + CAPNOTE)
   + "<h2>Engineering</h2><p>Les outils de précision : réseaux FM/CROSS, matrice de modulation, banques, Randomizer, état et presets, diagnostics.</p>"
   "<h2>L'Inspector</h2><p>Cliquez sur n'importe quel réglage : l'Inspector (à droite, rétractable) affiche son identifiant stable, sa plage, sa valeur de base et, s'il est modulé, sa valeur effective. Vous pouvez y ajouter une modulation, activer le MIDI Learn ou le verrouiller contre le Randomizer. « Épingler » conserve l'Inspector sur ce réglage pendant que vous en touchez d'autres.</p>"
   + box("fact", "Base et effective", "<p>Le bouton montre la valeur de <strong>base</strong> (celle que la DAW automatise). Un anneau bleu extérieur montre la valeur <strong>effective</strong>, après modulation. La modulation n'écrit jamais dans l'automation.</p>")
   + "<h2>Moniteurs</h2><p>SCOPE et SPECTRE montrent la sortie du moteur. XY montre la position de base (cercle) et effective (point) dans la grille de la banque, Z dans une barre séparée : ce n'est pas une troisième dimension stockée. RÉSEAU dessine le graphe réel des liaisons (trait plein : immédiat ; tirets : retardé).</p>")

ch(5, "ch-05", "Comprendre le son",
   "Quelques notions suffisent pour tout explorer : oscillateur, modulation, enveloppe, filtre.",
   "<p>Un <strong>oscillateur</strong> répète une forme d'onde : sa vitesse donne la hauteur, sa forme donne le timbre. Une <strong>modulation</strong> fait varier un réglage dans le temps : lentement (un LFO qui balaie un filtre) ou très vite (un oscillateur qui déforme un autre oscillateur : c'est la FM). Une <strong>enveloppe</strong> dessine l'évolution d'une note. Un <strong>filtre</strong> retire ou accentue des fréquences.</p>"
   "<p>SUBDESTRUCKT pousse une idée : faire dialoguer deux oscillateurs de natures différentes. La wavetable apporte des formes riches et mouvantes ; le FM apporte des spectres métalliques ; le CROSS les fait se moduler mutuellement.</p>"
   + adv("Approfondir : pourquoi certaines liaisons sont « retardées »",
         "<p>Quand A module B et que B module A, il faudrait connaître chaque sortie avant de la calculer. SUBDESTRUCKT résout ce cercle en retardant d'une durée fixe (1/48000 s) uniquement la liaison qui ferme le cercle (contrat B-T2). Une liaison garde sa classe tant qu'elle existe, pour que le son ne change pas silencieusement quand vous modifiez une autre liaison.</p>"))

ch(6, "ch-06", "La wavetable XYZ",
   "Une grille de formes d'onde que l'on parcourt avec X et Y, et des transformations Z appliquées en temps réel.",
   "<h2>X, Y, Z</h2><p><strong>X</strong> choisit la forme dans une table, <strong>Y</strong> choisit la table, <strong>Z</strong> applique une transformation calculée (Z n'est pas stocké). Les bords de la grille se règlent par axe : Clamp (bloque), Wrap (le dernier rejoint le premier), Mirror (rebondit). Le passage entre formes est progressif (Linear) ou par paliers (Stepped).</p>"
   "<h2>Les banques</h2><p>Sept banques d'usine générées par le moteur : Saw (b5), Basic Shapes, Pulse Width, Harmonic Series, Metal Spectra, Folded Sine, Formants. Vous pouvez importer vos formes (Engineering › Banques › Importer WAV) : le fichier est découpé en cycles de la longueur choisie, jusqu'à 64×16 formes. La banque importée est enregistrée dans votre projet et dans vos presets.</p>"
   "<h2>Z : trois transformations</h2><ul><li><strong>Harmonic Fold</strong> : replie la forme sur elle-même, ce qui ajoute des harmoniques.</li><li><strong>Phase Warp</strong> : déforme la lecture dans le cycle (même hauteur, timbre plus aigu).</li><li><strong>Spectral Mutation</strong> : petites variations aléatoires d'un cycle à l'autre (reproductibles).</li></ul>"
   "<p>Deux emplacements Z en série (ordre A→B ou B→A), chacun avec sa profondeur et sa courbe, sous le Z Master.</p>"
   + box("warn", "Limitation", "<p>Le catalogue Z prévoit huit transformations. Cinq transformations historiques n'ont pas été retrouvées et ne sont pas implémentées : elles ne sont pas inventées.</p>")
   + "{{AB:e04_z_fold|e05_z_warp|Z : Harmonic Fold puis Phase Warp}}{{AUDIO:e06_z_mutation}}"
   + adv("Approfondir : Clean, Raw, interpolation, Character, phase, sync",
         "<p><strong>Clean</strong> lit des versions limitées en bande (sans repliement), <strong>Raw</strong> lit la forme brute (repliement assumé, mesuré au moins 10 dB plus fort). L'interpolation Linear ou Cubic concerne la lecture dans le cycle. <strong>Character</strong> propose trois profils numériques (quantification de phase, réduction de bits, maintien d'échantillon). La phase peut être libre, relancée, fixe ou aléatoire à chaque note. La sync dure ou douce suit la note (source interne) ou un opérateur.</p>"))

ch(7, "ch-07", "Le moteur FM",
   "Quatre opérateurs sinusoïdaux (ou d'autres formes) qui se modulent les uns les autres.",
   "<p>Chaque opérateur a un <strong>niveau audio</strong> (ce que vous entendez) distinct de la <strong>profondeur de modulation</strong> (ce qu'il fait aux autres). <strong>Mute</strong> coupe le son mais garde la modulation ; <strong>Enable</strong> désactive l'opérateur entièrement.</p>"
   "<h2>Fréquence</h2><p>En mode Ratio, l'opérateur suit la note (×0,25 à ×16, plus Coarse ±48 demi-tons et Fine ±100 cents) ; en mode Fixed, il garde une fréquence fixe. Offset ajoute un décalage en Hz.</p>"
   "<h2>Quatre modes de modulation</h2><ul><li><strong>PM</strong> : modulation de phase (le son FM classique).</li><li><strong>Linear FM</strong> : modulation de fréquence ; la fréquence ne passe pas sous zéro.</li><li><strong>Through-Zero FM</strong> : la fréquence peut s'inverser.</li><li><strong>Exponential FM</strong> : modulation en octaves.</li></ul>"
   "{{AB:e07_fm_pm|e08_fm_lin|PM puis Linear FM}}{{AUDIO:e09_fm_exp}}"
   "<h2>Matrice et topologies</h2><p>La matrice 4×4 donne la profondeur de chaque liaison (une route doit exister dans Engineering › Réseaux pour agir). Le menu Topologie installe l'une des dix topologies (Cascade, Dual Pair, Triple Stack, Fan Out, Diamond, Parallel, Dual Cross, Ring Network, Recursive Stack, Hybrid Mesh) en remplaçant les seules routes FM internes.</p>"
   + adv("Approfondir : formes, Character, AC/DC", "<p>Neuf formes : Sine, Triangle, Saw, Pulse, Folded Sine, Asymmetric, Phase Distorted, Harmonic, Wavetable (lit la banque active à une position X/Y lente). Character quantifie la phase (multiplié par FM Character Master). AC retire la composante continue de la sortie de modulation, DC la conserve (défaut).</p>"))

ch(8, "ch-08", "Le CROSS",
   "Le cœur de l'instrument : la wavetable et le FM se modulent mutuellement, et leurs sons peuvent se combiner.",
   "<h2>Les liaisons</h2><p>La wavetable peut moduler chaque opérateur (WT→OP1..OP4). Chaque opérateur, ou le <strong>bus FM</strong> (mélange pondéré des opérateurs), peut moduler la wavetable sur sept cibles : phase, fréquence, X, Y, Z, niveau, Character. CROSS Master règle l'ensemble ; WT>FM et FM>WT dosent chaque sens ; Direction limite à un sens. Mettre CROSS à zéro annule l'interaction sans réinitialiser les phases.</p>"
   "<h2>Cross Audio</h2><p>Après les chaînes de traitement de chaque moteur : <strong>Mix</strong> (somme), <strong>Crossfade</strong> (de la WT seule au FM seul), <strong>Ring</strong> (produit des deux), <strong>AM</strong> (un moteur module l'amplitude de l'autre).</p>{{AUDIO:e17_cross_ring}}"
   "<h2>Cross Reset</h2><p>Efface l'historique des liaisons CROSS retardées, sans toucher aux phases, au feedback FM interne, aux effets ni au Drone.</p>"
   + adv("Approfondir : classes causales", "<p>Engineering › Réseaux affiche pour chaque route absente la classe qu'elle aurait si vous l'ajoutiez. Une route qui ferme un cercle immédiat devient retardée (T2) ; elle le reste tant qu'elle existe.</p>"))

ch(9, "ch-09", "Filtres et résonateurs",
   "Trois emplacements (WT, FM, global), un filtre chacun, avant ou après la destruction.",
   "<ul><li><strong>F1 Multimode</strong> : passe-bas, passe-bande, passe-haut (12 ou 24 dB/oct) et coupe-bande, résonance jusqu'à l'auto-oscillation.</li>"
   "<li><strong>F2 Comb</strong> : résonance accordée (accord vérifié à 5 cents près), feedback positif ou négatif, amortissement.</li>"
   "<li><strong>F3 Nonlinear</strong> : filtre saturé (drive, bias, character).</li>"
   "<li><strong>F4 Modal</strong> : banque de 1 à 16 résonateurs (Harmonic, Metal, Membrane, Inharmonic), decay, damping, brightness.</li></ul>"
   "{{AUDIO:e10_f1_lp}}{{AB:e11_f2_comb|e12_f4_modal|Comb accordé puis résonateur modal}}"
   "<p>Keytrack fait suivre la note au filtre. En Dual Mono, Spread écarte les canaux gauche et droit. Excitation injecte une impulsion à chaque note (Note) ou à la demande (Manual, bouton Excitation).</p>"
   + box("warn", "États autonomes", "<p>L'auto-oscillation et le Freeze du résonateur modal ne persistent après la fin de la note que si vous avez coché <strong>Autonomous Armed</strong>. Sinon, un garde-fou arrête la résonance dix secondes après la fin de la note.</p>"))

ch(10, "ch-10", "La destruction",
   "Six processeurs (deux par chaîne), chacun avec son mélange sec/traité et son gain de sortie.",
   "<ul><li><strong>N1 Asymmetric Wavefolder</strong> : drive, repli, asymétrie, 1 à 4 étages, tone.</li><li><strong>D1 Variable Rate Crusher</strong> : bits, sous-échantillonnage, jitter, Raw/Controlled.</li>"
   "<li><strong>Y1 Transient Destroyer</strong> : compression, gate, accentuation des attaques.</li><li><strong>S1 Ring/Frequency Shifter</strong> : modulation en anneau ou décalage de fréquence.</li>"
   "<li><strong>N2 Feedback Waveshaper</strong> : boucle courte saturée (auto-oscillation seulement si armée).</li><li><strong>D2 Glitch Buffer</strong> : boucles de fragments, lecture inversée, Freeze.</li>"
   "<li><strong>Y2 Envelope Shaper</strong> : remodèle les attaques ; son enveloppe est une source de modulation.</li><li><strong>S2 Spectral Freeze</strong> : fige le spectre (latence = taille FFT, compensée).</li></ul>"
   "{{AUDIO:e13_n1_fold}}{{AB:e14_d1_crush|e15_s1_ring|Crusher puis ring modulator}}"
   + box("warn", "Niveaux", "<p>Les gains de sortie s'additionnent : avec plusieurs processeurs poussés à +24 dB, la garde numérique (+30 dBFS) peut s'engager. Gardez une marge, ou activez le limiteur musical (onglet VOIX).</p>")
   + adv("Approfondir : repliement de N1", "<p>N1 utilise une méthode anti-repliement (ADAA). Avec un repli extrême, le repliement reste élevé ; Quality High double le suréchantillonnage du moteur et le réduit (mesures dans le rapport ARCH-G4).</p>"))

ch(11, "ch-11", "Modulation",
   "Faire bouger le son : enveloppes, LFO, aléatoire, chaos, macros, et une matrice de 64 liaisons.",
   "<h2>Les sources</h2><p>Cinq enveloppes (Amp, WT, FM, AUX1, AUX2 ; Delay, Attack, Hold, Decay, Sustain, Release, courbe linéaire ou exponentielle, Loop), quatre LFO (huit formes dont Custom, vitesse libre ou synchronisée au tempo, phase, relance à la note), Random Hold, Random Smooth, Chaos, trois suiveurs d'enveloppe, les sorties Y2, la vélocité, la note, le pitch bend, la molette, le souffle, l'expression, la pression et la pédale.</p>"
   "<h2>La matrice</h2><p>64 liaisons au plus, macros comprises. Chaque liaison : source, destination, profondeur (±1), courbe, polarité, Via (une seconde source qui dose la première), lissage, et rythme Control ou Audio (16 liaisons audio au plus). Une liaison impossible reste en place, inactive, avec sa raison affichée (Engineering › Modulation).</p>"
   "{{AUDIO:e16_mod_lfo}}"
   "<h2>Les macros</h2><p>M1 à M4, nommées par preset, se pilotent en Performance et peuvent être des sources ou des destinations (sans boucle entre macros).</p>"
   + box("warn", "Limitation", "<p>Les enveloppes ne se synchronisent pas encore au tempo : la règle n'est pas encore arbitrée.</p>"))

ch(12, "ch-12", "Voix, MIDI, sortie et Quality",
   "Comment SUBDESTRUCKT répond au clavier et protège la sortie.",
   "<p>Voix monophonique : <strong>Mono</strong> (chaque nouvelle note relance les enveloppes) ou <strong>Legato</strong> (pas de relance si une note est déjà tenue), priorité à la dernière, la plus basse ou la plus haute note. Pitch bend indépendant vers le haut et vers le bas (±2 demi-tons par défaut). Accordage de référence (La 440 Hz par défaut) et micro-accordage des 12 notes.</p>"
   "<p><strong>Drone</strong> (barre du haut) maintient la note après son relâchement. Il est désarmé par défaut, n'est jamais activé par un preset, et n'est restauré à l'ouverture d'un projet que si vous l'avez autorisé (Engineering › État). <strong>PANIC</strong> coupe en 5 ms, purge les états et désarme le Drone.</p>"
   "<p><strong>Quality</strong> (préférence du projet, pas du preset) : Normal reproduit 0.1.0-b5 ; High double le suréchantillonnage (plus de CPU, plus de latence à 44,1 kHz) ; Eco est identique à Normal pour l'oscillation. La latence annoncée à la DAW dépend aussi de S2.</p>"
   "<p>Le <strong>limiteur musical</strong> (onglet VOIX) est facultatif et distinct de la garde numérique permanente.</p>")

ch(13, "ch-13", "Presets, état et Randomizer",
   "Sauvegarder, explorer, revenir en arrière.",
   "<h2>Presets</h2><p>Engineering › État / Presets : charger, marquer comme favori, sauvegarder avec des tags. Les presets utilisateur sont des fichiers <code>.sdpreset</code>. Un preset ne contient ni la Quality ni le Drone.</p>"
   "<h2>Undo / Redo</h2><p>Jusqu'à 128 étapes par instance (non enregistrées dans le projet). Un geste continu sur un bouton compte pour une étape.</p>"
   + fig("assets/ui_eng_random.png", "Randomizer", "Engineering › Randomizer. " + CAPNOTE)
   + "<h2>Randomize, Mutate, Evolve</h2><p><strong>Randomize</strong> crée une nouvelle configuration, <strong>Mutate</strong> varie autour de l'actuelle, <strong>Evolve</strong> fait dériver lentement le son sans toucher à vos réglages (Reset efface la dérive, Commit l'enregistre). Profils Controlled, Industrial, Experimental, Custom ; familles cochables ; intensité (0 = aucun effet) ; verrous par réglage depuis l'Inspector. Les tirages sont reproductibles et réversibles par Undo.</p>")

ch(14, "ch-14", "Engineering, diagnostics et registre des paramètres",
   "Les outils de précision et la liste complète des réglages.",
   fig("assets/ui_eng_diag.png", "Diagnostics", "Engineering › Diagnostics : fréquences, suréchantillonnage, retard T2, latence, charge CPU mesurée, compteurs de sûreté. " + CAPNOTE)
   + "<p>La charge CPU affichée est mesurée sur le dernier bloc, sur votre machine : ce n'est pas une certification. Les compteurs de sûreté (non-finis, garde +30 dBFS, borne PM, limiteur, Panic) restent normalement à zéro.</p>"
   "<h2>Registre complet</h2><p>Chaque réglage a un identifiant stable (StableID) qui ne change jamais d'une version à l'autre.</p>{{PARAM_TABLE}}")

ch(15, "ch-15", "Exercices de sound design",
   "Dix exercices progressifs, réalisables avec les commandes de cette build. Chacun part de Init.",
   "{{EXERCISES}}")

ch(16, "ch-16", "Exemples audio",
   "Des sons produits par le moteur DSP réel de cette build, avec tous leurs réglages.",
   box("fact", "Comment ces sons ont été produits",
       "<ul><li><strong>Moteur :</strong> le cœur DSP non modifié (<code>source/</code>) au commit indiqué dans <code>audio/examples.json</code>, par l'outil <code>tools/manual-audio/render_examples_g4.cpp</code>.</li>"
       "<li><strong>Format :</strong> WAV 24 bits, mono (stéréo mixée), 48 kHz, Quality Normal.</li>"
       "<li><strong>Niveau :</strong> partie tenue normalisée à −20 dBFS RMS, crête limitée à −1 dBFS.</li>"
       "<li><strong>Plateforme de rendu :</strong> Windows (MSVC) à partir des mêmes sources que le plugin macOS ; l'identité échantillon par échantillon avec le plugin macOS n'est pas vérifiée.</li></ul>")
   + "{{AUDIO_CATALOG}}")

ch(17, "ch-17", "Recette et dépannage",
   "La recette physique vérifie sur un vrai Mac ce que les tests automatisés ne peuvent pas vérifier.",
   '<p><a class="btn primary" href="recette.html">Ouvrir l\'assistant de recette interactif</a></p>'
   "<p>Chaque test commence à PENDING ; vous choisissez PASS, FAIL ou N/A (justification obligatoire). Les réponses sont enregistrées automatiquement sur cet appareil uniquement ; exportez le fichier <code>SUBDESTRUCKT_ARCH_G4_RESULTS.json</code> ou copiez les résultats pour la direction technique.</p>"
   "{{RECETTE_SUMMARY}}"
   "<h2>Dépannage</h2><ul><li><strong>Le plugin n'apparaît pas :</strong> relancez l'analyse des plugins VST3 de la DAW ; vérifiez la présence de <code>/Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3</code>.</li>"
   "<li><strong>Silence :</strong> vérifiez WT Level et les niveaux des opérateurs, Output Gain, le Cross Audio (Crossfade à une extrémité), puis PANIC.</li>"
   "<li><strong>Son qui persiste :</strong> un emplacement armé (Autonomous Armed) ou le Drone ; décochez-les ou PANIC.</li>"
   "<li><strong>Clic au changement d'algorithme :</strong> les changements structurels sont amortis mais aucune absence de clic n'est garantie.</li>"
   "<li><strong>Projet 0.2.0 ouvert dans 0.1.0-b5 :</strong> l'état est refusé (format plus récent) ; gardez une copie avant tout retour arrière.</li></ul>")

ch(18, "ch-18", "Glossaire et limitations",
   "Les termes du manuel, puis la liste exacte de ce qui manque dans cette build.",
   "{{GLOSSARY}}<h2>Limitations connues</h2><ul>"
   "<li>Cinq transformations Z historiques non implémentées (catalogue non retrouvé).</li><li>Morph spectral entre formes, éditeur graphique de formes et générateur procédural dans l'interface : absents.</li>"
   "<li>Enveloppes non synchronisables au tempo ; balance WT/FM de façade non exposée (décisions non arbitrées).</li>"
   "<li>Repliement élevé de N1 en repli extrême (Quality High recommandé) ; garde +30 dBFS possible avec des gains cumulés extrêmes.</li>"
   "<li>Aucune absence de clic garantie lors des changements structurels ; MIDI Learn appliqué à la cadence de l'interface (≈ 33 ms).</li>"
   "<li>Performances non certifiées sur M3/M4 ; graphisme provisoire ; tâches d'ergonomie non mesurées ; aucun test DAW réel effectué à ce jour.</li></ul>")

# ---------------------------------------------------------------- glossaire (b5 + G4)
gl = json.load(open("manual/src/0.1.0-b5/glossary.json", encoding="utf-8"))
remap = {"ch-06": "ch-06", "ch-07": "ch-07", "ch-08": "ch-08", "ch-09": "ch-12", "ch-10": "ch-14", "ch-04": "ch-04", "ch-05": "ch-05",
         "ch-03": "ch-03", "ch-02": "ch-02", "ch-12": "ch-16", "ch-13": "ch-17", "ch-14": "ch-17", "ch-16": "ch-18", "ch-01": "ch-01", "ch-11": "ch-15"}
for t in gl:
    t["see"] = remap.get(t.get("see", "ch-05"), "ch-05")
    t["long"] = t["long"].replace("0.1.0-b5", "0.2.0")
new = [
    ("xyz", "XYZ (wavetable)", "X choisit la forme, Y la table, Z la transformation calculée.", "Dans SUBDESTRUCKT, X et Y parcourent une grille de formes stockées ; Z applique une transformation calculée en temps réel, ce n'est pas une troisième dimension stockée.", "ch-06"),
    ("banque", "Banque", "Ensemble de formes d'onde organisé en grille X×Y.", "Une banque est immuable : modifier ou importer crée une nouvelle banque avec un nouvel identifiant (SHA-256).", "ch-06"),
    ("inspector", "Inspector", "Panneau qui détaille le réglage sélectionné.", "Il montre l'identifiant stable, la plage, la valeur de base, la valeur effective et les modulations ; il permet d'ajouter une modulation, le MIDI Learn et un verrou.", "ch-04"),
    ("macro", "Macro", "Commande unique qui pilote plusieurs réglages.", "Quatre macros, nommées par preset ; leurs liaisons comptent dans les 64 de la matrice.", "ch-11"),
    ("matrice", "Matrice de modulation", "Table des liaisons source → destination.", "64 liaisons au plus, avec courbe, Via, lissage et rythme Control ou Audio.", "ch-11"),
    ("via", "Via", "Seconde source qui dose une liaison de modulation.", "Effet multiplicatif : V = (1 − a) + a × v, où a est la quantité de Via.", "ch-11"),
    ("drone", "Drone", "Maintien de la note après son relâchement.", "Désarmé par défaut ; jamais activé par un preset ; PANIC le désarme.", "ch-12"),
    ("quality", "Quality", "Préférence de calcul du projet : Eco, Normal, High.", "Distincte de Clean/Raw (choix artistique du preset) ; High double le suréchantillonnage.", "ch-12"),
    ("undo", "Undo / Redo", "Annuler / rétablir.", "128 étapes par instance ; un geste continu = une étape ; non enregistré dans le projet.", "ch-13"),
    ("randomize", "Randomize / Mutate / Evolve", "Création aléatoire, variation, dérive lente.", "Reproductibles (seed), réversibles (Undo), avec familles, verrous et intensité.", "ch-13"),
    ("comb", "Filtre en peigne (comb)", "Filtre qui résonne sur une note et ses harmoniques.", "Un retard bouclé dont la longueur fixe la hauteur de résonance.", "ch-09"),
    ("modal", "Résonateur modal", "Banque de résonances qui imite un objet frappé.", "Chaque mode est une résonance accordée ; les modèles imitent une corde, un métal, une membrane.", "ch-09"),
    ("wavefolder", "Wavefolder", "Repli de l'onde sur elle-même.", "Au-delà d'un seuil, le signal est réfléchi, ce qui crée des harmoniques.", "ch-10"),
    ("armement", "Armement (Autonomous Armed)", "Autorisation explicite d'un état qui persiste seul.", "Sans armement, auto-oscillation et gel s'arrêtent après la fin de la note (garde-fou).", "ch-09"),
    ("stableid", "StableID", "Identifiant permanent d'un réglage.", "Il ne change jamais d'une version à l'autre : projets et automations restent valides.", "ch-14"),
]
have = {t["id"] for t in gl}
for i, term, short, long_, see in new:
    if i not in have:
        gl.append({"id": i, "term": term, "aliases": [], "short": short, "long": long_, "see": see})
json.dump(gl, open(D + "/glossary.json", "w", encoding="utf-8"), ensure_ascii=False, indent=1)

# ---------------------------------------------------------------- exercices
def ex(n, title, level, dur, goal, concepts, start, steps, values, expected, listen, mistakes, variation, audio=None, ab=None):
    e = dict(n=n, title=title, level=level, duration=dur, goal=goal, concepts=concepts, start=start, steps=steps, values=values,
             expected=expected, listen=listen, mistakes=mistakes, variation=variation)
    if audio:
        e["audio"] = audio
    if ab:
        e["ab"] = ab
    return e


exs = [
    ex(1, "Parcourir une banque", "Débutant", "5 min", "Entendre le rôle de X.", ["wavetable", "X"], "Init.",
       ["Sound Design : banque Basic Shapes.", "OP1 Level 0.", "Tournez WT X de 0 à 1."], "Banque Basic · OP1 Level 0", "Du sinus au carré en passant par triangle et dent de scie.",
       "La brillance augmente avec X.", "Oublier de couper OP1 : le FM masque la WT.", "Essayez Stepped au lieu de Linear.", ab=["e02_xy_basic", "e03_xy_square"]),
    ex(2, "Plier la forme", "Débutant", "5 min", "Découvrir Z.", ["Z", "Harmonic Fold"], "Init, OP1 Level 0.",
       ["WT ZA Transform : Harmonic Fold.", "Montez WT Z Master doucement."], "ZA Harmonic Fold · Z 0 → 0,6", "Des harmoniques apparaissent progressivement, la hauteur ne change pas.",
       "Le timbre devient métallique.", "Z Master à 0 : aucune transformation.", "Placez Phase Warp en ZB.", audio="e04_z_fold"),
    ex(3, "FM : PM contre Linear", "Intermédiaire", "10 min", "Comparer deux modes FM.", ["FM", "PM", "Linear FM"], "Init, WT Level 0.",
       ["OP1 Mod Mode : PM, FM OP2>OP1 à 2.", "Puis Linear FM, profondeur 1."], "OP2 Ratio 2", "Deux spectres différents à profondeur comparable.", "Linear FM fait légèrement dériver la hauteur perçue.",
       "Pas de route OP2>OP1 dans Réseaux : la profondeur n'agit pas.", "Essayez Exponential FM.", ab=["e07_fm_pm", "e08_fm_lin"]),
    ex(4, "Une topologie en un clic", "Débutant", "5 min", "Changer l'architecture FM.", ["topologie"], "Init.",
       ["Sound Design › FM : Topologie FM › Cascade.", "Montez OP3 et OP4 dans la matrice."], "Cascade", "Un son plus complexe ; les routes CROSS sont conservées.", "Comparez avec Parallel.",
       "Profondeurs à 0 : rien ne change.", "Hybrid Mesh pour un son chaotique."),
    ex(5, "CROSS dans les deux sens", "Intermédiaire", "10 min", "Entendre l'interaction.", ["CROSS"], "Init.",
       ["Performance : CROSS Master de 0 à 1.", "Sound Design : CROSS WT>FM puis FM>WT séparément."], "CROSS Master 0 → 1", "L'interaction enrichit et durcit le son.", "Le son vit davantage quand les deux sens sont actifs.",
       "Confondre CROSS (modulation) et Cross Audio (mélange).", "Cross Audio Ring à 0,8.", audio="e17_cross_ring"),
    ex(6, "Comb accordé", "Intermédiaire", "10 min", "Faire chanter un filtre.", ["comb", "keytrack"], "Init.",
       ["PROCESS › Global : F2 Comb, Keytrack 1, Feedback 0,85, Mix 0,5.", "Jouez plusieurs notes."], "F2 Comb · Key 1", "Une résonance qui suit les notes.", "Le timbre devient creux et métallique.",
       "Keytrack à 0 : la résonance reste fixe.", "Feedback négatif : une octave plus bas.", audio="e11_f2_comb"),
    ex(7, "Destruction en chaîne", "Avancé", "15 min", "Combiner deux processeurs.", ["destruction", "ordre"], "Init.",
       ["PROCESS › Global : Destruction 1 = N1, Destruction 2 = D1.", "Réglez Mix 0,5 sur chacun.", "Basculez le filtre Pre/Post."], "N1 drive 12 · D1 5 bits", "Une saturation pliée puis granuleuse.",
       "L'ordre change le résultat.", "Gains de sortie trop élevés.", "Remplacez D1 par S1 Ring.", ab=["e14_d1_crush", "e15_s1_ring"]),
    ex(8, "Moduler avec la matrice", "Intermédiaire", "10 min", "Créer une modulation.", ["matrice", "LFO", "Inspector"], "Init.",
       ["Cliquez sur FM OP2>OP1.", "Inspector : source LFO1, profondeur 0,4, + Modulation.", "MOD : LFO1 Rate 0,7 Hz."], "LFO1 → FM OP2>OP1, 0,4", "Le spectre FM respire.",
       "L'anneau bleu montre la valeur effective.", "Choisir une destination discrète : refusée.", "Rythme Audio, LFO à 200 Hz.", audio="e16_mod_lfo"),
    ex(9, "Explorer avec Mutate", "Débutant", "5 min", "Trouver des variations.", ["Mutate", "Undo"], "Un preset d'usine.",
       ["Engineering › Randomizer : Controlled, intensité 0,3.", "MUTATE plusieurs fois.", "Undo pour revenir."], "Controlled · 0,3", "Des variations proches de l'original.",
       "Comparez Industrial et Experimental.", "Intensité 0 : aucun changement.", "Verrouillez CROSS Master depuis l'Inspector."),
    ex(10, "Son tenu avec Drone", "Débutant", "5 min", "Tenir une note sans clavier.", ["Drone", "PANIC"], "Doom Fold Comb.",
       ["Jouez une note.", "Cochez Drone.", "Relâchez la touche.", "PANIC pour arrêter."], "Drone on", "La note continue après le relâchement ; PANIC l'arrête et désarme le Drone.",
       "Le son évolue lentement.", "Croire que le preset active le Drone : jamais.", "Ajoutez un LFO lent sur WT Y."),
]
json.dump(exs, open(D + "/exercises.json", "w", encoding="utf-8"), ensure_ascii=False, indent=1)

# ---------------------------------------------------------------- recette
def rt(i, title, proc, expected, why, steps, passc, failc, onfail, hint, prereq=None, skills="Utiliser une DAW."):
    return {"id": f"R-{i:02d}", "title": title, "official": {"procedure": proc, "expected": expected}, "why": why,
            "prerequisites": prereq or ["SUBDESTRUCKT 0.2.0 installé (sauf R-01 et R-02).", "Une DAW et un clavier MIDI."],
            "skills": skills, "steps": steps, "expected": expected, "pass": passc, "fail": failc, "onFail": onfail, "evidenceHint": hint}


tests = [
    rt(1, "Intégrité de l'archive", "shasum -a 256 du ZIP ; shasum -a 256 -c SHA256SUMS.txt", "Empreinte identique à celle communiquée ; 4 lignes OK.",
       "Vérifier que l'archive est exactement celle construite et testée.", ["Calculez l'empreinte du ZIP et comparez-la à celle communiquée.", "Dézippez, puis shasum -a 256 -c SHA256SUMS.txt."],
       "Empreinte identique ET 4 lignes OK.", "Empreinte différente ou ligne FAILED.", "Ne pas installer ; signaler les deux empreintes.", "Texte des deux commandes.",
       ["Le ZIP reçu par le canal privé et son empreinte.", "Terminal."], "Coller une commande dans le Terminal."),
    rt(2, "Installation et mise à jour", "Installer.pkg par-dessus 0.1.0-b5 ou sur un Mac propre", "Installation réussie ; bundle présent ; version 0.2.0.",
       "Vérifier l'installation et la mise à jour.", ["Fermez les DAW.", "Ouvrez l'Installer et suivez-le.", "Vérifiez /Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3."],
       "Installation réussie, bundle présent.", "Échec, message d'erreur, bundle absent.", "Noter le message ; joindre /var/log/install.log.", "Capture de la fin d'installation."),
    rt(3, "Découverte dans la DAW", "Analyse des plugins VST3", "SUBDESTRUCKT (Labtechno, instrument) listé et instanciable.", "Vérifier la découverte VST3.",
       ["Relancez l'analyse des plugins.", "Insérez SUBDESTRUCKT sur une piste instrument.", "Ouvrez l'éditeur."], "Plugin trouvé, éditeur affiché.", "Plugin absent ou plantage.", "Noter la DAW et sa version.", "Capture de l'éditeur."),
    rt(4, "Projet 0.1.0-b5", "Ouvrir un projet enregistré avec 0.1.0-b5", "Le son est identique ; aucun message d'erreur.", "Vérifier la migration d'état.",
       ["Ouvrez un projet 0.1.0-b5 existant.", "Écoutez et comparez à votre souvenir ou à un rendu antérieur."], "Son identique.", "Son différent ou état refusé.", "Conserver le projet ; noter le message de la barre.", "Rendu audio avant/après si disponible."),
    rt(5, "Performance", "Espace Performance", "X, Y, Z, CROSS, macros et FM INDEX agissent audiblement ; aucune commande inactive.", "Vérifier la façade jouable.",
       ["Chargez Cross Metal.", "Tournez X, Y, Z, CROSS Master, Macro 1.", "Tournez FM INDEX ; lisez le paramètre affiché dessous."], "Chaque commande agit.", "Une commande sans effet audible.", "Noter la commande.", "Courte vidéo ou description."),
    rt(6, "Wavetable et banques", "Sound Design › WT", "Changement de banque, X/Y, Z (3 transformations), Clean/Raw audibles.", "Vérifier le moteur WT.",
       ["Choisissez Formants puis Metal Spectra.", "Essayez Harmonic Fold, Phase Warp, Spectral Mutation.", "Comparez Clean et Raw sur une note aiguë."], "Tout est audible et sans craquement anormal.", "Silence, craquement, plantage.", "Noter le réglage.", "Description."),
    rt(7, "FM 4 opérateurs", "Sound Design › FM", "Modes PM/Linear/TZ/Exp et topologies changent le son.", "Vérifier le moteur FM.",
       ["Topologie Cascade.", "OP1 Mod Mode : PM, Linear FM, Through-Zero FM, Exponential FM."], "Changements audibles.", "Absence d'effet ou instabilité.", "Noter la topologie.", "Description."),
    rt(8, "CROSS et réseau", "Engineering › Réseaux", "Ajout/retrait de routes ; classes affichées ; aucune instabilité.", "Vérifier les liaisons et la causalité.",
       ["Ajoutez OP2>WT.X puis WT>OP3.", "Lisez la classe (Immediate/Delayed).", "Retirez-les en jouant."], "Routes ajoutées et retirées sans plantage.", "Refus inexpliqué, plantage, bruit.", "Noter le message de la barre.", "Capture de la liste."),
    rt(9, "Filtres", "PROCESS : F1 à F4", "Chaque type agit ; Excitation (Note) fait résonner F2/F4.", "Vérifier les filtres.",
       ["Global : F1 LP12, puis F2 Comb (Key 1), F3, F4 Metal.", "Excitation Note sur F4."], "Effets audibles.", "Effet absent, auto-oscillation non armée persistante au-delà de 10 s.", "Noter le type.", "Description."),
    rt(10, "Destruction", "PROCESS : N1 à S2", "Chaque processeur agit ; Mix 0 = contournement.", "Vérifier la destruction.",
       ["Essayez les 8 processeurs sur Global 1.", "Mix à 0 puis 1."], "Effets audibles, Mix 0 neutre.", "Effet absent ou plantage.", "Noter le processeur.", "Description."),
    rt(11, "Modulation et MIDI", "Inspector et matrice ; pitch bend, molette, pression, sustain", "Une modulation créée agit ; l'expression MIDI agit.", "Vérifier la modulation et l'expressivité.",
       ["Inspector : LFO1 vers WT X.", "Pitch bend, molette (source Mod Wheel), pédale de sustain."], "Effets conformes.", "Sans effet.", "Noter la source.", "Description."),
    rt(12, "Presets, Undo, Randomizer", "Engineering › État et Randomizer", "Sauvegarde/rechargement ; Undo/Redo ; Mutate reproductible.", "Vérifier l'état.",
       ["Sauvegardez un preset, rechargez-le.", "MUTATE puis Undo, Redo."], "Retour exact ; preset identique.", "Écart, perte de réglage.", "Noter l'étape.", "Nom du preset."),
    rt(13, "Rechargement de projet", "Enregistrer, fermer, rouvrir le projet", "Son et réglages identiques ; Drone non réactivé sans autorisation.", "Vérifier l'état DAW.",
       ["Modifiez des réglages et une route.", "Enregistrez, fermez, rouvrez."], "Identique.", "Différence.", "Noter la différence.", "Rendu avant/après."),
    rt(14, "Fréquences et latence", "44,1 / 48 / 88,2 / 96 kHz ; S2 actif", "Son correct aux 4 fréquences ; compensation de latence correcte avec S2.", "Vérifier fréquences et PDC.",
       ["Changez la fréquence de la DAW.", "Activez S2 sur Global 1 ; vérifiez l'alignement avec une autre piste."], "Correct.", "Décalage, silence.", "Noter la fréquence.", "Description."),
    rt(15, "Instances et performance", "4 instances, Quality Normal puis High", "Lecture sans décrochage ; charge affichée cohérente.", "Vérifier la charge sur votre Mac (non certifiante).",
       ["Ouvrez 4 instances avec Glitch Lab.", "Lisez la charge CPU (Diagnostics) et celle de la DAW."], "Pas de décrochage.", "Décrochages audio.", "Noter la machine et la taille de bloc.", "Valeurs relevées."),
    rt(16, "Panic et Drone", "PANIC ; Drone", "PANIC coupe en moins d'une seconde et désarme le Drone.", "Vérifier la sûreté.",
       ["Drone activé, note tenue.", "PANIC."], "Silence et Drone désarmé.", "Son qui persiste.", "Noter le réglage.", "Description."),
    rt(17, "Désinstallation", "Uninstaller.pkg", "Bundle retiré ; presets et MIDI Learn conservés.", "Vérifier la désinstallation.",
       ["Fermez les DAW.", "Lancez l'Uninstaller.", "Vérifiez la conservation de vos presets."], "Bundle absent, données conservées.", "Bundle présent ou données perdues.", "Joindre /var/log/install.log.", "Capture."),
    rt(18, "Réinstallation", "Installer.pkg après désinstallation", "Plugin de nouveau disponible ; projets rechargés.", "Vérifier la réinstallation.",
       ["Réinstallez.", "Rouvrez le projet de R-13."], "Fonctionne.", "Échec.", "Noter le message.", "Capture."),
]
groups = [{"id": "installation", "title": "Installation", "tests": ["R-01", "R-02", "R-03", "R-04"]},
          {"id": "musical", "title": "Validation musicale", "tests": ["R-05", "R-06", "R-07", "R-08", "R-09", "R-10", "R-11"]},
          {"id": "technique", "title": "Validation technique", "tests": ["R-12", "R-13", "R-14", "R-15", "R-16"]},
          {"id": "packages", "title": "Désinstallation et réinstallation", "tests": ["R-17", "R-18"]}]
json.dump({"schemaVersion": 1, "campaignId": "SUBDESTRUCKT-ARCH-G4-RECETTE-PHYSIQUE", "source": "manual/src/0.2.0/recette/tests.json",
           "groups": groups, "tests": tests}, open(D + "/recette/tests.json", "w", encoding="utf-8"), ensure_ascii=False, indent=1)
open(D + "/parameters.json", "w", encoding="utf-8").write('{"note": "0.2.0 : tableau genere depuis source/core/ParamTable.inc"}\n')
print("ok", len(os.listdir(D + "/chapters")), "chapitres")
