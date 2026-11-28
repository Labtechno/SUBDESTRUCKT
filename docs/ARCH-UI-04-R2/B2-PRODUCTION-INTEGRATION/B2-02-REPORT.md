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

À compléter avec le run de preuves sur l'habillage R1 et la CI de production correspondante.
