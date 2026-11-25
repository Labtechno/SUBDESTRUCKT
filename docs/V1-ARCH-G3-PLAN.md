# SUBDESTRUCKT — ARCH-G3 — Plan, contrats et stratégie de test (étape 1)

Date : 2026-11-25. Branche : `feature/arch-g3-vertical-slice`, créée depuis `main` `5cc21a61138740c3122741ee5d56d98de954d03a`.
Statut : **plan de l'étape 1, rédigé et commité avant tout code**. Les seuils de test ci-dessous sont fixés ici et ne
seront pas ajustés après mesure. En cas d'échec, la règle est : corriger le code, ou rapporter l'échec.

Cadre normatif : `V1-ARCH-G3-MISSION.md`, `V1-CAUSAL-CONTRACT.md`, `V1-SOFTWARE-ARCHITECTURE.md`,
`V1-CORE-CONTRACTS-P0.md`, `V1-PLATFORM-REQUIREMENTS.md`, `V1-PACKAGING-RELEASE-CONTRACT.md`. Ce plan n'en modifie
aucun.

## 1. Périmètre (rappel de la mission) et exclusions

**Dans la tranche :**

- MIDI monophonique : note-on et note-off appliqués à leur offset ;
- un oscillateur wavetable minimal ;
- deux opérateurs FM en modulation de phase (PM) ;
- CROSS bidirectionnel avec B-T2 ;
- amplificateur et sortie stéréo ;
- preset minimal (état initial) ;
- UI de diagnostic.

**Hors tranche :**

- axe Z, Destruction, filtres, matrice globale, banques factory, UI finale ;
- LinFM, TZFM et ExpFM (dette conservée) ;
- pitch bend, aftertouch, CC autres que la sécurité, MPE ;
- Quality Eco / Normal / High, Drone, Freeze, undo/redo de topologie.

Aucun paramètre artistique T1. Aucun interpolateur choisi automatiquement selon le preset.

## 2. Outillage et dépendances épinglés

| élément | version épinglée | vérification et licence |
|---|---|---|
| JUCE | **9.0.6**, commit `654bdbc7a9db0dae90849b44abe91f339e9f0768` | Clone `--branch 9.0.6`, puis vérification de `HEAD`. **Licence : double AGPLv3 / licence commerciale JUCE 9**, d'après `LICENSE.md` au tag. |
| SDK VST3 | 3.8.0, fourni avec JUCE dans `juce_audio_processors_headless/format_types/VST3_SDK` | **MIT** (SBOM `JUCE.spdx.json`). « VST » est une marque de Steinberg. |
| pluginval | **1.0.4** (Tracktion), asset `pluginval_macOS.zip` | SHA-256 `3c4c533bda0c5059eea3ddaea752d757ee2025041f0f47e6bcb0e87f6082b29f`, vérifié en CI. |
| CMake | ≥ 3.22 (exigence de JUCE 9) | Version réelle du runner journalisée. |
| Compilateur | Apple Clang de l'image `macos-15` (Xcode par défaut) | Version journalisée. C++17. |
| CI | GitHub Actions `macos-15`, arm64, runner standard gratuit pour dépôt public | Caractéristiques de la VM journalisées. |
| Analyse | Python 3, bibliothèque standard uniquement | — |

**Licence, point bloquant pour toute diffusion :** JUCE 9 est utilisable soit sous AGPLv3, auquel cas l'ensemble
distribué doit être compatible AGPLv3, soit sous licence commerciale JUCE 9. Ce choix n'est pas arbitré. Le dépôt est
public : un lien de téléchargement dans ce dépôt rend techniquement le binaire accessible publiquement. Ce point est à
arbitrer par la direction technique avant toute diffusion au-delà de la recette interne. Les mentions de licence
tierces (JUCE AGPLv3, VST3 SDK MIT) sont livrées dans le bundle et rappelées dans le README.

**Cible de déploiement macOS :** **14.0**, déclarée et utilisée par la compilation (`CMAKE_OSX_DEPLOYMENT_TARGET`) et
par le précontrôle de l'installateur. Justification : aucun Mac M3, la cible minimale, ne fonctionne sous un macOS
antérieur à 14. **Ce n'est pas une version minimale validée.** Seule la VM macOS 15.x de la CI exécutera des tests. La
version minimale officielle reste à décider, comme le prévoit le contrat.

