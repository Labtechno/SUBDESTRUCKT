# SUBDESTRUCKT — ARCH-G4.3 — DSP FINAL LOCK : spécification et critères (écrits AVANT toute mesure)

Date : 2026-11-27. Base : `a9cde4ba8f2f506cf2472158832a6ea54816febb` (ARCH-G4.2 final, run CI 38 SUCCESS), branche unique
`feature/arch-g4-full-development`. Référentiel documentaire `main` : `9a07d7654cfce951900cda9e8bec7303b8007a15`.
Version interne proposée : **0.3.2** (aucune étiquette, aucune édition de manuel ni chaîne `0.3.2` existante : pas de
collision). Format d'état proposé : **4**.

Règles : aucun seuil ci-dessous ne sera assoupli après mesure ; les critères et résultats historiques (ELP-01, D2-04,
D2-04c, AUT-01..08, AUT-02c, CAP-02w, G-15, DST-09…) sont **conservés tels quels** ; les critères ci-dessous les
complètent. Une évolution d'un comportement historique est consignée comme **révision fonctionnelle validée** (§1).
ARCH-UI-04, le Design System final et le Symbiotic Core 3D ne sont pas commencés.

## 1. Arbitrages intégrés (décisions du propriétaire) et révisions fonctionnelles

| Arbitrage | Décision | Conséquence technique |
|---|---|---|
| SPEC-HR-01 | ELP adoptée | `dsp/Elp.h` inchangée ; G-15/DST-09 : PASS **non conditionnel** à partir de G4.3 (FAIL historiques c741fb8 conservés) |
| ELP-01 | plateau numérique accepté | ELP-01 reste FAIL historique ; nouveau critère prospectif ELP-01R (§4) |
| A-G42-01 | D2 C1 adoptée, compatibilité historique | modes LEGACY / REGENERATIVE par emplacement (§5) |
| A-G42-02 | aucune persistance des autorisations | inchangé ; l'autorisation est une donnée d'exécution (§3) |
| A-G42-03 | Panic : révocation systématique | **révision fonctionnelle** : G4.2 ne révoquait qu'avec un armement configuré |
| A-G42-04 | captures R2 : validation complémentaire avec décroissance | trajectoire spectrale R2-01..04 (§6) |
| B-01 | catalogue Z de huit transformations | Z1 Harmonic Tilt, Z4 Harmonic Mask, Z5 Harmonic Focus, Z6 Harmonic Shift, Z7 Phase Scatter (§7) |
| — | autorisation autonome déclenchée par le transport | TRANSPORT AUTHORIZATION (§3) ; validation finale conditionnée à Reason 14 et Ableton Live |
| D-LIC-1 | JUCE Starter retenue sous réserve | aucune acceptation de conditions par l'agent ; le script exige toujours le choix déclaré |

Révisions fonctionnelles validées (critères historiques conservés, résultats historiques consignés) :
- **RF-01 (A-G42-03)** : Panic révoque l'autorisation même sans armement configuré (AUT-05 historique : inchangé dans son
  scénario, qui comportait un armement).
- **RF-02 (transport)** : en G4.2, seule l'action Autoriser levait la suspension ; désormais une transition STOP→PLAY, un
  début d'enregistrement ou un rendu hors temps réel en lecture l'accordent aussi (§3). Autoriser est conservée.
- **RF-03 (Drone)** : en G4.2, `drone_restore=1` réactivait le Drone dès la restauration, sans suspension ; désormais le
  Drone restauré est une configuration soumise à l'autorisation d'exécution (§3.6).
- **RF-04 (D2)** : les nouvelles instances et nouveaux presets utilisent REGENERATIVE ; tout état ou preset antérieur au
  format 4 est lu en LEGACY (rendu historique).

## 2. Inventaire des identifiants (append-only)

695 StableID existants inchangés (identité, ordre, plage, défaut), identifiants VST3 des 695 inchangés. Ajouts en fin de
registre, version 4 :

| # | StableID | Type | Défaut | Rôle |
|---|---|---|---|---|
| 695–700 | `dst.{wt1,wt2,fm1,fm2,gl1,gl2}.d2.fb_mode` | Choice `Legacy|Regenerative` | Regenerative | mode de Feedback D2 par emplacement |
| 701–702 | `wt.za.shape`, `wt.zb.shape` | Float [−1, 1] | 0 | second axe des nouvelles transformations Z (§7.2) ; ignoré par Fold, Warp, Mutation |

