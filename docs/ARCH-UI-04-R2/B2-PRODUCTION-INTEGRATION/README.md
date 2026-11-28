# SUBDESTRUCKT · ARCH-UI-04-R2-B2 — intégration des composants en production

Transfert du Design System R1 (gelé par la décision AR-15 PASS du 2026-11-28) du Component Lab vers le plugin de
production. C'est une mission d'intégration, de robustesse, de conformité et de non-régression. Ce n'est ni une mission
artistique ni une réorganisation spatiale.

**État : R2-B2-00 GO (addendum [B2-00-ADDENDUM.md](B2-00-ADDENDUM.md)) ; exécution de R2-B2 autorisée.** Arbitrages
DB2-01 à DB2-07 et amendements CE-B2-01 à CE-B2-05 consignés.

| Document | Contenu |
|---|---|
| [B2-00-ADDENDUM-B2-03.md](B2-00-ADDENDUM-B2-03.md) | **Addendum normatif B2-03** : DB2-09 = B, CE-B2-06 à 08, contrat NEXUS / PLASMA définitif, critères EC-19 |
| [B2-00-ADDENDUM.md](B2-00-ADDENDUM.md) | **Addendum normatif** : GO, DB2-01 à DB2-07, CE-B2-01 à CE-B2-05, seuils GS définitifs, PL-06 amendé (prime sur le dossier) |
| [B2-00-DOSSIER.md](B2-00-DOSSIER.md) | Dossier d'exécution : état du dépôt, stratégie, dépendances, risques, PB-01 et PB-02, performances, fichiers, décisions, verdict |
| [B2-00-COMPONENT-MAP.md](B2-00-COMPONENT-MAP.md) | Cartographie de la production, du laboratoire et des données disponibles |
| [B2-CONFORMITY-MATRIX.md](B2-CONFORMITY-MATRIX.md) / [.csv](B2-CONFORMITY-MATRIX.csv) | Composant actuel → composant R1 → contrat → dépendances → risques → tests |
| [B2-ACCEPTANCE-CRITERIA.md](B2-ACCEPTANCE-CRITERIA.md) | Critères INT-01 à INT-21 (enregistrés avant mesure) |
| [B2-TEST-PLAN.md](B2-TEST-PLAN.md) | Plan de tests, reproductions PB-01 et PB-02, migrations OD-01 |
| [B2-GRAPHICS-PERFORMANCE-PROTOCOL.md](B2-GRAPHICS-PERFORMANCE-PROTOCOL.md) | Protocole GC-01 comparatif 0.4.1 / intégré |
| [B2-DEVIATIONS-REGISTER.md](B2-DEVIATIONS-REGISTER.md) | Registre des écarts (EC-01 à EC-20) |
| [B2-DECISIONS-PENDING.md](B2-DECISIONS-PENDING.md) | Décisions soumises au propriétaire (STOP ciblés) : DB2-08, DB2-09 (validées), **DB2-10 à arbitrer** |
| [B2-03-REPORT.md](B2-03-REPORT.md) | **B2-03** : intégration des familles (NEXUS, puis PLASMA, NEURAL CELL, SYNAPTIC, HUD, PANIC) |
| [PB/](PB/README.md) | **B2-01** : reproductions PB-01, PB-02, PB-N, PB-J (journaux Windows et macOS) |
| [GC01/](GC01/README.md) | **GC-01** : banc, référence figée `0517426`, mesures comparatives |
| [VIS/](VIS/README.md) | **Conformité visuelle** (CE-B2-03) : chaîne référence R1 → laboratoire → production |
| `CAPTURES/` | **Captures de production** (`MainEditor` réel, rendu hors écran), par plateforme et par jalon. Ce ne sont jamais des planches du laboratoire. |

**Livrables à venir** (étapes 01 à 08, après GO) :

- `PB/` (reproductions) ;
- rapport de performances ;
- rapport d'accessibilité ;
- `CAPTURES/` de production (distinctes des planches du laboratoire) ;
- rapport de non-régression ;
- rapport de clôture R2-B2.

Références : référentiel [`../README.md`](../README.md) ; Design System R1
[`../B1-R1-FINAL-ARTISTIC-REFINEMENT/COMPONENT-DESIGN-FINAL.md`](../B1-R1-FINAL-ARTISTIC-REFINEMENT/COMPONENT-DESIGN-FINAL.md) ;
décision [`../B1-R1-FINAL-ARTISTIC-REFINEMENT/OWNER-DECISION-AR15.md`](../B1-R1-FINAL-ARTISTIC-REFINEMENT/OWNER-DECISION-AR15.md).
