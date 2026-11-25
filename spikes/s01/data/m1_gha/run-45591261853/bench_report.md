# S0.1 — benchmark T1/T2 — Apple M1 virtualisé — runner GitHub macos-15 arm64

Généré par `scripts/bench_report.py` à partir des CSV bruts. Les seuils sont ceux de `bench/THRESHOLDS.md`, fixés avant la mesure.

**Machine virtuelle : ces résultats ne certifient pas le temps réel sur un Mac M1 physique.**

## Machine

```
date_utc            : 2026-11-25T10:03:33Z
run                 : https://github.com/Labtechno/SUBDESTRUCKT/actions/runs/45591261853 (tentative 1)
commit              : 64b13c6a0ed909caad35c729d410ac738de0ad81
runner              : GitHub Actions 1000000091, image macos15 20261118.0550.1, label macos-15
uname -m            : arm64
uname -a            : Darwin iad20-fj920-6746689e-6007-4c2d-ae81-348b525aedbe-C6422A8F3825.local 24.6.0 Darwin Kernel Version 24.6.0: Sun Oct 11 23:44:11 PDT 2026; root:xnu-11417.140.69.713.47~2/RELEASE_ARM64_VMAPPLE arm64
macOS               : macOS 15.8.2 (24H232)
cpu                 : Apple M1 (Virtual)
hw.ncpu             : 3
hw.physicalcpu      : 3
hw.perflevel0 (P)   : 3 cœurs, Standard
hw.perflevel1 (E)   :  cœurs, 
hw.memsize          : 7168 Mio
hw.optional.arm64   : 1
kern.hv_vmm_present : 1  (1 = machine virtuelle)
hw.model            : VirtualMac2,1
translated (Rosetta): 0
compilateur         : Apple clang version 17.0.0 (clang-1700.0.13.5)
cible clang         : arm64-apple-darwin24.6.0
python              : Python 3.15.2
build/s01_tests : arm64 — Mach-O 64-bit executable arm64
build/s01_bench_matrix : arm64 — Mach-O 64-bit executable arm64
S0.1 tests: 87662 pass, 0 fail
```

Configurations : 2000 (1920 mesurées, 80 rejetées). Blocs chronométrés : 11252304.

Dispersion entre répétitions ((max − min) / médiane des moyennes par répétition) : médiane 98.1 %, P95 228.9 %.

## Seuils

| id | critère | mesuré | seuil | résultat |
|---|---|---|---|---|
| S0 | T2 44,1 kHz / 1× rejeté (80 configurations), aucun autre rejet | 80 rejets attendus sur 80 | 80 / 80 | PASSE |
| S1 | NaN/Inf, allocations RT, crête | 0 NaN/Inf, 0 allocations, crête 0.997 | 0, 0, ≤ 2.0 | PASSE |
| S2a (T1) | charge moyenne max, représentatives, politique K, blocs ≥ 64 | 2.775 % | ≤ 2.0 % | ÉCHEC |
| S2b (T1) | charge moyenne max, stress, politique K, blocs ≥ 64 | 4.736 % | ≤ 5.0 % | PASSE |
| S3a (T1) | P99 max / budget, politique K, blocs ≥ 64 | 17.47 % | ≤ 20.0 % | PASSE |
| S3b (T1) | P99 max / budget, politique K, bloc 32 | 16.06 % | ≤ 30.0 % | PASSE |
| S2a (T2) | charge moyenne max, représentatives, politique K, blocs ≥ 64 | 2.532 % | ≤ 2.0 % | ÉCHEC |
| S2b (T2) | charge moyenne max, stress, politique K, blocs ≥ 64 | 4.171 % | ≤ 5.0 % | PASSE |
| S3a (T2) | P99 max / budget, politique K, blocs ≥ 64 | 22.79 % | ≤ 20.0 % | ÉCHEC |
| S3b (T2) | P99 max / budget, politique K, bloc 32 | 14.06 % | ≤ 30.0 % | PASSE |
| S4 | T2/T1 à K identique (880 paires) | médiane 1.034, P90 1.235 | ≤ 1.1 / ≤ 1.25 | PASSE |
| S5 | politique 2× à 44,1 kHz (T2 K2 / T1 K1, blocs ≥ 64) | rapport médian 2.42 ; +0.995 pt (repr.), +1.464 pt (stress) | ≤ 3.0 ; ≤ 1.0 ; ≤ 2.5 | PASSE |
| S6 | latence ajoutée à 44,1 kHz | +0.726 ms | ≤ 1.0 ms | PASSE |

