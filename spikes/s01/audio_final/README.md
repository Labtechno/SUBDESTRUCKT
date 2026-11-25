# S0.1 — recette perceptive finale (campagne ciblée 96 kHz / 2×)

Campagne d'écoute complémentaire demandée par la direction technique après les 18 comparaisons de la première campagne
(`spikes/s01/audio/`). Elle teste une seule hypothèse : l'avantage numérique de T2 observé à 2× et 4× est-il audible
quand les rendus 96 kHz utilisent eux aussi un oversampling 2× ? Les premiers rendus 96 kHz étaient en 1×.

Cette campagne n'ajoute aucun développement DSP produit et ne modifie aucune spécification. **Aucune écoute n'a été
faite par Claude, et ce document ne contient aucune conclusion perceptive.**

## Livrable d'écoute

`SUBDESTRUCKT_S01_FINAL_LISTENING.zip` contient exactement 14 fichiers :

- les 12 WAV `C1|C2|C3|C5 _ REF|X|Y .wav` ;
- `FEUILLE_ECOUTE_S01_FINALE.odt`, la feuille d'écoute en français, avec 4 rubriques par cas ;
- `INSTRUCTIONS_ECOUTE.txt`.

L'archive ne contient pas la clé X/Y. La taille, l'empreinte SHA-256 et les contrôles des fichiers sont dans
`INTEGRITY.md`.

## Cas et configurations

Les cas sont repris de `audioCases()` dans `src/s01_cli.cpp`, la fonction partagée avec la première campagne. Topologie,
mode, profondeur, f0, forme d'onde, mix et état initial sont donc identiques à la première campagne.

| cas | identifiant | description |
|---|---|---|
| C1 | `c1_fb_moderate` | OP1 en auto-modulation → OP2, PM 1,2 rad, 110 Hz |
| C2 | `c2_fb_high` | OP1 auto-modulé + cycle OP1 ↔ OP2, FM exponentielle 1,5 oct, 110 Hz |
| C3 | `c3_wtfm_moderate` | WT (dent de scie) ↔ OP1, PM 1,0 rad, 110 Hz |
| C5 | `c5_dense` | matrice 4×4 opérateurs, PM 1,0 rad, 110 Hz |

| rôle | rendu moteur | retard des arêtes Delayed | sortie |
|---|---|---|---|
| REF | 48 kHz, K = 2 (96 kHz moteur) | 2 éch. moteur = 20,833 µs | 48 kHz natif |
| X / Y = T1 | 96 kHz, K = 2 (192 kHz moteur) | 2 éch. moteur = 10,417 µs | 96 → 48 kHz hors ligne |
| X / Y = T2 | 96 kHz, K = 2 (192 kHz moteur) | 4 éch. moteur = 20,833 µs | 96 → 48 kHz hors ligne |

Plusieurs propriétés tiennent par construction :

- À 48 kHz, T1 et T2 donnent le même retard. La campagne rend REF avec T1 puis avec T2, et vérifie que les deux rendus
  sont identiques bit à bit ; sinon elle s'arrête.
- À 96 kHz/2×, les deux retards sont des nombres entiers d'échantillons moteur. Le lecteur fractionnaire (Lagrange3,
  comme dans la première campagne) n'intervient donc pas.
- La latence du décimateur temps réel est compensée en échantillons entiers : 32 échantillons hôte pour chaque rendu.

## Chaîne de traitement des fichiers d'écoute

1. **Rendu** : 3,000 s, plus une marge de 2 × 1024 échantillons à 96 kHz pour la queue du filtre, en double précision.
2. **Conversion 96 → 48 kHz**, appliquée identiquement à X et Y ; REF n'est pas rééchantillonné.
   - Filtre : `OfflineDecim2` dans `src/s01_cli.cpp`, outil hors moteur et non temps réel. Sinc à fenêtre de Kaiser,
     2049 coefficients symétriques, coupure 22 kHz, β = 15,571 (A = 150 dB), gain DC normalisé à 1.
   - Phase nulle : le filtre est centré, donc il n'y a aucun retard à compenser.
   - Les valeurs suivantes sont mesurées et enregistrées dans `render/resampler_check.csv`.

   | contrôle | valeur |
   |---|---|
   | bande passante 0–20 kHz | ±3·10⁻⁸ dB |
   | bande passante 0–21,5 kHz | 0,000000 dB |
   | 21,8 kHz | −0,0005 dB |
   | atténuation ≥ 22,5 kHz | ≥ 165 dB |
   | atténuation ≥ 24 kHz | ≥ 168,4 dB |
   | atténuation ≥ 28 kHz (ce qui se replierait dans 0–20 kHz) | ≥ 176,7 dB |
   | erreur sur une sinusoïde 100 Hz / 1 kHz / 10 kHz / 19 kHz | −220 / −199 / −192 / −175 dB |
   | résidu de repliement pour 28 / 36 / 45 kHz | −177 / −183 / −184 dB |

