# S0.1 — tableaux de synthèse (générés par scripts/summarize.py)

## 0. Intégrité et stabilité

- Rendus matrice : 4608 ; rendus effectués : 4400 ; refusés (T2 non représentable, K insuffisant) : 208 (tous à 44,1 kHz K=1 avec au moins une arête Delayed : True).
- NaN : 0 ; Inf : 0 ; crête hôte max : 1.109 (sortie moteur ≤ 1 par construction, dépassement = suroscillation du décimateur) ; |valeur retardée| max : 1.294.

| mode | rendus | |f| inst. max (Hz) | fraction max d'échantillons bornés | % chaotiques | rendus |DC|>0.1 | dont à 48 kHz (T1≡T2) | LinFM : fraction max f<0 bloquée | TZFM : fraction max phase rétrograde |
|---|---|---|---|---|---|---|---|---|
| PM | 1100 | 191999.990 | 0.534 | 46.909 | 23 | 6 | 0.000 | 0.000 |
| LinFM | 1100 | 8660.000 | 0.800 | 33.545 | 185 | 46 | 0.997 | 0.000 |
| TZFM | 1100 | 8660.000 | 0.283 | 19.273 | 208 | 52 | 0.000 | 0.506 |
| ExpFM | 1100 | 42993.130 | 0.595 | 23.091 | 181 | 46 | 0.000 | 0.000 |

PM : |f| inst. est dérivé du saut de phase de lecture (borné à 0,5 cycle/éch.) et n'est pas une borne. ExpFM : f0·2^4 ; avec f0 = 1760 Hz et ratio 3, la fréquence dépasse Nyquist hôte (aucune compensation, conforme au baseline).

Verrouillage DC (|DC|>0,1) T1/T2 sur paires identiques : les deux 256, T1 seul 18, T2 seul 47, aucun 1775.

## 1. Retard effectif mesuré (sonde 997 Hz, 10⁻³ rad)

| réf. | fractionnaire | arête | n | min (µs) | max (µs) | |erreur| max (µs) |
|---|---|---|---|---|---|---|
| T1 | — | immediate | 12 | -0.00000 | -0.00000 | 0.00000 |
| T1 | — | delayed | 12 | 10.41667 | 22.67574 | 0.00000 |
| T2 | lagrange3 | immediate | 11 | -0.00000 | -0.00000 | 0.00000 |
| T2 | lagrange3 | delayed | 11 | 20.83333 | 20.83334 | 0.00000 |
| T2 | linear | immediate | 11 | -0.00000 | -0.00000 | 0.00000 |
| T2 | linear | delayed | 11 | 20.83332 | 20.83421 | 0.00088 |
| T2 | thiran1 | immediate | 11 | -0.00000 | -0.00000 | 0.00000 |
| T2 | thiran1 | delayed | 11 | 20.83325 | 20.83452 | 0.00119 |

Refus explicites (T2 à 44,1 kHz K=1) : 6 sondes. Aucune substitution silencieuse par 22,676 µs.

T1 mesuré : 44.1 kHz = 22.676 µs, 48 kHz = 20.833 µs, 88.2 kHz = 11.338 µs, 96 kHz = 10.417 µs ; T2 = 20,833 µs partout où il est autorisé.

## 2. Délai fractionnaire T2 : réponse théorique

