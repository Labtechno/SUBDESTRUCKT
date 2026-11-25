# SUBDESTRUCKT — Architecture logicielle VST3 V1
Date : 2026-11-24. Statut : **ARCHITECTURE CIBLE / SANS CODE**.
Références normatives :
- [V1-DESIGN-BASELINE.md](V1-DESIGN-BASELINE.md)
- [V1-CORE-CONTRACTS-P0.md](V1-CORE-CONTRACTS-P0.md)
- [V1-PLATFORM-REQUIREMENTS.md](V1-PLATFORM-REQUIREMENTS.md)
- [V1-GENERAL-AUDIT.md](V1-GENERAL-AUDIT.md)

Périmètre : VSTi VST3 macOS Apple Silicon natif, M3 minimum (cible non certifiée). Pas de standalone, pas de Windows, pas de hardware dans la phase actuelle. Aucun code n'est autorisé par ce document seul.

## 1. Objectif architectural
L'architecture doit permettre :
1. un cœur DSP indépendant du wrapper VST3/JUCE;
2. un traitement audio temps réel sans allocation, verrou bloquant, I/O fichier ni travail lourd imprévisible;
3. un registre canonique unique des paramètres;
4. un moteur d'événements sample-accurate;
5. une séparation stricte Base Value / Effective Value;
6. des graphes de modulation déterministes;
7. des changements structurels transactionnels;
8. des presets/assets versionnés et préparés hors RT;
9. une UI qui observe et édite l'état sans devenir source de vérité DSP;
10. des tests unitaires et de non-régression possibles sans DAW.

## 2. Architecture en couches
### L0 — Platform / Host Adapter
Responsable de :
- VST3 processor/controller;
- buffers audio/MIDI fournis par le host;
- automation host;
- déclaration/actualisation de latence;
- sauvegarde/restauration du state VST3;
- informations transport/tempo;
- création et cycle de vie du plugin;
- pont vers JUCE pour UI, fenêtres, ressources et utilitaires non DSP.

Ne contient pas :
- règles musicales;
- logique de modulation;
- algorithmes de synthèse;
- migrations de presets;
- vérité canonique des paramètres.

### L1 — Application / Orchestration
Responsable de :
- Parameter Registry;
- Engine Configuration;
- transactions;
- chargement/sauvegarde;
- gestion de presets/assets;
- compilation des graphes;
- préparation Quality;
- échanges UI ↔ moteur;
- gestion des workers non RT.

Cette couche décide **quand** et **comment** une configuration préparée remplace la configuration active.

### L2 — Real-Time Engine
Responsable de :
- Event Timeline;
- Voice State;
- modulation runtime;
- WT/FM/CROSS;
- filtres/destruction;
- mixage;
- Amp/Tail/Drone/Panic;
- protections numériques;
- télémétrie RT minimale.

Cette couche ne connaît ni fichiers, ni widgets, ni chemins disque, ni format JSON/plist, ni API graphique.

### L3 — DSP Primitives
Bibliothèque interne de briques déterministes :
- oscillateurs;
- interpolation;
- resampling;
- enveloppes/LFO;
- filtres;
- wavefolders;
- delay lines;
- FFT wrappers si nécessaires;
- fonctions de mapping unités/domaines;
- RNG;
- smoothing;
- mesures/guards numériques.

Dépendances unidirectionnelles : L0 → L1 → L2 → L3. L2 ne dépend jamais de L0 ou UI.

## 3. Composants principaux

### 3.1 ParameterRegistry
Source de vérité statique des paramètres.
Chaque entrée possède au minimum :
- StableID immuable;
- type;
- unité;
- domaine de sommation;
- base range;
- artistic range;
- DSP hard range;
- valeur par défaut;
- capacités de rate;
- smoothing;
- automation;
- randomization;
- preset scope;
- version;
- disponibilité/dépendances;
- catégorie : continuous / event / structural / read-only telemetry.

Le registre ne stocke pas la valeur courante. Il décrit le contrat.

### 3.2 ParameterState
Stocke les Base Values et états structurels actifs.
Deux représentations :
- **Editable State** hors RT : riche, transactionnel, sérialisable.
- **Compiled RT State** : forme compacte, prête à être consommée par l'engine.

