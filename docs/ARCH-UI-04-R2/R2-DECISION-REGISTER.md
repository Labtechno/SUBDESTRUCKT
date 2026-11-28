# ARCH-UI-04-R2-A — registre des décisions

Ce registre consigne les **15 arbitrages validés** par le propriétaire. Ils ne sont pas rouverts par R2-A. Chaque
arbitrage est accompagné de son texte normatif, de ses limites et des exigences qu'il produit. Les modalités que le
propriétaire n'a pas validées sont classées à part (§3). Elles ne sont pas normatives.

## 1. Arbitrages validés

| ID | Domaine | Option retenue | Texte normatif (traduction R2) | Ce que l'arbitrage n'autorise pas | Exigences dérivées |
|---|---|---|---|---|---|
| DA-A01 | Langue et nomenclature | **C — English technical compact** | Toute l'interface visible est en anglais. La sérigraphie est compacte, en majuscules, et dépend du contexte. Le nom complet anglais apparaît au focus, dans l'Inspector et dans l'infobulle. | Renommer les noms hôtes ou les StableID. Abréger au point de créer une homonymie. Appliquer automatiquement les 1 102 propositions Codex. | NAM-01..NAM-14 |
| DA-A02 | Identité globale | **C — SYMBIOTIC INDUSTRIAL Hybrid** | Le châssis industriel porte des signaux vivants issus du moteur (master §4) | Un décor animé sans donnée. Le style CRT ou rétro. L'abandon de la palette de référence. | DS-01..DS-05 |
| DA-A03 | Commandes continues | **C — NEXUS Hybrid** | Rotatif usiné : index de Base, anneau d'Effective, marques de modulation. Trois tailles (XL, S, Micro). | Changer les gestes ou la granularité de l'automation | NX-01..NX-12 |
| DA-A04 | Interrupteurs et actions | **C — NEURAL CELL Hybrid** | Cellule à six types distincts (action instantanée, bascule, armement, sélection exclusive, sûreté, indisponible). L'état est porté par la forme et le texte. | Fusionner FREEZE, ARM, AUTHORIZE et CAPTURE. Modifier PANIC. | NC-01..NC-10 |
| DA-A05 | Afficheurs | **D — Cybernetic Display** (et non Optical Glass Hybrid) | Instrumentation futuriste : graduations techniques, marqueurs lumineux maîtrisés, données dynamiques, encadrements optiques, hiérarchie stricte. S'applique aux valeurs, courbes, enveloppes, tables WT, analyseurs, crêtes, états et diagnostics. | Afficher une mesure non publiée. Le balayage CRT. Les faux instruments de mesure. | CD-01..CD-08 |
| DA-A06 | Canaux | **C — Plasma Hybrid** | Curseur mécanique, canal lumineux de Base, marqueur d'Effective indépendant, graduations et unité lisibles. La mesure audio est visuellement distincte. | Confondre gain réglé et crête mesurée | PL-01..PL-06 |
| DA-A07 | Core | **C — Hybrid Reactive Core** | Core central X = `cross.fwd`, Y = `cross.rev`, Z = `cross.master`, réactif aux valeurs Effective. Alternative numérique et repli 2D. | Livrer un rendu 3D avant ARCH-UI-05. Changer les axes. Laisser l'animation compromettre les gestes. | CORE-01..CORE-16 |
| DA-A08 | PERFORMANCE | **C — Adaptive Instrument** | L'organisation s'adapte à la taille et à l'Inspector. Les éléments essentiels ne sont jamais masqués sans indication. L'objectif est d'éviter le défilement au nominal et au minimum. | Masquer PANIC, la crête de sortie ou l'état d'autonomie. Faire défiler la PERFORMANCE au nominal à 100 %. | WS-P01..WS-P16 |
| DA-A09 | SOUND DESIGN | **C — Adaptive Engine Workspace** | WT, FM et CROSS restent simultanément accessibles. La répartition des surfaces suit le contexte d'édition explicite (WT XYZ, WT Z, FM, CROSS, PROCESS, MOD), dans l'ordre réel du moteur. | Redistribuer les zones pendant un geste. Réordonner le DSP ou changer les capacités des moteurs. Rendre inaccessible un paramètre conditionnel. | WS-S01..WS-S10 |
| DA-A10 | ENGINEERING | **C — Engineering Workbench** | Cinq groupes : NETWORK (FM, CROSS), MODULATION (Mod Matrix, MIDI Learn), BANK LAB, STATE (Voice/Tuning, Presets/State, Captures/Assets), TOOLS (Randomize, Diagnostics). Les dix outils sont conservés. | Supprimer ou fusionner un outil. Dégrader significativement les parcours actuels. | WS-E01..WS-E09 |
| DA-A11 | Inspector | **C — Adaptive Inspector** | Ordre d'affichage : 1 nom complet, 2 Base, 3 Effective, 4 plage et unité, 5 modulations, 6 automation, 7 détails techniques, 8 StableID et diagnostics. Mode compact selon l'espace. | Masquer Base ou Effective en mode compact. Monopoliser une part disproportionnée de la façade. | WS-I01..WS-I10 |
| DA-A12 | Représentation des réseaux | **D — Neural Network** | FM Network, CROSS Network et Global Modulation Matrix sont représentés comme des réseaux de nœuds et de liaisons | Un patcher libre. Une modification de topologie ou de causalité. Un modèle de routage artificiellement identique pour les trois réseaux. | NN-01..NN-08, NN-26, NN-27 |
| DA-A13 | Adaptativité des réseaux | **C — Adaptive Neural Network** | Positions des nœuds stables. Filtres, focus et regroupement suivent l'usage. Lisibilité garantie à la densité maximale. | Déplacer des nœuds spontanément. Réorganiser pendant un geste. | NN-09..NN-16, NN-25 |
| DA-A14 | Double vue | **C — Dual View** | Vues NEURAL et TABLE sur les mêmes paramètres, le même état, les mêmes identités de route, avec sélection synchronisée et transactions identiques. Changer de vue n'écrit rien. | Deux modèles de données. Une vue qui écrit l'état. | NN-17..NN-24 |
| DA-A15 | Flux du signal | **C — Reactive Signal Flow** | Le flux réagit à l'activité réelle publiée par le moteur | Une animation décorative. Une donnée non publiée. Une modification du moteur sans autorisation distincte. | TEL-01..TEL-09 |

