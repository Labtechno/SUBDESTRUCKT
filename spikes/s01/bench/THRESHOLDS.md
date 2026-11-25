# S0.1 — Benchmark Apple Silicon (runner GitHub macos-15 arm64) : protocole et seuils provisoires

Ce document fixe le protocole, les seuils d'acceptation et la règle de décision **avant toute mesure**. Il est commité
avant le premier lancement du workflow `.github/workflows/s01-bench-m1.yml`. Les seuils sont aussi codés dans
`scripts/bench_report.py`, qui les applique sans ajustement après coup.

Ces seuils sont **provisoires**. Ils servent à éclairer l'arbitrage de la direction technique, qui garde la décision
finale. Une machine virtuelle macOS sur M1 n'est pas un Mac M1 physique : ce benchmark **ne certifie pas** le temps réel.

## Unité de mesure

- **Instance :** un réseau FM/CROSS complet de 5 nœuds (WT + OP1 à OP4), avec moteur, retards causaux et décimateur
  temps réel, traité sur un seul cœur.
- **Budget d'un bloc :** durée audio du bloc, soit `taille_bloc / fréquence`.
- **Charge :** temps DSP divisé par le budget, en pourcentage d'un cœur.

## Matrice

| dimension | valeurs |
|---|---|
| fréquence hôte | 44,1 ; 48 ; 88,2 ; 96 kHz |
| oversampling K | 1, 2, 4 |
| référence temporelle | T1 ; T2 (lecteur Lagrange3, comme dans S0.1 ; Thiran1 en plus, au bloc 128, quand le retard est fractionnaire) |
| taille de bloc | 32, 64, 128, 256, 512 |
| topologies | représentatives : `self_feedback` (1 boucle), `wt_op1` (WT ↔ OP1) ; stress : `dense4x4` (16 routes), `wtfm_dense_2cycles` (8 routes, 2 cycles) |
| modes | PM, LinFM, TZFM, ExpFM, à la profondeur nominale `depthLevels(mode)[1]` |

**Validité causale :** T2 à 44,1 kHz / 1× doit être rejeté par le moteur, qui lève `T2NotRepresentable` lors de
`Engine::apply`. Le benchmark tente la configuration pour chaque topologie et mode, et consigne le rejet comme une ligne
`status=rejected` sans mesure. Une configuration acceptée à tort fait échouer le critère S0.

**Protocole contre les biais de virtualisation :**

1. Chauffe globale de 3 s.
2. Chauffe de 0,25 s pour chaque configuration.
3. R = 7 répétitions de 1 s audio par configuration. Les configurations sont parcourues dans un ordre aléatoire
   différent à chaque répétition (graine fixe), pour répartir une éventuelle dérive (fréquence CPU, voisins sur
   l'hôte) sur toutes les configurations.
4. Chaque bloc est chronométré avec `steady_clock`.
5. Les statistiques regroupent tous les blocs de toutes les répétitions. La dispersion entre répétitions est aussi
   publiée.

**Politique de K testée (politique produit) :** K minimal causalement valide, soit T1 : K = 1 partout ; T2 : K = 2 à
44,1 kHz et K = 1 ailleurs.

## Seuils provisoires

| id | critère | seuil |
|---|---|---|
| S0 | validité causale | T2 44,1 kHz / 1× rejeté pour toutes les topologies et tous les modes ; aucune autre configuration rejetée |
| S1 | stabilité | 0 NaN/Inf ; crête de sortie ≤ 2,0 ; 0 allocation dans la boucle temps réel |
| S2a | charge moyenne, représentatives, politique K, blocs ≥ 64 | ≤ 2 % d'un cœur |
| S2b | charge moyenne, stress, politique K, blocs ≥ 64 | ≤ 5 % d'un cœur |
| S3a | queue de distribution : P99 du temps par bloc, toutes topologies, politique K, blocs ≥ 64 | ≤ 20 % du budget |
| S3b | idem, bloc 32 | ≤ 30 % du budget |
| S4 | coût T2/T1 à K identique (rapport des moyennes, toutes configurations valides) | médiane ≤ 1,10 et P90 ≤ 1,25 |
| S5 | surcoût de la politique 2× permanente à 44,1 kHz : T2 K = 2 contre T1 K = 1, blocs ≥ 64 | rapport médian ≤ 3,0 ; charge ajoutée médiane ≤ 1,0 point (représentatives) et ≤ 2,5 points (stress) |
| S6 | latence algorithmique ajoutée par la politique à 44,1 kHz | ≤ 1,0 ms |

Le **maximum** du temps par bloc est publié mais **n'est pas un critère**, car il est dominé par les préemptions de
l'hyperviseur. Les configurations K = 4 et les blocs de 32 entrent dans le critère S4. Pour la charge, elles sont
seulement rapportées, puisqu'elles ne font pas partie de la politique produit.

## Règle de décision, fixée avant la mesure

- **NO-GO :** S0 ou S1 échoue, ou T1 lui-même ne tient pas S2 ou S3. La stratégie B serait alors inabordable, quelle
  que soit la référence temporelle.
- **GO-B-T2 :** S0 à S6 sont tous respectés.
- **GO-B-T1 :** au moins un des critères S2 à S6 échoue pour la politique T2, alors que T1 les respecte.

Les résultats d'écoute (`docs/V1-S0.1-FINAL-LISTENING-REPORT.md`) ne font pas partie de ces seuils. Ils sont rappelés
dans le rapport comme élément d'arbitrage distinct.
