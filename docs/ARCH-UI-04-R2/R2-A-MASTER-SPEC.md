# ARCH-UI-04-R2-A — spécification maîtresse

Version 1 du 2026-11-28 · mission documentaire · base fonctionnelle 0.4.1 (`d2d08ac`) · revue Codex `dbc21c8`.

## 1. Objet

Ce document fixe le cadre commun des spécifications R2 : ce qui est figé, ce qui est décidé, comment les exigences sont
formulées et vérifiées, et en quoi les arbitrages DA-A01..DA-A15 modifient les contrats d'interface antérieurs. Les
spécifications détaillées sont dans les documents dédiés ([README](README.md)). Ce document ne répète pas leur contenu.

## 2. Hiérarchie des sources

Lorsqu'une exigence se contredit entre deux sources, la plus haute l'emporte. La contradiction est alors consignée au §7.

| Rang | Source | Portée |
|---|---|---|
| 1 | DSP FINAL LOCK révisé 0.4.1 (`../V1-DSP-FINAL-LOCK.md` §7), contrat d'autonomie 0.4.1 (`../V1-AUTONOMY-CONTRACT.md` §2 bis), causalité B-T2, contrat Z (`../V1-Z-TRANSFORMS-CONTRACT.md`) | Comportement sonore, autorisation, topologie. **Non modifiable par R2.** |
| 2 | Registre compilé : 703 StableID, noms hôtes, types, plages, format d'état 4 (`source/core/ParamTable.inc`) | Identités et automation. **Non modifiable par R2.** |
| 3 | Arbitrages DA-A01..DA-A15 du propriétaire ([R2-DECISION-REGISTER.md](R2-DECISION-REGISTER.md)) | Direction artistique, nomenclature, organisation |
| 4 | Ce référentiel R2 | Traduction mesurable des rangs 1 à 3 |
| 5 | Contrats d'interface antérieurs (`../V1-UI-UX-DESIGN.md`, `../V1-ARCH-UI-03-SPEC.md`, `../V1-ARCH-UI-04-DESIGN-SYSTEM.md`) | Restent en vigueur sauf points remplacés listés au §7 |
| 6 | Revue Codex (`../ARCH-UI-04-DA/`) | Constats et mesures. Ses propositions ne sont pas normatives. |

## 3. Invariants

Toute livraison R2-B à R2-H, ARCH-UI-05 et ARCH-UI-06 DOIT respecter les invariants suivants. Leur vérification est
reprise dans [R2-ACCEPTANCE-MATRIX.md](R2-ACCEPTANCE-MATRIX.md) (série INV).