Les identifiants d'exigences renvoient à [R2-ACCEPTANCE-MATRIX.md](R2-ACCEPTANCE-MATRIX.md).

## 2. Éléments dérivés (conséquences directes, normatives)

| ID | Élément | Arbitrages | Justification |
|---|---|---|---|
| DE-01 | Les libellés courts sont des alias d'affichage. Le nom hôte reste celui du registre. | DA-A01 + INV-02 | L'automation et les sessions existantes dépendent des noms hôtes |
| DE-02 | Une même fonction porte le même libellé court dans tous les espaces. Un même libellé ne désigne jamais deux fonctions dans un même champ visuel. | DA-A01 | Constat Codex DA-09 : homonymie « Z Master » |
| DE-03 | La grammaire de route est unique : `SOURCE→DEST`, flèche unique, ports en majuscules | DA-A01, A12, A14 | Constat Codex DA-08 |
| DE-04 | La crête de sortie (PEAK, dBFS) est un afficheur de mesure permanent. Elle n'est pas un libellé du gain. | DA-A06, A08 | Codex P1-01 |
| DE-05 | Les vues NEURAL et TABLE partagent les composants de liaison (`ParamLink`) et les transactions (`Topology` / `ModMatrix`) | DA-A14 | Aucun second modèle |
| DE-06 | Le Core accepte le geste XY sur le plan X/Y et le geste Z sur un canal dédié. Les trois axes restent pilotables au clavier et par saisie numérique. | DA-A07 | Accessibilité, automation |
| DE-07 | Les cinq groupes ENGINEERING sont des niveaux de navigation, pas de nouveaux espaces | DA-A10 | Les trois espaces restent |
| DE-08 | Toute animation est une projection d'une donnée publiée. Trois cadences distinctes (C05) : lecture de la télémétrie (30 Hz en 0.4.1), rafraîchissement des composants (à la lecture ou à l'événement) et rendu du futur Core 3D (fixé par ARCH-UI-05, pas imposé à 30 FPS). Une interpolation visuelle ne crée pas de donnée. | DA-A02, A05, A15 | INV-11 |

## 3. Propositions non validées (non normatives)