Les Effective Values ne sont jamais la source de vérité persistante.

### 3.3 EventTimeline
Transforme les événements host en timeline déterministe.
Types :
- Safety/Transport;
- expression MIDI;
- Note Off;
- Note On;
- structural events préparés;
- sync/events internes.

Règles :
- sample offsets conservés;
- tri stable et déterministe selon Core Contract;
- pas de quantification silencieuse au bloc;
- événements structurels lourds ne déclenchent jamais leur préparation sur le thread audio.

### 3.4 VoiceEngine
V1 monophonique mais structurée autour d'une abstraction VoiceContext extensible.
Contient :
- note/gate/priority;
- pitch/microtuning;
- enveloppes;
- états LFO/Random/Chaos liés à la voix si applicables;
- Wavetable Engine;
- FM/PM Engine;
- Cross Engine;
- Amp contract;
- états Tail/Drone;
- pipeline local/global.

Le nombre de voix V1 reste 1, mais les états ne doivent pas être codés comme variables globales rendant une future polyphonie impossible.

### 3.5 WavetableEngine
Sous-systèmes :
- BankView immutable;
- phase/pitch;
- X/Y morph;
- Z Slot A;
- Z Slot B;
- Character/Clean-Raw;
- Sync;
- modulation ports.

Le catalogue Z incomplet interdit toute implémentation définitive des cinq transformations manquantes.

### 3.6 FmEngine
Contient quatre OperatorRuntime identiques.
Sous-systèmes :
- tuning mode;
- waveform/source;
- matrix 4×4;
- modulation mode par destination;
- self/cross feedback;
- audio level/pan séparés du modulation level;
- Character;
- internal bus exports.

La causalité du FmEngine est régie par [V1-CAUSAL-CONTRACT.md](V1-CAUSAL-CONTRACT.md).

### 3.7 CrossEngine
Responsable de :
- WT→FM;
- OP1–4/FM Bus→WT;
- Cross Master et directions;
- Sync;
- Cross Audio Mix/Crossfade/Ring/AM;
- Cross Reset.

**Contrat causal validé : stratégie B.** Le graphe spécialisé FM/CROSS classe les nouvelles arêtes fermant un cycle instantané Delayed; les autres nouvelles arêtes restent Immediate. Les classes survivantes ne sont pas recalculées. La classe causale d'une route est sticky et persistante : une route survivante conserve sa classe lors d'une transaction ultérieure. Les objets de configuration doivent donc sérialiser au minimum StableRouteID + CausalClass, avec validation que le sous-graphe Immediate reste acyclique.

### 3.8 ProcessingChain
Trois emplacements logiques :
- WT local;
- FM local;
- Global.

Chaque emplacement :
- 1 filtre/résonateur;
- 0–2 Destruction slots;
- ordre Pre/Post selon état structurel;
- alignement latence;
- bypass/DryWet.

Les modules sont instanciés à partir de configurations préparées hors RT. Les changements d'algorithme sont structurels, jamais de simples floats audio-rate.

### 3.9 ModulationGraph
Deux niveaux conceptuellement séparés :
1. **Specialized recursive networks** : FM + Cross.
2. **Global Modulation Graph** : 64 connexions max, acyclique.

Le Global Graph est compilé hors RT en ordre d'évaluation déterministe.
Chaque route compilée contient :
- source handle;
- destination handle;
- conversion domaine/unité;
- profondeur;
- polarity/invert;
- curve;
- Via;
- smoothing;
- rate class;
- availability guard.

Aucun parcours dynamique complexe de graphe ne doit être reconstruit par sample.

### 3.10 RngService
Gestion centralisée des streams :
- Audio RNG;
- Randomize;
- Mutate;
- Evolve;
- autres streams explicitement nommés.

Chaque stream possède StableStreamID, seed, version et counter/state.
Le choix PCG32/xoshiro reste à benchmarker; l'API doit masquer l'algorithme.

### 3.11 StateManager
Responsable de :
- configuration recall;
- Creative Recall;
- migrations;
- transactions;
- Undo/Redo;
- Randomize/Mutate/Evolve state;
- assets references;
- validation avant commit.