| sr | K | D (éch. moteur) | méthode | f (Hz) | gain (dB) | erreur de retard de phase (%) |
|---|---|---|---|---|---|---|
| 44100 | 2 | 1.837 | lagrange3 | 10000 | 0.028 | 0.098 |
| 44100 | 2 | 1.837 | lagrange3 | 20000 | 0.311 | 1.225 |
| 44100 | 2 | 1.837 | lagrange3 | 30000 | 0.853 | 4.072 |
| 44100 | 2 | 1.837 | lagrange3 | 40000 | 1.236 | 7.575 |
| 44100 | 2 | 1.837 | linear | 10000 | -0.297 | 0.430 |
| 44100 | 2 | 1.837 | linear | 20000 | -1.150 | 1.794 |
| 44100 | 2 | 1.837 | linear | 30000 | -2.353 | 4.240 |
| 44100 | 2 | 1.837 | linear | 40000 | -3.305 | 7.535 |
| 44100 | 2 | 1.837 | thiran1 | 10000 | -0.000 | 0.574 |
| 44100 | 2 | 1.837 | thiran1 | 20000 | 0.000 | 2.261 |
| 44100 | 2 | 1.837 | thiran1 | 30000 | -0.000 | 4.862 |
| 44100 | 2 | 1.837 | thiran1 | 40000 | 0.000 | 7.781 |
| 44100 | 4 | 3.675 | lagrange3 | 10000 | -0.003 | 0.001 |
| 44100 | 4 | 3.675 | lagrange3 | 20000 | -0.043 | 0.019 |
| 44100 | 4 | 3.675 | lagrange3 | 30000 | -0.209 | 0.093 |
| 44100 | 4 | 3.675 | lagrange3 | 40000 | -0.621 | 0.283 |
| 44100 | 4 | 3.675 | linear | 10000 | -0.121 | 0.045 |
| 44100 | 4 | 3.675 | linear | 20000 | -0.490 | 0.184 |
| 44100 | 4 | 3.675 | linear | 30000 | -1.121 | 0.438 |
| 44100 | 4 | 3.675 | linear | 40000 | -2.041 | 0.845 |
| 44100 | 4 | 3.675 | thiran1 | 10000 | 0.000 | 0.106 |
| 44100 | 4 | 3.675 | thiran1 | 20000 | 0.000 | 0.430 |
| 44100 | 4 | 3.675 | thiran1 | 30000 | 0.000 | 0.985 |
| 44100 | 4 | 3.675 | thiran1 | 40000 | 0.000 | 1.794 |
| 88200 | 4 | 7.350 | lagrange3 | 10000 | -0.000 | -0.000 |
| 88200 | 4 | 7.350 | lagrange3 | 20000 | -0.003 | -0.001 |
| 88200 | 4 | 7.350 | lagrange3 | 30000 | -0.015 | -0.003 |
| 88200 | 4 | 7.350 | lagrange3 | 40000 | -0.045 | -0.009 |
| 88200 | 4 | 7.350 | linear | 10000 | -0.031 | -0.005 |
| 88200 | 4 | 7.350 | linear | 20000 | -0.126 | -0.020 |
| 88200 | 4 | 7.350 | linear | 30000 | -0.284 | -0.045 |
| 88200 | 4 | 7.350 | linear | 40000 | -0.509 | -0.082 |
| 88200 | 4 | 7.350 | thiran1 | 10000 | 0.000 | -0.040 |
| 88200 | 4 | 7.350 | thiran1 | 20000 | 0.000 | -0.156 |
| 88200 | 4 | 7.350 | thiran1 | 30000 | -0.000 | -0.343 |
| 88200 | 4 | 7.350 | thiran1 | 40000 | 0.000 | -0.588 |

Coût par lecture : Lagrange3 = 4 MAC, Linear = 2 MAC, Thiran1 = 2 MAC + 1 état IIR par source. 44,1 kHz K=2 et 88,2 kHz K=1 partagent fsE = 88,2 kHz et D = 1,8375 (cas le plus défavorable).

### 2b. Méthodes fractionnaires dans les boucles (écart spectral moyen par bandes, dB)

