# ARCH-UI-04-R2-B2 — GC-01 : ligne de base 0.4.1 et mesures comparatives

Protocole : [`../B2-GRAPHICS-PERFORMANCE-PROTOCOL.md`](../B2-GRAPHICS-PERFORMANCE-PROTOCOL.md). Seuils définitifs
GS-01 à GS-07 et exigences de robustesse CE-B2-04 : [`../B2-00-ADDENDUM.md`](../B2-00-ADDENDUM.md) §3-4. Les seuils sont
enregistrés **avant** la première mesure de la version intégrée.

## 1. Banc

`tests/perf/Gc01Bench.cpp` (cible `subdestruckt_gc01_bench`). Il n'est pas exécuté par la CI de production ; il tourne
dans le workflow `r2b2-evidence.yml` sous macOS et localement sous Windows.

| Élément | Méthode |
|---|---|
| Éditeur | `MainEditor` réel, dans l'hôte de test commun (`tests/support/HeadlessPeer.h`), qui enregistre les zones réellement invalidées |
| Rendu | `paintEntireComponent` dans une image, échelle 2 (équivalent Retina), rendu forcé jusqu'au bout par la lecture d'un pixel. macOS : image native CoreGraphics (sur CPU, celle du plugin). Windows : moteur logiciel de JUCE, car l'image native Direct2D impose une relecture GPU → CPU qui dominait la mesure (×180, écartée après essai). |
| Données | Réelles : blocs audio du processeur à 48 kHz par blocs de 256, note tenue, LFO 1 routé vers les commandes continues visibles de PERFORMANCE. Aucune télémétrie simulée. |
| Robustesse | 5 rendus de chauffe, 30 répétitions (×6 pour le rafraîchissement), données brutes publiées (`gc01-raw-*.csv`), statistiques (n, médiane, p95, p99, MAD, min, max, valeurs > médiane + 5 MAD comptées et conservées), conditions (`gc01-conditions-*.txt`) |
| Alternance | Le workflow exécute référence, courant, référence, courant sur la même VM. La référence est l'arbre de la ligne de base, exporté par `git archive`, avec le même banc. |
| Limites | Un snapshot mesure un **coût de rendu**, pas la fluidité perçue dans Reason. La VM GitHub n'est pas une machine M3 : GS-02 (partie absolue, 11,1 ms) et GS-06 (16,7 ms) exigent la mesure physique M3 (DB2-01). Windows : plateforme secondaire, mesures publiées à part. |

## 1 bis. Révision du banc avant la référence (v2)

La première ligne de base macOS (run 2, `run2-ad180b9/`, conservée pour traçabilité) a révélé deux **artefacts du
banc**, corrigés avant que la référence ne soit fixée. Les seuils et le code mesuré n'ont pas changé.

| Mesure | Artefact du banc v1 | Correction (v2) |
|---|---|---|
| GP-04 | « Échéances manquées » d'environ 2 780 blocs sur 3 750, que l'éditeur soit actif ou fermé, pour un traitement médian de 0,25 ms : le compteur mesurait le retard de réveil d'un thread sans priorité temps réel sur une VM à 3 cœurs | Dépassement = **temps de traitement d'un bloc > période (5,33 ms)**. Le retard de réveil est publié à part (`-retard-reveil-info`), pour information seulement. |
| GP-05 | +118 MiB après 100 cycles sous macOS : sans boucle d'événements, les objets CoreGraphics et Objective-C ne sont libérés qu'à la vidange d'un *autorelease pool* | Pool de libération autour de chaque rendu et de chaque cycle (`JUCE_AUTORELEASEPOOL`) |

**Banc v3.** La ligne de base macOS v2 (run 3, `run3-44fc225/`) montre encore +119 MiB après 100 cycles, malgré les
pools de libération. Une mesure unique ne distingue pas une fuite d'une montée en charge des caches ou de l'allocateur.
Le banc v3 relève donc la mémoire résidente tous les 10 cycles sur 200 cycles, et publie séparément la croissance de la
**première** et de la **seconde** moitié. GS-05 (« aucune croissance persistante après stabilisation ») se juge sur la
seconde moitié, comparée à la référence.

La référence GC-01 est l'arbre du commit qui contient le banc v3, sans modification de `ui/`. Ce commit est consigné
dans `r2b2-evidence.yml` (`GC01_REF_SHA`) au début des modifications visuelles.

