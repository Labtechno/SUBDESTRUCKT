# ARCH-UI-04-R2-A — plan d'exécution

**Aucun lot n'est autorisé par ce document.** Chaque lot exige un GO explicite du propriétaire, après la contre-expertise
de l'architecte. La gouvernance des missions précédentes s'applique à chaque lot : un agent, une branche
(`feature/arch-g4-full-development`), pas de worktree parallèle, pas de fusion dans `main`, pas de publication, pas de
coût supplémentaire, pas de SDK payant, invariants INV-01 à INV-12.

## 1. Découpage (brief §19) et contenu proposé

Le découpage R2-A à R2-H, ARCH-UI-05 et ARCH-UI-06 est celui du brief. Le contenu détaillé de chaque lot est une
proposition de R2-A.

| Lot | Objet | Arbitrages | Entrées | Sorties | Critères (matrice) |
|---|---|---|---|---|---|
| **R2-A** | Référentiel documentaire (ce dossier) | DA-A01…A15 | Revue Codex, code 0.4.1 | 14 documents | — |
| **R2-B** | Design System (brief : composants et tokens graphiques) : tokens R2, typographie, NEXUS Hybrid, NEURAL CELL Hybrid, Plasma Hybrid, variantes Cybernetic Display. **Proposition** : y rattacher l'application de la sérigraphie anglaise (table d'alias d'affichage) et la stratégie d'identification des tests (OD-18). | A01–A06 | CSV validé, OD-01, OD-06, OD-10, OD-11, OD-18 | Composants, table d'alias, tests NAM/NX/NC/CD/PL | §11 de la matrice |
| **R2-C** | PERFORMANCE Adaptive Instrument : dispositions par seuils, barre (PEAK, autonomie, une rangée), enveloppe du Core en 2D | A07, A08 | R2-B ; OD-04 ; prototype géométrique | Dispositions, Core 2D conforme au contrat | WS-P*, CORE-01…14, AX-02 |
| **R2-D** | SOUND DESIGN Adaptive Engine Workspace : contexte, ancrage, ordre moteur, conditionnels, éditeurs de courbes et d'enveloppes | A09 | R2-B | Plaques réorganisées | WS-S*, CD-07, CD-08 |
| **R2-E** | ENGINEERING Workbench (5 groupes, 10 outils) et Adaptive Inspector (DOCKED / BAND / DRAWER) | A10, A11 | R2-B ; OD-05 ; OD-01 (UI-01w) | Navigation à deux niveaux, Inspector adaptatif | WS-E*, WS-I*, WS-P03 |
| **R2-F** | Adaptive Neural Network et Dual View. Sous-lots proposés (OD-19) : **R2-F1** FM et CROSS (réseaux fixes), **R2-F2** Mod Matrix (configurable). | A12–A14 | R2-B, R2-E ; PT-03 | Vues NEURAL/TABLE | NN-01…27, NAM-12 |
| **R2-G** | Reactive Signal Flow sur données A/B ; modes si OD-07 les valide | A15 | R2-F ; OD-07 | Flux réactif | TEL-01…09 |
| **R2-H** | Validation (brief) : recette automatique, physique, ergonomique et artistique ; zooms, accessibilité, coût graphique mesuré, Reason 14 | tous | R2-B…G | Rapport de recette, captures | PHYS et SUBJ de la matrice, VAL-01, GC-01 |
| **ARCH-UI-05** | Core volumétrique dans l'enveloppe | A07 | Contrat Core figé et validé (R2-C), OD-12 | Rendu 3D avec repli 2D | CORE-03, 06, 12–15, TEL-01 |
| **ARCH-UI-06** | Intégration et finitions finales : harmonisation, documentation FR/EN, report des remplacements de contrats (master §7) | tous | ARCH-UI-05, R2-H | Version intégrée | Matrice complète |

## 2. Dépendances

```
R2-A ─► R2-B ─► R2-C ─► R2-D ─► R2-E ─► R2-F ─► R2-G ─► R2-H ─► ARCH-UI-05 ─► ARCH-UI-06
                  │                        ▲
                  └── contrat Core figé ───┼────────────────────► ARCH-UI-05
                                 Inspector ┘ (synchronisation de sélection)
```

| Dépendance | Raison |
|---|---|
| R2-B avant tout | Tous les lots consomment les composants et la nomenclature. Les tests dépendent des chaînes visibles (OD-01). |
| R2-C avant ARCH-UI-05 | L'enveloppe du Core doit être livrée en 2D et validée avant tout rendu volumétrique (Codex P1-06) |
| R2-E avant R2-F | Les réseaux vivent dans le groupe NETWORK et se synchronisent avec l'Inspector adaptatif |
| R2-F avant R2-G | Le flux réactif s'appuie sur les vues de réseau et la grammaire de route |
| R2-H après R2-G | La recette physique porte sur l'ensemble intégré |

## 3. Prototypes et études préalables

| ID | Étude | Lot | Livrable | Décision attendue |
|---|---|---|---|---|
| PT-01 | Prototype géométrique de la disposition COMPACT à 960 × 640 (mesure du pad, des rangs 1 à 9 et du défilement) | début de R2-C | Mesures automatiques, captures | Confirmation de WS-P02 et des seuils OD-04 |
| PT-02 | Mesure JUCE des 2 213 libellés proposés | début de R2-B | CSV mesuré (remplace les estimations) | Confirmation de NAM-08 |
| PT-03 | Coût de rendu des réseaux à densité maximale | début de R2-F | ms par image, machine de recette | NN-25, repli éventuel |
| PT-04 | Coût de la réactivité | début de R2-G | ms par image | TEL-08 |
| PT-05 | Disponibilité des glyphes `→ · … −` dans la police embarquée | R2-B | Rapport | OD-11 |

## 4. Ordonnancement recommandé

1. **R2-B**, avec PT-02 et PT-05 en ouverture. C'est le lot qui débloque tous les autres et qui traite les constats Codex
   P1-02, P1-03, P1-05 et P2-02.
2. **R2-C**, avec PT-01 en ouverture. Il porte le plus fort impact en usage (Codex P1-01, P1-04, P1-06). Il fige le
   contrat du Core.
3. **R2-E** avant **R2-D** si l'Inspector adaptatif est jugé prioritaire au minimum (WS-P03). Sinon l'ordre du §1 est
   conservé. Choix à arbitrer (OD-13).
4. **R2-F**, puis **R2-G**.
5. **R2-H**, puis la décision du propriétaire sur ARCH-UI-05.

## 5. Règles de lot

| Règle | Détail |
|---|---|
| Critères avant mesure | Les seuils candidats d'un lot sont confirmés par le propriétaire avant le début du lot, puis consignés dans le tracker. |
| Tests | Aucun test historique n'est modifié pour obtenir un PASS. Toute mise à jour d'un test dépendant d'une chaîne visible suit OD-01 et est justifiée dans le rapport de lot. |
| DSP | INV-01. Une donnée C ou D de télémétrie exige une mission distincte. |
| PB-01 / PB-02 | Traités uniquement si OD-10 l'autorise. Sinon, les lots ne modifient pas les gestes concernés. |
| Audio | INV-13 : toute modification de `plugin/` ou `ui/` susceptible d'affecter l'audio est contrôlée (REG-01, TA, absence d'écriture en navigation). |
| Version | Un incrément de version par lot livré (0.5.x proposé), décidé par le propriétaire. |
| Livraison | CI macOS SUCCESS, rapport, SHA distant vérifié, recette physique si le lot la prévoit |