**Règle de décision appliquée mécaniquement : NO-GO.** Cette décision technique ne remplace pas l'arbitrage de la direction technique.

## Charge moyenne (% d'un cœur), bloc 128 — max sur les 4 modes

| fs | K | réf. | self_feedback | wt_op1 | dense4x4 | wtfm_dense_2cycles |
|---|---|---|---|---|---|---|
| 44.1 | 1 ★ | T1 | 0.974 | 0.966 | 1.649 | 1.269 |
| 44.1 | 1 | T2 | rejeté | rejeté | rejeté | rejeté |
| 44.1 | 2 | T1 | 2.831 | 2.180 | 2.988 | 3.431 |
| 44.1 | 2 ★ | T2 | 1.984 | 2.361 | 4.171 | 3.725 |
| 44.1 | 4 | T1 | 4.168 | 4.351 | 5.069 | 6.008 |
| 44.1 | 4 | T2 | 4.774 | 4.503 | 6.677 | 6.092 |
| 48 | 1 ★ | T1 | 0.908 | 0.920 | 1.917 | 1.406 |
| 48 | 1 ★ | T2 | 0.906 | 1.089 | 1.633 | 1.654 |
| 48 | 2 | T1 | 1.831 | 2.498 | 3.158 | 3.567 |
| 48 | 2 | T2 | 2.003 | 2.102 | 3.326 | 3.068 |
| 48 | 4 | T1 | 4.575 | 5.163 | 6.141 | 6.446 |
| 48 | 4 | T2 | 4.870 | 4.369 | 5.773 | 7.301 |
| 88.2 | 1 ★ | T1 | 1.753 | 1.974 | 3.099 | 2.638 |
| 88.2 | 1 ★ | T2 | 1.851 | 1.709 | 3.007 | 2.795 |
| 88.2 | 2 | T1 | 4.189 | 4.622 | 6.301 | 6.272 |
| 88.2 | 2 | T2 | 4.437 | 4.721 | 7.189 | 6.102 |
| 88.2 | 4 | T1 | 8.019 | 7.131 | 12.283 | 10.805 |
| 88.2 | 4 | T2 | 10.136 | 8.808 | 13.897 | 11.648 |
| 96 | 1 ★ | T1 | 1.963 | 2.333 | 3.196 | 3.029 |
| 96 | 1 ★ | T2 | 2.220 | 1.950 | 3.000 | 2.750 |
| 96 | 2 | T1 | 4.872 | 4.155 | 6.050 | 5.957 |
| 96 | 2 | T2 | 4.337 | 5.249 | 7.120 | 5.953 |
| 96 | 4 | T1 | 10.103 | 8.419 | 12.951 | 12.416 |
| 96 | 4 | T2 | 8.475 | 11.040 | 11.677 | 12.559 |

★ = politique K (K minimal causalement valide).

## Taille de bloc — politique K, toutes topologies et modes

| bloc | réf. | charge moy. max (%) | P99/budget max (%) | max/budget max (%) | ns/éch. hôte médian |
|---|---|---|---|---|---|
| 32 | T1 | 3.317 | 16.06 | 4511.4 | 211.5 |
| 32 | T2 | 7.157 | 14.06 | 42244.7 | 225.9 |
| 64 | T1 | 3.325 | 10.72 | 5840.6 | 211.4 |
| 64 | T2 | 3.439 | 12.62 | 2867.6 | 232.1 |
| 128 | T1 | 3.196 | 11.43 | 2720.5 | 205.6 |
| 128 | T2 | 4.171 | 22.79 | 2575.7 | 227.2 |
| 256 | T1 | 4.736 | 17.47 | 2615.8 | 209.8 |
| 256 | T2 | 4.158 | 18.68 | 2095.2 | 231.2 |
| 512 | T1 | 3.420 | 9.87 | 478.5 | 195.9 |
| 512 | T2 | 3.564 | 11.41 | 241.2 | 232.8 |

