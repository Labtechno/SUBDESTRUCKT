# ARCH-UI-04-R2-B2-00 — dossier d'exécution (audit préalable et contractualisation)

Date : 2026-11-28. **Aucune intégration de production n'est engagée.** Ce dossier est soumis à la contre-expertise de
ChatGPT. Les étapes R2-B2-01 à 08 attendent son GO.

## 1. AR-15

**AR-15 : PASS** par décision humaine du propriétaire (option A, validation artistique complète, 2026-11-28).

- ARCH-UI-04-R2-B1-R1 : **GO FINAL**.
- Design System R1 : **gelé**.
- LAB-A01 à LAB-A08 : validés.
- Cette validation ne vaut ni validation fonctionnelle du plugin intégré ni recette physique.

Enregistrement :

- [`../B1-R1-FINAL-ARTISTIC-REFINEMENT/OWNER-DECISION-AR15.md`](../B1-R1-FINAL-ARTISTIC-REFINEMENT/OWNER-DECISION-AR15.md) ;
- addenda B1-R1 ;
- registre §3 quater ;
- tracker §3duodecies.

Les preuves historiques ne sont pas réécrites.

## 2. État du dépôt (vérifié avant toute intervention)

| Contrôle | Résultat |
|---|---|
| HEAD local / distant | `73b633b1c382dd5303b58a3889ea8efeca9e0569` / identique (`git ls-remote`) |
| Divergence | 0 / 0 |
| Modifications locales | aucune |
| DSP FINAL LOCK | `git diff d2d08ac -- source/` **vide** |
| Version de production | 0.4.1 |
| Documents relus | tracker, DSP FINAL LOCK, R2 (design system, registre, risques, nomenclature, matrice d'acceptation, plan d'exécution, télémétrie), CSV R1, COMPONENT-DESIGN-FINAL, rapport et décisions B1-R1, plan de tests B1 (OD-01, PB-01, PB-02), exigences de plateforme |

**Contradictions normatives relevées.** Aucune n'est résolue en silence ; toutes figurent au registre des écarts.

| Écart | Contradiction | Traitement |
|---|---|---|
| EC-07 | Matériel minimum : M1 (mission) ou M3 de base (révision validée du 2026-11-25) | DB2-01 |
| EC-05 | Gestes PLASMA : PL-06, 0.4.1 et laboratoire divergent | DB2-02 |
| EC-08 | NAM-01 face à OD-02 (P2 et P3 non validés) et à la microcopie hors CSV | DB2-03 |
| EC-15 | OD-17 (0.5.0) face à l'interdiction de changer de version sans autorisation | DB2-04 |
| EC-04 | NAM-07 cite XAUDIO AMT, remplacé par LAB-A08 | Conséquence directe d'un arbitrage : addendum consigné dans la matrice d'acceptation |

## 3. Cartographie de l'existant

Détail : [B2-00-COMPONENT-MAP.md](B2-00-COMPONENT-MAP.md). Matrice composant par composant :
[B2-CONFORMITY-MATRIX.md](B2-CONFORMITY-MATRIX.md) ([CSV](B2-CONFORMITY-MATRIX.csv), 32 lignes).

En résumé :

- **Production.** `ContinuousControl` porte à la fois NEXUS et PLASMA. `SynapticTrack`, `NeuralCell` et `ActionCell`
  (PANIC compris) complètent les composants liés. `TabStrip`, `Plate` et `OpticalDisplay` servent la navigation et la
  présentation. Les moniteurs et les éditeurs de courbes sont à part. `SymbioticLook` habille les widgets JUCE
  génériques, très présents en ENGINEERING. Les tests dépendent des types (`ContinuousControl`, `ToggleButton`,
  `TextButton`, `ComboBox`), de l'interface `BoundControl` et de textes français. Aucun identifiant `setComponentID`.
- **Laboratoire.** Ses rendus R1 sont intégrables. Ses gestionnaires d'événements ne le sont pas : simplifications et
  divergences avec 0.4.1 (EC-10).
- **Données.** Celles de classes A et B suffisent pour toutes les variantes HUD validées. Les données C et D restent
  absentes et ne sont pas simulées.

**Écarts préexistants trouvés** :

