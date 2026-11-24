# SUBDESTRUCKT — Rapport du spike S0 — Causalité FM/CROSS
Date : 2026-11-24. Statut : **RÉSULTATS À ARBITRER**. Aucune spécification produit modifiée. Code et données : `spikes/s0/` (branche `spike/s0-causality`, non fusionnée).

## 1. Recommandation
**GO-B-T2** — retard ciblé sur les seuls cycles (B), durée de retard physique fixe de 1/48000 s (T2), avec les conditions du §8.

Niveau de confiance par composante :
- **B plutôt que A : bien étayé.** Les deux stratégies sont causalement correctes, déterministes et stables. B garde un retard nul sur les routes acycliques (mesuré), A les retarde de 10 à 23 µs.
- **T2 plutôt que T1 : moins étayé.** T2 donne un retard constant et mesuré. Les métriques spectrales de cette campagne ne séparent pas T1 de T2 au-delà du bruit de mesure (§5). Le choix repose sur l'invariance physique du retard, pas sur une différence audible démontrée. Le coût de l'inverse est faible : passer à GO-B-T1 supprime le délai fractionnaire.

## 2. Ce qui a été construit
Harnais C++17 autonome (hors VST3, hors JUCE, sans allocation dans la boucle de rendu) :
- 5 nœuds sinus (0 = WT simplifié, 1–4 = OP1–4), PM en radians, matrice 5×5 avec feedback, somme bornée à ±32 rad.
- Stratégie **A** : toute route lit une valeur retardée.
- Stratégie **B** : les routes sont classées par priorité de création. Une route est retardée seulement si elle ferme un cycle avec des routes plus anciennes, ou si c'est un feedback propre. Les autres sont évaluées dans le même échantillon, en ordre topologique.
- Variante **Bc** : classification canonique par DFS, sans priorité. Elle sert à mesurer la stabilité du choix des routes retardées.
- **T1** : retard = 1 échantillon hôte (K échantillons moteur à K× oversampling). **T2** : 1/48000 s, délai fractionnaire causal (Lagrange cubique sur `y[n-d0 … n-d0-3]`, `d0 ≥ 1`), plancher à 1 échantillon moteur.
- Transactions de topologie appliquées à l'échantillon exact, état conservé (phases, historiques).
- Les rendus et les 10 configurations de la mission sont dans `spikes/s0/`. Le détail des tableaux est dans `spikes/s0/data/summary.md`.

## 3. Invariants (§9 de la mission)
| Invariant | Résultat | Preuve |
|---|---|---|
| Aucune dépendance instantanée implicite | **PASS** | Vérification statique de l'ordre d'évaluation sur toutes les configurations. Preuve dynamique : les sorties courantes non calculées sont remplies de NaN. 0 NaN sur 3 stratégies × 2 références × 8 configurations × K=1/2/4 × 2 fréquences d'échantillonnage. Témoin négatif : un cycle forcé en immédiat est bien détecté. |
| Même rendu, même configuration | **PASS** | Rejeu et reset d'état : hash identique. |
| Indépendance à la taille de bloc | **PASS, bit-exact** | Blocs 16/32/64/128/256/512/1024, 4 fréquences d'échantillonnage, K=1/2/4, 3 stratégies, 2 références. Événements structurels à des échantillons non alignés sur le bloc. |
| Pas de NaN/Inf | **PASS** | 4224 rendus de la matrice : 0 NaN, 0 Inf. Sortie moteur bornée par construction (sinus). Pic observé 1,20 après décimation FIR (suroscillation du filtre). |
| Topologie transactionnelle | **PASS** | Voir §6. |
| Profondeur zéro sans reclassification | **PASS** | La classification ne lit jamais la profondeur. Balayage continu de −1 à +1 : 0 reclassification, 0 NaN. |
| Pas d'allocation / de verrou dans la boucle | **PASS (revue de code)** | Tableaux de taille fixe, aucune allocation après construction. Non mesuré par un outil d'instrumentation. |

Les 5371 vérifications sont dans `tests/s0_tests.cpp` (`make test`).

## 4. Retard effectif mesuré
Sonde petit signal (997 Hz, profondeur 10⁻³ rad), phase relevée par démodulation complexe.

