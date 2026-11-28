# ARCH-UI-04-R2-B2-02 — fondation technique R1 : rapport

## 1. Architecture du module partagé `ui/r2/`

| Fichier | Contenu | Consommateurs |
|---|---|---|
| `R2Style.h/.cpp` | Tokens dérivés (`tok2`), polices, domaines, états imposés, `ValueSource`, `GestureGuard`, primitives `draw::*` | Plugin, laboratoire |
| `R2Painters.h/.cpp` | Peintres purs et géométries publiques : NEXUS (`NexusView`, `paintNexus`, `nexusKnobArea`, `nexusValueArea`), PLASMA (`PlasmaView`, `paintPlasma`, `plasmaSlot`, `plasmaPosOf`), NEURAL CELL avec PANIC (`CellView`, `paintCell`, `paintCellIcon`), SYNAPTIC (`TrackView`, `paintTrack`, `trackArea`, `trackSegment`) | Plugin (à partir de B2-03), laboratoire |
| `R2Hud.h/.cpp` | `DisplayData`, `HudVariant`, `HudView`, `paintHud` (neuf variantes Cybernetic HUD) | Plugin (à partir de B2-03), laboratoire |
| `ComponentIds.h` + `.inc` (généré) | Identifiants stables OD-01 : `param:<StableID>` (703) et registre en ajout seul `action:`, `state:`, `nav:` (70) | Plugin, tests |
| `Labels.h` + `.inc` (généré) | Alias d'affichage de la nomenclature R1 : 2 214 occurrences, dont 537 options et 324 validées | Plugin (B2-04), tests |

**Contrat des peintres** (B2-00-ADDENDUM §6). Un peintre reçoit une vue et une géométrie, puis dessine. Il n'écrit
aucun paramètre, n'ouvre aucun geste hôte, ne lit ni l'automation ni le DSP, et ne garde aucun état mutable partagé.
Les gestionnaires d'événements restent propres à chaque consommateur : ceux du laboratoire ne sont pas intégrés au plugin.

**Ce qui reste au laboratoire seulement** :

- la révision B1 (`Rev`) ;
- les primitives B1 (`led`, `opticalFrame`) ;
- l'étiquette DEMO DATA (`demoTag`), dans `lab/r2/R2StyleB1.cpp`.

**Extraction.** Les corps de rendu ont été transférés **par programme, sans modification**. Seules les références aux
membres sont devenues des champs de vue. En ignorant les fins de ligne, `R2Style.cpp` ne diffère que par son en-tête et
le retrait des trois primitives B1, et `R2Hud.cpp` que par ces substitutions.

## 2. Conformité visuelle (CE-B2-03, étape 1)

Le laboratoire, recâblé sur les peintres partagés, est comparé aux **captures R1 historiques immuables** :

- Windows : **15/15 planches identiques** sur l'image entière (0 pixel différent) ;
- macOS (VM, component-lab run 6) : **15/15 identiques** sur l'image entière.

Détail : [VIS/README.md](VIS/README.md). Le workflow `component-lab.yml` ne réécrit plus les références.

## 3. Identifiants et libellés

| Test | Résultat |
|---|---|
| ID-UNIQ | PASS : 703 `param:` + 70 identifiants de registre, aucun mal formé, aucun doublon |
| LBL-TABLE | PASS : 2 214 occurrences, dont 537 options de liste, 324 validées (P1), aucune validée hors P1 |

Générateur reproductible : `tools/gen_ui_labels.py --check`, qui vérifie l'identité et l'ajout seul.

Classification explicite des éléments hors registre :

- afficheurs → `state:` ;
- libellés statiques et vocabulaire d'état → aucun identifiant ;
- éléments interactifs → `action:` ;
- onglets → `nav:<espace>/<groupe>/<outil>`.

## 4. Habillage R1 des widgets génériques (DB2-06)

`SymbioticLook` utilise les primitives partagées, sans conception nouvelle. Widgets et contrats sont conservés.