## 3. Identité produit (stable, à ne jamais changer)

| élément | valeur |
|---|---|
| nom | SUBDESTRUCKT |
| fabricant | Labtechno |
| code fabricant / code plugin (VST3 Class ID dérivé par JUCE) | `Lbtn` / `Sbdk` |
| identifiant du bundle VST3 | `com.labtechno.subdestruckt` |
| reçu du paquet composant | `com.labtechno.subdestruckt.vst3` |
| produit installateur / désinstallateur | `com.labtechno.subdestruckt.installer` / `com.labtechno.subdestruckt.uninstaller` |
| version | **0.1.0** (`CFBundleShortVersionString`, version VST3) |
| identité de build | `0.1.0-b<N>`, N = numéro de run CI (`CFBundleVersion` = N ; clés `SUBDESTRUCKTBuild` et `SUBDESTRUCKTCommit` dans `Info.plist`) |
| version des paquets | `0.1.0.<N>` |
| archive | `SUBDESTRUCKT_v0.1.0-b<N>_macOS_arm64.zip` |

Chaque reconstruction produit un N distinct, donc une identité distincte. Une livraison publiée n'est jamais écrasée.

## 4. Plan de fichiers (séparation des couches)

| dossier | couche | contenu | dépend de JUCE |
|---|---|---|---|
| `source/dsp/` | L3 primitives | wavetable limitée en bande, opérateur sinus, lecteur de retard causal (entier / Thiran1), décimateur demi-bande, ADSR, rampes, garde DC | non |
| `source/core/` | L1 orchestration | registre des paramètres (StableID), table de routes FM/CROSS (B sticky, transactions, compilation), codec d'état versionné | non |
| `source/engine/` | L2 temps réel | moteur (voix mono, réseau WT/OP1/OP2, transitions, amp, sûreté, télémétrie), échange lock-free des topologies | non |
| `plugin/` | L0 wrapper | `AudioProcessor` JUCE (pont StableID ↔ paramètres hôte, MIDI, état, latence) | oui |
| `ui/` | UI | éditeur de diagnostic (paramètres, topologie, télémétrie) | oui |
| `tests/core/` | tests natifs | tests du cœur sans hôte ni JUCE | non |
| `tests/wrapper/` | tests du wrapper | `AudioProcessor` instancié directement : allocations, état, instances | oui |
| `tests/host/` | tests d'hôte | chargement du `.vst3` compilé par l'hébergement VST3 de JUCE | oui |
| `packaging/` | livraison | scripts pkg, Distribution, textes FR, tests de paquets | non |
| `.github/workflows/arch-g3.yml` | CI | compilation, tests, pluginval, paquets, livraison | — |

Dépendances unidirectionnelles : `plugin`/`ui` → `engine` → `core` → `dsp`. Le cœur n'inclut aucun en-tête JUCE et ne
contient aucun code conditionnel de plateforme.

## 5. Contrats de la tranche

### 5.1 Graphe FM/CROSS et B-T2 (V1-CAUSAL-CONTRACT)

- **Nœuds :** WT, OP1, OP2. **Huit routes possibles :**
  - CROSS WT → FM : WT→OP1, WT→OP2 ;
  - CROSS FM → WT : OP1→WT, OP2→WT ;
  - FM interne : OP2→OP1, OP1→OP2, et les auto-boucles OP1→OP1, OP2→OP2.
- **Classe d'une route ajoutée :** Delayed si c'est une auto-boucle ou si elle ferme un cycle dans le sous-graphe
  Immediate courant, en tenant compte des ajouts antérieurs de la même transaction ; sinon Immediate.
- **Persistance des classes :** une route survivante garde sa classe (sticky), même si le cycle disparaît. Modifier la
  source ou la destination revient à retirer puis ajouter, avec un nouvel identifiant. Une profondeur nulle ne retire
  ni la route ni sa classe.