## 2. Ligne de base 0.4.1

Le code mesuré est `ui/` de 0.4.1, inchangé, avec le banc ajouté. Voir les dossiers par plateforme :

- [`windows/baseline/`](windows/baseline) : poste de développement, deux exécutions de la référence avec le banc v2
  (`baseline-v1/`, `baseline-v2/` : bancs remplacés, conservés) ;
- [`macos/`](macos) : VM GitHub, deux exécutions de la référence par run.

Observations de référence (valables sur les deux plateformes, à confirmer par les fichiers) :

- **Repos stabilisé (GS-03).** 0.4.1 produit **un repeint par rafraîchissement** : l'oscilloscope `ScopeView` (zone
  814 × 152) se redessine à chaque tick même sans signal. C'est donc un repeint périodique au repos dans la référence
  elle-même. GS-03 s'applique à la version intégrée : le HUD de l'oscilloscope ne devra se redessiner que si la donnée
  change (B2-03 (5)).
- **Mémoire (GS-05).** Aucune croissance persistante après 100 cycles d'ouverture et de fermeture.
- **Audio (GS-04).** Aucune échéance manquée, éditeur actif ou fermé.

### Valeurs de référence (banc v3, arbre `0517426`)

macOS : `macos/run4-0517426/`, VM GitHub macOS 15.8.2, « Apple M1 (Virtual) », 3 cœurs, **non certifiant**.
Windows : `windows/baseline/`, poste de développement.

| Mesure | Série | macOS VM (exécutions 1 / 2) | Windows (exécutions 1 / 2) |
|---|---|---|---|
| GP-01 | Rendu complet, médiane des médianes des 42 vues × zooms (ms) | 13.17 / 12.05 | 11.39 / 11.37 |
| GP-01 | p95 le plus élevé, toutes vues (ms) | 75.28 / 84.80 | 27.45 / 30.16 |
| GP-02 | Rafraîchissement dynamique, médiane (ms) | 17.71 / 14.28 | 10.45 / 9.33 |
| GP-02 | Rafraîchissement dynamique, p95 (ms) | 29.45 / 21.62 | 13.95 / 12.84 |
| GP-03 | Zones invalidées au repos par rafraîchissement | 1.00 / 1.00 | 1.00 / 1.00 |
| GP-07 | Interaction directe NEXUS, p95 (ms) | 2.57 / 2.30 | 0.76 / 0.81 |
| GP-08 | HUD oscilloscope, médiane (ms) | 4.22 / 3.53 | 2.00 / 1.98 |
| GP-08 | HUD spectre, médiane (ms) | 6.66 / 5.92 | 3.50 / 3.46 |
| GP-06 | Redimensionnement 1440 × 900, médiane (ms) | 41.30 / 37.12 | 27.66 / 27.49 |
| GP-04 | Bloc audio, éditeur actif, p99 (ms) | 0.53 / 0.57 | 0.24 / 0.24 |
| GP-04 | Dépassements de traitement, éditeur actif (blocs) | 0.00 / 2.00 | 0.00 / 0.00 |
| GP-04 | Dépassements de traitement, éditeur fermé (blocs) | 1.00 / 0.00 | 0.00 / 0.00 |
| GP-05 | Surcoût mémoire de l'éditeur (MiB) | 95.30 / 89.73 | 38.52 / 38.64 |
| GP-05 | Croissance, 100 premiers cycles (MiB) | 123.08 / 122.89 | 1.88 / 1.91 |
| GP-05 | Croissance, 100 cycles suivants (MiB) | 0.83 / 0.83 | 2.55 / 2.95 |

**Lecture** :