3. **Fondus** : cosinus surélevé de 20 ms en entrée et en sortie, appliqués à 48 kHz de façon identique aux trois
   fichiers.
4. **Niveau** : gain par fichier pour obtenir −20,000 dBFS RMS. La règle prévoit un abaissement commun au cas si une
   crête dépasse −1 dBFS ; il n'a pas été nécessaire. La crête vraie maximale, sur-échantillonnée 4×, est de
   −11,11 dBFS.
5. **Format** : WAV PCM 24 bits, mono, 48 kHz, sans dither, comme la première campagne. Le plancher de quantification
   est à −144 dBFS.
6. **Tirage X/Y** : un tirage `mt19937_64` indépendant par cas. La graine vient de `std::random_device` ; elle est
   enregistrée dans la clé, ce qui permet de reproduire les rendus exactement.

## Clé X/Y et engagement d'intégrité

Le dépôt est public. La clé n'est donc **pas** commitée avant l'écoute : elle reste sur la machine de génération,
ignorée par Git.

| fichier local (non versionné) | SHA-256 |
|---|---|
| `spikes/s01/audio_final/render/KEY_NE_PAS_OUVRIR_AVANT_ECOUTE.md` | `029e9bead88367ddba161fdbfd43af77d03567c98e043840c6699d3a9daa9220` |
| `spikes/s01/audio_final/render/KEY_final_metrics.csv` | `6666a44035192e12d134ac32385b392e00ff818e80963f7b963f683eb1d93a25` |

Ces empreintes sont commitées avant l'écoute. Après réception de la feuille remplie, on pourra donc commiter la clé et
vérifier qu'elle n'a pas changé. `KEY_final_metrics.csv` contient aussi des métriques objectives par fichier : écart
de bandes de tiers d'octave et NMSE par rapport à REF. Elles révèlent indirectement l'attribution et restent donc avec
la clé.

## Reproduction

```
cd spikes/s01
build_msvc.bat                      # ou make (clang/g++)
build\s01_cli.exe final audio_final\render <graine_hex_de_la_clé>
py -3 -I scripts\make_final_listening.py audio_final\render audio_final
```

Sans graine, `s01_cli final` en tire une nouvelle, ce qui produit un nouveau tirage et une nouvelle clé. Avec la même
graine, les 15 fichiers de `render/` sont identiques octet pour octet ; cela a été vérifié sur cette machine.
`make_final_listening.py` génère l'ODT et les instructions, relit et contrôle les 12 WAV indépendamment, construit
l'archive de façon déterministe, puis la vérifie (CRC, liste exacte, contenu identique aux sources, absence de clé).
Il écrit enfin `INTEGRITY.md`.

## Limites méthodologiques

- **Aucune conclusion perceptive.** Claude n'a rien écouté. Les métriques objectives restent avec la clé pour ne pas
  biaiser l'écoute.
- **Faible puissance statistique.** Un auditeur, 4 cas, une comparaison par cas. Si une réponse est « incertain » ou si
  une différence est faible, un test ABX répété (par exemple 10 à 16 essais par cas, avec foobar2000) serait
  nécessaire pour conclure.
- **Écart prévu entre T2 et REF.** À 96 kHz/2×, T2 a exactement le même retard en secondes que REF, et T1 a la moitié.
  Le test mesure donc si l'écart numérique attendu en faveur de T2 est audible ; il ne teste rien d'autre.
- **Haut du spectre.** REF passe par le décimateur interne, dont la bande passante va jusqu'à environ 21,6 kHz. X et Y
  passent en plus par le filtre hors ligne, plat jusqu'à 21,5 kHz. Les différences de traitement se limitent donc
  au-delà de 21,5 kHz, et sont identiques pour X et Y.
- **Régimes sensibles.** Certains cas récursifs peuvent être sensibles aux conditions initiales (voir le rapport S0.1,
  indicateur de chaos). X ou Y peuvent alors s'écarter de REF par leur trajectoire, sans différence de timbre
  systématique.
- **Composante continue.** Les patches originaux produisent une composante continue, jusqu'à environ 0,045 pleine
  échelle sur C2. Elle est identique dans les trois fichiers d'un cas et incluse dans la normalisation RMS. Le niveau
  de la partie audible est donc légèrement inférieur à −20 dBFS, de façon égale pour REF, X et Y.
- **Fichiers courts.** 3 s en mono, comme la première campagne, ce qui ne laisse que peu d'éléments de comparaison.
- **Coût CPU non mesuré.** Le coût de 96 kHz/2× n'est pas évalué ici. Le benchmark M1 reste **PENDING USER MACHINE**.