- **Identifiants :** StableRouteID strictement croissants, jamais réutilisés.
- **Transactions :** atomiques. Une transaction invalide est rejetée sans effet partiel.
- **Existence des routes :** structurelle et transactionnelle (UI de diagnostic, état). La profondeur, elle, est un
  paramètre hôte continu.
- **T2 :** durée nominale 1/48000 s, appliquée uniquement aux arêtes Delayed. Retard en échantillons moteur
  `D = Fs·K/48000`, toujours ≥ 1.
  - D entier : lecture entière exacte.
  - Sinon : Thiran1 causal, `d0 = floor(D − 0,5)`, `f = D − d0`, `a = (1 − f)/(1 + f)`, avec
    `y[n] = a·x[n−d0] + x[n−d0−1] − a·y[n−1]` (ne lit que des échantillons passés).
  - Les lecteurs par source tournent en continu ; leur état est initialisé explicitement à la préparation et au Panic.
- **Politique de K (provisoire, Quality non implémentée) :**
  - 44,1 kHz : K = 2, en permanence, même sans route Delayed ;
  - 48, 88,2 et 96 kHz : K = 1, d'où D = 1 (entier), 1,8375 (Thiran1) et 2 (entier) ;
  - autre fréquence : plus petit K de {1, 2, 4} donnant D ≥ 1 ; en dessous, silence et drapeau de sûreté ;
  - K n'est jamais modifié par une édition de topologie.
- **Horloge commune** WT/OP1/OP2/CROSS à `Fs·K`.
- **Latence :** celle du décimateur, déclarée à l'hôte (`setLatencySamples`). Elle vaut 32 échantillons hôte à 44,1 kHz
  et 0 pour K = 1, et ne change qu'au `prepareToPlay`. Le retard récursif T2 ne fait pas partie de la PDC.
- **Transitions de topologie :** préparées et compilées hors RT, puis publiées par échange lock-free. Côté RT :
  1. fondu de sortie de 128 échantillons hôte sur les arêtes retirées, avec l'ancien graphe ;
  2. bascule atomique ;
  3. fondu d'entrée de 128 échantillons hôte sur les arêtes ajoutées, avec le nouveau graphe.

  **L'union de l'ancien et du nouveau graphe n'est jamais exécutée.** Les survivants gardent leur état et leur classe.
  Pas de double slot. Aucune promesse d'absence de clic : seules des mesures sont rapportées. Les graphes retirés sont
  libérés hors RT.

### 5.2 Voix, MIDI, amp, sûreté (CORE-CONTRACTS-P0)

- **Monophonie, priorité dernière note :**
  - une note-on relance l'enveloppe Amp à partir de son niveau courant ;
  - au relâchement de la note jouée, si d'autres notes sont tenues, la voix revient à la plus récente, sans relance ;
  - les phases des oscillateurs sont libres : jamais remises à zéro par une note.
- **Ordre à l'intérieur d'un même échantillon (C11) :** sécurité → expression → note-off → note-on.
- **Messages de sécurité :** CC120 déclenche le Panic ; CC123 est un « All Notes Off ».
- **Note-off (C02) :** ferme le gain de voix sans remettre les états DSP à zéro.
- **Panic (C06) :** rampe de 5 ms vers le silence, puis purge des historiques de retard, des états Thiran, du
  décimateur et de la garde DC.
- **Amp :** ADSR à segments linéaires appliqué à la fréquence moteur, avant décimation (alignement avec la latence) ;
  vélocité réglable.
- **Sortie :** gain de sortie, garde DC (passe-haut d'ordre 1 à 5 Hz, sur la sortie audio seulement, jamais sur un
  chemin de modulation), puis garde numérique.
  - Une valeur non finie provoque la purge des états, la sortie de zéros pour le bloc et l'incrément d'un compteur.
  - Au-delà de |y| > 32 (+30 dBFS), écrêtage et compteur. Dépasser 0 dBFS en flottant reste autorisé (C21).
  - Dénormaux : `ScopedNoDenormals` dans le wrapper.
- **Base et Effective (C24) :**
  - l'hôte automatise la valeur Base ; le cœur calcule la valeur Effective sans réécrire le paramètre hôte ;
  - CROSS : Effective = profondeur × CROSS Master × intensité de la direction ;
  - lissage par rampes linéaires de 20 ms, qui atteignent la cible exactement ;
  - borne DSP dure de ±32 rad sur la somme PM d'une destination.