Aucun état runtime complet R3.

### 3.12 AssetManager
Gère :
- banques WT;
- ressources utilisateur;
- captures Freeze/Glitch;
- cache;
- hash/revision;
- Copy-on-Write;
- package portable;
- state DAW autonome.

Toute lecture disque, décompression, hash lourd ou génération se fait hors RT.

### 3.13 QualityManager
Quality est séparé de Clean/Raw.
Responsable de :
- politiques de resampling/oversampling;
- préparation d'instances DSP compatibles;
- calcul de latence prévisible;
- transition sécurisée entre configurations;
- reporting de latence vers VST3.

Une transition Quality suit : prepare → validate → publish → crossfade/switch sécurisé → retire ancien état hors RT.

### 3.14 SafetyManager
Garantit :
- NaN/Inf guards;
- denormal policy;
- DC guard lorsque nécessaire;
- DSP hard bounds;
- Panic;
- headroom nominal;
- séparation protection numérique / limiteur musical.

Le limiter musical n'est jamais le mécanisme de sûreté obligatoire.

### 3.15 TelemetryBridge
RT → UI uniquement pour données observables :
- meters;
- scope samples décimés;
- spectrum input frames;
- XY position;
- graph activity;
- CPU counters locaux si pertinents;
- safety flags.

Contraintes :
- buffer borné/prealloué;
- lock-free ou wait-free adapté;
- drop autorisé pour télémétrie non critique;
- jamais de backpressure sur audio;
- UI n'exige jamais une valeur “instantanée exacte” inaccessible à sa cadence.

## 4. Ownership et threading

### Audio Thread
Peut :
- lire snapshot RT courant;
- consommer EventTimeline;
- calculer DSP;
- écrire télémétrie bornée;
- publier quelques compteurs atomiques.

Ne peut pas :
- allouer/libérer dynamiquement;
- prendre mutex bloquant;
- ouvrir/lire/écrire fichiers;
- compresser/décompresser;
- recalculer banques;
- compiler graphes;
- créer UI;
- logger de façon synchrone;
- faire migration de state.

### UI / Message Thread
Peut :
- éditer Editable State;
- déclencher transactions;
- demander une préparation;
- lire télémétrie;
- afficher erreurs/états.

Ne modifie jamais directement un objet DSP actif.

### Worker Pool non RT
Responsable de :
- import/génération WT;
- asset hashing;
- compilation ModGraph;
- migration state;
- préparation preset;
- préparation Quality;
- FFT précalculées/LUT;
- compression;
- analyses coûteuses;
- chargement de captures.

### Publication d'état
Pattern cible :
1. UI/host demande une modification.
2. L1 crée une transaction.
3. Les workers préparent un nouvel objet immutable/compiled.
4. Validation complète.
5. Publication atomique d'un handle/snapshot vers RT.
6. RT bascule à un point sûr.
7. Ancien objet placé dans une file de reclamation hors RT.

Aucune mutation partielle d'un graphe actif.

## 5. Modèle de snapshots
Trois familles :
- **Static Definition** : registre, tables immuables, metadata.
- **Configuration Snapshot** : structure du patch, routes, algorithmes, assets handles.
- **Runtime State** : phases, delays, envelopes, RNG runtime, feedback histories.

Preset et DAW state sérialisent principalement Configuration + Creative State autorisé. Runtime State intégral n'est pas persisté.

## 6. Transactions
Une transaction contient :
- TransactionID;
- cause;
- before/after configuration;
- RNG before/after si nécessaire;
- references assets;
- validation result;
- schema/DSP model versions.

Transactions obligatoires :
- preset load;
- structural parameter change;
- ModGraph merge/edit;
- Randomize;
- Mutate;
- Evolve Commit;
- bank revision switch;
- capture replacement.

Un geste de knob continu correspond à une transaction Undo logique, mais ses valeurs audio peuvent évidemment évoluer continûment.