Énumérations étendues en fin (append-only) : `wt.za.type`, `wt.zb.type` : `Off|Harmonic Fold|Phase Warp|Spectral
Mutation|Harmonic Tilt|Harmonic Mask|Harmonic Focus|Harmonic Shift|Phase Scatter` (indices 0–3 inchangés, 4–8 nouveaux).
Ces deux paramètres ne sont pas automatisables (aucune courbe d'automation hôte en valeur normalisée n'existe) ; l'état
les enregistre en valeur exacte.

## 3. TRANSPORT AUTHORIZATION

### 3.1 Modèle

Quatre catégories inchangées depuis G4.2 : configuration persistée ; captures R2 ; état d'exécution ; **autorisation
d'exécution** (jamais persistée). Armement effectif = configuré ET autorisé (9 armements) ; Drone effectif = Drone
configuré ET autorisé.

Machine d'états (thread audio, `engine/AutonomyGate.h`, sans allocation ni verrou) évaluée à chaque frontière de bloc :

| Variable | Domaine |
|---|---|
| transport observé | INCONNU (pas de playhead, position absente, ou contexte VST3 absent : temps non fini) / ARRÊT / LECTURE (± enregistrement) |
| dernier état connu | AUCUN (jamais observé) / ARRÊT / LECTURE ; un bloc INCONNU ne le modifie pas |
| autorisation | accordée / suspendue (génération de chargement) / révoquée (Panic) |
| verrou Panic | posé par Panic, levé par l'observation d'un ARRÊT réel ou par une autorisation explicite |
| génération | incrémentée par chaque restauration suspensive ; une commande Autoriser porte la génération visée |