### 5.3 Paramètres hôte (StableID explicites, jamais dérivés d'indices JUCE)

| StableID | unité | plage | défaut |
|---|---|---|---|
| `amp.attack` / `amp.decay` / `amp.sustain` / `amp.release` | s / s / lin / s | 0,001–5 / 0,001–5 / 0–1 / 0,001–10 | 0,005 / 0,25 / 0,8 / 0,3 |
| `amp.velocity` | lin | 0–1 | 1 |
| `out.gain` | dB | −48 à +6 | −6 |
| `wt.level` / `op1.level` / `op2.level` | lin | 0–1 | 0,5 / 0,5 / 0 |
| `op1.ratio` / `op2.ratio` | × f0 | 0,25–16 | 1 / 2 |
| `fm.op2_op1.depth`, `fm.op1_op2.depth`, `fm.op1_op1.depth`, `fm.op2_op2.depth` | rad | 0–8 | 1 / 0 / 0 / 0 |
| `cross.wt_op1.depth`, `cross.wt_op2.depth`, `cross.op1_wt.depth`, `cross.op2_wt.depth` | rad | 0–8 | 0,5 / 0 / 0,5 / 0 |
| `cross.master` / `cross.fwd` (WT→FM) / `cross.rev` (FM→WT) | lin | 0–1 | 1 / 1 / 1 |

**Preset minimal (état initial) :** les paramètres ci-dessus, avec la topologie construite dans cet ordre :

1. OP2→OP1 → classe I ;
2. WT→OP1 → classe I ;
3. OP1→WT → classe D, car elle ferme le cycle WT→OP1→WT.

### 5.4 État persistant (format v1)

- Texte UTF-8 : en-tête `SUBDESTRUCKT_STATE 1`, valeurs Base par StableID, puis les routes (`id`, `src`, `dst`,
  `class`) et `nextRouteId`.
- **Validation complète avant tout effet :**
  - version : une version future est refusée explicitement ;
  - valeurs : finies, dans leur plage ;
  - nœuds valides ;
  - identifiants uniques et inférieurs à `nextRouteId` ;
  - paires uniques ;
  - une auto-boucle doit être de classe D ;
  - sous-graphe Immediate acyclique.
- Un état invalide est refusé, et l'état courant est conservé.
- Le Runtime State (phases, historiques) n'est pas persisté.

## 6. Hôtes et API réellement disponibles

| moyen | disponible | usage |
|---|---|---|
| Hébergement VST3 de JUCE 9.0.6 (`VST3PluginFormat`), dans un exécutable de test sans interface graphique | oui, en CI macOS arm64 | découverte, instanciation, latence rapportée, matrice fréquences × blocs, état, instances multiples |
| pluginval 1.0.4 | oui, en CI | validation VST3 automatisée |
| Validateur Steinberg | **non** : absent du sous-ensemble du SDK VST3 fourni avec JUCE | non exécuté |
| DAW commerciale (lecture réelle, PDC dans un projet, clavier MIDI) | **non** en CI | **PENDING** : recette physique sur le M4 Max, puis sur un M3 physique avant toute Beta |
| RTSan (LLVM, `-fsanitize=realtime`) | à essayer en CI (LLVM via Homebrew) | job complémentaire ; s'il ne peut pas s'exécuter, ce sera indiqué comme non exécuté |

## 7. Stratégie de test et seuils (fixés avant le code)

Chaque test produit un statut : **PASS** (exécuté et réussi), **ÉCHEC**, **NON EXÉCUTÉ** (avec motif) ou **N/A**
(justifié). La CI macOS arm64 fait foi. Une compilation locale sur un autre système ne sert que de vérification de
développement et n'est jamais rapportée comme résultat.