## Coût relatif T2/T1 à oversampling identique (rapport des temps moyens, médiane [min ; max])

| fs | K | médiane | min | max | retard T2 |
|---|---|---|---|---|---|
| 44.1 | 2 | 1.081 | 0.473 | 1.596 | 1.8375 éch. (lagrange3) |
| 44.1 | 4 | 1.072 | 0.760 | 1.543 | 3.6750 éch. (lagrange3) |
| 48 | 1 | 1.003 | 0.546 | 2.212 | 1.0000 éch. (integer) |
| 48 | 2 | 1.040 | 0.708 | 1.602 | 2.0000 éch. (integer) |
| 48 | 4 | 0.984 | 0.645 | 1.403 | 4.0000 éch. (integer) |
| 88.2 | 1 | 1.037 | 0.557 | 1.525 | 1.8375 éch. (lagrange3) |
| 88.2 | 2 | 1.058 | 0.744 | 1.782 | 3.6750 éch. (lagrange3) |
| 88.2 | 4 | 1.062 | 0.773 | 1.576 | 7.3500 éch. (lagrange3) |
| 96 | 1 | 0.926 | 0.684 | 3.761 | 2.0000 éch. (integer) |
| 96 | 2 | 1.007 | 0.653 | 1.718 | 4.0000 éch. (integer) |
| 96 | 4 | 1.010 | 0.758 | 1.311 | 8.0000 éch. (integer) |

Variante Thiran1 (bloc 128, retards fractionnaires) : T2/T1 médian 1.077 [0.580 ; 1.791].

## Surcoût de la politique 2× permanente à 44,1 kHz (T2 K = 2 contre T1 K = 1)

| topologie | bloc | T1 K1 charge (%) | T2 K2 charge (%) | rapport | points ajoutés |
|---|---|---|---|---|---|
| self_feedback | 64 | 0.753 | 1.699 | 2.25 | +0.945 |
| self_feedback | 128 | 0.676 | 1.392 | 2.06 | +0.716 |
| self_feedback | 512 | 0.621 | 1.660 | 2.67 | +1.039 |
| wt_op1 | 64 | 0.728 | 1.653 | 2.27 | +0.925 |
| wt_op1 | 128 | 0.753 | 1.642 | 2.18 | +0.889 |
| wt_op1 | 512 | 0.680 | 1.796 | 2.64 | +1.116 |
| dense4x4 | 64 | 1.167 | 2.690 | 2.30 | +1.523 |
| dense4x4 | 128 | 1.035 | 2.744 | 2.65 | +1.709 |
| dense4x4 | 512 | 1.089 | 2.625 | 2.41 | +1.536 |
| wtfm_dense_2cycles | 64 | 1.195 | 2.457 | 2.06 | +1.262 |
| wtfm_dense_2cycles | 128 | 0.977 | 2.460 | 2.52 | +1.484 |
| wtfm_dense_2cycles | 512 | 1.057 | 2.548 | 2.41 | +1.491 |

(médianes sur les 4 modes)

## Latence algorithmique (décimateur ; le retard causal n'ajoute pas de latence)

| fs | K | latence (éch. hôte) | latence (ms) |
|---|---|---|---|
| 44.1 | 1 | 0 | 0.000 |
| 44.1 | 2 | 32 | 0.726 |
| 44.1 | 4 | 35 | 0.794 |
| 48 | 1 | 0 | 0.000 |
| 48 | 2 | 32 | 0.667 |
| 48 | 4 | 35 | 0.729 |
| 88.2 | 1 | 0 | 0.000 |
| 88.2 | 2 | 32 | 0.363 |
| 88.2 | 4 | 35 | 0.397 |
| 96 | 1 | 0 | 0.000 |
| 96 | 2 | 32 | 0.333 |
| 96 | 4 | 35 | 0.365 |

## Coûts isolés

| mesure | nom | K | ns |
|---|---|---|---|
| delay_reader | integer | 2 | 1.934 |
| delay_reader | lagrange3 | 2 | 4.204 |
| delay_reader | linear | 2 | 3.494 |
| delay_reader | thiran1 | 2 | 3.872 |
| decimator | latency32_macs67 | 2 | 73.533 |
| decimator | latency35_macs97 | 4 | 133.808 |