## 7. Base Value → Effective Value
Pipeline conceptuel :
1. Base Value host/preset;
2. automation host appliquée;
3. modulation contributions dans leur domaine correct;
4. Via;
5. sommation;
6. boundary policy artistique;
7. hard DSP bounds;
8. smoothing au bon stade;
9. Effective Value consommée par DSP.

Les domaines (Hz, octaves, radians, dB, linéaire, normalisé, cyclique) ne doivent pas être convertis en simple 0..1 trop tôt.

## 8. Rates
Classes :
- Event Rate;
- Control Rate;
- Audio Rate.

Le Control Rate 1 kHz reste candidat jusqu'au benchmark.
Une destination annonce ses rates autorisés.
Aucun downgrade silencieux audio→control.
Si indisponible : route conservée inactive avec reason code.

## 9. Latence
Une LatencyGraph logique agrège :
- resamplers;
- FFT processors;
- delay compensation;
- chains locales;
- global chain.

Objectifs :
- dry/wet sample-aligned;
- WT/FM aligned avant Cross Audio/global;
- valeur déclarée VST3 cohérente;
- changement de latence seulement lors de transitions structurelles contrôlées.

La stratégie exacte de notification host sera définie au moment du wrapper VST3 et testée dans les DAW cibles.

## 10. VST3 wrapper / JUCE
Le wrapper expose :
- bus stéréo output;
- MIDI input;
- paramètres host automatable;
- state blob;
- editor;
- latency;
- tempo/transport si fourni.

Le wrapper traduit StableID ↔ host parameter identity.
Interdit :
- générer StableID à partir d'indices JUCE;
- laisser l'ordre des paramètres devenir identité persistante;
- stocker la logique de migration dans les widgets;
- utiliser les ValueTree/UI objects comme modèle DSP runtime.

JUCE est autorisé comme framework d'intégration, pas comme modèle métier.

## 11. Organisation logique des modules
Arborescence conceptuelle, non prescriptive :
- core/
  - parameters/
  - events/
  - state/
  - assets/
  - modulation/
  - rng/
  - safety/
- dsp/
  - primitives/
  - wavetable/
  - fm/
  - cross/
  - filters/
  - destruction/
  - voice/
- runtime/
  - engine/
  - snapshots/
  - quality/
  - telemetry/
- plugin/
  - vst3/
  - juce_adapter/
- ui/
  - performance/
  - sound_design/
  - engineering/
  - inspector/
- tests/
  - unit/
  - dsp/
  - state/
  - regression/
  - host_integration/
  - performance/
- tools/
  - asset_build/
  - golden_render/
  - benchmark/

Cette structure peut évoluer; ce qui est normatif est la séparation des responsabilités, pas les noms de dossiers.

## 12. Tests requis par architecture

### Unit
- parameter domains/conversions;
- StableID uniqueness;
- event ordering;
- graph cycle detection;
- curves/Via;
- RNG golden vectors;
- migration;
- transaction atomicity.

### DSP
- oscillator invariance;
- no NaN/Inf;
- headroom/protection;
- tails/Panic;
- latency alignment;
- sample-rate tests;
- Quality transitions.

### State
- save/load;
- undo/redo RNG;
- assets missing/corrupt;
- Copy-on-Write;
- captures;
- Evolve Commit.

### Host integration
- automation;
- sample-offset MIDI;
- state restore;
- latency changes;
- resize/UI lifecycle;
- multiple instances.

### Performance
- M3 physique comme référence minimale avant Beta publique; M4 Max comme machine principale de recette;
- UI open/closed;
- 44.1/48/88.2/96 kHz;
- buffer ladder;
- one/multiple instances;
- Eco/Normal/High;
- worst-case stress configurations.

## 13. Décisions explicitement NON prises ici
- validation d'intégration du lecteur fractionnaire Thiran1 et transitions B-T2;
- choix PCG32 vs xoshiro;
- modèle chaos;
- cinq transformations Z manquantes;
- facteurs exacts d'oversampling;
- fréquence Control Rate définitive;
- budgets CPU/RAM/latence;
- version minimale macOS;
- DAW officiellement supportés;
- format physique final des presets/captures;
- détail graphique final UI.

## 14. Gates
### ARCH-G0 — Documentation
Core Contracts P0 validés. **PASS documentaire.**

