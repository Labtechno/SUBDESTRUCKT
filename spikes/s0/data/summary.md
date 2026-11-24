## 0. Intégrité numérique

- Rendus analysés : 4224 ; NaN : 0 ; Inf : 0 ; pic max : 1.1993 ; |DC| max : 0.1510
- T2 plafonné (D<1 éch. moteur) : 96 rendus, tous à sr=44100 K=1 : [44100] K=[1]

## 1. Retard effectif mesuré (sonde petit signal, 997 Hz)

| strat | ref | edge | max_abs_err_us | measured_min_us | measured_max_us |
|---|---|---|---|---|---|
| A | T1 | delayed_probe | 0.0000 | 10.4167 | 22.6757 |
| A | T1 | immediate_probe | 0.0000 | 10.4167 | 22.6757 |
| A | T2 | delayed_probe | 0.0000 | 20.8333 | 22.6757 |
| A | T2 | immediate_probe | 0.0000 | 20.8333 | 22.6757 |
| B | T1 | delayed_probe | 0.0000 | 10.4167 | 22.6757 |
| B | T1 | immediate_probe | 0.0000 | 0.0000 | 0.0000 |
| B | T2 | delayed_probe | 0.0000 | 20.8333 | 22.6757 |
| B | T2 | immediate_probe | 0.0000 | 0.0000 | 0.0000 |
| Bc | T1 | delayed_probe | 0.0000 | 10.4167 | 22.6757 |
| Bc | T1 | immediate_probe | 0.0000 | 0.0000 | 0.0000 |
| Bc | T2 | delayed_probe | 0.0000 | 20.8333 | 22.6757 |
| Bc | T2 | immediate_probe | 0.0000 | 0.0000 | 0.0000 |

Retard moyen par (strat, ref, sr, K) pour l'arête retardée, en µs :

| ref | K | 44100 | 48000 | 88200 | 96000 |
|---|---|---|---|---|---|
| T1 | 1 | 22.676 | 20.833 | 11.338 | 10.417 |
| T1 | 2 | 22.676 | 20.833 | 11.338 | 10.417 |
| T1 | 4 | 22.676 | 20.833 | 11.338 | 10.417 |
| T2 | 1 | 22.676 | 20.833 | 20.833 | 20.833 |
| T2 | 2 | 20.833 | 20.833 | 20.833 | 20.833 |
| T2 | 4 | 20.833 | 20.833 | 20.833 | 20.833 |

## 2. A vs B sur cascade acyclique (cfg 1) et autres topologies — distance spectrale (moy. |ΔdB| par bande 1/3 oct) et NMSE

| cfg | name | ref | depth | LSD_A_vs_B | NMSE_B_vs_A_dB | ndel_A | ndel_B |
|---|---|---|---|---|---|---|---|
| 1 | cascade_OP1_OP2_OP3_OP4 | T1 | 0.01 | 0.00 | -72.88 | 3 | 0 |
| 1 | cascade_OP1_OP2_OP3_OP4 | T1 | 1.00 | 0.01 | -29.79 | 3 | 0 |
| 1 | cascade_OP1_OP2_OP3_OP4 | T1 | 8.00 | 0.64 | -2.47 | 3 | 0 |
| 2 | self_feedback_OP1 | T1 | 0.01 | 0.00 | -84.92 | 2 | 1 |
| 2 | self_feedback_OP1 | T1 | 1.00 | 0.30 | -38.69 | 2 | 1 |
| 2 | self_feedback_OP1 | T1 | 8.00 | 0.13 | -9.17 | 2 | 1 |
| 3 | cycle_OP1_OP2 | T1 | 0.01 | 0.00 | -84.82 | 2 | 1 |
| 3 | cycle_OP1_OP2 | T1 | 1.00 | 1.24 | -24.54 | 2 | 1 |
| 3 | cycle_OP1_OP2 | T1 | 8.00 | 0.91 | -4.15 | 2 | 1 |
| 4 | cycle_OP1_OP2_OP3 | T1 | 0.01 | 0.00 | -76.47 | 3 | 1 |
| 4 | cycle_OP1_OP2_OP3 | T1 | 1.00 | 1.46 | -16.37 | 3 | 1 |
| 4 | cycle_OP1_OP2_OP3 | T1 | 8.00 | 1.26 | -1.17 | 3 | 1 |
| 5 | dense_4x4 | T1 | 0.01 | 0.15 | -68.90 | 16 | 10 |
| 5 | dense_4x4 | T1 | 1.00 | 5.75 | -0.19 | 16 | 10 |
| 5 | dense_4x4 | T1 | 8.00 | 1.44 | 1.24 | 16 | 10 |
| 6 | WT_OP1_both_ways | T1 | 0.01 | 0.00 | -83.17 | 2 | 1 |
| 6 | WT_OP1_both_ways | T1 | 1.00 | 1.66 | -24.21 | 2 | 1 |
| 6 | WT_OP1_both_ways | T1 | 8.00 | 1.73 | -3.72 | 2 | 1 |
| 7 | WT_OP1_both_ways_fb_OP1 | T1 | 0.01 | 0.00 | -83.17 | 3 | 2 |
| 7 | WT_OP1_both_ways_fb_OP1 | T1 | 1.00 | 1.13 | -18.83 | 3 | 2 |
| 7 | WT_OP1_both_ways_fb_OP1 | T1 | 8.00 | 0.72 | -2.95 | 3 | 2 |
| 8 | WT_multi_OP_both_ways | T1 | 0.01 | 0.00 | -78.03 | 6 | 3 |
| 8 | WT_multi_OP_both_ways | T1 | 1.00 | 1.01 | -13.35 | 6 | 3 |
| 8 | WT_multi_OP_both_ways | T1 | 8.00 | 0.49 | 0.64 | 6 | 3 |