| Référence | 44,1 kHz | 48 kHz | 88,2 kHz | 96 kHz |
|---|---|---|---|---|
| T1, toutes K | 22,676 µs | 20,833 µs | 11,338 µs | 10,417 µs |
| T2, K=1 | **22,676 µs (plancher)** | 20,833 µs | 20,833 µs | 20,833 µs |
| T2, K=2 et K=4 | 20,833 µs | 20,833 µs | 20,833 µs | 20,833 µs |

- Erreur de mesure inférieure à 10⁻⁴ µs partout.
- **A** : toute route mesure ce retard. **B** : les routes immédiates mesurent **0,000 µs**, les routes retardées mesurent ce retard.
- T1 varie d'un facteur 2,2 entre 44,1 et 96 kHz.
- **T2 n'est pas honorable à 44,1 kHz en K=1** : 20,833 µs vaut 0,919 échantillon, ce qui exigerait de lire l'échantillon courant. Le plancher donne 22,676 µs (+8,8 %). À 44,1 kHz il faut K ≥ 2 pour obtenir exactement 20,833 µs.
- Délai fractionnaire (`data/lagrange_response.md`) : erreur de gain ≤ 0,31 dB à 20 kHz et erreur de retard ≤ 0,1 % dans tous les cas fractionnaires. Il n'est pas la source des différences mesurées plus bas.

## 5. Comparaisons spectrales
Métrique : écart moyen en dB sur 23 bandes de tiers d'octave (63 Hz – 15,8 kHz), niveaux relatifs à la puissance 0–20 kHz, plancher −80 dB. Plus NMSE temporel. Fenêtre de 1 s après 0,1 s de transitoire.

**A contre B (T1, 48 kHz, K=1, f0=220 Hz) :**

| Topologie | Profondeur 0,01 rad | 1 rad | 8 rad |
|---|---|---|---|
| Cascade OP1→OP2→OP3→OP4 | 0,00 dB (NMSE −73) | 0,01 dB (NMSE −30) | 0,64 dB (NMSE −2,5) |
| Cycle OP1↔OP2 | 0,00 (−85) | 1,24 (−25) | 0,90 (−4,2) |
| Dense 4×4 | 0,15 (−69) | **5,42 (−0,2)** | 1,40 (+1,2) |
| WT↔OP1 + feedback OP1 | 0,00 (−83) | 1,29 (−19) | 0,73 (−3,0) |

Lecture : à profondeur élevée, la forme d'onde de A et de B diffère complètement (NMSE proche de 0 dB), même si le spectre par bandes reste proche. Sur la cascade à 8 rad, A et B ne produisent plus le même signal. L'écart moyen des raies reste modeste, sauf le réseau dense à 1 rad.

**T1 contre T2 (même stratégie, depth ≥ 1) :** écart moyen 0,56–1,70 dB selon la fréquence d'échantillonnage (0 à 48 kHz, où T1 = T2 par construction).

**Invariance à la fréquence d'échantillonnage (écart à 48 kHz) :** 0,7–1,5 dB à 44,1 kHz et 2,0–3,2 dB à 88,2 et 96 kHz. T2 est légèrement meilleur à 96 kHz et légèrement moins bon à 44,1 kHz avec K ≥ 2. Ces différences sont du même ordre que la variabilité des régimes chaotiques : **la campagne ne discrimine pas T1 de T2 sur cette métrique.**

**Invariance à l'oversampling (48 kHz, depth ≥ 1) :** écart K=1 contre K=4 de 1,9 dB, K=2 contre K=4 de 0,9 dB, identique pour T1 et T2. B donne un NMSE meilleur que A (−9,2 contre −3,7 dB à K=1), à interpréter avec prudence car les régimes sont chaotiques.

## 6. Transitions topologiques
Cinq scénarios (quatre détaillés ci-dessous, le cinquième est un ajout fermant un cycle via le WT), tous causaux et sans NaN après transaction. Le « click » est le plus grand saut d'échantillon autour de l'événement, divisé par la variation normale du signal (1,0 = indiscernable).

| Scénario | Routes existantes reclassées (A / B / Bc) | Saut brut, médiane (A / B) | Saut avec rampe de profondeur, médiane (A / B) |
|---|---|---|---|
| Ajout d'une route qui ferme un cycle (cascade) | 0 / 0 / 0 | 3,3 / 6,9 | 2,8 / 3,7 |
| Ajout fermant un cycle (chaîne inversée) | 0 / 0 / **1** | 3,1 / 6,0 | 3,2 / 3,2 |
| Retrait de la plus ancienne route (cycle à 3) | 0 / **1** / 1 | 3,3 / 4,8 | 1,6 / 1,7 |
| Retrait de la plus ancienne route (dense) | 0 / 0 / 0 | 1,2 / 0,6 | 0,8 / 0,3 |