| sr | K | régime | variante | n | vs Lagrange3 moy. | méd. | vs T1 moy. | méd. |
|---|---|---|---|---|---|---|---|---|
| 44100 | 2 | chaotique | T1 | 18 | 3.657 | 0.656 | 0.000 | 0.000 |
| 44100 | 2 | chaotique | T2_lagrange3 | 21 | 0.000 | 0.000 | 5.679 | 0.697 |
| 44100 | 2 | chaotique | T2_linear | 19 | 1.745 | 0.670 | 2.976 | 0.669 |
| 44100 | 2 | chaotique | T2_thiran1 | 21 | 1.761 | 0.540 | 4.735 | 0.573 |
| 44100 | 2 | régulier | T1 | 30 | 2.018 | 0.046 | 0.000 | 0.000 |
| 44100 | 2 | régulier | T2_lagrange3 | 27 | 0.000 | 0.000 | 0.263 | 0.040 |
| 44100 | 2 | régulier | T2_linear | 29 | 0.686 | 0.011 | 1.717 | 0.047 |
| 44100 | 2 | régulier | T2_thiran1 | 27 | 0.089 | 0.000 | 0.274 | 0.035 |
| 44100 | 4 | chaotique | T1 | 18 | 5.750 | 0.667 | 0.000 | 0.000 |
| 44100 | 4 | chaotique | T2_lagrange3 | 19 | 0.000 | 0.000 | 8.596 | 0.677 |
| 44100 | 4 | chaotique | T2_linear | 20 | 4.750 | 0.574 | 5.927 | 0.715 |
| 44100 | 4 | chaotique | T2_thiran1 | 20 | 0.728 | 0.579 | 8.321 | 0.689 |
| 44100 | 4 | régulier | T1 | 30 | 3.363 | 0.051 | 0.000 | 0.000 |
| 44100 | 4 | régulier | T2_lagrange3 | 29 | 0.000 | 0.000 | 1.416 | 0.048 |
| 44100 | 4 | régulier | T2_linear | 28 | 0.397 | 0.005 | 0.474 | 0.049 |
| 44100 | 4 | régulier | T2_thiran1 | 28 | 0.051 | 0.000 | 1.250 | 0.042 |
| 88200 | 1 | chaotique | T1 | 23 | 3.214 | 1.155 | 0.000 | 0.000 |
| 88200 | 1 | chaotique | T2_lagrange3 | 21 | 0.000 | 0.000 | 1.997 | 1.075 |
| 88200 | 1 | chaotique | T2_linear | 19 | 1.748 | 0.671 | 2.515 | 1.033 |
| 88200 | 1 | chaotique | T2_thiran1 | 21 | 1.766 | 0.541 | 2.887 | 1.072 |
| 88200 | 1 | régulier | T1 | 25 | 1.218 | 0.252 | 0.000 | 0.000 |
| 88200 | 1 | régulier | T2_lagrange3 | 27 | 0.000 | 0.000 | 2.313 | 0.294 |
| 88200 | 1 | régulier | T2_linear | 29 | 0.687 | 0.011 | 2.425 | 0.873 |
| 88200 | 1 | régulier | T2_thiran1 | 27 | 0.089 | 0.000 | 2.308 | 0.252 |
| 88200 | 2 | chaotique | T1 | 21 | 8.210 | 1.301 | 0.000 | 0.000 |
| 88200 | 2 | chaotique | T2_lagrange3 | 19 | 0.000 | 0.000 | 4.711 | 1.012 |
| 88200 | 2 | chaotique | T2_linear | 20 | 4.752 | 0.576 | 2.701 | 0.902 |
| 88200 | 2 | chaotique | T2_thiran1 | 20 | 0.728 | 0.579 | 4.627 | 0.826 |
| 88200 | 2 | régulier | T1 | 27 | 0.835 | 0.151 | 0.000 | 0.000 |
| 88200 | 2 | régulier | T2_lagrange3 | 29 | 0.000 | 0.000 | 3.636 | 0.189 |
| 88200 | 2 | régulier | T2_linear | 28 | 0.397 | 0.005 | 2.333 | 0.196 |
| 88200 | 2 | régulier | T2_thiran1 | 28 | 0.051 | 0.000 | 3.497 | 0.176 |

NaN : 0. |valeur retardée| max : T1 1.000, T2_lagrange3 1.294, T2_linear 1.000, T2_thiran1 1.386.

## 3. B-T1 contre B-T2 à configuration identique

| sr | régime | paires | écart T1/T2 moy. (dB) | méd. | p90 | rendus bit-identiques | plancher (perturbation 1e-9) moy. |
|---|---|---|---|---|---|---|---|
| 44100 | chaotique | 113 | 3.880 | 0.719 | 4.210 | 0 | 1.486 |
| 44100 | régulier | 255 | 0.146 | 0.002 | 0.222 | 48 | 0.001 |
| 48000 | chaotique | 180 | 0.000 | 0.000 | 0.000 | 180 | 1.106 |
| 48000 | régulier | 396 | 0.000 | 0.000 | 0.000 | 396 | 0.105 |
| 88200 | chaotique | 184 | 2.892 | 1.013 | 4.409 | 0 | 1.573 |
| 88200 | régulier | 392 | 0.490 | 0.022 | 0.983 | 48 | 0.004 |
| 96000 | chaotique | 186 | 3.799 | 0.779 | 6.804 | 0 | 1.577 |
| 96000 | régulier | 390 | 0.622 | 0.024 | 1.242 | 48 | 0.110 |

Changement de régime entre T1 et T2 (hors 48 kHz) : T1 chaotique/T2 régulier 27, T1 régulier/T2 chaotique 21, sur 1520 paires.