| ID | Invariant |
|---|---|
| INV-01 | `git diff d2d08ac -- source/` vide. Les rendus de référence (REG-01, goldens) restent identiques. Sans autorisation distincte, aucune modification des algorithmes DSP, des moteurs WT/FM/CROSS, des filtres, de la Destruction Engine, de la modulation, de la causalité, de la latence ni de TRANSPORT AUTHORIZATION. Toute exception exige un rapport distinct et l'autorisation préalable du propriétaire, comme pour R1. |
| INV-02 | Les 703 StableID, leur ordre, leurs noms hôtes, types, plages, unités, défauts et indices de choix restent inchangés, ainsi que la sémantique des presets. `tests/core/expected_param_ids.txt` reste inchangé. Les libellés courts R2 sont des **alias d'affichage**, indépendants de l'identité technique du paramètre. |
| INV-03 | Le format d'état reste en version 4. Un choix de vue (NEURAL/TABLE, repli, onglet, zoom) n'écrit aucun paramètre et ne marque pas le preset comme modifié. |
| INV-04 | Le contrat d'autonomie 0.4.1 reste respecté. ARM est une configuration persistée. AUTHORIZE est une autorisation d'instance non persistée. L'autorisation de rendu est temporaire. Le verrou PANIC se lève uniquement selon le contrat. FREEZE et CAPTURE restent des opérations distinctes. |
| INV-05 | La topologie FM/CROSS (55 emplacements), les classes causales (I/D, figées), l'ordre du signal et l'ordre effectif des transformations Z restent ceux du moteur. Aucune vue ne peut créer de route hors des emplacements contractuels, ni modifier une classe causale. |
| INV-06 | Les gestes hôte (`beginChangeGesture`/`endChangeGesture` via `juce::ParameterAttachment`) et la granularité Undo restent inchangés. Une action utilisateur produit une seule transaction. |
| INV-07 | Les 703 StableID restent accessibles depuis l'interface (UI04-COV), y compris les paramètres conditionnels et inactifs. |
| INV-08 | Aucun rendu 3D avant ARCH-UI-05. Aucune nouvelle dépendance, aucun nouveau SDK, aucune licence nouvelle. |
| INV-09 | Les réserves R-29 et R-30, les limites d'API VST3 (`../V1-AUTONOMY-CONTRACT.md` §5) et les neuf échecs historiques (`tests/known_failures.txt`) restent consignés tels quels. Un essai non exécuté n'est jamais déclaré PASS. |
| INV-10 | PB-01 et PB-02 restent au backlog pré-BETA. Un lot qui modifie les gestes concernés DOIT les traiter dans un périmètre autorisé explicitement (voir [R2-RISKS-AND-OPEN-DECISIONS.md](R2-RISKS-AND-OPEN-DECISIONS.md) OD-10). |
| INV-11 | Aucune mesure, aucune activité de signal, aucune valeur n'est affichée si la télémétrie ne la publie pas ([R2-TELEMETRY-FEASIBILITY.md](R2-TELEMETRY-FEASIBILITY.md)). Une donnée illustrative est marquée comme telle. |
| INV-12 | Le rouge (`tok::alert`) reste réservé aux alertes et à PANIC. La couleur n'est jamais le seul porteur d'un état. Chaque état porte aussi une forme et un texte. |
| INV-13 | Le contrôle de `source/` ne suffit pas. Toute modification de `plugin/` ou de `ui/` susceptible d'affecter le comportement audio est contrôlée : écritures de paramètres hors geste, timers, capture, autorisation, chargement d'état. Le contrôle combine REG-01, la suite TA et un test « aucune écriture pendant la navigation ». |

## 4. Identité : SYMBIOTIC INDUSTRIAL Hybrid

DA-A02 retient une identité **hybride**, de signature *Mechanical precision · Cybernetic intelligence · Living matter*.
L'instrument évoque une machine physique haut de gamme qui intègre une intelligence cybernétique et un noyau
biomécanique vivant. Le partage est fonctionnel :

| Registre | Où | Matériaux et moyens |
|---|---|---|
| **Précision mécanique** | Tout ce qui se manipule : commandes, touches, plaques, barre, sérigraphie | Graphite minéral, carbone sombre, titane noir usiné, périphéries usinées ou moletées, relief mécanique |
| **Intelligence cybernétique** | Tout ce qui informe : afficheurs, réseaux, Inspector, état d'autonomie | Verre optique, graduations techniques, marqueurs lumineux maîtrisés, données dynamiques **publiées** (INV-11) |
| **Matière vivante** | **Le Core seulement** (matière biomécanique réservée au Core) | Structure organique et technologique, animation de repos discrète, réactivité musicale maîtrisée (ARCH-UI-05) |

Interdictions du brief : esthétique rétro, écrans CRT, lignes de balayage décoratives, néons excessifs, effets qui
réduisent la lisibilité, textures envahissantes, faux instruments de mesure, animations qui laissent croire à une
activité DSP inexistante (DS-05). La sémantique des couleurs de sécurité et des états existants est conservée.

La palette de référence (`ui/DesignTokens.h`) est conservée telle quelle. Les nouveaux tokens R2 sont des dérivés
([R2-DESIGN-SYSTEM-SPEC.md](R2-DESIGN-SYSTEM-SPEC.md) §2).

## 5. Organisation produit