| id | domaine | critère et seuil |
|---|---|---|
| G-01 | DAG / sticky | Règles de classification, auto-boucle → D, cycle créé par l'union d'ajouts d'une même transaction → D sur la route qui le ferme, survivants jamais reclassés, profondeur 0 sans retrait. ≥ 1000 séquences de transactions aléatoires : sous-graphe Immediate toujours acyclique, identifiants jamais réutilisés. **0 échec.** |
| G-02 | transactions | Transaction invalide (doublon, identifiant inconnu, nœud invalide) : rejet complet, état identique bit à bit. **0 effet partiel.** |
| G-03 | état | Aller-retour sérialisation → désérialisation identique. Refus : version future, cycle Immediate, auto-boucle I, identifiant ou paire en double, valeur hors plage ou non finie. **100 % des cas.** |
| G-04 | causalité | Empoisonnement par NaN des sorties non encore calculées : aucune lecture Immediate d'un nœud non calculé, aucune lecture retardée d'un échantillon courant. Contrôle négatif détecté. **0 violation.** |
| G-05 | retard | D mesuré par impulsion pour chaque (fréquence, K) de la politique : D entier exact ; Thiran1 : \|H\| = 1 ± 1e−12, retard de phase en DC = D ± 1e−6, à 1 kHz = D ± 0,5 %. Retard de phase jusqu'à 20 kHz **rapporté** (pas de seuil : un passe-tout ne garantit pas un délai de groupe constant). \|a\| < 1. |
| G-06 | latence | Pic de la réponse impulsionnelle du décimateur à exactement la latence déclarée (K = 2 et 4). Latence = 32 à 44,1 kHz, 0 à 48, 88,2 et 96 kHz. **Égalité exacte.** |
| G-07 | déterminisme | Deux rendus identiques bit à bit. Invariance à la taille de bloc (1, 7, 32, 64, 128, 256, 512, et tailles variables), paramètres constants, MIDI à offsets fixes : **différence maximale = 0.** |
| G-08 | offsets MIDI | À K = 1, premier échantillon non nul exactement à l'offset de la note-on. Ordre dans un même échantillon (note-off puis note-on) vérifié. Priorité dernière note vérifiée. |
| G-09 | WT seul | Niveaux FM à 0, CROSS sans effet : énergie inharmonique ≤ −40 dB ; harmoniques 1 à 10 en 1/k à ±0,5 dB (dent de scie limitée en bande). |
| G-10 | FM seul | WT à 0, OP2→OP1 en PM, indice I = 1 rad, f_m = f0/4 : bandes latérales k = 0 à 3 conformes à \|J_k(1)\| à **±0,5 dB**. |
| G-11 | CROSS WT→FM | Sortie FM seule, WT→OP1 actif contre `cross.fwd` = 0 : NMSE ≥ −20 dB et écart spectral moyen ≥ 1 dB. Inactif : identique bit à bit, avant la garde DC, à la référence sans route. |
| G-12 | CROSS FM→WT | Sortie WT seule, OP1→WT actif contre `cross.rev` = 0 : mêmes seuils que G-11. |
| G-13 | CROSS = 0 sans reset | Passage de `cross.master` à 0 en cours de note : une fois la rampe terminée, sortie moteur (avant garde DC) **identique bit à bit** à une référence n'ayant jamais eu de CROSS. Cette égalité prouve aussi que les phases n'ont pas été remises à zéro. |
| G-14 | transitions | Ajouts et retraits en cours de note, y compris le cas de l'union cyclique : 0 NaN/Inf ; **0 échantillon** où des arêtes retirées et des arêtes ajoutées sont actives en même temps ; métrique de clic (Δ⁴) **rapportée** sans seuil. |
| G-15 | bornes | ≥ 200 configurations aléatoires (paramètres et topologies valides), 2 s chacune : 0 NaN/Inf ; garde \|y\| > 32 jamais déclenchée ; borne PM ±32 rad respectée. |
| G-16 | reset / récupération | Panic : silence (crête < −120 dBFS) en ≤ 5 ms + latence, puis la note suivante est normale. NaN injecté par un crochet de test : sortie finie, compteur incrémenté, récupération. |
| G-17 | silence et notes | Avant toute note : sortie exactement nulle. 1 s après la fin du relâchement : crête < −120 dBFS. |
| G-18 | temps réel | **0 allocation** pendant `Engine::process` (compteur `operator new`) et pendant `AudioProcessor::processBlock` (test du wrapper), aucun verrou dans notre code. RTSan : 0 violation s'il est exécuté. |
| G-19 | instances | Deux instances entrelacées : sorties identiques bit à bit aux rendus séparés. |
| G-20 | fréquences × blocs (hôte VST3) | 44,1 / 48 / 88,2 / 96 kHz × blocs de 32 à 512 : chargement, latence rapportée = attendue, rendu d'une note fini, non silencieux, crête < 0 dBFS, invariance aux blocs (différence maximale = 0). |
| G-21 | état (hôte VST3) | `getState` → nouvelle instance → `setState` : rendu identique bit à bit ; paramètres et topologie (identifiants, classes) restaurés. |
| G-22 | identité | StableID uniques, et identifiants de paramètres VST3 identiques d'une compilation à l'autre (liste de référence comparée). |
| G-23 | pluginval | Niveau de rigueur **10** : PASS exigé. Les tests d'interface sont indiqués s'ils ne peuvent pas s'exécuter sur le runner. |
| G-24 | bundle | `lipo` : arm64 uniquement ; `codesign --verify` (signature ad hoc) ; version et clés de build cohérentes dans `Info.plist`. |
| PKG-01 à 08 | livraison | Voir § 8. |