## 4. Invariance inter-sample-rate (écart spectral à 48 kHz, même K, même référence temporelle)

À 48 kHz, T1 et T2 sont identiques bit à bit : la référence est commune. Seules les configurations avec au moins une arête Delayed sont comptées.

| régime | K | sr | n | T1 moy. | T2 moy. | T1 méd. | T2 méd. | T2 plus proche (>0,1 dB) | T1 plus proche | plancher moy. |
|---|---|---|---|---|---|---|---|---|---|---|
| chaotique | 1 | 88200 | 77 | 2.490 | 2.939 | 1.125 | 1.412 | 17 | 41 | 1.804 |
| chaotique | 1 | 96000 | 79 | 2.104 | 4.556 | 1.170 | 1.290 | 20 | 42 | 2.107 |
| chaotique | 2 | 44100 | 73 | 0.801 | 2.878 | 0.580 | 0.738 | 14 | 41 | 2.812 |
| chaotique | 2 | 88200 | 73 | 2.974 | 1.613 | 0.970 | 1.071 | 26 | 29 | 2.276 |
| chaotique | 2 | 96000 | 72 | 2.683 | 3.023 | 0.971 | 1.069 | 22 | 29 | 2.004 |
| chaotique | 4 | 44100 | 41 | 2.798 | 3.971 | 0.592 | 0.853 | 10 | 24 | 1.398 |
| chaotique | 4 | 88200 | 40 | 2.850 | 2.868 | 1.350 | 1.359 | 19 | 15 | 1.459 |
| chaotique | 4 | 96000 | 41 | 3.162 | 5.156 | 1.457 | 1.334 | 19 | 12 | 1.972 |
| régulier | 1 | 88200 | 131 | 0.904 | 1.241 | 0.560 | 0.623 | 20 | 28 | 0.010 |
| régulier | 1 | 96000 | 129 | 0.572 | 0.964 | 0.255 | 0.250 | 22 | 27 | 0.000 |
| régulier | 2 | 44100 | 135 | 0.445 | 0.460 | 0.285 | 0.285 | 8 | 12 | 0.000 |
| régulier | 2 | 88200 | 135 | 0.654 | 0.487 | 0.322 | 0.309 | 30 | 7 | 0.000 |
| régulier | 2 | 96000 | 136 | 0.723 | 0.208 | 0.037 | 0.010 | 31 | 8 | 0.315 |
| régulier | 4 | 44100 | 71 | 0.515 | 0.568 | 0.320 | 0.308 | 5 | 4 | 0.000 |
| régulier | 4 | 88200 | 72 | 0.669 | 0.462 | 0.389 | 0.313 | 16 | 2 | 0.000 |
| régulier | 4 | 96000 | 71 | 0.428 | 0.129 | 0.015 | 0.002 | 20 | 1 | 0.001 |

### 4b. Par mode (régimes réguliers, K = 2 et 4, 88,2 et 96 kHz)

| mode | n | T1 moy. | T2 moy. | T1 méd. | T2 méd. |
|---|---|---|---|---|---|
| PM | 76 | 0.689 | 0.417 | 0.317 | 0.115 |
| LinFM | 99 | 0.962 | 0.257 | 0.112 | 0.081 |
| TZFM | 120 | 0.383 | 0.263 | 0.143 | 0.071 |
| ExpFM | 119 | 0.601 | 0.401 | 0.212 | 0.129 |

## 5. 44,1 kHz : asymétrie T1 1× / T2 2×

Repliement mesuré par l'énergie hors grille harmonique (inharm, dB ; régimes réguliers seulement) et énergie moteur au-dessus de Nyquist hôte avant décimation (oob).

| mode | n | T1 K1 inharm méd. | T1 K2 inharm méd. | T2 K2 inharm méd. | oob K2 méd. (dB) |
|---|---|---|---|---|---|
| PM | 17 | -89.0 | -88.9 | -88.9 | -43.1 |
| LinFM | 20 | -88.4 | -88.3 | -88.3 | -43.2 |
| TZFM | 24 | -46.4 | -51.1 | -33.5 | -42.7 |
| ExpFM | 23 | -13.9 | -13.9 | -13.9 | -43.2 |

Le gain de repliement vient de K=2, pas de T2 : T1 à K=2 l'obtient aussi.

## 6. Transitions topologiques (rampes de profondeur + transaction sticky)