## 3. T1 vs T2 (même stratégie) — distance spectrale par sr (moyenne sur cfg, K, depth ≥ 1.0)

| strat | sr | LSD_T1_vs_T2 |
|---|---|---|
| A | 44100 | 0.77 |
| A | 48000 | 0.00 |
| A | 88200 | 1.70 |
| A | 96000 | 0.93 |
| B | 44100 | 0.56 |
| B | 48000 | 0.00 |
| B | 88200 | 1.28 |
| B | 96000 | 0.74 |

## 4. Invariance au sample rate — LSD entre sr et la référence 48 kHz (K=4, depth ≥ 1.0, moyenne sur cfg)

| strat | ref | K | 44100 | 88200 | 96000 |
|---|---|---|---|---|---|
| A | T1 | 1 | 0.92 | 2.43 | 2.41 |
| A | T1 | 2 | 1.01 | 2.54 | 2.56 |
| A | T1 | 4 | 0.74 | 2.65 | 2.63 |
| A | T2 | 1 | 0.92 | 2.36 | 1.99 |
| A | T2 | 2 | 1.44 | 2.87 | 2.11 |
| A | T2 | 4 | 1.55 | 3.16 | 2.12 |
| B | T1 | 1 | 0.90 | 2.41 | 2.34 |
| B | T1 | 2 | 0.98 | 2.44 | 2.47 |
| B | T1 | 4 | 0.76 | 2.60 | 2.60 |
| B | T2 | 1 | 0.90 | 2.32 | 2.05 |
| B | T2 | 2 | 1.26 | 2.72 | 2.15 |
| B | T2 | 4 | 1.32 | 3.02 | 2.22 |

## 5. Invariance à l'oversampling — LSD (K vs K=4) et NMSE moyen, sr=48 kHz, depth ≥ 1.0

| strat | ref | K | LSD_vs_K4 | median_NMSE_dB |
|---|---|---|---|---|
| A | T1 | 1 | 1.97 | -3.74 |
| A | T1 | 2 | 0.93 | -10.11 |
| A | T2 | 1 | 1.97 | -3.74 |
| A | T2 | 2 | 0.93 | -10.11 |
| B | T1 | 1 | 1.89 | -9.24 |
| B | T1 | 2 | 0.94 | -17.66 |
| B | T2 | 1 | 1.89 | -9.24 |
| B | T2 | 2 | 0.94 | -17.66 |

## 6. Sensibilité (48 kHz, K=1) — LSD A vs B selon f0 et jeu de ratios (depth 1.0)

| f0 | rset | LSD_A_vs_B |
|---|---|---|
| 55 | irrational | 1.86 |
| 55 | rational | 2.07 |
| 110 | irrational | 1.86 |
| 110 | rational | 1.93 |
| 220 | irrational | 1.75 |
| 220 | rational | 2.18 |
| 440 | irrational | 1.68 |
| 440 | rational | 2.96 |
| 997 | irrational | 1.57 |
| 997 | rational | 2.62 |

## 7. Transitions topologiques

click = max|Δy| autour de l'événement / RMS|Δy| précédent ; control = même mesure sans événement (max sur 6 instants). hard = route ajoutée/retirée à profondeur pleine ; ramped = profondeur en rampe 256 éch. (la transaction structurelle se fait à profondeur 0).

Colonnes hard/ramped : click ÷ control (1.0 = indiscernable du signal normal).