**Non applicables (justifiés) :**

- undo/redo de topologie, non exposé ;
- verrouillage DC en LinFM/TZFM, puisque seule la PM est implémentée ;
- changement de Quality, non implémenté ;
- ExpFM : dette conservée.

## 8. Livraison et recette des paquets

**Construction :**

- `Installer.pkg`, construit avec `pkgbuild` et `productbuild` :
  - charge utile : le bundle sous `/Library/Audio/Plug-Ins/VST3/`, avec `BundleIsRelocatable = false` ;
  - Distribution limitée à arm64, avec précontrôle de macOS ≥ 14.0 déclaré ;
  - scripts : refus d'un bundle tiers ou non identifié et des liens symboliques ; demande de fermer les DAW qui ont
    le plugin chargé (`lsof`) ; sauvegarde puis restauration du bundle précédent en cas d'échec ; manifeste versionné.
- `Uninstaller.pkg`, sans charge utile :
  - vérifie l'identité et la version avant toute suppression ;
  - refuse de supprimer une version plus récente, une cible inattendue ou un lien symbolique ;
  - ne fait aucune suppression large ;
  - préserve les données personnelles ;
  - oublie uniquement le reçu exact.

**Recette en CI, sur VM :**

- installation, réinstallation, mise à jour simulée depuis un faux bundle antérieur ;
- désinstallation, puis réinstallation ;
- refus attendus : ancien désinstallateur, bundle tiers, lien symbolique, absence du plugin ;
- échec simulé suivi d'une restauration ;
- découverte et instanciation depuis `/Library/Audio/Plug-Ins/VST3/` par l'hôte de test JUCE.

**Statuts :** PASS (VM CI) quand le test est exécuté en CI. **Toute recette physique est PENDING** : M4 Max (DAW,
son, MIDI, PDC, rechargement de projet, désinstallation et réinstallation), puis M3 physique avant toute Beta.

**Signature :**

- bundle signé ad hoc, paquets non signés, aucune notarisation, faute de certificat Developer ID ;
- **livraison interne uniquement, distribution publique bloquée** ;
- aucune désactivation globale de Gatekeeper ne sera demandée.

**Publication :** archive ZIP, fichier `SHA256SUMS`, commit de livraison sur la branche et lien direct vers ce commit.
Aucune release publique automatique.

## 9. Errata S0.1 (exigés par l'étape 1)

Publiés sur `spike/s0.1-causality-validation` au commit `fb669170dba183fb5b0c814da5f551fe09d7822e`, dans
`docs/V1-S0.1-ERRATA.md` :

- **E1 :** 4 différences audibles à 44,1 kHz (C1/C5) lors de la première campagne ;
- **E2 :** dispersion recalculée à partir de `bench_reps.csv` : 66,2 % en médiane et 155,7 % au P95 ;
- **E3 :** règle de décision v2, prospective.

Aucune donnée modifiée, verdict historique inchangé.