- Rendus : 3168 ; NaN : 0 ; routes survivantes reclassées : 0.
- Classe des nouvelles routes : add_cycle → D, add_ff → I, depth_zero_cross → -, mod_dst_seq → I, mod_dst_xfade → I, mod_src_seq → I, mod_src_xfade → I, remove_del → -, remove_imm → -.

Bouffée HF aux instants structurels (Δ⁴, fenêtres de 32 éch.), en dB relatif au maximum des régimes stationnaires avant/après. ≤ 0 dB = rien au-delà du signal lui-même.

| R (éch. hôte) | courbe | n | burst instants méd. | p99 | max | % > 1 dB | % > 6 dB | burst fenêtre complète max | saut p99 |
|---|---|---|---|---|---|---|---|---|---|
| 0 | hard | 288 | -24.47 | 12.02 | 19.23 | 5.21 | 3.47 | 19.23 | 1.84 |
| 64 | linear | 288 | -4.51 | -0.02 | 0.00 | 0.00 | 0.00 | 0.00 | 0.97 |
| 64 | smooth | 288 | -4.31 | -0.01 | 0.00 | 0.00 | 0.00 | 0.00 | 0.98 |
| 128 | linear | 288 | -4.22 | -0.06 | -0.01 | 0.00 | 0.00 | 0.00 | 0.98 |
| 128 | smooth | 288 | -4.27 | 0.02 | 0.27 | 0.00 | 0.00 | 0.27 | 1.00 |
| 256 | linear | 288 | -3.83 | -0.00 | 0.48 | 0.00 | 0.00 | 5.21 | 1.01 |
| 256 | smooth | 288 | -4.23 | -0.00 | 0.62 | 0.00 | 0.00 | 2.09 | 1.03 |
| 512 | linear | 288 | -4.21 | -0.00 | 0.71 | 0.00 | 0.00 | 2.04 | 1.08 |
| 512 | smooth | 288 | -4.23 | 0.01 | 0.83 | 0.00 | 0.00 | 3.29 | 1.04 |
| 1024 | linear | 288 | -9.77 | -0.02 | 0.26 | 0.00 | 0.00 | 3.47 | 1.10 |
| 1024 | smooth | 288 | -9.75 | -0.15 | 0.01 | 0.00 | 0.00 | 2.82 | 1.05 |

Maximum par scénario (burst aux instants, dB) :

| scénario | sec (R=0) | 64 | 128 | 256 | 512 | 1024 |
|---|---|---|---|---|---|---|
| add_cycle | -2.28 | -0.06 | 0.27 | -0.38 | -0.41 | 0.12 |
| add_ff | -3.83 | 0.00 | -0.06 | -0.01 | 0.04 | -2.68 |
| depth_zero_cross | 19.23 | -0.01 | 0.01 | -0.03 | -0.02 | -2.29 |
| mod_dst_seq | -1.86 | -0.09 | 0.10 | 0.62 | 0.83 | -0.02 |
| mod_dst_xfade | -1.86 | -0.05 | -0.10 | -0.04 | 0.21 | 0.26 |
| mod_src_seq | 6.76 | -0.02 | -0.01 | 0.07 | 0.01 | -0.10 |
| mod_src_xfade | 6.76 | -0.03 | -0.09 | -0.00 | 0.00 | -2.35 |
| remove_del | 11.86 | -0.01 | -0.11 | -0.00 | -0.01 | -2.41 |
| remove_imm | 9.77 | -0.02 | -0.07 | -0.01 | -0.02 | -2.49 |

Latence de geste : R pour ajout/retrait/fondu croisé, 2R pour la modification séquentielle (descente, échange, montée).

## 7. Coût CPU — x86 INDICATIF (AMD Ryzen 7 5800X, Windows, MSVC /O2), PAS Apple M1

- Allocations pendant la boucle temps réel : 0 sur 336 configurations.

