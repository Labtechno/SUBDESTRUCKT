# ARCH-UI-04-R2-B2 — registre des écarts

Écarts constatés pendant l'audit R2-B2-00 (2026-11-28, SHA `73b633b`) :

- entre le code de production et le référentiel R2 ;
- entre le laboratoire et la production ;
- entre les documents normatifs eux-mêmes.

Aucun n'est corrigé en R2-B2-00. Chaque écart a un traitement prévu et, lorsqu'il faut une décision, une référence au
§12 du dossier ([B2-00-DOSSIER.md](B2-00-DOSSIER.md)). Le registre sera tenu à jour pendant R2-B2.

**Nature** :

- **PRE** : défaut préexistant de 0.4.1 ;
- **NORM** : contradiction entre documents normatifs ;
- **LAB** : simplification du laboratoire à ne pas transposer ;
- **GEO** : géométrie.

| ID | Nature | Constat | Preuve | Traitement prévu | Étape | Décision |
|---|---|---|---|---|---|---|
| EC-01 | PRE | Point de prélèvement de l'oscilloscope **faux à l'écran** : l'afficheur indique « SCOPE sortie moteur (avant gain) » et l'en-tête de `ui/Monitors.h` dit « avant gain de sortie ». Le moteur prélève en **sortie finale** (après OUT GAIN, garde ELP, écrêtage de sécurité, limiteur et rampe PANIC). | `ui/Monitors.cpp:56`, `ui/Monitors.h:3-4` ; `source/engine/Engine.cpp:739` ; `R2-TELEMETRY-FEASIBILITY.md` T-18 | Correction du texte affiché et du commentaire (interface seulement, aucune modification DSP) ; source affichée en pied de HUD (CD-02, TEL-06) | B2-06 | — |
| EC-02 | PRE | Spectre sans mention INDICATIVE ; le repliement dans la bande affichée n'est pas signalé | `ui/Monitors.cpp:69` ; CD-06 ; RK-17 | Pastille INDICATIVE et source explicite (variante HUD validée) | B2-06 | — |
| EC-03 | PRE | Le PLASMA OUT GAIN dessine une **mesure audio réelle** (crête) dans le canal du paramètre, ce que PL-04 interdit ; PL-05 demande une crête distincte et adjacente | `ui/PerformanceView.cpp:243` (`setMeter`) ; PL-04, PL-05 | Mesure retirée du canal et affichée par un HUD PEAK (MEAS) dans la même plaque, à l'emplacement actuel de `outInfo_` (aucune réorganisation) | B2-03 (2) et (5) | — |
| EC-04 | NORM | NAM-07 cite encore « XAUDIO AMT ». LAB-A08 (normatif, postérieur) abandonne XAUDIO : AMOUNT et MODE sous l'en-tête CROSS AUDIO | `R2-ACCEPTANCE-MATRIX.md` NAM-07 ; `R2-DECISION-REGISTER.md` §3 ter | NAM-07 se lit avec les distinctions de LAB-A08 ; addendum consigné dans la matrice d'acceptation (conséquence directe d'un arbitrage, RK-15) | B2-00 (consigné) | — |
| EC-05 | NORM | **Gestes PLASMA** : PL-06 (« gestes identiques au NEXUS ») contredit le comportement 0.4.1 (clic = position absolue, glisser rapporté à la longueur du canal) et celui du laboratoire (glisser relatif sur 220 px) | `R2-ACCEPTANCE-MATRIX.md` PL-06 ; `ui/Components.cpp:399-436` ; `lab/r2/R2PlasmaChannel.cpp:139-163` | Décision du propriétaire | B2-03 (2) | **DB2-02** |
| EC-06 | PRE | `DiagnosticEditor` est compilé (`cmake/Plugin.cmake:29`) mais n'est jamais instancié (`createEditor` crée `MainEditor`) | `plugin/PluginProcessor.cpp:303` | Hors périmètre ; documenté, non modifié | — | — |
| EC-07 | NORM | **Matériel de référence** : la mission R2-B2 retient le M1 comme minimum. `docs/V1-PLATFORM-REQUIREMENTS.md` fixe l'**Apple M3 de base** depuis la révision validée du 2026-11-25 (M1 et M2 hors support officiel) | mission §7 R2-B2-07 ; `V1-PLATFORM-REQUIREMENTS.md` §1 et révision du 2026-11-25 | Décision du propriétaire. Le protocole GC-01 est comparatif et indépendant de la machine en attendant. | B2-07 | **DB2-01** |
| EC-08 | NORM | **Nomenclature et NAM-01** : NAM-01 exige qu'aucune chaîne française ne reste visible. Mais OD-02 ne valide que les 324 occurrences P1 ; 1 890 occurrences P2 et P3 restent proposées. Environ 180 littéraux français (infobulles, menus, messages, titres de plaque) ne figurent pas dans le CSV (relevé heuristique). | `R2-CONTROL-LABELS-R1.csv` (statuts) ; relevé sur `ui/*.cpp` | Décision du propriétaire | B2-04 | **DB2-03** |
| EC-09 | PRE | UI-01w parcourt `tab < 6` et ne visite pas les onglets ENGINEERING 6 à 9 | `tests/wrapper/WrapperTests.cpp:349-384` ; `B1-COMPONENT-LAB/INTERACTION-TEST-PLAN.md` §2 | Énumération par registre de navigation (OD-01). La couverture s'élargit, l'assertion reste inchangée. | B2-05 | — |
| EC-10 | LAB | Simplifications du laboratoire : NEXUS sans composante horizontale du glisser ; pas Int du laboratoire ; saisie numérique réduite à un drapeau ; aucun menu contextuel ; pas de menu de liste ; gestes PLASMA différents ; valeur accessible non formatée | `lab/r2/R2Nexus.cpp:180-286`, `R2PlasmaChannel.cpp`, `R2SynapticTrack.cpp` | Non transposées : la production conserve ses gestionnaires d'événements ; seul le rendu R1 est intégré (TB2-01) | B2-03 | — |
| EC-11 | PRE | Valeur accessible des NEXUS et PLASMA : nombre brut sans unité (`AccessibilityRangedNumericValueInterface`) | `ui/Components.cpp:160-172` | Valeur formatée avec unité exposée en plus de la plage numérique (AX-01) ; à vérifier sous VoiceOver (PHYS) | B2-05 | — |
| EC-12 | GEO | Les cellules PLASMA verticales de 0.4.1 ne correspondent à aucun gabarit validé : OUT GAIN 96 × 158 ; Z du Core 74 × hauteur variable | `ui/PerformanceView.cpp:65, 371, 422` | Gabarit COMPACT 48 × 150 centré dans la cellule (il tient sans déformation). STANDARD dès qu'un emplacement d'au moins 64 × 180 existe (R2-C). Règle de choix par largeur **et** hauteur (TB2-03). | B2-03 (2) | — |
| EC-13 | GEO | `Z · CROSS LVL` : marge de 1,0 px sous macOS et deux lignes sous Windows (budget 70 px), dans le Core (lot R2-C) | `B1-R1-FINAL-ARTISTIC-REFINEMENT/P1-NAMING-VALIDATION.md` | Réserve géométrique. Mesure dans la disposition de production. Si la marge est inférieure à 8 px (seuil d'alerte OK_MARGE de PT-02), la zone de libellé est adaptée localement sans déplacer d'autres commandes ; sinon, la réserve passe à R2-C. Jamais réduire le corps ni changer le vocabulaire. | B2-04 | — |
| EC-14 | PRE | PB-01 (gestes imbriqués au double-clic) : constat **statique** seulement (H-04) ; le même chemin existe pour les PLASMA (`ContinuousControl` commun) | `ui/Components.cpp:399-447` | Reproduction préalable obligatoire (OD-10) | B2-01 | — |
| EC-15 | NORM | Version : OD-17 (A) fixe 0.5.0 pour la première livraison intégrée R2-B. La mission R2-B2 interdit tout changement de version sans justification ni autorisation. | `R2-RISKS-AND-OPEN-DECISIONS.md` §2 bis | Confirmation demandée ; aucun changement de version avant la clôture de R2-B2 | B2-08 | **DB2-04** |
| EC-16 | NORM | Aucune variante HUD validée pour les éditeurs interactifs (enveloppe, courbe) ni pour les vues XY, réseau, ordre Z et flux | `B1-R1-FINAL-ARTISTIC-REFINEMENT/COMPONENT-DESIGN-FINAL.md` §5 | Décision du propriétaire | B2-03 (5) | **DB2-05** |

## Addendum B2-00 (2026-11-28, après GO)

État des écarts après arbitrage (§8 de l'addendum) : voir [B2-00-ADDENDUM.md](B2-00-ADDENDUM.md), qui prime sur ce document pour les points qu'il traite.

## Écarts constatés en B2-01 (2026-11-28, reproduction sur 0.4.1)

| ID | Nature | Constat | Preuve | Traitement | Étape | Décision |
|---|---|---|---|---|---|---|
| EC-14 (mise à jour) | PRE | PB-01 (double-clic) **NON REPRODUIT** : JUCE 9.0.6 émet `mouseDoubleClick` après le `mouseUp` du second clic ; l'ordre supposé par H-04 ne se produit pas | `PB/README.md` ; `juce_Component.cpp:2667-2676` | Aucune correction (OD-10) ; tests permanents | — | — |
| EC-17 | PRE | **Saut de valeur des PLASMA** au clic sur le libellé (PLASMA vertical et horizontal) ou sur la valeur (PLASMA horizontal), jusqu'à une borne (OUT GAIN 0 → +6 dB) | PB-J1, PB-J2, PB-J3, PB-01c | Soumis avant modification (DB2-02) | B2-03 (2) | **DB2-08** |
| EC-18 | PRE | **PB-02a et PB-02d REPRODUITS** : la destruction de l'éditeur pendant un glisser du Core ou d'une poignée d'enveloppe laisse 2 (Core) ou 1 (enveloppe) gestes hôte ouverts. Conséquence mesurée : **l'Undo de la session est bloqué** (0 transaction pour l'édition suivante), car le compteur de gestes du processeur ne revient jamais à 0. | PB-02a, PB-02d ; `plugin/PluginProcessor.cpp:452-461` | Correction réservée à **R2-C** (Core) et **R2-D** (enveloppe), selon OD-10 et la mission ; gravité transmise à ces lots | R2-C, R2-D | — |
| EC-19 | PRE | Un double-clic dans le canal d'un PLASMA passe d'abord par la position cliquée (2 transactions Undo, valeur transitoire) ; inhérent au clic absolu (DB2-02) | PB-01b | Information ; aucune modification proposée | — | — |
| EC-20 | PRE | **PB-01e REPRODUIT** : molette puis clic avant 450 ms : gestes hôte imbriqués (profondeur 2) sur un NEXUS ou un PLASMA | PB-01e | Correction autorisée (famille PB-01, gestes NEXUS et PLASMA) : refermer le geste molette au `mouseDown` ; test permanent en assertion BE | B2-03 (1) | — |