| Espace / volet | Arbitrage | Rôle | Spécification |
|---|---|---|---|
| PERFORMANCE | DA-A08 Adaptive Instrument | Jouer. Lecture critique permanente, gestes majeurs | [R2-WORKSPACE-SPEC.md](R2-WORKSPACE-SPEC.md) §2 |
| SOUND DESIGN | DA-A09 Adaptive Engine Workspace | Programmer les moteurs, les chaînes et la modulation, dans l'ordre du moteur | [R2-WORKSPACE-SPEC.md](R2-WORKSPACE-SPEC.md) §3 |
| ENGINEERING | DA-A10 Engineering Workbench | Réseaux, matrice, Bank Lab, état, outils | [R2-WORKSPACE-SPEC.md](R2-WORKSPACE-SPEC.md) §4 |
| Inspector | DA-A11 Adaptive Inspector | Détail du paramètre sélectionné, transversal | [R2-WORKSPACE-SPEC.md](R2-WORKSPACE-SPEC.md) §5 |
| Réseaux | DA-A12, A13, A14 | FM Network, CROSS Network, Global Modulation Matrix | [R2-NEURAL-NETWORK-SPEC.md](R2-NEURAL-NETWORK-SPEC.md) |
| Core | DA-A07 Hybrid Reactive Core | Pilotage CROSS X/Y/Z | [R2-CORE-INTEGRATION-CONTRACT.md](R2-CORE-INTEGRATION-CONTRACT.md) |
| Flux du signal | DA-A15 Reactive Signal Flow | Lecture de l'activité réelle | [R2-TELEMETRY-FEASIBILITY.md](R2-TELEMETRY-FEASIBILITY.md) |

## 6. Langue de l'interface (DA-A01)

L'interface visible DOIT être en **anglais technique compact**. Cela couvre la sérigraphie, les titres, les onglets, les
états, les actions, les champs de l'Inspector, les infobulles et les messages d'état. Le registre et les noms hôtes sont
déjà en anglais et ne changent pas (INV-02). L'orientation française des propositions Codex est **rejetée**. Ses
mesures, budgets et constats restent utilisés ([R2-CONTROL-NAMING-SPEC.md](R2-CONTROL-NAMING-SPEC.md) §9). La
documentation sera, à terme, en français et en anglais (brief §7). Le calendrier relève d'OD-03.

## 7. Divergences avec les contrats antérieurs

Les divergences ci-dessous sont des **remplacements à consigner** dans les contrats d'origine au moment du lot concerné.
Les contrats historiques ne sont pas modifiés par R2-A.

| ID | Contrat antérieur (énoncé) | Arbitrage | Résolution retenue par R2-A | Statut |
|---|---|---|---|---|
| DV-01 | `V1-UI-UX-DESIGN.md` l.78 : la vue d'ensemble de modulation sert principalement à comprendre, filtrer et naviguer, l'édition détaillée se fait dans List/Inspector. `V1-ARCH-UI-03-SPEC.md` l.29 : vue d'ensemble en lecture/navigation, pas de patcher libre. | DA-A12 D Neural Network, DA-A14 C Dual View | La vue NEURAL devient une vue principale **éditable**. Elle n'édite que les emplacements contractuels : profondeurs, ajout et retrait par transaction validée. Elle ne crée jamais de connexion libre (INV-05). L'interdiction du patcher libre est **maintenue**. Le rôle « lecture seule » est **remplacé**. | Remplacement partiel à consigner (R2-F) |
| DV-02 | `V1-ARCH-UI-04-DESIGN-SYSTEM.md` et `ui/DesignTokens.h` : afficheurs en « verre optique fumé », sobres. `V1-UI-UX-DESIGN.md` l.22 (déjà remplacé) : CRT/OLED stylisé. | DA-A05 **D** Cybernetic Display, **et non** Optical Glass Hybrid | Instrumentation résolument futuriste : graphismes cybernétiques, graduations techniques, marqueurs lumineux maîtrisés, données dynamiques, encadrements et repères optiques, hiérarchie stricte. Le verre optique reste un matériau de l'identité (fond, monture), mais il n'est plus le style dominant. CRT, rétro et balayage restent exclus. | Remplacement à consigner (R2-B) |
| DV-03 | Chaînes visibles françaises dans `ui/` (onglets, actions, états, Inspector) | DA-A01 C | Passage complet en anglais ([R2-CONTROL-NAMING-SPEC.md](R2-CONTROL-NAMING-SPEC.md)) | Remplacement (R2-B) |
| DV-04 | `V1-ARCH-UI-04-DESIGN-SYSTEM.md` : libellé = nom du registre sans préfixe de groupe | DA-A01 C | Libellé = alias de sérigraphie contextuel (lexique R2). Le nom hôte apparaît dans l'Inspector et l'infobulle. | Remplacement (R2-B) |
| DV-05 | `ui/DesignTokens.h` : Inspector fixe de 300 px. Performance minimale 1080 × 812. Barre sur deux rangées à 1440 px. | DA-A08 C, DA-A11 C | Disposition par seuils et Inspector adaptatif. L'objectif est d'éviter le défilement en PERFORMANCE au nominal et au minimum ([R2-WORKSPACE-SPEC.md](R2-WORKSPACE-SPEC.md) §2.3). | Remplacement (R2-C, R2-E) |
| DV-06 | ENGINEERING : dix onglets plats | DA-A10 C | Cinq groupes (NETWORK, MODULATION, BANK LAB, STATE, TOOLS) pour les dix outils conservés | Remplacement (R2-E) |
| DV-07 | Tests historiques fondés sur des chaînes visibles françaises ou des indices d'onglets (AUT-08, TA-07w, UI-01w, UI04-OPS, …) | DA-A01, DA-A10 | Les tests ne sont pas modifiés par R2-A. La stratégie d'identification (titres stables, identifiants de composant) reste à autoriser (OD-01). | Décision ouverte |
| DV-08 | **Code ↔ contrat** : `ui/DesignTokens.h` l.2 « aucun composant ne définit de dimension en dur », mais `PerformanceView::resized` emploie 196, 300 et 330 px en dur et `MainEditor::layoutBar` des largeurs de boutons en dur. `tok::stripH = 92` n'est pas utilisé (bandeau réel : 196 px). | DA-A08 | Ces dimensions deviennent des tokens de disposition par seuil (R2-C). Le contrat n'est pas modifié. | Écart constaté, à corriger en R2-C |
| DV-09 | **Code ↔ moteur** : l'afficheur indique « SPECTRE sortie moteur », alors que l'oscilloscope et la crête sont prélevés en sortie finale (après OUT GAIN, limiteur et rampe PANIC), en mono, une valeur sur deux | DA-A05 | Point de prélèvement nommé exactement (CD-02) | Écart de libellé, à corriger en R2-B |

