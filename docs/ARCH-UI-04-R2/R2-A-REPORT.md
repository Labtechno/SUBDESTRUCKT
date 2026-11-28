# ARCH-UI-04-R2-A — rapport de clôture

Mission : **DESIGN & ERGONOMICS SPECIFICATION**, exclusivement documentaire. Date : 2026-11-28. Base : HEAD distant
vérifié avant intervention `dbc21c8cb29c28a7d8ab7aea049d72e0ccec28fe` (revue Codex), arbre de travail propre, aucune
modification locale écrasée.

## 1. Verdict

**GO documentaire.** Le référentiel couvre les 15 arbitrages. Il est mesurable : critères typés et identifiés, vérifiés
par test automatique, recette physique ou validation subjective. Il ne déclare aucun résultat non exécuté. Les lacunes
de données sont explicites.

Ce verdict **n'autorise pas R2-B**. Il faut attendre la contre-expertise de l'architecte (ChatGPT) et la validation
explicite du propriétaire.

## 2. Couverture des 15 arbitrages

| Arbitrage | Option | Spécification | Exigences |
|---|---|---|---|
| DA-A01 | C English technical compact | [R2-CONTROL-NAMING-SPEC.md](R2-CONTROL-NAMING-SPEC.md), CSV | NAM-01…14 |
| DA-A02 | C SYMBIOTIC INDUSTRIAL Hybrid | [R2-A-MASTER-SPEC.md](R2-A-MASTER-SPEC.md) §4, [R2-DESIGN-SYSTEM-SPEC.md](R2-DESIGN-SYSTEM-SPEC.md) §1–3 | DS-01…05 |
| DA-A03 | C NEXUS Hybrid | Design system §4 | NX-01…12 |
| DA-A04 | C NEURAL CELL Hybrid | Design system §5 | NC-01…10 |
| DA-A05 | D Cybernetic Display (et non Optical Glass Hybrid) | Design system §6 | CD-01…08 |
| DA-A06 | C Plasma Hybrid | Design system §7 | PL-01…06 |
| DA-A07 | C Hybrid Reactive Core | [R2-CORE-INTEGRATION-CONTRACT.md](R2-CORE-INTEGRATION-CONTRACT.md) | CORE-01…16 |
| DA-A08 | C Adaptive Instrument | [R2-WORKSPACE-SPEC.md](R2-WORKSPACE-SPEC.md) §1–2 | WS-P01…16 |
| DA-A09 | C Adaptive Engine Workspace | Espaces §3 | WS-S01…10 |
| DA-A10 | C Engineering Workbench | Espaces §4 | WS-E01…09 |
| DA-A11 | C Adaptive Inspector | Espaces §5 | WS-I01…10 |
| DA-A12 | D Neural Network | [R2-NEURAL-NETWORK-SPEC.md](R2-NEURAL-NETWORK-SPEC.md) §1–3, §6 | NN-01…08, NN-26, NN-27 |
| DA-A13 | C Adaptive Neural Network | Réseaux §4, §7 | NN-09…16, NN-25 |
| DA-A14 | C Dual View | Réseaux §5 | NN-17…24 |
| DA-A15 | C Reactive Signal Flow | [R2-TELEMETRY-FEASIBILITY.md](R2-TELEMETRY-FEASIBILITY.md) | TEL-01…09 |

Les arbitrages ne sont pas rouverts. Les modalités non validées sont classées en propositions (PR-01…10) ou en
décisions ouvertes (OD-01…19). La matrice couvre aussi les critères transversaux du brief §21 (UR, RS, ED, MI, SV, GC,
RT, ZM, VAL) et les recommandations Codex P1-01…P1-06, P2-01…P2-04 et P3-01 (matrice §9 et §10).

## 3. Documents produits

Quatorze fichiers dans `docs/ARCH-UI-04-R2/` : README, MASTER-SPEC, DECISION-REGISTER, DESIGN-SYSTEM-SPEC,
CONTROL-NAMING-SPEC, CONTROL-LABELS-PROPOSALS.csv, WORKSPACE-SPEC, NEURAL-NETWORK-SPEC, CORE-INTEGRATION-CONTRACT,
TELEMETRY-FEASIBILITY, ACCEPTANCE-MATRIX, EXECUTION-PLAN, RISKS-AND-OPEN-DECISIONS, ce rapport. Le tracker principal est
mis à jour (§3nonies, journal, prochaine action).