Ordre dans un bloc : (1) suspension (nouvelle génération) ; (2) mise à jour du transport ; (3) candidats d'autorisation
(commande explicite, début de lecture, début d'enregistrement, rendu hors temps réel) ; (4) **si le bloc contient un Panic
(All Sound Off, bouton Panic), aucun candidat n'est accordé** ; (5) événements MIDI ; Panic révoque et pose le verrou.

Sources d'autorisation :
- **TRANSPORT** : bloc LECTURE alors que le dernier état connu est ARRÊT, verrou levé.
- **RECORD** : bloc LECTURE+ENREGISTREMENT alors que le bloc connu précédent était LECTURE sans enregistrement, verrou
  levé (punch-in). Un drapeau d'enregistrement sans lecture n'autorise pas.
- **RENDER** (borné) : `isNonRealtime()` (VST3 `kOffline`), transport connu en LECTURE, verrou levé, autorisation absente.
  Ni un mode hors temps réel arrêté, ni un mode hors temps réel sans information de transport n'autorisent.
- **EXPLICITE** : Autoriser (barre globale), mise en marche d'un armement ou du Drone par un clic dans l'éditeur ; lève le
  verrou. Une valeur reçue par automation, par l'hôte ou par une restauration d'état n'est jamais explicite.

Ce qui ne change pas l'autorisation : boucle, seek, tempo, PLAY maintenu, fermeture/réouverture de l'éditeur,
`prepareToPlay` (fréquence, taille de bloc), désactivation/réactivation, changement de Quality, absence temporaire du
playhead. Une instance neuve démarre autorisée (configuration par défaut sans état autonome : aucun son autonome).

Restauration suspensive (projet DAW, duplication) : état portant un armement, une capture ou un Drone restauré. Preset :
armements désarmés (G4.2), captures retenues, suspension si captures. Les captures sont **préparées hors RT** au
chargement et publiées immédiatement au moteur, qui les **retient** ; elles sont appliquées une fois, à la première
frontière de bloc autorisée où l'emplacement exécute D2/S2 (fenêtre de 2 s inchangée), puis à chaque nouvelle
autorisation (après Panic ou re-préparation), jamais à chaque bloc.

### 3.2 Critères automatisés (moteur, transport simulé ; puis wrapper, playhead simulé)

| ID | Critère |
|---|---|
| TA-01 | Instance neuve à l'arrêt, configuration par défaut : énergie de sortie sans note < 1e−9 pendant 3 s ; travail à l'arrêt : un armement (D2 gelé) configuré dans la session est actif sans transport |
| TA-02 | Restauration à l'arrêt (9 armements + Drone + captures D2/S2) : 3 s ARRÊT ⇒ rendu **bit à bit** identique au même état désarmé sans Drone ; captures retenues (0 application) ; état « suspendu » publié |
| TA-03 | STOP→PLAY : autorisation au **premier** bloc LECTURE (source TRANSPORT) ; rendu bit à bit identique à un moteur de référence autorisé explicitement à la même frontière de bloc ; captures appliquées exactement une fois |
| TA-04 | Enregistrement : (a) ARRÊT→LECTURE+ENR autorise ; (b) restauration pendant LECTURE puis punch-in (ENR 0→1) autorise (source RECORD) ; (c) ENR sans LECTURE n'autorise pas |
| TA-05 | STOP après PLAY : autorisation conservée ; 3 s d'ARRÊT : rendu bit à bit identique à la référence autorisée |
| TA-06 | Boucle (10 retours de position), seek, 10 changements de tempo pendant LECTURE : autorisations = 1, applications de captures = 1 ; rendu bit à bit identique à la référence sans ces événements (configuration sans source synchronisée au tempo) |
| TA-07 | Autoriser explicite à l'arrêt ou sans transport : autorisation au bloc suivant, sans confirmation ; commande antérieure à une restauration plus récente : ignorée (génération périmée) |
| TA-08 | Preset avec armements et captures chargé pendant ARRÊT et pendant LECTURE : aucun comportement autonome (rendu ≡ désarmé bit à bit), captures retenues ; puis STOP→PLAY : autorisation sans armement (aucun comportement autonome) |
| TA-09 | Panic sans aucun armement configuré : autorisation révoquée (RF-01), verrou posé ; avec armements : énergie autonome < 1e−9 après la rampe ; configuration inchangée |
| TA-10 | Priorité de Panic : dans un même bloc que (a) un début de lecture, (b) une commande Autoriser, (c) un premier bloc de rendu : non autorisé à la fin du bloc et rendu du bloc bit à bit identique au cas « Panic sans candidat » ; Panic au décalage 0 et au décalage 300 |
| TA-11 | LECTURE maintenue 5 s après Panic, y compris un punch-in : toujours révoquée |
| TA-12 | ARRÊT puis LECTURE après Panic : nouvelle autorisation ; gel D2 et S2 restaurés depuis les captures (contenu D2 identique bit à bit à la capture) |
| TA-13 | Transport INCONNU : LECTURE, INCONNU×50, LECTURE ⇒ aucune transition ; ARRÊT, INCONNU×50, LECTURE ⇒ transition ; INCONNU seul ⇒ ni autorisation ni révocation ; dans le wrapper : playhead nul, position absente et contexte VST3 absent (temps non fini) classés INCONNU |
| TA-14 | Premier bloc déjà en LECTURE (instance créée ou projet chargé pendant la lecture, duplication pendant la lecture, temps réel) : pas d'autorisation ; puis ARRÊT→LECTURE : autorisation |
| TA-15 | Rendu hors temps réel : (a) restauration puis premier bloc hors temps réel en LECTURE (sans ARRÊT observé, sans éditeur, sans service du thread message) : autorisé (source RENDER), sortie bit à bit identique à une référence autorisée au même bloc, captures et Drone présents ; (b) hors temps réel + INCONNU : non autorisé ; (c) hors temps réel + ARRÊT : non autorisé ; (d) verrou Panic actif : non autorisé ; (e) retour en temps réel : autorisation conservée |
| TA-16 | Re-préparation (44,1→96 kHz, blocs 512→64, Quality Normal→High→Eco, release/prepare) : autorisation, verrou, dernier état connu et captures retenues conservés ; suspendu reste suspendu ; autorisé : captures réappliquées après re-préparation (gel D2 rejoué = capture) |
| TA-17 | Concurrence déterministe : chargement / Autoriser / Panic dans les 6 ordres possibles avant une même frontière de bloc : résultat = table documentée (Panic domine ; une commande antérieure à la génération courante est périmée) ; chargement A puis B avant tout bloc : seules les captures de B appliquées |
| TA-18 | Aucune persistance : aucun champ d'autorisation dans l'état ; état d'une instance autorisée et d'une instance suspendue de même configuration et mêmes captures : identiques octet par octet ; restauration dans une instance neuve : suspendue |
| TA-19 | Indépendance des instances : transport, Panic et Autoriser envoyés à A sans effet sur B (rendu de B bit à bit inchangé) |
| TA-20 | Temps réel : 0 allocation dans `Engine::process` sur toutes les séquences TA ; `processBlock` avec playhead simulé : 0 allocation (compteur du wrapper) ; RTSan CI : 0 violation ; aucun verrou ni appel de `authorizeAutonomy()` dans le chemin audio (lecture de code) |
| TA-21 | Drone : (a) `drone=1 drone_restore=1` restauré à l'arrêt : configuré, suspendu (Note Off relâche normalement : énergie < 1e−9 après la release) ; (b) STOP→PLAY : Drone maintient le Gate ; (c) `drone_restore=0` : non restauré ; (d) Drone mis en marche à l'arrêt depuis l'éditeur : actif (explicite) ; (e) Panic : Drone désarmé (P0-C06) et révocation |
| TA-22 | MIDI : All Sound Off (CC120) = Panic (révocation, verrou) ; All Notes Off (CC123) : aucune révocation, Drone maintenu, notes relâchées |
| TA-23 | Sans éditeur : toutes les séquences TA exécutées sans éditeur ni timer ; ouverture/fermeture de l'éditeur ×10 : autorisation inchangée |
| TA-24 | Compatibilité : états v1, v2, v3 lus ; état v3 armé restauré : suspendu puis autorisé par STOP→PLAY ; résultats des tests AUT-01..08 et CAP-02w inchangés |

### 3.3 Diagnostic de recette

Journal d'événements sans verrou (moteur → thread message) : transitions de transport observées (INCONNU/ARRÊT/LECTURE,
ENR, hors temps réel), autorisations (source), suspensions (génération), révocations, applications de captures, horodatées
en échantillons. Écrit dans `…/Labtechno/SUBDESTRUCKT/ta_log.txt` **uniquement** si le fichier
`…/Labtechno/SUBDESTRUCKT/ta_diagnostics.on` existe (aucune écriture sinon) ; affiché dans l'onglet Diagnostics.