## 8. Glossaire normatif

| Terme | Définition | Ne pas confondre avec |
|---|---|---|
| Base | Valeur du paramètre, celle que l'hôte automatise et que le preset enregistre | Effective |
| Effective | Valeur appliquée par le moteur après modulation, publiée par la télémétrie (`effective[703]`, décimée par bloc) pour les seules destinations modulées. Sinon Effective = Base. | Mesure audio |
| Mesure | Grandeur audio mesurée sur le signal (crête de sortie dBFS, oscilloscope, spectre) | Valeur de paramètre |
| ARM | Configuration persistée : autoriser un module à fonctionner de manière autonome (`*.arm`, `voice.drone`) | AUTHORIZE |
| AUTHORIZE | Autorisation d'exécution autonome pour cette instance (explicite, transport ou enregistrement). Non persistée. | ARM, rendu |
| Autorisation de rendu | Autorisation temporaire pendant un rendu hors ligne, close au premier bloc temps réel (0.4.1) | AUTHORIZE |
| FREEZE | Paramètre de gel du contenu D2/S2 (automatisable) | CAPTURE |
| CAPTURE | Action qui mémorise le contenu gelé D2/S2 dans l'état du preset et du projet | FREEZE |
| PANIC | All Sound Off et révocation, avec verrou selon le contrat | STOP |
| Route inexistante | Paire source/destination non autorisée par la topologie (55 emplacements FM/CROSS) ou non admise par la matrice | Route absente |
| Route disponible inactive | Emplacement autorisé sans route présente. La profondeur est conservée sans effet. | Profondeur 0 |
| Route active de profondeur nulle | Route présente, profondeur 0 | Route absente |
| Classe causale | I (Immediate) ou D (Delayed, T2), figée à la création (B-T2) | Signe de la profondeur |
| Profondeur configurée | Valeur Base de la profondeur d'une route | Activité instantanée |
| Activité instantanée | Grandeur signal par route, **non publiée** à ce jour ([R2-TELEMETRY-FEASIBILITY.md](R2-TELEMETRY-FEASIBILITY.md)) | Profondeur effective (`effectiveDepth[55]`, publiée) |