### ARCH-G1 — Architecture
Ce document doit être accepté comme architecture cible avant code produit.

### ARCH-G2 — S0 Causality — CHOIX ARCHITECTURAL CLOS
B-T2 et sticky retenus; validation runtime et performances produit encore ouvertes. Voir [V1-CAUSAL-CONTRACT.md](V1-CAUSAL-CONTRACT.md).

### ARCH-G3 — Vertical Slice — PASS dans la portée testée
MIDI mono → WT simple → 2 OP FM → CROSS → Amp → Output → preset minimal → UI diagnostic.
La livraison ARCH-G3 inclut dès la première version installable le ZIP et les deux PKG macOS arm64 ainsi que la recette PKG-01 à PKG-08, selon [V1-PACKAGING-RELEASE-CONTRACT.md](V1-PACKAGING-RELEASE-CONTRACT.md). Packaging/build/release hors DSP et hors processus audio; aucune livraison VST3 nue. ARCH-G3 est validé dans la portée de 0.1.0-b5; voir [V1-ARCH-G3-CLOSURE.md](V1-ARCH-G3-CLOSURE.md).

### ARCH-G4 — Expansion P0
La recette de la vertical slice est acceptée. Préparation de l'expansion P0 par lots et spikes ciblés; mission distincte obligatoire.

## 15. Verdict
Cette architecture réduit le risque principal du projet : que l'UI, le host, les presets et les algorithmes DSP deviennent mutuellement dépendants.

**GO ARCH-G3 dans la portée testée. Préparation ARCH-G4 par lots; pas de release publique.**


### Révision plateforme — décision utilisateur du 2026-11-25
Apple M3 de base devient la configuration matérielle minimale cible, en remplacement du M1. M1 et M2 (y compris leurs variantes) sortent du support officiel, sans obligation de blocage logiciel. M4 et générations ultérieures constituent la classe recommandée, sous réserve de recette; le Mac Studio M4 Max de l'utilisateur est la machine principale de développement et de recette physique. La validation sur M3 physique en hôte VST3 est obligatoire avant Beta publique. Aucun résultat de performance M3 n'est encore disponible. Le benchmark M1 virtualisé S0.1 (run 45591261853, rapport sur la branche expérimentale au commit a85f1947e48b1fd8d5d11a9f27df09b78437e5af) et son verdict NO-GO selon les seuils historiques restent inchangés. Ils ne certifient ni M1 ni M3. Cette révision ne gèle pas T2, ne change pas les seuils historiques, ne clôt pas les autres conditions de causalité et n'autorise pas à elle seule ARCH-G3.


## Décision de clôture S0.1 — 2026-11-25
B-T2 est retenu comme contrat produit, avec minimum permanent K=2 à 44,1 kHz. ARCH-G3 est autorisé sous conditions de recette d'intégration, sans certification de performance. Voir [V1-CAUSAL-CONTRACT.md](V1-CAUSAL-CONTRACT.md) et [V1-ARCH-G3-MISSION.md](V1-ARCH-G3-MISSION.md). Le NO-GO historique du benchmark M1 reste inchangé; M3 physique avant Beta publique. Les formulations antérieures conditionnant le choix à S0.1 ou au M1 sont historiques et remplacées pour les travaux futurs par cette décision. Aucun GO ARCH-G4 ni release publique.


## Clôture ARCH-G3 — recette physique du 25 novembre 2026
GO ARCH-G3 dans la portée de la tranche 0.1.0-b5, 15/15 PASS déclarés et acceptés par la direction technique. R-08 et R-09 complétés par confirmation explicite : 44,1/48/88,2/96 kHz et PDC à 44,1 kHz. Voir [V1-ARCH-G3-CLOSURE.md](V1-ARCH-G3-CLOSURE.md). Les restrictions antérieures « avant recette » sont levées pour le prototype validé; la préparation d'ARCH-G4 par lots est ouverte, pas l'implémentation monolithique ni la diffusion publique. M3 physique, budget CPU produit, licence JUCE, signature/notarisation et dettes d'intégration restent ouverts. Aucun merge du code ni nouvelle release par cette décision.