## 4. Nomenclature

- **2 213 occurrences** : 703 StableID canoniques, 851 variantes d'emplacement mesurées par Codex, 537 options de liste,
  122 éléments hors registre (commandes, titres, états, champs de l'Inspector, unités).
- Libellés actuels : mesures Codex pour les commandes du registre, relevé du code 0.4.1 pour les chaînes hors registre.
- Largeur proposée **estimée** (0,525 × corps par caractère, médiane de 734 mesures JUCE de Codex). Estimations : 703/703
  canoniques et 850/851 variantes dans le budget (une seule exception, `Z · CROSS LVL` sur PLASMA V, deux lignes
  requises) ; 537/537 options, dont 2 à marge inférieure à 8 px. **Confirmation par mesure JUCE requise (PT-02).**
- Distinctions critiques résolues : WT Z, CROSS LVL, WT→FM, FM→WT, BALANCE (pré-chaînes), XAUDIO AMT (post-chaînes),
  OUT GAIN, PEAK (dBFS mesuré).
- Écart avec l'orientation du propriétaire : `out.gain` → OUT GAIN (et non GAIN), à confirmer.
- 1 102 propositions Codex : orientation française rejetée ; mesures, budgets, familles, priorités et constats repris.
- Les noms hôtes et les titres accessibles restent ceux du registre (INV-02, UI-01w).

## 5. Divergences relevées

Neuf divergences ([R2-A-MASTER-SPEC.md](R2-A-MASTER-SPEC.md) §7), dont sept avec les contrats antérieurs :

- DV-01 : la vue réseau devient éditable (l'interdiction du patcher libre est maintenue).
- DV-02 : afficheurs cybernétiques sur verre fumé.
- DV-03 : chaînes visibles françaises.
- DV-04 : libellé = nom du registre sans préfixe.
- DV-05 : Inspector fixe de 300 px, minimum PERFORMANCE de 1 080 × 812, barre sur deux rangées à 1 440 px.
- DV-06 : dix onglets ENGINEERING plats.
- DV-07 : tests fondés sur les chaînes visibles.

Deux sont des écarts entre code et contrat :

- DV-08 : dimensions en dur dans la mise en page PERFORMANCE et la barre, contraires à la règle de `DesignTokens.h` ;
  `tok::stripH` (92) n'est pas utilisé, le bandeau réel mesure 196 px.
- DV-09 : libellé « SPECTRE sortie moteur » face au prélèvement réel en sortie finale.

Une erreur de lecture a été corrigée pendant la mission : l'oscilloscope et la crête sont prélevés en sortie finale
(après OUT GAIN, limiteur et rampe PANIC), et non avant le gain.

## 6. Faisabilité du réseau neuronal

Faisable **sans modification du moteur** pour :

- la structure (55 emplacements FM/CROSS, 64 liens) ;
- les huit états de route ;
- la profondeur configurée et la profondeur effective (`effectiveDepth[55]`) ;
- les fondus de topologie (`edgeGain[55]`) ;
- l'activité des sources (`sources[33]`) ;
- la Dual View sur un modèle unique.

Points à démontrer en prototype : coût de rendu à densité maximale (budget candidat ≤ 2 ms par image) et lisibilité à
64 liens (faisceaux). Non faisable sans évolution du moteur : niveau de signal par route.

## 7. Disponibilité de la télémétrie

30 données inventoriées :

- **classe A** (publiées) : autorisation, y compris l'autorisation de rendu, sources de modulation, valeurs Effective,
  positions WT, profondeurs effectives, fondus, crête, oscilloscope, compteurs de sûreté, CPU du dernier bloc ;
- **classe B** (dérivables côté UI) : spectre limité à fs/4, maintien de crête, moyenne CPU, statut des liens, ordre Z,
  présence des captures, contribution d'un lien en ESTIMATE ;
- **classe C** (internes, non publiées) : étapes d'enveloppe, phase des LFO, exécution autonome par étage ;
- **classe D** (non calculées) : niveaux par route, par opérateur et par chaîne, lecture d'automation.

Le Reactive Signal Flow est réalisable sur A et B seulement. Les modes OFF/REACTIVE/IMMERSIVE restent une proposition.

## 8. Risques principaux

- RK-01 : dépendance des tests aux chaînes visibles.
- RK-04 : marge géométrique de la disposition COMPACT (pad estimé à environ 312 px pour 300 requis).
- RK-05 : redistribution pendant une interaction.
- RK-06 et RK-07 : interactions avec PB-01 et PB-02.
- RK-11 : pression sur le DSP FINAL LOCK pour publier des données C/D.
- RK-19 : effet audio d'une modification de l'interface ou du wrapper hors `source/` (INV-13).

Liste complète : [R2-RISKS-AND-OPEN-DECISIONS.md](R2-RISKS-AND-OPEN-DECISIONS.md) (20 risques).

## 9. Décisions ouvertes

Dix-neuf décisions sont ouvertes. Cinq sont bloquantes pour R2-B :

- OD-01 : repérage des tests (recommandation : identifiant de composant stable) ;
- OD-02 : validation des libellés P1 ;
- OD-06 : plancher typographique à 75 % ;
- OD-10 : périmètre PB-01/PB-02 ;
- OD-18 : rattachement de l'application de la nomenclature à R2-B (le brief définit R2-B comme « composants et tokens »).

OD-11 (glyphes) est à confirmer par PT-05.

## 10. Ordonnancement des lots

R2-B (fondations et nomenclature) → R2-C (PERFORMANCE, barre, enveloppe Core 2D) → R2-D (SOUND DESIGN) → R2-E
(ENGINEERING, Inspector) → R2-F (réseaux) → R2-G (flux réactif) → R2-H (recette intégrée) → ARCH-UI-05 (Core 3D) →
ARCH-UI-06 (intégration finale). C'est le découpage du brief §19. Variantes : R2-E avant R2-D (OD-13) ; R2-F en deux
sous-lots, F1 pour FM et CROSS, F2 pour la matrice (OD-19). Prototypes PT-01…PT-05 en ouverture des lots concernés.

## 11. Critères d'acceptation de R2-B

Entrée : référentiel validé, OD-01, OD-02, OD-06, OD-10, OD-11, OD-18. Sortie : INV-01…13, NAM-01…13, DS-01/02/05,
NX-01…10, NC-01…10, CD-01…08, PL-01…06, AX-01/03, UR-01, ED-01 (absence de crash), MI-01, SV-01, RT-01, ZM-01 ; GC-01
mesuré ; barrière d'inventaire PASS ; CI macOS SUCCESS ; recette physique ciblée NC-03,
NC-05 et NC-07 ; validation subjective NAM-14, DS-03 et CD-03
([R2-ACCEPTANCE-MATRIX.md](R2-ACCEPTANCE-MATRIX.md) §11).

## 12. Contrôle DSP FINAL LOCK

- `git diff d2d08ac -- source/` : **vide** (contrôlé avant le commit).
- Aucune modification de `source/`, `tests/`, `ui/`, `plugin/`, du registre ou de la version.
- Aucune compilation, aucune dépendance, aucun SDK, aucun agent parallèle, aucun worktree supplémentaire.
- Réserves inchangées : R-29 et R-30 non exécutés ; PANIC et sauvegarde pendant un export non exécutés physiquement ;
  limites VST3 ; neuf échecs historiques ; PB-01 et PB-02 au backlog.

## 13. Fichiers modifiés

- Créés : `docs/ARCH-UI-04-R2/` (14 fichiers).
- Modifié : `docs/V1-ARCH-G4-TRACKER.md` (§3nonies, journal, prochaine action).
- Aucun autre fichier.

## 14. SHA

Commit documentaire unique sur `feature/arch-g4-full-development`, parent `dbc21c8`. Le SHA publié est vérifié sur le
dépôt distant après la poussée et consigné dans le message de clôture de la mission. Un fichier ne peut pas contenir le
SHA du commit qui l'introduit.

## 15. Liens

- Dossier : https://github.com/Labtechno/SUBDESTRUCKT/tree/feature/arch-g4-full-development/docs/ARCH-UI-04-R2
- Revue Codex : https://github.com/Labtechno/SUBDESTRUCKT/tree/feature/arch-g4-full-development/docs/ARCH-UI-04-DA
- Tracker : https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/V1-ARCH-G4-TRACKER.md