| Widget | Habillage |
|---|---|
| Boutons d'action | Titane usiné, course à l'appui |
| Boutons basculés | Logement encastré |
| Bascules | Témoin encastré |
| Listes | Rail Machined Track et chevron |
| Curseurs | Gorge, canal PLASMA, curseur en titane |
| Barres de défilement | Gorge et titane |
| Champs | Fenêtre carbone |

Captures de production : [`CAPTURES/windows/b2-02-lookandfeel-r1/`](CAPTURES/windows/b2-02-lookandfeel-r1) ; macOS au
run de preuves marqué `[captures]`. Les composants liés (NEXUS, PLASMA, NEURAL CELL, SYNAPTIC, HUD, PANIC) restent en
0.4.1 jusqu'à leur intégration (B2-03).

## 5. Performances (GC-01)

Le contrôle A/A (run 5) établit le plancher de bruit de la VM ([GC01/README.md](GC01/README.md) §4). La mesure après
fondation, avec l'habillage R1, est consignée au §7.

## 6. Vérifications

| Contrôle | Résultat |
|---|---|
| `git diff d2d08ac -- source/` | vide |
| `plugin/` | inchangé |
| Références R1 historiques | aucune modification |
| Barrière des résultats (Windows) | PASS (77 critères, AUT-02 connu) |
| CI de production 62 (`0d576ed`) | SUCCESS |

## 7. Mesure après fondation et verdict

**GC-01 après fondation** : run 7 sur `42a605e` (code de rendu de `25702f5`, habillage R1), comparé à la référence
`0517426` en alternance. Plancher de bruit : contrôle A/A (run 5) et dispersion interne des deux exécutions de chaque
binaire. Verdicts : [`GC01/macos/run7-42a605e/gc01-verdicts.txt`](GC01/macos/run7-42a605e/gc01-verdicts.txt).

| Seuil | Verdict (VM, comparatif) | Rapport courant / référence |
|---|---|---|
| GS-01 rendu complet | PASS | ×0,99 (pire vue × zoom) |
| GS-02 rafraîchissement (partie comparative) | PASS | ×0,94 ; partie absolue (11,1 ms) : M3 physique |
| GS-03 repos | PASS | 1 zone par rafraîchissement, identique à la référence (oscilloscope 0.4.1) |
| GS-04 audio | **NON CONCLUANT** | ×1,47 sur le p99, éditeur fermé, mais le **même binaire de référence** varie de ×2,2 en p99 (×6 en médiane) d'une exécution à l'autre. Sur la VM Apple Silicon, un thread ordinaire est placé tantôt sur un cœur performance, tantôt sur un cœur efficacité. Le traitement audio ne peut pas dépendre de l'habillage, éditeur fermé : `source/` et `plugin/` sont intacts. |
| GS-05 mémoire | PASS | ×1,02 ; croissance de la seconde moitié non supérieure à la référence |
| GS-06 interaction (partie comparative) | PASS | ×1,04 ; objectif absolu (16,7 ms) : M3 physique |
| GS-07 éditeur fermé | **NON CONCLUANT** | Même série que GS-04 |

GS-04 et GS-07 restent **ouverts** pour la mesure physique M3 (DB2-01). Amélioration prévue du banc en B2-07 : thread
audio en priorité temps réel, comme dans un hôte, appliquée de façon identique aux deux arbres.

**CI** :

- production 62 (`0d576ed`) SUCCESS ;
- production 63 (`25702f5`) SUCCESS ;
- Component Lab macOS 6 et 7 SUCCESS ;
- preuves 5 et 7 SUCCESS.

La preuve 6 a échoué au versement ; le workflow a été rendu robuste et relancé (run 7).

**Captures de production macOS** : [`CAPTURES/macos/run7-42a605e/`](CAPTURES/macos).

**Verdict B2-02 : GO technique.**

- Module partagé en place, extraction prouvée sans dérive sur les deux plateformes.
- Identifiants et libellés générés et testés.
- Habillage R1 intégré.
- Mesures comparatives conformes là où elles sont concluantes.
- Aucun invariant touché.

Réserves : GS-04 et GS-07 (physique M3) ; étapes 2 et 3 de CE-B2-03 à faire par famille en B2-03.