| scenario | strat | reclassified | causal | hard_med | hard_max | ramped_med | ramped_max | nan |
|---|---|---|---|---|---|---|---|---|
| add_closing_cycle_cascade | A | 0 | 1 | 3.31 | 10.60 | 2.75 | 6.44 | 0 |
| add_closing_cycle_cascade | B | 0 | 1 | 6.88 | 27.33 | 3.66 | 7.62 | 0 |
| add_closing_cycle_cascade | Bc | 0 | 1 | 6.88 | 27.33 | 3.66 | 7.62 | 0 |
| add_closing_cycle_reverse_chain | A | 0 | 1 | 3.10 | 9.84 | 3.15 | 6.25 | 0 |
| add_closing_cycle_reverse_chain | B | 0 | 1 | 6.03 | 18.76 | 3.18 | 6.41 | 0 |
| add_closing_cycle_reverse_chain | Bc | 1 | 1 | 6.55 | 19.86 | 3.65 | 7.12 | 0 |
| add_closing_cycle_via_WT | A | 0 | 1 | 2.22 | 6.98 | 2.34 | 3.72 | 0 |
| add_closing_cycle_via_WT | B | 0 | 1 | 5.85 | 36.25 | 2.71 | 8.67 | 0 |
| add_closing_cycle_via_WT | Bc | 0 | 1 | 5.85 | 36.25 | 2.71 | 8.67 | 0 |
| remove_oldest_route_cycle3 | A | 0 | 1 | 3.27 | 15.47 | 1.62 | 4.03 | 0 |
| remove_oldest_route_cycle3 | B | 1 | 1 | 4.77 | 36.58 | 1.72 | 6.46 | 0 |
| remove_oldest_route_cycle3 | Bc | 1 | 1 | 4.77 | 36.58 | 1.72 | 6.46 | 0 |
| remove_oldest_route_dense | A | 0 | 1 | 1.21 | 2.21 | 0.79 | 2.23 | 0 |
| remove_oldest_route_dense | B | 0 | 1 | 0.59 | 1.90 | 0.25 | 1.22 | 0 |
| remove_oldest_route_dense | Bc | 0 | 1 | 0.47 | 1.26 | 0.20 | 1.25 | 0 |

## 8. CPU (x86 Linux de développement — NON représentatif du M1), ns par échantillon moteur

| cfgname | strat | ref | 1 | 2 | 4 |
|---|---|---|---|---|---|
| WT_multi_OP_both_ways | A | T1 | 65.3 | 48.6 | 47.3 |
| WT_multi_OP_both_ways | A | T2 | 49.6 | 51.3 | 50.4 |
| WT_multi_OP_both_ways | B | T1 | 77.3 | 74.4 | 98.5 |
| WT_multi_OP_both_ways | B | T2 | 101.3 | 98.4 | 98.2 |
| WT_multi_OP_both_ways | Bc | T1 | 98.0 | 98.0 | 97.3 |
| WT_multi_OP_both_ways | Bc | T2 | 71.8 | 73.8 | 73.0 |
| cascade_OP1_OP2_OP3_OP4 | A | T1 | 100.8 | 104.9 | 94.4 |
| cascade_OP1_OP2_OP3_OP4 | A | T2 | 88.5 | 93.5 | 97.4 |
| cascade_OP1_OP2_OP3_OP4 | B | T1 | 109.3 | 119.0 | 155.3 |
| cascade_OP1_OP2_OP3_OP4 | B | T2 | 102.3 | 133.9 | 155.4 |
| cascade_OP1_OP2_OP3_OP4 | Bc | T1 | 134.0 | 129.9 | 141.3 |
| cascade_OP1_OP2_OP3_OP4 | Bc | T2 | 120.3 | 132.4 | 103.9 |
| cycle_OP1_OP2 | A | T1 | 63.2 | 68.1 | 71.5 |
| cycle_OP1_OP2 | A | T2 | 59.4 | 59.2 | 61.0 |
| cycle_OP1_OP2 | B | T1 | 57.8 | 75.2 | 72.0 |
| cycle_OP1_OP2 | B | T2 | 85.5 | 62.0 | 107.2 |
| cycle_OP1_OP2 | Bc | T1 | 105.9 | 89.2 | 109.7 |
| cycle_OP1_OP2 | Bc | T2 | 112.0 | 113.3 | 102.0 |
| dense_4x4 | A | T1 | 120.0 | 130.1 | 120.4 |
| dense_4x4 | A | T2 | 73.5 | 113.8 | 86.7 |
| dense_4x4 | B | T1 | 109.0 | 97.6 | 94.7 |
| dense_4x4 | B | T2 | 90.5 | 84.6 | 101.7 |
| dense_4x4 | Bc | T1 | 116.2 | 106.0 | 128.8 |
| dense_4x4 | Bc | T2 | 130.5 | 127.8 | 100.6 |

Temps de compilation du graphe (hors RT), ns :

| cfgname | A | B | Bc |
|---|---|---|---|
| WT_multi_OP_both_ways | 36 | 354 | 177 |
| cascade_OP1_OP2_OP3_OP4 | 66 | 340 | 214 |
| cycle_OP1_OP2 | 42 | 259 | 221 |
| dense_4x4 | 79 | 396 | 225 |

