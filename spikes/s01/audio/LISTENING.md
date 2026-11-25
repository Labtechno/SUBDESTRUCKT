# S0.1 — feuille d'écoute (à remplir par une personne ; Claude n'a rien écouté)

Pour chaque cas : écouter `REF_48k` (T1 = T2 à 48 kHz), puis X et Y. Niveaux RMS alignés à -20 dBFS.
Noter : différence audible (oui/non/incertain), lequel est le plus proche de la référence 48 kHz, préférence, remarques.

| cas | paire | audible ? | plus proche de REF | préférence | remarques |
|---|---|---|---|---|---|
| c1_fb_moderate | 44k1_policy | | | | |
| c1_fb_moderate | 44k1_sameK2 | | | | |
| c1_fb_moderate | 96k | | | | |
| c2_fb_high | 44k1_policy | | | | |
| c2_fb_high | 44k1_sameK2 | | | | |
| c2_fb_high | 96k | | | | |
| c3_wtfm_moderate | 44k1_policy | | | | |
| c3_wtfm_moderate | 44k1_sameK2 | | | | |
| c3_wtfm_moderate | 96k | | | | |
| c4_wtfm_extreme | 44k1_policy | | | | |
| c4_wtfm_extreme | 44k1_sameK2 | | | | |
| c4_wtfm_extreme | 96k | | | | |
| c5_dense | 44k1_policy | | | | |
| c5_dense | 44k1_sameK2 | | | | |
| c5_dense | 96k | | | | |
| c6_drone | 44k1_policy | | | | |
| c6_drone | 44k1_sameK2 | | | | |
| c6_drone | 96k | | | | |

Descriptions des cas :

- `c1_fb_moderate` : OP1 self-feedback -> OP2, PM 1.2 rad, 110 Hz
- `c2_fb_high` : OP1 self + cycle OP1<->OP2, Exponential FM 1.5 oct, 110 Hz
- `c3_wtfm_moderate` : WT(saw)<->OP1, PM 1.0 rad, 110 Hz
- `c4_wtfm_extreme` : WT(asym)<->OP1..3, TZFM 4000 Hz, 110 Hz
- `c5_dense` : dense 4x4 OP, PM 1.0 rad, 110 Hz
- `c6_drone` : WT/FM 2 cycles independants, ExpFM 0.3 oct, 55 Hz