| ID | Critère |
|---|---|
| TA-25 | Journal : chaque événement des séquences TA-03/09/12/15 présent dans l'ordre ; débordement compté, jamais bloquant ; aucune écriture disque sans le fichier d'activation |

### 3.4 Exports audio — limites connues (à vérifier en recette physique)

Audit JUCE 9.0.6 (`juce_audio_plugin_client_VST3.cpp`) : `setNonRealtime(processMode == kOffline)` est appelé dans
`setupProcessing` et à **chaque** `process` ; le playhead expose `kPlaying`, `kRecording`, `kCycleActive` ; sans
`processContext` le contexte est mis à zéro (`isPlaying` faux, temps = 0/0 non fini), d'où la classe INCONNU. Ambiguïtés non
résolubles par l'API : un hôte qui rend en mode temps réel dans une instance neuve déjà en lecture (indiscernable d'une
insertion pendant la lecture) ; un hôte qui rend hors temps réel sans contexte de transport ; un hôte qui n'appelle pas le
traitement à l'arrêt (le premier bloc observé est alors une LECTURE). Ces cas restent sans autorisation automatique
(sécurité) et sont listés pour la recette ; alternative générique proposée : Autoriser avant l'export.

## 4. ELP — clôture SPEC-HR-01

Fonction et chaîne inchangées (`dsp/Elp.h`). ELP n'est **pas** une protection auditive.

| ID | Critère prospectif |
|---|---|
| ELP-01R | (a) identité exacte (==) sur 10⁶ points de [−16, 16] ; (b) symétrie exacte f(−y) = −f(y) sur 10⁵ points de ]16, 10³⁰⁰] ; (c) monotonie non stricte : f(b) ≥ f(a) pour b > a sur une grille logarithmique de 10⁵ points de ]16, DBL_MAX] et 10⁶ paires aléatoires ; (d) continuité : \|f(16+ε) − (16+ε)\| ≤ ε²/16, ε ∈ {1e−6…1e−1}, et 1-lipschitzienne sur 10⁵ paires ; (e) borne : \|f(y)\| < 32 pour toute entrée finie et `(float)f(y) < 32.0f` ; (f) entrée non finie : traitée par la garde numérique (compteur distinct), jamais par ELP |

Non-régression exécutée : ELP-02..07, G-15, G-15b/c, DST-09/09b, HR-01/02 (résultats identiques à G4.2 attendus).