- La dispersion d'une exécution à l'autre (jusqu'à 25 % sur le rafraîchissement dynamique de la VM) justifie
  l'alternance et les deux paires de mesures.
- Mémoire macOS : la croissance des 100 premiers cycles a lieu **entièrement pendant les 10 premiers** (montée en charge
  des caches). La seconde moitié ne croît que de 0,8 MiB.
- Les dépassements de traitement de la VM (0 à 2 blocs sur 3 750, éditeur actif comme fermé) relèvent du bruit de la
  VM. Ils servent de plancher de comparaison et ne sont pas attribués à l'interface.

Les valeurs chiffrées complètes sont dans `gc01-summary-*.csv`. Le rapport de performances de B2-07 les comparera,
série par série, aux mesures de la version intégrée.

## 3. Statut des mesures

| Mesures | Statut |
|---|---|
| `macos/run2-ad180b9/`, `windows/baseline-v1/` (banc v1) | **Invalidées** (artefacts GP-04 et GP-05), conservées pour la traçabilité, **jamais utilisées en comparaison** |
| `macos/run3-44fc225/`, `windows/baseline-v2/` (banc v2) | **Remplacées** (mémoire non concluante sans série), conservées, jamais utilisées en comparaison |
| `macos/run4-0517426/`, `windows/baseline/` (banc v3, arbre `0517426`) | **Référence figée.** Elle n'est jamais recalculée pour faciliter une validation. |

Plateformes : Windows (développement), macOS VM (comparatif, non certifiant) et macOS physique M3 (certification,
recette du propriétaire) sont toujours publiés séparément.

## 4. Contrôle A/A et plancher de bruit de la VM (CE-B2-04)

**Run 5** (`macos/run5-0d576ed/`) : première alternance référence `0517426` / courant `0d576ed`. Le **rendu de production est
identique de part et d'autre** : le module `ui/r2` est compilé dans le plugin mais n'est pas encore utilisé. C'est donc un
contrôle A/A, qui mesure le bruit propre à la VM GitHub.

| Série | Écart A/A observé | Conséquence |
|---|---|---|
| GP-01, rendu complet (pire vue × zoom) | jusqu'à +23 % | Marge de GS-01 (50 %) au-dessus du bruit : jugeable sur VM |
| GP-02, rafraichissement (p95) | jusqu'à 26 % | Marge de GS-02 (50 %) au-dessus du bruit : jugeable sur VM (partie comparative) |
| GP-04, bloc audio, éditeur fermé (p99) | **+16 %** | Marge de GS-04 et GS-07 (5 %) **dans le bruit** : **NON CONCLUANT sur VM**, mesure physique M3 requise |
| GP-05, surcoût mémoire | 1,6 % | Jugeable |
| GP-07, interaction (p95) | 2,4 % | Jugeable |

**Outil.** `tools/gc01_compare.py <run> --aa=<run A/A>` confronte chaque run aux seuils GS définitifs. Un verdict est **NON
CONCLUANT** lorsque l'écart A/A de la même série atteint la marge du seuil : ni PASS ni ÉCHEC. Les parties absolues de
GS-02 (11,1 ms) et de GS-06 (16,7 ms) ne sont jamais jugées sur VM.

## 5. Dispersion interne et première mesure après fondation (run 7)

Le run 7 révèle une **dispersion bimodale du traitement audio sur la VM**. Le même binaire de référence donne une médiane
par bloc de 0,08 ms puis de 0,51 ms d'une exécution à l'autre. Un thread ordinaire est placé sur un cœur performance ou
sur un cœur efficacité de la VM Apple Silicon. L'outil intègre donc la **dispersion interne** des exécutions d'un même
binaire au plancher de bruit (CE-B2-04). GS-04 et GS-07 sont NON CONCLUANTS sur VM.

Verdicts du run 7 : [`macos/run7-42a605e/gc01-verdicts.txt`](macos/run7-42a605e/gc01-verdicts.txt) ; détail par série :
`gc01-compare.csv`.

## 6. B2-03.1 NEXUS (run 8, `f22bf8c`)

Verdicts : [`macos/run8-f22bf8c/gc01-verdicts.txt`](macos/run8-f22bf8c/gc01-verdicts.txt).

| Seuil | Verdict |
|---|---|
| **GS-01** | **ÉCHEC**, ×1,62 sur ENG-1 à 75 % |
| GS-02, GS-03, GS-05, GS-06 | PASS |
| GS-04, GS-07 | NON CONCLUANT |

GS-01 : les exécutions du courant et de la référence ne se recouvrent pas, la régression est réelle. Cause : mesures de
texte de la sérigraphie refaites à chaque peinture. Correction sans dérive : 270 → 99 µs par NEXUS (B2-03-REPORT §1.6).
GS-01 est remesuré avec B2-03.2. Les seuils ne sont pas modifiés.