| ID | Proposition | Origine | Statut |
|---|---|---|---|
| PR-01 | Modes de réactivité du flux OFF / REACTIVE / IMMERSIVE | Brief R2-A | Proposition. Sémantique décrite dans [R2-TELEMETRY-FEASIBILITY.md](R2-TELEMETRY-FEASIBILITY.md) §5. Arbitrage OD-07. |
| PR-02 | Seuils de disposition PERFORMANCE (WIDE / COMPACT / MINIMAL) | R2-A | Seuils candidats ([R2-WORKSPACE-SPEC.md](R2-WORKSPACE-SPEC.md) §2.4). Arbitrage OD-04. |
| PR-03 | Inspector adaptatif DOCKED / BAND / DRAWER. Passage hors de DOCKED quand l'ancrage ramènerait le canvas logique sous 1 080 px. | Codex DA-12, P1-04, R2-A | Seuil candidat. Arbitrage OD-05. |
| PR-04 | Plancher typographique de 9 px écran pour les repères critiques à 75 % | Codex P1-05 | Seuil candidat. Arbitrage OD-06. |
| PR-05 | Boutons de barre « Ouvrir » / « Enregistrer » renommés PRESETS et SAVE… (navigation vers Presets/State) | R2-A, constat Codex | Proposition de nomenclature (CSV) |
| PR-06 | Libellés contextuels FM INDEX / FM FDBK en PERFORMANCE | ARCH-UI-03 §8 (non arbitré) | Conservés tels quels, à arbitrer (OD-08) |
| PR-07 | Libellés courts du CSV hors exemples du propriétaire | R2-A | « Proposé R2-A — validation propriétaire » |
| PR-08 | Exemples du brief (X, WT Z, CROSS LVL, FREQ, RES, FDBK, FB MODE, ATK, REL, MIX, GAIN, ARM, FREEZE, AUTHORIZE, CAPTURE, BASE, EFFECTIVE) | Propriétaire, à titre d'orientation | « Orientation propriétaire (§7) — à confirmer R2-B ». Écart R2-A : `out.gain` → OUT GAIN (et non GAIN), pour le distinguer du GAIN de sortie des étages. |
| PR-09 | Rattacher l'application de la nomenclature à R2-B (le brief définit R2-B comme « composants et tokens graphiques ») | R2-A | Proposition (OD-18) |
| PR-10 | Sous-lots R2-F1 (réseaux fixes FM, CROSS) et R2-F2 (matrice configurable) | R2-A, brief §19 | Proposition (OD-19) |

## 3 bis. Arbitrages complémentaires (R2-B1)

OD-01 B, OD-02 B, OD-06 B, OD-10 B, OD-11 B, OD-17 A, OD-18 A : détail dans `R2-RISKS-AND-OPEN-DECISIONS.md` §2 bis.
PR-09 (nomenclature rattachée à R2-B) est donc retenue (OD-18 A).

Corrections issues de la contre-expertise de R2-A, intégrées en R2-B1 :

| ID | Correction | Documents |
|---|---|---|
| C01 | Les 2 213 occurrences sont mesurées avec les polices JUCE (PT-02). Les largeurs du CSV R2-A restent des estimations. | `B1-COMPONENT-LAB/TYPOGRAPHY-MEASUREMENTS.csv` |
| C02 | Les outils de génération et de vérification sont versionnés (`lab/tools/`). Le CSV se régénère à l'identique, octet pour octet. | `lab/README.md` |
| C03 | Le spectre est indicatif dans toute sa bande affichée, la limite à fs/4 n'empêche pas la contamination par repliement | `R2-TELEMETRY-FEASIBILITY.md` T-19, RK-17, CD-06 |
| C04 | STOP, SUSPENDED et silence sont trois états distincts ; SUBDESTRUCKT peut produire un signal transport arrêté | TEL-05 |
| C05 | Trois cadences distinctes ; 30 FPS non imposé au rendu ARCH-UI-05 | DE-08, TEL-04, contrat Core §7 |
| C06 | `tests/known_failures.txt` : dix entrées = huit échecs documentés + deux critères remplacés (ZC-15, TA-15) | INV-09, réserves |

## 3 ter. Arbitrages artistiques du Component Lab (R2-B1-R1)

| ID | Option | Décision |
|---|---|---|
| LAB-A01 | A | Conservation des NEXUS Hybrid (géométrie, moletage XL, biseau S, Micro, index, graduations, arc BASE, marqueur EFFECTIVE, fenêtre numérique, états) |
| LAB-A02 | B | NEURAL CELL Machined Titanium |
| LAB-A03 | B | SYNAPTIC Machined Track |
| LAB-A04 | C | Cybernetic HUD (neuf variantes) |
| LAB-A05 | A | Conservation de l'apparence PLASMA |
| LAB-A06 | C | Symbiotic Industrial Hybrid |
| LAB-A07.1 | B | PANIC : libellé carbone sur rouge (≥ 4,5:1 sur la surface rendue) |
| LAB-A07.2 | C | PLASMA vertical STANDARD 64 x 180 et COMPACT 48 x 150, même composant |
| LAB-A08 | B | Nomenclature P1 optimisée : AMOUNT et MODE sous CROSS AUDIO (XAUDIO abandonné), KEEP FUND, OUT GAIN ; P2 et P3 restent sous OD-02 |