## 5. D2 Glitch Buffer — LEGACY / REGENERATIVE

Sémantique REGENERATIVE = C1 de G4.2 (dans la boucle, à l'index lu, hors gel) **en plus** de l'écriture historique à la
tête d'enregistrement, inchangée. LEGACY = code historique strict. Mode lu par emplacement à chaque bloc ; un changement de
mode ne vide ni ne réinitialise le tampon. Migration : format < 4 ⇒ les 6 modes = LEGACY ; `defaultState()` (instance
neuve) ⇒ REGENERATIVE ; presets d'usine existants : LEGACY explicite (rendu inchangé).

| ID | Critère |
|---|---|
| D2R-01 | Registre : 6 StableID ajoutés en fin (695–700), 695 premiers identiques (REG-ID), identifiants VST3 des 695 inchangés |
| D2R-02 | Migration : états v1/v2/v3 et presets fichiers antérieurs ⇒ LEGACY (6/6) ; instance neuve ⇒ REGENERATIVE ; aller-retour format 4 exact ; preset sauvegardé : valeur explicite |
| D2R-03 | LEGACY ≡ moteur historique (option interne 0 de G4.2) bit à bit, 30 configurations aléatoires dont Feedback > 0 |
| D2R-04 | Feedback = 0 : REGENERATIVE ≡ LEGACY bit à bit, 30 configurations (tailles, vitesses ±, Reverse, Repeat, Crossfade, gel, positions) |
| D2R-05 | Position 0 : REGENERATIVE, écart fb vs fb = 0 ≥ −20 dB et croissant (0,25/0,5/1) ; Position 0,5 et 1 : écart ≥ −20 dB à fb = 1 |
| D2R-06 | Gel : tampon gelé inchangé bit à bit dans les deux modes (fb > 0) |
| D2R-07 | Transitions de mode à chaque bloc et toutes les 100 ms pendant une boucle (fb = 0,7) : sortie finie, \|y\| ≤ 4, tampon fini et ≤ 4, deux rendus identiques bit à bit |
| D2R-08 | Indépendance par emplacement : deux D2 (WT1 LEGACY, FM1 REGENERATIVE) : chaque chaîne bit à bit identique à sa référence à un seul mode |
| D2R-09 | Automation : Feedback en rampes et sauts, Position, vitesse et Reverse sautés par bloc : fini, déterministe (2 rendus identiques), 0 allocation |
| D2R-10 | Protocole de stabilité (deux modes) : fb ∈ {0,5 ; 0,9 ; 1}, vitesses {−4, −1, 0,25, 1, 4}, Repeat 1, tailles {10, 100, 500 ms}, bruit stéréo décorrélé pleine échelle 60 s puis silence 120 s, sans état autonome armé. (a) **divergence** : 0 non-fini, \|y\| ≤ 4 sur tout le rendu, deux canaux ; (b) **fluctuation bornée** : RMS 10 s des fenêtres 20–60 s ∈ [0,25 ; 4] × leur médiane ; (c) **rejeu** : après coupure, toute fenêtre 10 s ≤ 2 × (+6 dB) la RMS maximale des fenêtres 20–60 s ; (d) **décroissance à long terme** : RMS(170–180 s) ≤ RMS(50–60 s) − 40 dB ; (e) descriptif : rapport fenêtre par fenêtre (pas de décroissance stricte exigée) |
| D2R-11 | Instance neuve : D2 REGENERATIVE par défaut ; exemples audio du manuel (presets d'usine) identiques octet par octet à 0.3.1 |

D2-04 et D2-04c restent FAIL historiques (moteur LEGACY non modifié).

## 6. Captures R2 — clôture R50 (portée automatisée)

| ID | Critère |
|---|---|
| R2-01 | Trajectoire S2 avec Decay ∈ {0,3 ; 0,5 ; 0,8 ; 0,95}, Fs ∈ {44,1 ; 48 ; 96 kHz} : instance de référence activée depuis la capture C ; instance restaurée (suspendue) puis réautorisée depuis C ; à temps écoulé égal depuis l'activation, trames de 100 ms sur 5 s : énergie par bande de tiers d'octave (bandes à > −60 dB du maximum) écart RMS ≤ 0,5 dB, énergie de trame ±0,5 dB |
| R2-02 | Contenu initial restauré : spectre tenu relu après application = C (écart relatif ≤ 1e−6 à Fs égale) ; à Fs différente (capture 48 kHz restituée à 44,1 et 96 kHz) enveloppe par bande ±0,5 dB |
| R2-03 | Décroissance contractuelle : pente d'énergie mesurée = −6,02 dB par demi-vie T = 0,1 + 30·Decay² s, à ±10 % (Decay < 1) ; Decay = 1 : énergie constante ±0,1 dB |
| R2-04 | Restitution via TRANSPORT AUTHORIZATION (STOP→PLAY) et via RENDER : mêmes critères que R2-01 ; D2 : segment restauré identique bit à bit à la capture |
| R2-05 | Sécurité : Panic pendant un gel restauré : silence < 1e−9 après la rampe ; réautorisation : contenu = capture (et non l'état décru avant Panic) |

R50 passe à PASS si AUT (G4.2) + TA + R2-01..05 passent ; la recette physique reste listée à part. L'exactitude de tous
les états d'exécution après fermeture du DAW (R3) n'est pas exigée.

## 7. Wavetable Z — huit transformations

### 7.1 Architecture commune

Domaines (ordre de composition fixe, inchangé pour l'existant) : **spectral (table)** → **phase** (Warp, Mutation) →
lecture → **forme d'onde** (Fold). L'ordre A→B / B→A s'applique entre transformations du même domaine ; il ne permute pas
les domaines (déjà le cas pour Warp et Fold). Les cinq nouvelles transformations sont **spectrales** : elles modifient les
coefficients harmoniques complexes cₕ (h = 1…H) de la forme lue, calculés depuis les spectres préparés de la banque (morph
Linear/Stepped : combinaison complexe linéaire, identique à l'interpolation temporelle ; Spectral : morph spectral
existant), puis resynthèse par FFT inverse de 2048 points toutes les 128 échantillons moteur, fondu linéaire entre
tables (mécanisme du morph spectral existant, sans allocation, aucune FFT par échantillon). Limitation de bande : même
profil que la lecture mip (niveaux i0, i0+1 et poids w, mode Clean) ou H = 1024 (Raw), appliqué **après** transformation.
DC forcé à 0 ; h = 1024 (Nyquist de la table) traité comme réel. Préservation de la fondamentale (`wt.z_fund`) : c₁ de
sortie = c₁ d'entrée. Compensation de gain (`wt.z_gain`) : énergie de Parseval de sortie = énergie d'entrée, gain borné
[0,25 ; 4]. Z = 0 ou profondeur 0 : chemin spectral non engagé (bit à bit). Modulation : profondeur, Shape et Z Master
échantillonnés toutes les 128 échantillons moteur (≈ 344–750 Hz) avec fondu ; la modulation audio de Z par CROSS reste
par échantillon pour Fold/Warp/Mutation et **à cadence de contrôle** pour les transformations spectrales (limitation
documentée). Coût : mesuré (§7.4).

### 7.2 Formalisation (v = profondeur effective ∈ [0,1] = depth × courbe(Z Master) ; σ = Shape ∈ [−1,1] ;
λ(σ) = 0,5 + 1,5σ ∈ [−1 ; 2] ; gains appliqués à cₕ)

| Z | Transformation | Fonction | σ | Interprétation |
|---|---|---|---|---|
| Z1 | Harmonic Tilt | gₕ = h^(v·λ(σ)) ; pente = 6,02·v·λ dB/octave de rang | −1 : −6 dB/oct (sombre, massif) ; 0 : +3 dB/oct ; +1 : +12 dB/oct (abrasif) | pondération monotone du rang, liée à la note (≠ filtre de sortie : avant FM/CROSS/Fold, sans résonance ni phase) |
| Z4 | Harmonic Mask | gₕ = 1 − v·(1 − mₕ), mₕ = ½ − ½·cos(2πh/p) ; p = 3·(8/3)^σ (σ ≥ 0), 3·(2/3)^(−σ) (σ < 0) | p ∈ [2 ; 8] continu : −1 = harmoniques paires (creux), 0 = une sur trois, +1 = une sur huit | peigne périodique sur le rang, continu en p (morph et automation sans saut) |
| Z5 | Harmonic Focus | gₕ = (1 − v) + v·exp(−log2(h/C)²/(2b²)), C = 2^(3,5(σ+1)) ∈ [1 ; 128], b = 0,25 + 1,25(1 − v) octave | centre de la région | concentration unimodale autour du rang C (formant, cavité) ; compensation de gain recommandée |
| Z6 | Harmonic Shift | décalage de rang k = 16·v·λ(σ)/2 = 8·v·λ ∈ [−8 ; +16] : c′ₕ = interpolation (amplitude linéaire, phase par moyenne circulaire pondérée) de c en h − k ; sources hors [1 ; H] = 0 | −1 : vers le bas ; 0 : +4 rangs ; +1 : +16 rangs | période conservée (aucune transposition), sortie harmonique (≠ Frequency Shifter) ; collisions : application bijective, poids linéaires entre deux sources voisines ; contenu sortant de la bande supprimé (aucun repli) ; fondamentale : c₁ conservé si `z_fund`, sinon la région h ≤ k est vide |
| Z7 | Phase Scatter | φ′ₕ = φₕ + v·π·ρₕ(σ), \|c′ₕ\| = \|cₕ\| ; ρₕ(σ) = (1−f)·r(h, i) + f·r(h, i+1), position de motif P = 3,5(σ+1), i = ⌊P⌋, f = P − i, r = hachage déterministe → [−1 ; 1] | motif (8 motifs fixes interpolés) | relations de phase modifiées, spectre d'amplitude inchangé ; aucune dépendance au temps, à la graine d'instance ni à un tirage par échantillon |

### 7.3 Critères

| ID | Critère |
|---|---|
| ZC-01 | Registre : énumérations étendues en fin (indices 0–3 inchangés), `wt.za.shape`/`wt.zb.shape` ajoutés (701–702) ; états v2/v3 lus sans changement de valeur |
| ZC-02 | Neutralité : Z Master = 0 ou profondeur 0, chaque nouvelle transformation, 20 configurations : bit à bit identique au moteur sans Z |
| ZC-03 | Existant inchangé : Fold, Warp, Mutation (20 configurations, Shape quelconque) : empreintes de rendu identiques aux empreintes produites par le moteur 0.3.1 (`a9cde4b`) avant modification |
| ZC-04 | Continuité à 0⁺ : v = 1e−4 vs v = 0 : écart RMS ≤ −40 dB du signal (5 transformations, morph Linear et Spectral, Clean et Raw) |
| ZC-05 | Progressivité : distance spectrale (\|cₕ\| en dB pondéré, ou écart temporel pour Z7) non décroissante sur 21 profondeurs, σ ∈ {−1, 0, 1} ; effet à v = 1 ≥ 3 dB (Z7 : écart temporel ≥ −20 dB) |
| ZC-06 | Fondamentale : `z_fund` = 1 : c₁ de sortie = c₁ d'entrée (écart relatif ≤ 1e−9), toutes profondeurs |
| ZC-07 | Gain : `z_gain` = 1 : RMS de table = RMS source ±0,1 dB (hors butée [0,25 ; 4], signalée) ; Z7 : gain 1 exact |
| ZC-08 | DC/Nyquist/aliasing : DC ≤ 1e−9·RMS ; aucune énergie au-delà de la bande (≤ −120 dB) ; notes MIDI 24–108 à 48 kHz, Clean, profondeur 1 : énergie hors harmoniques ≤ −60 dB du total |
| ZC-09 | Ordre : deux transformations spectrales non commutatives (Tilt/Shift, Mask/Shift) : A→B ≠ B→A (écart ≥ −40 dB) et chaque ordre = composition de référence calculée hors moteur (≤ 1e−9 par harmonique) ; composition inter-domaines conforme à §7.1 |
| ZC-10 | Morph XY et Quality : positions XY fractionnaires (Linear, Stepped, Spectral), Eco/Normal/High : rendus finis, ZC-04 et ZC-08 tenus |
| ZC-11 | Modulation rapide : profondeur et Shape par LFO 20 Hz, Z Master par CROSS (taux audio) : fini, crête ≤ 4 × crête statique, saut maximal entre échantillons ≤ 1,5 × le maximum des rendus statiques (profondeurs 0 ; 0,25 ; 0,5 ; 0,75 ; 1) |
| ZC-12 | Déterminisme : deux rendus bit à bit ; Z7 identique entre instances de graines différentes |
| ZC-13 | Intérêt musical de Z7 dans les chaînes réelles : spectre d'amplitude de WT inchangé (≤ 0,01 dB par harmonique) mais (a) sortie FM avec route CROSS WT→FM : écart ≥ −30 dB ; (b) après Harmonic Fold (slot B) : énergie par bande de tiers d'octave différente d'au moins 1 dB dans au moins 3 bandes ; (c) facteur de crête modifié ≥ 1 dB |
| ZC-14 | Distinction : à v = 1, σ = 0, distance d'enveloppe Tilt/Mask, Tilt/Focus, Mask/Focus ≥ 3 dB ; propriétés : Tilt monotone en rang, Mask périodique (minima aux multiples de p), Focus unimodal maximal en C ±1 rang |
| ZC-15 | Propriétés exactes : Z1 gains = formule (≤ 1e−9) ; Z4 atténuation v aux multiples entiers de p ; Z6 k entier ⇒ c′ₕ = c_{h−k} exact ; Z7 amplitudes inchangées (≤ 1e−9) |
| ZC-16 | Stabilité : 60 s, modulations aléatoires de tous les paramètres Z, deux banques (usine et utilisateur), formes simples (sinus, scie) et complexes : 0 non-fini, crête bornée |
| ZC-17 | Coût (mesure, sans seuil, machine de développement Windows ≠ certification M3/M4 Max) : Z spectral statique et modulé vs Z off, temps par bloc P50/P95 ; mémoire ajoutée ; 0 allocation RT |
| ZC-18 | Exemples audio : au moins 2 par nouvelle transformation, rendus par le moteur réel (manuel 0.3.2) |

R19 passe à PASS si les cinq transformations sont implémentées et ZC-01..18 PASS.

## 8. Compilation locale — durcissement

| ID | Critère (harnais `tools/local-build/test_build_local.sh`) |
|---|---|
| LB-08 | Ordre : compilation → tests → signature ad hoc → identité → pluginval rigueur 10 → paquets construits **à partir du bundle validé** → vérification du contenu → ZIP ; empreinte de l'arbre du bundle validé = empreinte du bundle extrait du paquet ; échec de pluginval ⇒ aucun paquet créé |
| LB-09 | Échec après construction des paquets (vérification du ZIP ou du contenu) : livraison supprimée (aucun dossier de livraison présenté valide) |
| LB-10 | Barrière exhaustive (`tools/check_results.sh`, partagée CI/local) : inventaire attendu `tests/expected_results.txt` (portée + identifiant) ; chaque résultat obligatoire présent exactement une fois ; doublon incohérent, résultat manquant, ligne de synthèse absente ou incohérente (suite partielle, crash, timeout, interruption), code de sortie incohérent avec les résultats ⇒ refus ; exceptions `tests/known_failures.txt` bornées à leur portée (`core:` / `wrapper:`) ; inventaire = identifiants des sources (contrôle CI) |
| LB-11 | Variable de filtrage (`SBD_ONLY`) présente : refus de livrer (suite partielle détectée et refusée) |
| LB-12 | Symlinks et collisions : `local-build/` ou une sortie symbolique ⇒ refus ; livraison existante de même version et build ⇒ refus (aucun écrasement) ; ZIP final réextrait et vérifié fichier par fichier contre SHA256SUMS |
| LB-13 | LB-01..07 inchangés et PASS |

## 9. Compatibilité et non-régression

| ID | Critère |
|---|---|
| MIG-01 | 703 StableID : 695 premiers identiques (ordre, type, version), 8 ajoutés en fin ; identifiants VST3 des 695 inchangés (G-22w étendu) |
| MIG-02 | États v1/v2/v3 lus ; rendu des états v3 (20 configurations dont D2 fb > 0 et Z existant) identique aux empreintes 0.3.1 ; état futur (5) refusé |
| MIG-03 | B-T2 sticky, Base/Effective, banques, MIDI Learn, Randomize/Mutate/Evolve, Undo/Redo : suites existantes PASS ; les 8 nouveaux paramètres : `fb_mode` et `shape` randomisables, jamais armements |
| MIG-04 | Cartographie ARCH-UI-03 actualisée (8 StableID, actions Autoriser et journal TA) ; couverture 703/703 |

## 10. Recette physique (EN ATTENTE, non exécutable par l'agent)

Reason 14 et Ableton Live (version relevée) sur Mac Studio M4 Max, VST3 arm64 : protocole en 20 étapes et protocole
d'exports (rapport §13–14). La CI ne prouve pas la compatibilité physique. Verdict C (DSP FINAL LOCK) uniquement après
cette recette.