| Écart | Constat |
|---|---|
| EC-01 | Point de prélèvement de l'oscilloscope faux à l'écran |
| EC-02 | Spectre non marqué INDICATIVE |
| EC-03 | Mesure de crête dessinée dans le canal OUT GAIN, contraire à PL-04 |

## 4. Stratégie d'intégration

### 4.1 Choix techniques (soumis à la contre-expertise)

| ID | Choix | Justification |
|---|---|---|
| TB2-01 | **Habillage en place.** Les classes de production et leurs interfaces sont conservées (`BoundControl`, `ParamLink`, gestionnaires d'événements 0.4.1). Seul le rendu est remplacé par les peintres R1. Les classes du laboratoire ne remplacent pas celles de la production. | Contrats d'interaction 0.4.1 préservés (NX-05) ; aucune transposition des simplifications du laboratoire ; tests historiques valides sans changement de type ; commits réversibles par famille |
| TB2-02 | **Module partagé `ui/r2/`** : tokens dérivés, primitives `draw::*`, peintres purs (fonctions d'un modèle de vue et d'un état), `GestureGuard`, interface `ValueSource`. Le plugin et le laboratoire le consomment tous deux ; le laboratoire délègue son rendu aux mêmes peintres. | Aucune duplication. La non-régression pixel à pixel du laboratoire contre les captures R1 publiées prouve que l'extraction n'introduit pas de dérive (INT-02, INT-20). Les preuves B1 et B1-R1 restent reproductibles depuis leurs SHA. |
| TB2-03 | Choix du gabarit PLASMA vertical par **largeur et hauteur** : STANDARD si au moins 64 × 180, COMPACT si au moins 48 × 150, centré dans la cellule | Les cellules 0.4.1 (OUT GAIN 96 × 158 ; Z du Core 74 × variable) accueillent COMPACT sans déformation et sans réorganisation (EC-12). STANDARD sera placé en R2-C. |
| TB2-04 | Registre `ui/r2/ComponentIds.h` (ID-03), en ajout seulement. Migrations OD-01 faites dans le commit qui change le texte visible. | Règles ID-01 à ID-08 |
| TB2-05 | Table d'alias d'affichage **générée** depuis `R2-CONTROL-LABELS-R1.csv` (`tools/gen_ui_labels.py --check`, reproductible). Résolution par StableID et contexte d'emplacement. Test NAM-03 en bijection. | OD-18 ; aucun libellé saisi à la main |
| TB2-06 | Workflow macOS dédié aux preuves R2-B2 (captures de production, GC-01), qui verse ses résultats par un commit `[skip ci]`. `arch-g4.yml` n'est pas modifié ; les nouveaux tests y entrent par la cible de tests du wrapper. | Pas d'alourdissement ni de contournement de la CI de production |
| TB2-07 | Ordre des familles : NEXUS, **PLASMA**, NEURAL CELL, SYNAPTIC, HUD, PANIC | Ajustement justifié de l'ordre proposé (PLASMA passe avant SYNAPTIC). NEXUS et PLASMA partagent la classe `ContinuousControl` et la correction PB-01 ; les intégrer à la suite évite un état intermédiaire où la même classe aurait deux rendus et deux comportements de double-clic. |
| TB2-08 | Ligne de base GC-01 mesurée sur 0.4.1 avant toute modification visuelle | Comparaison loyale (protocole §1) |

### 4.2 Étapes (après GO du dossier)

| Étape | Contenu | Sortie |
|---|---|---|
| B2-01 | Instrument begin / end (tests seulement) ; reproductions PB-01a–e, PB-02a–e, PB-N1–N3 **sur 0.4.1** ; résultats publiés | `PB/` : CSV, journaux, statut REPRODUIT, NON REPRODUIT ou BLOCKED |
| B2-02 | Ligne de base GC-01 ; module `ui/r2/` ; tokens R1 dans `DesignTokens.h` (palette inchangée) ; `SymbioticLook` R1 ; `Plate` R1 ; registre d'identifiants ; générateur de libellés ; laboratoire branché sur les peintres partagés (TB2-02) et contrôle pixel à pixel | Fondation, VIS-01, ID-UNIQ |
| B2-03 | Familles, chacune dans un ou plusieurs commits atomiques, avec ses tests et ses captures : (1) NEXUS et correction PB-01 ; (2) PLASMA (COMPACT, mesure de crête sortie du canal) ; (3) NEURAL CELL ; (4) SYNAPTIC (pistes, listes, onglets, sélecteur de moniteur) ; (5) HUD (selon DB2-05) ; (6) PANIC et commandes de sûreté | Preuves par famille avant de passer à la suivante |
| B2-04 | Nomenclature : P1 (324), puis le reste selon DB2-03 ; PT-02 dans la disposition de production ; `Z · CROSS LVL` (EC-13) | NAM-*, rapport typographique |
| B2-05 | Interactions et accessibilité (AX-01, AX-03, focus, clavier, menus, automation, fermeture et redimensionnement pendant un geste) | Rapport d'accessibilité |
| B2-06 | Intégrité de la télémétrie (EC-01, EC-02, inventaire INV-11, TEL-05) | Inventaire « donnée → source » |
| B2-07 | Mesures GC-01 aux points du protocole §4 | Rapport de performances |
| B2-08 | Validation finale, captures de production, non-régression, clôture ; version selon DB2-04 | Rapport de clôture R2-B2 |

## 5. Analyse des dépendances

- **Ordre technique.** B2-01 précède toute modification de geste. B2-02 précède les familles (peintres et tokens). Le
  HUD (5) dépend de B2-06 pour les textes de source. PANIC (6) dépend de NEURAL CELL (3) : même classe `ActionCell`,
  dont le chemin « danger » reste sur le rendu 0.4.1 jusqu'à l'étape 6.
- **Code.** Lecture seule de `plugin/PluginProcessor.h` (`telemetry()`, état d'autonomie, captures, Undo). **Aucune
  modification de `plugin/` n'est prévue.** Si une donnée manquait, le chantier s'arrêterait pour arbitrage.
- **Lots voisins** (non anticipés) :

| Lot | Périmètre |
|---|---|
| R2-C | Disposition de PERFORMANCE, barre, Core 2D, PB-02 Core, STANDARD placé |
| R2-D | Réorganisation SOUND DESIGN, éditeurs de courbes, PB-02 enveloppe |
| R2-E | Workbench ENGINEERING, Inspector adaptatif, remplacement des widgets génériques |
| R2-F | Réseaux, matrice |

## 6. Risques et mitigations

| ID | Risque | Mitigation |
|---|---|---|
| RB2-01 | Régression de geste (NX-05) en reprenant le code du laboratoire | TB2-01 : gestionnaires 0.4.1 conservés ; BE-01 à 05 avant et après ; UI04-KEY, UI04-BIND |
| RB2-02 | Tests historiques masquant ou subissant une régression (types, textes) | TB2-01 (types conservés) ; migrations OD-01 sans affaiblissement, dans le commit qui change le texte ; barrière des résultats |
| RB2-03 | Coût de rendu (GC-01) des primitives R1 sur des centaines de commandes | Ligne de base, mesure après chaque famille, optimisations invisibles d'abord ; seuils confirmés avant mesure |
| RB2-04 | Dérive artistique à l'extraction des peintres | VIS-01 et non-régression pixel à pixel du laboratoire (TB2-02) |
| RB2-05 | Effet audio d'une modification d'interface (INV-13, RK-19) | UI04-RT, UI04-DSP, REG-01, test « aucune écriture pendant la navigation » ; aucune modification de `source/` ni de `plugin/` |
| RB2-06 | Destruction d'une commande pendant un geste par reconstruction d'un `ParamPanel` (RK-05) | PB-N3 ; `GestureGuard` ; ASan en local |
| RB2-07 | Correction de PB-01 qui change le nombre de transactions Undo | UR-01 et UI04-UNDO : défaut au double-clic = une transaction |
| RB2-08 | Libellés mal rattachés à leur contexte | Table générée ; NAM-03 en bijection ; NAM-06 |
| RB2-09 | Dépassements typographiques dans la disposition réelle (anglais, R1) | PT-02 en production (3 tailles × 3 zooms) ; UI04-CLIP ; adaptation géométrique locale, jamais de corps réduit |
| RB2-10 | Densité des stries, lisibilité secondaire des HUD, PANIC moins visible (vigilances de la contre-expertise) | Captures de l'interface complète à chaque famille ; mesure du plancher à 75 % sur les textes HUD ; PANIC-CON ; aucune reconception sans arbitrage |
| RB2-11 | État statique partagé entre instances (caches des peintres) | Peintres sans état mutable statique ; MI-01, UI04-MULTI |
| RB2-12 | Écart Windows / macOS (polices, Direct2D) | macOS fait foi ; contexte `Graphics` fermé avant toute lecture de pixels |
| RB2-13 | Contradictions normatives non tranchées | DB2-01 à DB2-07 avant le début des étapes concernées |
| RB2-14 | Recette physique impossible pour l'agent (Reason, VoiceOver, machine de référence) | Protocoles et scripts fournis ; statut NON EXÉCUTÉ conservé (INT-18) |

## 7. Stratégie PB-01 / PB-02 (OD-10)

- **Reproduction d'abord.** Sur le code 0.4.1, avant toute modification de composant (B2-01). Résultats bruts publiés,
  puis statut REPRODUIT, NON REPRODUIT ou BLOCKED. Jamais de PASS sur lecture statique.
- **PB-01** : NEXUS et PLASMA, par la classe commune. Si la reproduction confirme le défaut, la correction est faite dans
  B2-03 (1) et (2), lots qui modifient ces gestes : le défaut est écrit **dans** le geste ouvert par le second clic, ce
  qui donne une profondeur de 1 et une seule transaction. Le test de reproduction est retiré avec sa raison documentée et
  laisse la place aux critères BE.
- **PB-02** :
  - Core (a, b, c, e) et enveloppe (d) sont reproduits et documentés, **non corrigés** en B2. Ils seront corrigés en
    R2-C et R2-D, qui modifient ces gestes. DB2-05 garde les gestes des éditeurs inchangés.
  - Les cas NEXUS et PLASMA (N1 à N3) relèvent de B2, puisque B2 modifie leurs gestes.

Scénarios détaillés : [B2-TEST-PLAN.md](B2-TEST-PLAN.md) §3.

## 8. Protocole de performances graphiques

Voir [B2-GRAPHICS-PERFORMANCE-PROTOCOL.md](B2-GRAPHICS-PERFORMANCE-PROTOCOL.md).

- **Principe** : neuf mesures (GP-01 à GP-09), comparatives 0.4.1 / intégré, sur la même machine, en alternance A, B, A,
  B. Scénarios statiques et dynamiques sur données réelles.
- **Seuils candidats** : relatifs (GS-01 à GS-07), justifiés par la cadence de 30 Hz et l'invariance audio, à confirmer
  avant mesure (DB2-07).
- **Portée** : la VM ne certifie rien. Le matériel physique de référence dépend de DB2-01.

## 9. Plan de tests

Voir [B2-TEST-PLAN.md](B2-TEST-PLAN.md) :

- règles T-R01 à T-R06 ;
- instrument begin / end ;
- reproductions ;
- tests par famille (existants non modifiés, puis ajoutés : VIS-01, ID-UNIQ, PLAS-T, PANIC-CON, HUD-SRC, NAM-*, AX-01,
  GP-*) ;
- migrations OD-01 ;
- maintien du laboratoire ;
- recette physique.

## 10. Fichiers prévus (après GO)

| Nature | Fichiers |
|---|---|
| Nouveaux (production) | `ui/r2/Primitives.h/.cpp`, `ui/r2/Painters.h/.cpp`, `ui/r2/GestureGuard.h`, `ui/r2/ValueSource.h`, `ui/r2/ComponentIds.h`, `ui/r2/Labels.h`, `ui/r2/Labels.inc` (généré) ; `tools/gen_ui_labels.py` |
| Modifiés (production) | `ui/DesignTokens.h` (tokens dérivés R1 ; palette de référence inchangée), `ui/Components.h/.cpp`, `ui/SymbioticLook.h/.cpp`, `ui/Monitors.h/.cpp`, `ui/CurveEditors.cpp` (rendu seul, selon DB2-05), `ui/MainEditor.h/.cpp`, `ui/PerformanceView.cpp`, `ui/SoundDesignView.cpp`, `ui/EngineeringView.cpp` (rendu, libellés, identifiants ; **aucune réorganisation**), `cmake/Plugin.cmake` (nouvelles sources) |
| Tests | Nouveaux : `tests/wrapper/GestureTests.cpp`, `tests/wrapper/R2B2Tests.cpp`, banc GC-01. Modifiés : `tests/wrapper/WrapperTests.cpp`, `tests/wrapper/Ui04Tests.cpp` (migrations OD-01 seules), `tests/expected_results.txt` (critères ajoutés avant mesure), CMake de la cible de tests |
| Laboratoire | `lab/CMakeLists.txt`, `lab/r2/*` (délégation aux peintres partagés, si TB2-02 est retenu) |
| CI | Nouveau `.github/workflows/r2b2-evidence.yml` (TB2-06) |
| Documentation | `docs/ARCH-UI-04-R2/B2-PRODUCTION-INTEGRATION/**`, tracker, référentiel R2 |
| **Interdits** | `source/**`, `plugin/**` (aucune modification prévue), `tests/core/**`, `tests/core/expected_param_ids.txt`, `tests/host/expected_vst3_param_ids.txt`, `tests/known_failures.txt`, `packaging/**`, `manual/**`, `.github/workflows/arch-g4.yml`, `arch-g3.yml`, preuves B1 et B1-R1, version 0.4.1 (sauf DB2-04 à la clôture) |

## 11. Critères d'acceptation

Voir [B2-ACCEPTANCE-CRITERIA.md](B2-ACCEPTANCE-CRITERIA.md).

- INT-01 à INT-18 (mission), complétés par INT-19 (zooms et redimensionnement), INT-20 (laboratoire opérationnel) et
  INT-21 (Undo/Redo, instances multiples).
- Chacun est rattaché aux identifiants existants de la matrice R2. Aucune collision : le préfixe INT n'existait pas.
- Tous les critères sont enregistrés avant mesure.
- INT-01 est vérifié à l'ouverture :
  - `git diff d2d08ac -- source/` vide au SHA `73b633b` ;
  - REG-01 et goldens en SUCCESS dans la CI de production 55 sur `7cbfa43`. Le code y est identique à celui de
    `73b633b` : les commits suivants ne touchent que la documentation.

## 12. Décisions nécessaires (propriétaire)

| ID | Question | Options | Conséquences et risques | Recommandation |
|---|---|---|---|---|
| **DB2-01** | Matériel de référence de GC-01 (EC-07) | (A) **M3 de base**, minimum officiel validé le 2026-11-25, le M1 restant hors support. (B) M1, comme le dit la mission : cela rouvre une décision de plateforme validée. (C) M3 comme minimum officiel, M1 comme mesure complémentaire conservatrice si une machine est disponible, sans changer le support. | (B) contredit `V1-PLATFORM-REQUIREMENTS.md` ; (A) et (C) le respectent. L'agent n'a accès à aucune machine physique : la mesure physique sera faite par le propriétaire. | **(C)** : la décision validée est conservée et la marge de sécurité est mesurée si possible. Le protocole est comparatif et ne dépend pas de la machine en attendant. |
| **DB2-02** | Gestes PLASMA (EC-05, PL-06) | (A) **Conserver les gestes 0.4.1** : clic = position absolue, glisser rapporté à la longueur. PL-06 se lit alors « mêmes règles que NEXUS pour le clavier, la molette, le défaut, la saisie et l'appariement des gestes ». (B) Aligner le glisser sur NEXUS (relatif, 220 px), conformément à la lettre de PL-06 : le comportement change pour l'utilisateur. | (A) ne change aucun contrat utilisé en 0.4.1 et ne transpose pas le laboratoire ; il faut amender le texte de PL-06. (B) modifie un geste en usage et doit être validé physiquement. | **(A)**, avec amendement de PL-06 |
| **DB2-03** | Nomenclature hors P1 et microcopie (EC-08, NAM-01) | (A) P1 seulement en B2 ; NAM-01 reste NON EXÉCUTÉ et l'interface est mixte. (B) **Valider par lots pendant B2-04** : dossier de revue des 1 890 occurrences P2 et P3 et de la microcopie hors CSV (infobulles, menus, messages, titres). Application après validation. NAM-01 n'est exigé qu'à la clôture et seulement si tout est validé. (C) Appliquer toutes les propositions sans validation : contraire à OD-02. | (B) suit le calendrier d'OD-02 (« P2 et P3 en cours de lot ») et demande du temps de revue au propriétaire. (A) laisse une interface bilingue. | **(B)** |
| **DB2-04** | Version à la clôture (EC-15) | (A) **0.5.0 à la clôture de R2-B2** (OD-17), avec autorisation explicite à ce moment-là ; aucun changement intermédiaire. (B) Rester en 0.4.1 jusqu'à R2-H. | OD-17 prévoit 0.5.0 pour la première livraison intégrée. Changer de version touche les paquets et le CHANGELOG. | **(A)**, confirmée au moment de la clôture |
| **DB2-05** | HUD sans variante validée : éditeurs d'enveloppe et de courbe, XY, réseau, ordre Z, flux, éditeur de banque (EC-16) | (A) **Monture HUD R1 seule** (verre, monture, rails, pastilles, source en pied). Contenu, poignées et gestes inchangés, aux couleurs des tokens. (B) Laisser ces composants en 0.4.1 jusqu'à leur lot (R2-C, D, F). (C) Les redessiner entièrement : conception non validée. | (A) apporte la cohérence sans créer de conception nouvelle ni toucher aux gestes : PB-02d reste en R2-D. (B) laisse deux styles d'afficheur côte à côte. (C) sort du gel R1. | **(A)** |
| **DB2-06** | Widgets JUCE génériques (ENGINEERING, barre) | (A) **Habillage R1 par le LookAndFeel en B2** ; remplacement par des composants R1 en R2-E / R2-F2. (B) Remplacement dès B2 : il faudrait des adaptateurs de valeurs hors registre (profondeurs de route…). | (B) anticipe R2-E et R2-F et alourdit le risque. | **(A)** |
| **DB2-07** | Seuils candidats GC-01 (GS-01 à GS-07) | Confirmer, ajuster ou remplacer avant la mesure | Ils ne pourront plus être abaissés après la mesure | Confirmer les seuils proposés |

Les choix TB2-01 à TB2-08 (§4.1) relèvent de l'architecture. Ils sont soumis à la contre-expertise, pas au propriétaire.

## 13. Documents publiés

| Document | Objet |
|---|---|
| [README.md](README.md) | Index du dossier R2-B2 |
| [B2-00-DOSSIER.md](B2-00-DOSSIER.md) | Ce dossier |
| [B2-00-COMPONENT-MAP.md](B2-00-COMPONENT-MAP.md) | Cartographie |
| [B2-CONFORMITY-MATRIX.md](B2-CONFORMITY-MATRIX.md) / [.csv](B2-CONFORMITY-MATRIX.csv) | Matrice R1 → production |
| [B2-ACCEPTANCE-CRITERIA.md](B2-ACCEPTANCE-CRITERIA.md) | INT-01 à INT-21 |
| [B2-TEST-PLAN.md](B2-TEST-PLAN.md) | Plan de tests, PB-01 et PB-02 |
| [B2-GRAPHICS-PERFORMANCE-PROTOCOL.md](B2-GRAPHICS-PERFORMANCE-PROTOCOL.md) | GC-01 |
| [B2-DEVIATIONS-REGISTER.md](B2-DEVIATIONS-REGISTER.md) | Écarts EC-01 à EC-16 |
| [`../B1-R1-FINAL-ARTISTIC-REFINEMENT/OWNER-DECISION-AR15.md`](../B1-R1-FINAL-ARTISTIC-REFINEMENT/OWNER-DECISION-AR15.md) | AR-15 PASS |

## 14. SHA

- SHA de départ : `73b633b`.
- Le SHA distant final est celui du commit qui publie ce dossier. Il est consigné dans le message de clôture de
  R2-B2-00 après vérification par `git ls-remote`, car un fichier ne peut pas contenir le SHA du commit qui l'introduit.

## 15. Verdict de préparation

**Prêt pour la contre-expertise.**

- Cartographie, matrice, stratégie, risques, protocoles, plan de tests et critères sont établis.
- Sept décisions du propriétaire (DB2-01 à DB2-07) sont à trancher avant les étapes concernées. DB2-01 et DB2-07
  conditionnent B2-07. DB2-02 conditionne B2-03 (2), DB2-03 conditionne B2-04, DB2-05 conditionne B2-03 (5).
- B2-01 (reproductions) ne dépend d'aucune décision ouverte. B2-02 (fondation) n'en dépend que pour l'habillage des
  widgets génériques (DB2-06) ; la ligne de base GC-01 sur 0.4.1 n'exige pas encore les seuils de DB2-07.

**R2-B2-00 — PRÊT POUR CONTRE-EXPERTISE / AUCUNE INTÉGRATION DE PRODUCTION ENGAGÉE.**