| sr | topologie | mode | T1 K1 ns/éch. hôte | T1 K1 p99/128 (ns) | K T2 imposé | T2 ns/éch. hôte | T2/T1 (politique) | T2/T1 à K=2 égal |
|---|---|---|---|---|---|---|---|---|
| 44100 | self_feedback | PM | 77.67 | 78.91 | 2 | 184.46 | 2.37 | 0.99 |
| 44100 | wt_op1 | PM | 86.49 | 88.28 | 2 | 200.78 | 2.32 | 1.01 |
| 44100 | dense4x4 | PM | 103.66 | 119.53 | 2 | 260.96 | 2.52 | 1.12 |
| 44100 | wtfm_dense_2cycles | PM | 114.49 | 126.56 | 2 | 268.84 | 2.35 | 1.06 |
| 44100 | self_feedback | LinFM | 78.05 | 99.22 | 2 | 184.83 | 2.37 | 1.04 |
| 44100 | wt_op1 | LinFM | 86.08 | 94.53 | 2 | 196.90 | 2.29 | 1.01 |
| 44100 | dense4x4 | LinFM | 85.47 | 86.72 | 2 | 222.57 | 2.60 | 1.12 |
| 44100 | wtfm_dense_2cycles | LinFM | 120.31 | 125.78 | 2 | 279.50 | 2.32 | 1.07 |
| 44100 | self_feedback | TZFM | 76.09 | 83.59 | 2 | 180.58 | 2.37 | 1.01 |
| 44100 | wt_op1 | TZFM | 85.78 | 89.84 | 2 | 196.34 | 2.29 | 0.99 |
| 44100 | dense4x4 | TZFM | 109.50 | 128.91 | 2 | 260.66 | 2.38 | 1.05 |
| 44100 | wtfm_dense_2cycles | TZFM | 119.85 | 135.16 | 2 | 273.25 | 2.28 | 1.05 |
| 44100 | self_feedback | ExpFM | 94.34 | 132.03 | 2 | 220.25 | 2.33 | 1.01 |
| 44100 | wt_op1 | ExpFM | 106.01 | 108.59 | 2 | 236.02 | 2.23 | 1.00 |
| 44100 | dense4x4 | ExpFM | 137.90 | 171.88 | 2 | 310.95 | 2.25 | 1.06 |
| 44100 | wtfm_dense_2cycles | ExpFM | 158.64 | 181.25 | 2 | 345.12 | 2.18 | 1.04 |
| 48000 | self_feedback | PM | 78.21 | 79.69 | 1 | 80.31 | 1.03 | 1.03 |
| 48000 | wt_op1 | PM | 88.21 | 125.00 | 1 | 87.64 | 0.99 | 0.99 |
| 48000 | dense4x4 | PM | 104.61 | 135.16 | 1 | 104.55 | 1.00 | 1.01 |
| 48000 | wtfm_dense_2cycles | PM | 115.18 | 140.62 | 1 | 115.29 | 1.00 | 0.99 |
| 48000 | self_feedback | LinFM | 76.74 | 79.69 | 1 | 79.78 | 1.04 | 1.01 |
| 48000 | wt_op1 | LinFM | 86.69 | 97.66 | 1 | 86.93 | 1.00 | 0.94 |
| 48000 | dense4x4 | LinFM | 86.85 | 133.59 | 1 | 85.71 | 0.99 | 0.99 |
| 48000 | wtfm_dense_2cycles | LinFM | 121.86 | 128.91 | 1 | 121.89 | 1.00 | 1.00 |
| 48000 | self_feedback | TZFM | 75.61 | 84.38 | 1 | 77.25 | 1.02 | 1.00 |
| 48000 | wt_op1 | TZFM | 87.26 | 128.91 | 1 | 85.96 | 0.99 | 1.01 |
| 48000 | dense4x4 | TZFM | 112.65 | 146.09 | 1 | 109.34 | 0.97 | 1.00 |
| 48000 | wtfm_dense_2cycles | TZFM | 120.25 | 137.50 | 1 | 121.23 | 1.01 | 1.00 |
| 48000 | self_feedback | ExpFM | 93.87 | 119.53 | 1 | 94.40 | 1.01 | 1.01 |
| 48000 | wt_op1 | ExpFM | 104.43 | 123.44 | 1 | 105.54 | 1.01 | 0.99 |
| 48000 | dense4x4 | ExpFM | 137.28 | 171.09 | 1 | 140.91 | 1.03 | 0.98 |
| 48000 | wtfm_dense_2cycles | ExpFM | 153.00 | 169.53 | 1 | 153.32 | 1.00 | 1.00 |
| 96000 | self_feedback | PM | 77.49 | 110.94 | 1 | 78.63 | 1.01 | 1.01 |
| 96000 | wt_op1 | PM | 84.62 | 107.03 | 1 | 84.23 | 1.00 | 1.00 |
| 96000 | dense4x4 | PM | 106.06 | 148.44 | 1 | 103.82 | 0.98 | 1.00 |
| 96000 | wtfm_dense_2cycles | PM | 113.28 | 140.62 | 1 | 113.76 | 1.00 | 0.96 |
| 96000 | self_feedback | LinFM | 74.81 | 82.81 | 1 | 75.02 | 1.00 | 1.01 |
| 96000 | wt_op1 | LinFM | 83.41 | 89.06 | 1 | 86.68 | 1.04 | 0.99 |
| 96000 | dense4x4 | LinFM | 87.02 | 135.94 | 1 | 85.29 | 0.98 | 0.94 |
| 96000 | wtfm_dense_2cycles | LinFM | 117.63 | 135.16 | 1 | 118.03 | 1.00 | 1.00 |
| 96000 | self_feedback | TZFM | 74.94 | 82.03 | 1 | 75.56 | 1.01 | 0.99 |
| 96000 | wt_op1 | TZFM | 83.84 | 116.41 | 1 | 82.76 | 0.99 | 0.99 |
| 96000 | dense4x4 | TZFM | 110.59 | 139.84 | 1 | 111.04 | 1.00 | 1.03 |
| 96000 | wtfm_dense_2cycles | TZFM | 117.81 | 136.72 | 1 | 119.99 | 1.02 | 0.99 |
| 96000 | self_feedback | ExpFM | 92.64 | 108.59 | 1 | 94.83 | 1.02 | 1.00 |
| 96000 | wt_op1 | ExpFM | 103.61 | 130.47 | 1 | 103.63 | 1.00 | 1.01 |
| 96000 | dense4x4 | ExpFM | 135.95 | 169.53 | 1 | 134.85 | 0.99 | 0.98 |
| 96000 | wtfm_dense_2cycles | ExpFM | 151.57 | 167.19 | 1 | 151.82 | 1.00 | 1.01 |