- Avec la classification par priorité, **ajouter une route ne change jamais la classe des routes existantes**. La variante canonique Bc le fait (1 route dans le scénario « chaîne inversée »).
- **Retirer une route ancienne peut faire passer une route plus récente de retardée à immédiate** (1 des 2 scénarios de retrait). Ce changement est décrit par la compilation du graphe mais il modifie le son de la route concernée.
- Un ajout ou un retrait à pleine profondeur produit un saut net dans A comme dans B. **Une rampe de profondeur est obligatoire** avant toute transaction structurelle, quelle que soit la stratégie.

## 7. Coût
Mesuré sur la machine de développement (x86 Linux, 2 cœurs partagés), **pas sur Apple M1**.
- Médianes de 72 ns/échantillon moteur (A) contre 98 ns (B), avec des écarts de 47 à 155 ns d'un cas à l'autre. Le bruit de mesure est du même ordre que l'écart : **aucune conclusion CPU**.
- Compilation du graphe hors RT : 36–80 ns (A) contre 260–400 ns (B). Négligeable à 5 nœuds.
- Les mesures M1, les coûts à plus de 5 nœuds et l'impact de l'oversampling réel (le moteur K× est ici sans filtre de rééchantillonnage temps réel) restent à faire.

## 8. Conditions attachées à la recommandation GO-B-T2
1. **Priorité des routes persistée** : la stabilité de B repose sur un ordre de priorité des routes stable et sauvegardé dans le preset (ordre de création ou identifiant de route). Sans cela, deux presets identiques peuvent classer leurs routes différemment.
2. **Rampes de profondeur obligatoires** avant ajout ou retrait de route (§6).
3. **Retrait d'une route** : la compilation doit signaler les routes survivantes dont la classe change, et l'interface doit pouvoir l'indiquer.
4. **Plancher T2 à 44,1 kHz en K=1** : à arbitrer (§9, question 1).
5. Le moteur K× reste à valider avec un vrai rééchantillonnage temps réel : le harnais n'applique que la décimation hors ligne.

## 9. Questions à arbitrer
1. **T2 à 44,1 kHz en K=1** : accepter 22,676 µs (+8,8 %) documenté, imposer K ≥ 2 pour les presets avec feedback à 44,1 kHz, ou choisir T1.
2. **Priorité des routes** : confirmer que l'ordre de création (ou un identifiant stable) fait partie de l'état de configuration persistant.
3. **Scope des sources de retard** : T2 s'applique-t-il aussi aux retards internes de la matrice 4×4 et au WT↔OP, ou seulement aux feedbacks ? Le harnais applique le même retard partout.

## 10. Hypothèses et limites
- 5 nœuds sinus, PM uniquement. FM linéaire, TZFM, FM exponentielle, formes d'onde multiples, vrai WT, Z, Character et Cross Bus ne sont pas testés.
- PM de sinus est borné par construction : aucun emballement possible, donc le critère « stabilité » n'a pas pu être discriminant ici. Il le sera pour FM linéaire ou exponentielle.
- Les régimes chaotiques sont sensibles aux détails numériques : les écarts spectraux inférieurs à 1–1,5 dB ne sont pas significatifs.
- La métrique par bandes de tiers d'octave a été corrigée pendant le spike : la première version, avec des bandes ISO, plaçait les raies de 220 Hz sur les bords de bande et donnait un plancher de bruit de 3 dB. La grille a été décalée d'un sixième d'octave, la normalisation unifiée sur 0–20 kHz et un plancher à −80 dB ajouté. Seuls les résultats de la dernière version sont rapportés.
- Pas d'écoute : aucune évaluation perceptive. Les écarts en dB ne disent rien du caractère audible ou musical.
- Mesures CPU non représentatives du M1. La décimation est hors ligne et n'est pas mesurée.
- Aucun test dans un hôte VST3.

## 11. Reproduction
```
cd spikes/s0
make test                 # 5371 vérifications
make
./build/s0_cli matrix data && ./build/s0_cli delay data
./build/s0_cli transition data && ./build/s0_cli perf data
python3 scripts/summarize.py data > data/summary.md
python3 scripts/lagrange_response.py > data/lagrange_response.md
```
Durée : environ 1 min 40 s sur 2 cœurs.
