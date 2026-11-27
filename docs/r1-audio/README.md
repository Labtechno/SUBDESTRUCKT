# Rendus de recette ARCH-G4.3-R1 (moteur réel, aucun binaire)

Fichiers WAV PCM 24 bits produits par les tests du cœur au commit de la build 0.3.3 (`SBD_R2_WAV_DIR`, `SBD_R1_WAV_DIR`).
Écoute A/B réservée au propriétaire (recette R-30 et R-29 du manuel 0.3.3).

| Fichier | Contenu | Gain |
|---|---|---|
| `r2ab_capture48k_restore48k_reference.wav` | capture S2 (Decay 1, dent de scie note 45) faite à 48 kHz, restituée à 48 kHz (référence), 1 s depuis l'activation, stéréo 48 kHz | commun au groupe r2ab |
| `r2ab_capture48k_restore44.1k.wav` | même capture restituée à 44,1 kHz (moteur 88,2 kHz) | commun |
| `r2ab_capture48k_restore88.2k.wav` | restituée à 88,2 kHz | commun |
| `r2ab_capture48k_restore96.0k.wav` | restituée à 96 kHz | commun |
| `za01_cross_audio_tilt.wav` | Harmonic Tilt (v 0,8, σ 0,6) dont Z Master est modulé à fréquence audio par le CROSS OP1→WT.Z (ratio 0,5) : suivi à la cadence de resynthèse (375 Hz à 48 kHz) | commun au groupe za01 |
| `za01_cross_audio_fold.wav` | même modulation appliquée à Harmonic Fold (suivi par échantillon) | commun |

Aucune normalisation indépendante : les fichiers d'un même groupe partagent un gain unique (crête du groupe à −6 dBFS).