- Lecteur de retard integer : 0.85 ns par lecture.
- Lecteur de retard lagrange3 : 1.96 ns par lecture.
- Lecteur de retard linear : 1.19 ns par lecture.
- Lecteur de retard thiran1 : 1.32 ns par lecture.
- Décimateur K=2 : 30.1 ns par échantillon hôte.
- Décimateur K=4 : 51.8 ns par échantillon hôte.

## 8. Rendus audio comparatifs — écarts objectifs (aucune écoute effectuée)

| cas | paire | écart X/Y (dB) | X vs REF 48k | Y vs REF 48k |
|---|---|---|---|---|
| c1_fb_moderate | 44k1_policy | 0.406 | 1.788 | 1.388 |
| c1_fb_moderate | 44k1_sameK2 | 0.406 | 1.788 | 1.388 |
| c1_fb_moderate | 96k | 3.824 | 0.000 | 3.824 |
| c2_fb_high | 44k1_policy | 0.016 | 1.123 | 1.112 |
| c2_fb_high | 44k1_sameK2 | 0.003 | 1.123 | 1.125 |
| c2_fb_high | 96k | 0.010 | 0.007 | 0.008 |
| c3_wtfm_moderate | 44k1_policy | 0.710 | 2.652 | 1.984 |
| c3_wtfm_moderate | 44k1_sameK2 | 0.694 | 2.652 | 2.690 |
| c3_wtfm_moderate | 96k | 0.620 | 1.846 | 2.344 |
| c4_wtfm_extreme | 44k1_policy | 1.710 | 0.367 | 1.625 |
| c4_wtfm_extreme | 44k1_sameK2 | 1.378 | 1.625 | 2.863 |
| c4_wtfm_extreme | 96k | 2.250 | 2.257 | 0.550 |
| c5_dense | 44k1_policy | 0.779 | 1.907 | 1.682 |
| c5_dense | 44k1_sameK2 | 0.803 | 1.907 | 1.602 |
| c5_dense | 96k | 0.601 | 0.658 | 0.150 |
| c6_drone | 44k1_policy | 0.343 | 0.621 | 0.720 |
| c6_drone | 44k1_sameK2 | 0.000 | 0.720 | 0.720 |
| c6_drone | 96k | 0.002 | 0.222 | 0.223 |

X/Y aveugles : correspondance T1/T2 dans audio/KEY.md et audio/KEY_audio_metrics.csv.