Application et preuves : `B1-R1-FINAL-ARTISTIC-REFINEMENT/LAB-ARTISTIC-DECISIONS.md`. PR-05 et PR-08 sont
précisées par LAB-A08 ; la variante PANIC et le gabarit PLASMA vertical, soumis en B1, sont tranchés.

## 3 quater. Validation artistique finale (AR-15) et gel du Design System R1

| ID | Option | Décision | Date |
|---|---|---|---|
| AR-15 | A | Validation artistique complète du Component Lab R1 : **PASS** (décision humaine). B1-R1 GO FINAL ; Design System R1 gelé (NEXUS Hybrid, NEURAL CELL Machined Titanium, SYNAPTIC Machined Track, Cybernetic HUD, PLASMA STANDARD et COMPACT, PANIC carbone sur rouge, tokens Symbiotic Industrial Hybrid, nomenclature R1) ; R2-B2 autorisé en commençant par R2-B2-00 seul. Toute modification du Design System R1 exige un nouvel arbitrage. | 2026-11-28 |

Trace : `B1-R1-FINAL-ARTISTIC-REFINEMENT/OWNER-DECISION-AR15.md`. Décisions ouvertes par R2-B2-00 : DB2-01 à DB2-07
(`B2-PRODUCTION-INTEGRATION/B2-00-DOSSIER.md` §12).

## 3 quinquies. Arbitrages R2-B2 (DB2) et amendements de contre-expertise (CE-B2)

| ID | Option | Décision | Date |
|---|---|---|---|
| DB2-01 | C | M3 de base minimum officiel ; M1 et M2 hors support ; mesures M1 complémentaires autorisées sans extension du support | 2026-11-28 |
| DB2-02 | A | Gestes PLASMA 0.4.1 conservés ; PL-06 amendé | 2026-11-28 |
| DB2-03 | B | P2, P3 et microcopie validés par lots groupés en B2-04 ; interface finale entièrement en anglais ; NAM-01 à la clôture | 2026-11-28 |
| DB2-04 | A | 0.4.1 pendant l'intégration ; 0.5.0 préparée à la clôture, sur autorisation finale | 2026-11-28 |
| DB2-05 | A | Monture HUD R1 seule pour les afficheurs sans variante validée | 2026-11-28 |
| DB2-06 | A | Habillage R1 des widgets JUCE génériques par le LookAndFeel | 2026-11-28 |
| DB2-07 | B | Seuils GS-01 à GS-07 définitifs (amendés) | 2026-11-28 |
| CE-B2-01 à 05 | — | Tests de reproduction permanents ; géométrie PLASMA par surface utile ; indépendance des preuves visuelles ; robustesse de GC-01 ; corrections EC-01 à EC-03 autorisées | 2026-11-28 |

Texte normatif : `B2-PRODUCTION-INTEGRATION/B2-00-ADDENDUM.md`. R2-B2-00 : GO ; R2-B2 : exécution autorisée.

## 4. Orientation rejetée

| ID | Orientation | Motif |
|---|---|---|
| RJ-01 | Propositions de libellés Codex en français (`CONTROL-LABELS-PROPOSALS.csv`, 1 102 lignes) | Contraire à DA-A01. Les mesures, budgets, familles, priorités et constats Codex sont conservés comme données d'entrée ([R2-CONTROL-NAMING-SPEC.md](R2-CONTROL-NAMING-SPEC.md) §9). |

## 5. Décisions héritées, toujours en vigueur

| Décision | Référence |
|---|---|
| DSP FINAL LOCK révisé 0.4.1 | `../V1-DSP-FINAL-LOCK.md` §7 |
| Autorisation de rendu temporaire et ressynchronisation | `../V1-AUTONOMY-CONTRACT.md` §2 bis |
| Restriction d'ordre inter-domaines des transformations Z, **non validée** et à représenter fidèlement | `../V1-Z-TRANSFORMS-CONTRACT.md` §3 ter |
| Balance audio post-Amp, avant les chaînes (B-02) | tracker §7 |
| Backlog PB-01 / PB-02, non corrigé | tracker, Backlog pré-BETA |
| Note descriptive Codex 6,7/10 et contre-expertise 5,7/10 : constats, pas critères | `../ARCH-UI-04-DA/SCORECARD.md` |
