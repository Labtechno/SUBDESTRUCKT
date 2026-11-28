# ARCH-UI-04-R2-B2 — protocole de performances graphiques (GC-01)

GC-01 est un **vrai risque d'intégration**. Les primitives R1 multiplient le coût de rendu :

- dégradés multiples ;
- stries d'usinage ;
- rails gradués ;
- reflets ;
- textes espacés.

Elles s'appliquent à des centaines de commandes visibles. Ce protocole compare 0.4.1 et la version intégrée **dans les
mêmes conditions**. Il ne certifie aucune machine.

## 1. Principes

| Principe | Règle |
|---|---|
| Comparatif | Chaque mesure est faite sur 0.4.1 (référence : `73b633b`, code de production inchangé depuis `d2d08ac` pour `source/`) **et** sur la version intégrée, avec le même binaire de mesure, la même machine, la même session et le même ordre de passage en alternance (A, B, A, B). |
| Ligne de base d'abord | Les mesures de 0.4.1 sont faites et publiées **avant** toute modification visuelle (ouverture de B2-02). |
| Seuils avant mesure | Les seuils du §5 sont des seuils candidats, à confirmer par le propriétaire avant la première mesure de la version intégrée (DB2-07). Ils ne sont jamais abaissés après mesure. |
| Plateformes | macOS est la référence. La VM GitHub `macos-15` arm64 donne une mesure **comparative, non certifiante**. Windows sert seulement au développement : ses mesures sont publiées à part et ne valent ni pour macOS ni pour une machine physique. La certification sur machine physique suit DB2-01. |
| Aucune donnée inventée | Les scénarios dynamiques passent par le vrai processeur : blocs audio, modulations réelles, automation par un autre thread. Aucune fausse télémétrie. |
| Instrumentation | Uniquement dans la cible de test (définition de compilation propre aux tests). Le plugin livré n'embarque aucune sonde. |
| Windows et Direct2D | Toute capture hors écran ferme le contexte `Graphics` avant de lire les pixels (leçon du Component Lab, `50cc8aa`). |

## 2. Mesures

| ID | Mesure | Méthode | Scénarios |
|---|---|---|---|
| GP-01 | Temps de rendu complet de l'éditeur | `createComponentSnapshot` de l'éditeur entier, par espace (PERFORMANCE, SOUND DESIGN et chaque onglet, ENGINEERING et chaque outil) et par zoom (75, 100, 150 %) ; 5 rendus de chauffe puis 30 rendus ; médiane et p95 | Statique : preset par défaut et preset de stress de référence |
| GP-02 | Coût d'un rafraîchissement (30 Hz) | Durée de `MainEditor::refreshNow()` (lecture télémétrie et `tick` des commandes visibles), puis rendu des zones invalidées. Mesure par snapshot des composants marqués pour repeint, à l'aide d'un compteur de `paint` propre aux tests | Dynamique : modulations actives (LFO vers 16 destinations visibles), notes tenues, crête variable |
| GP-03 | Fréquence et coût des repeints | Nombre de `paint` par composant et par seconde, coût cumulé par famille | Statique (aucun repeint attendu au repos) ; dynamique (GP-02) |
| GP-04 | Impact sur le traitement audio sous charge | Thread audio dédié traitant des blocs au rythme réel (48 kHz, 128 et 256 échantillons) pendant 60 s, éditeur visible et rafraîchi par le thread de messages ; temps par bloc (p50, p99, max) et échéances manquées. Comparaison avec et sans éditeur, puis avec 0.4.1. UI04-RT (0 allocation) et UI04-DSP (identité bit à bit) restent obligatoires. | Dynamique, preset de stress |
| GP-05 | Mémoire | Mémoire résidente du processus (task_info sous macOS) : avant ouverture, éditeur ouvert, après 100 cycles ouverture / fermeture (UI04-OPEN) | Statique |
| GP-06 | Redimensionnement | Durée de `setSize` puis rendu, sur un balayage de tailles (minimum, nominal, maximum) et de zooms ; vérification RS-01 pendant un glisser | Dynamique |
| GP-07 | Fluidité des interactions | Durée de traitement d'un `mouseDrag` sur NEXUS et PLASMA, repeint du composant compris (p50, p95) ; molette ; clavier | Dynamique |
| GP-08 | HUD actifs | GP-02 limité aux moniteurs : oscilloscope, spectre (FFT UI comprise), crête de la barre, diagnostics | Dynamique |
| GP-09 | Éditeur fermé | Temps par bloc et mémoire, éditeur jamais ouvert, puis fermé après ouverture ; aucun timer d'interface ne doit tourner | Statique |

