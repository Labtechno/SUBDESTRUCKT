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

Les valeurs chiffrées de référence sont dans `gc01-summary-*.csv`. Le rapport de performances de B2-07 les comparera,
série par série, aux mesures de la version intégrée.
