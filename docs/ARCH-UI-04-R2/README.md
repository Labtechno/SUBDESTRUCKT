# SUBDESTRUCKT · ARCH-UI-04-R2 — référentiel Design & Ergonomics

Point d'entrée du référentiel produit par **ARCH-UI-04-R2-A** (mission exclusivement documentaire, 2026-11-28). Ce
dossier transforme les **15 arbitrages DA-A01 à DA-A15** validés par le propriétaire en spécifications mesurables pour
les lots de développement R2-B à R2-H, puis ARCH-UI-05 (Core 3D) et ARCH-UI-06 (intégration finale).

> **Statut.** Référentiel proposé, en attente de la contre-expertise de l'architecte (ChatGPT) et de la validation
> explicite du propriétaire. **Aucun lot de développement n'est autorisé par ce dossier**, en particulier ni R2-B ni
> ARCH-UI-05. Les éléments marqués « proposition », « seuil candidat » ou « arbitrage résiduel » ne sont pas normatifs.

## Références

| Élément | Valeur |
|---|---|
| Branche | `feature/arch-g4-full-development` |
| Version fonctionnelle de référence | 0.4.1 |
| Révision DSP verrouillée | `d2d08ac1bb6ed09ee3ea1b5e5d5d409c41b586af` (invariant : `git diff d2d08ac -- source/` vide) |
| Build macOS audité physiquement | 0.4.1-b263320802 (Reason 14.1d100, Mac Studio M4 Max) |
| Revue indépendante Codex | `dbc21c8cb29c28a7d8ab7aea049d72e0ccec28fe`, dossier [`../ARCH-UI-04-DA/`](../ARCH-UI-04-DA/README.md) |
| Notes de référence | Codex 6,7/10 (descriptive, 101/150) ; contre-expertise ChatGPT 5,7/10 (descriptive) |

## Documents (ordre de lecture conseillé)

| # | Document | Périmètre (sans recouvrement) |
|---|---|---|
| 1 | [R2-A-MASTER-SPEC.md](R2-A-MASTER-SPEC.md) | Cadre : objectifs, invariants, identité SYMBIOTIC INDUSTRIAL, conventions normatives, divergences avec les contrats historiques |
| 2 | [R2-DECISION-REGISTER.md](R2-DECISION-REGISTER.md) | Registre des 15 arbitrages (texte normatif), éléments dérivés, propositions non validées, orientation rejetée |
| 3 | [R2-DESIGN-SYSTEM-SPEC.md](R2-DESIGN-SYSTEM-SPEC.md) | Tokens, matériaux, typographie ; NEXUS Hybrid, NEURAL CELL Hybrid, Cybernetic Display, Plasma Hybrid ; états ; gestes |
| 4 | [R2-CONTROL-NAMING-SPEC.md](R2-CONTROL-NAMING-SPEC.md) | Nomenclature anglaise compacte : règles, lexique, distinctions critiques, budgets, méthode du CSV |
| 5 | [R2-CONTROL-LABELS-PROPOSALS.csv](R2-CONTROL-LABELS-PROPOSALS.csv) | 2 213 occurrences : 703 StableID canoniques, 851 variantes d'emplacement, 537 options, 122 hors registre / titres / états / champs / unités (UTF-8 avec BOM) |
| 6 | [R2-WORKSPACE-SPEC.md](R2-WORKSPACE-SPEC.md) | Adaptive Instrument (Performance), Adaptive Engine Workspace (Sound Design), Engineering Workbench, Adaptive Inspector ; étude géométrique |
| 7 | [R2-NEURAL-NETWORK-SPEC.md](R2-NEURAL-NETWORK-SPEC.md) | Adaptive Neural Network et Dual View pour FM Network, CROSS Network, Global Modulation Matrix ; faisabilité |
| 8 | [R2-CORE-INTEGRATION-CONTRACT.md](R2-CORE-INTEGRATION-CONTRACT.md) | Enveloppe graphique et interactive du SYMBIOTIC CORE avant ARCH-UI-05 |
| 9 | [R2-TELEMETRY-FEASIBILITY.md](R2-TELEMETRY-FEASIBILITY.md) | Inventaire de la télémétrie réelle et classement pour Reactive Signal Flow |
| 10 | [R2-ACCEPTANCE-MATRIX.md](R2-ACCEPTANCE-MATRIX.md) | Critères d'acceptation traçables (obligatoires, objectifs, seuils candidats, faisabilité) |
| 11 | [R2-EXECUTION-PLAN.md](R2-EXECUTION-PLAN.md) | Lots R2-B à R2-H, ARCH-UI-05/06, dépendances, prototypes, ordonnancement recommandé |
| 12 | [R2-RISKS-AND-OPEN-DECISIONS.md](R2-RISKS-AND-OPEN-DECISIONS.md) | Risques techniques, ergonomiques et de gouvernance ; décisions restant à arbitrer |
| 13 | [R2-A-REPORT.md](R2-A-REPORT.md) | Rapport de clôture de R2-A |
| 14 | [B1-COMPONENT-LAB/](B1-COMPONENT-LAB/README.md) | R2-B1 : Component Lab (preuves historiques) |
| 15 | [B1-R1-FINAL-ARTISTIC-REFINEMENT/](B1-R1-FINAL-ARTISTIC-REFINEMENT/README.md) | R2-B1-R1 : Design System R1 final (AR-15 PASS, gelé) |
| 16 | [B2-PRODUCTION-INTEGRATION/](B2-PRODUCTION-INTEGRATION/README.md) | R2-B2 : intégration en production (R2-B2-00 : audit et contractualisation) |

## Conventions

- **DOIT / NE DOIT PAS** : exigence normative issue d'un arbitrage validé ou d'un contrat en vigueur.
- **DEVRAIT** : objectif de conception, à démontrer.
- **Seuil candidat** : valeur proposée, à confirmer avant mesure (jamais abaissée après mesure).
- **Proposition / arbitrage résiduel** : non validé ; listé dans [R2-RISKS-AND-OPEN-DECISIONS.md](R2-RISKS-AND-OPEN-DECISIONS.md).
- Les mesures citées proviennent soit des tests archivés (UI04, TA-R), soit des relevés de Codex, soit d'un **calcul à
  partir du code 0.4.1** explicitement signalé. Aucune mesure nouvelle n'est inventée.

## Production du CSV

Le CSV est produit de manière déterministe à partir du registre compilé (`source/core/ParamTable.inc`), de la
cartographie UI04-COV (`../V1-ARCH-UI-04-STABLEID-MAP.md`), des mesures JUCE de Codex
(`../ARCH-UI-04-DA/CONTROL-LABELS-MEASUREMENTS.csv`, `CONTROL-LABELS-CHOICES.csv`) et de ses propositions. Les règles
(lexique, libellés contextuels, modèle de largeur, calcul des collisions) sont intégralement décrites dans
[R2-CONTROL-NAMING-SPEC.md](R2-CONTROL-NAMING-SPEC.md) §8 ; le script de génération n'est pas versionné (mission
documentaire) et peut être reconstitué à partir de ces règles.

## Historique préservé

Les rapports historiques (`../V1-ARCH-UI-04-REPORT.md`, `../V1-ARCH-UI-04-R1-REPORT.md`, `../V1-ARCH-UI-04-DESIGN-SYSTEM.md`,
`../ARCH-UI-04-DA/`) ne sont pas modifiés. Le tracker principal référence R2-A (`../V1-ARCH-G4-TRACKER.md` §3nonies).