**Recette physique** (à exécuter par le propriétaire avec un script fourni, selon DB2-01) :

- charge CPU de l'hôte, éditeur ouvert puis fermé, dans Reason 14 ;
- fluidité perçue ;
- 75, 100 et 150 % sur écran Retina.

## 3. Outils

- **Banc de mesure** : nouvel exécutable de test, compilé avec le code de production (même principe que
  `lab/app/Compare.cpp`) et lancé deux fois :
  - sur l'arbre 0.4.1 (`73b633b`) ;
  - sur l'arbre intégré.
- **Résultats** : CSV horodaté par plateforme, avec SHA, machine, OS, fréquence d'échantillonnage, taille de bloc et
  zoom ; journal brut publié.
- **Planification** : workflow macOS dédié aux preuves R2-B2 (TB2-06). La CI de production `arch-g4.yml` n'est pas
  alourdie.

## 4. Points de mesure dans le lot

1. Ligne de base 0.4.1, avant B2-02.
2. Après B2-02 (fondation et LookAndFeel R1).
3. Après chaque famille de B2-03, pour attribuer un écart à la famille qui l'introduit.
4. Mesure finale de B2-07 ; rapport de performances dans le dossier R2-B2.

## 5. Seuils candidats (SC, à confirmer : DB2-07)

Les seuils sont **relatifs à 0.4.1, mesurés sur la même machine**. Les valeurs absolues sont rapportées pour information
seulement. Justification : la cadence de l'éditeur est de 30 Hz (`tok::refreshHz`), soit 33,3 ms par cycle, partagées
avec l'hôte sur le thread de messages.

| ID | Seuil candidat | Justification |
|---|---|---|
| GS-01 | GP-01 : médiane intégrée ≤ 1,5 × médiane 0.4.1, par espace et par zoom | Un rendu complet n'a lieu qu'à l'ouverture, au changement d'espace ou de zoom. Une hausse modérée est acceptable, un facteur supérieur signale un problème de primitive. |
| GS-02 | GP-02 : p95 intégré ≤ 1,5 × p95 0.4.1, **et** p95 ≤ 1/3 du cycle de 30 Hz (11,1 ms) sur la machine de référence | Le rafraîchissement est permanent : il doit laisser au moins deux tiers du cycle à l'hôte. |
| GS-03 | GP-03 : aucun repeint au repos (zéro `paint` sans changement de donnée), comme en 0.4.1 | Règle de cadence TEL-04 |
| GS-04 | GP-04 : échéances manquées intégrées ≤ échéances 0.4.1, et p99 du temps par bloc ≤ 1,05 × 0.4.1 ; UI04-RT et UI04-DSP inchangés | Le traitement audio ne doit pas dépendre de l'interface (INV-13). L'écart de 5 % absorbe la variabilité d'une VM. |
| GS-05 | GP-05 : surcoût mémoire de l'éditeur ouvert ≤ 1,25 × celui de 0.4.1 ; aucune croissance après 100 cycles | Les peintres R1 n'utilisent pas d'image en cache ; une croissance indiquerait une fuite. |
| GS-06 | GP-06 et GP-07 : p95 intégré ≤ 1,5 × 0.4.1 | Interaction directe |
| GS-07 | GP-09 : éditeur fermé identique à 0.4.1 (même temps par bloc à la variabilité près, aucun timer d'interface actif) | Aucun coût sans éditeur |

En cas de dépassement :

1. Analyse par famille, grâce aux points de mesure du §4.
2. Optimisations sans changement visuel : mise en cache de couches statiques et réutilisation des `Path`, entre autres.
   Une simplification visible exige un arbitrage artistique.
3. Si le dépassement persiste, déclaration au rapport et décision du propriétaire.

## Addendum B2-00 (2026-11-28, après GO)

Seuils GS définitifs (DB2-07) et exigences de robustesse CE-B2-04 : voir [B2-00-ADDENDUM.md](B2-00-ADDENDUM.md), qui prime sur ce document pour les points qu'il traite.

Les seuils du §5 sont remplacés par l'addendum §4. GS-02 est mesuré sur M3 de base physique pour sa partie absolue (DB2-01).
