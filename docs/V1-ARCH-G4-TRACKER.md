# SUBDESTRUCKT — ARCH-G4 / ARCH-G4.1 / ARCH-G4.2 / ARCH-G4.3 / G4.3-R1 / ARCH-UI-04 / ARCH-UI-04-R1 — Suivi d'avancement (fichier central de reprise)

Branche : `feature/arch-g4-full-development`, créée depuis `feature/arch-g3-vertical-slice` @ `8d23342` + docs `main` @ `58a8022`.
Mission : `docs/V1-ARCH-G4-FULL-MISSION.md` + instruction « développement séquentiel accéléré » du 2026-11-26.
Statuts : TODO · IN PROGRESS · IMPLEMENTED (code présent, critères non encore exécutés en CI) · PASS (critère exécuté et
réussi) · FAIL · BLOCKED (décision produit manquante). Un PASS local (MSVC Windows, vérification de développement) n'est
jamais rapporté comme résultat : la CI macOS arm64 fait foi (colonne « CI »).

## 1. Reprise rapide
- Dernier lot : voir §5 (journal). Prochaine action concrète : §6.
- Build local (dev) : `cmake -S . -B build-local -G Ninja -DSUBDESTRUCKT_BUILD_PLUGIN=OFF` (MSVC 14.38) puis
  `build-local/subdestruckt_core_tests.exe` depuis la racine. JUCE 9.0.6 épinglé cloné hors dépôt pour la compilation
  locale du wrapper.
- Registre : `python tools/gen_params.py` régénère `source/core/ParamTable.inc` et `docs/generated/PARAMETERS.md`.
  Append-only ; `tests/core/expected_param_ids.txt` et `tests/host/expected_vst3_param_ids.txt` à compléter en fin.

## 2. Décisions techniques (réversibles, consignées)
| ID | Décision | Justification |
|---|---|---|
| DT-01 | Registre généré (X-macro) ; 684 paramètres ; 22 v1 inchangés en tête ; discrets structurels non automatisables | StableID explicites, append-only vérifié (REG-ID) ; listes de choix extensibles sans casser l'automation hôte |
| DT-02 | Oracle figé `tests/regression/b5` (copie de 8d23342, namespace `sbd_b5`) compilé dans les tests | Non-régression bit à bit indépendante de la plateforme |
| DT-03 | Banque `factory:saw` en double, construite par l'algorithme b5 ; autres banques en float | Le réseau récursif est chaotique : un écart d'arrondi 1e-8 devient audible (mesuré −16 dB NMSE) ; l'identité exige des tables identiques |
| DT-04 | Graphe causal : 6 nœuds (WT, OP1–4, BUS), 11 ports ; BUS a des entrées implicites Immediate OPk→BUS | Les 55 routes du référentiel ; classification B sticky inchangée ; slots 0–7 = b5 |
| DT-05 | Sync : drapeaux de cycle de l'échantillon moteur précédent | Causal, indépendant de l'ordre d'évaluation, sans cycle |
| DT-06 | Panoramique à puissance constante normalisé au centre (gain 1 par canal au centre) | Non-régression b5 (mono dupliqué) |
| DT-07 | Quality : Normal = politique K b5 ; Eco = Normal pour K (K≥2 permanent à 44,1 kHz) et réduit l'oversampling des algorithmes ; High = 2×K | Pas de changement sonore/latence du défaut b5 ; distinct de Clean/Raw |
| DT-08 | Matrice : control-rate ≈1 kHz (période = round(Fs_moteur/1000)) avec interpolation linéaire ; audio-rate par échantillon ; macros évaluées d'abord, chaîne macro→macro avec 1 échantillon de retard | Candidat du référentiel, déterministe |
| DT-09 | Z : Phase Warp = distorsion de phase à coude (période conservée) ; Harmonic Fold = repli triangulaire du signal lu (gain 1+7z, neutre à 0) ; Spectral Mutation = marches aléatoires par cycle (adaptation DSWS RD-04) | Candidats R&D, neutres à Z=0 (chemin court-circuité), à valider à l'écoute |
| DT-10 | Character WT : profils Phase Quantize / Bit Reduce / Sample Hold ; Character OP = quantification de phase × Master | Candidats ; « profils » non spécifiés au référentiel |
| DT-11 | Morph « Spectral » retiré des choix tant qu'il n'est pas implémenté | Ne jamais présenter une commande inactive |
| DT-12 | Chaîne locale : toute la mémoire préallouée pire cas ; changement d'algorithme = fondu/réinitialisation sans allocation | RT sans allocation |

## 3. Critères préenregistrés (avant mesure ; jamais abaissés)
| ID | Critère et seuil |
|---|---|
| G-01…G-24 | Critères ARCH-G3 conservés (docs/V1-ARCH-G3-PLAN.md §7), étendus aux 55 routes |
| G-15b | (enregistré après l'échec de G-15 étendu, avant mesure) protocole G-15 (200 configurations, 2 s, topologies, notes, automation) restreint aux 22 paramètres v1, autres au défaut : 0 NaN, garde jamais déclenchée, borne PM respectée |
| G-15c | (idem) tous les paramètres aléatoires, gains de sortie des filtres/destruction ≤ 0 dB, makeup Y1 = 0 dB, out.gain ≤ 0 dB : 0 NaN, garde jamais déclenchée |
| REG-ID | Liste des StableID figée identique (ordre, type, version) ; 22 v1 identiques à l'oracle ; métadonnées C26 cohérentes |
| REG-01 | Moteur G4, état b5 migré : NMSE ≤ −100 dB et écart ≤ 1e-5 vs oracle b5 (5 scénarios × 4 Fs), L==R, latence identique ; identité bit à bit rapportée |
| ST-01 | 300 états v2 aléatoires valides (matrice, courbes, noms, seeds, instance) : aller-retour exact 100 % |
| ST-02 | Textes v1 produits par le codec b5 figé : migration acceptée, paramètres v1 exacts, autres au défaut, rendu bit à bit = oracle |
| ST-03 | ≥ 20 états v2 invalides refusés sans effet |
| MIDI-01 | Bend : rapport de fréquence 2^(±n/12) à 1e-9 près, Up/Down indépendants, défaut ±2 |
| MIDI-02 | Priorités Last/Low/High et Mono/Legato conformes sur séquences scriptées |
| MIDI-03 | Sustain CC64 : note tenue jusqu'au relâchement de la pédale |
| MIDI-04 | Même échantillon : expression appliquée avant note-on |
| ENV-01 | DAHDSR : delay/hold exacts à 1 échantillon ; attaque linéaire atteint 1 à round(A·Fs) ± 1 ; relâchement exp < −100 dB en ≤ 2× le temps ; Loop cyclique |
| LFO-01 | Fréquence libre à ±0,1 % ; sync tempo : période = beats·60/bpm ± 1 échantillon ; verrouillage de phase ppq |
| MOD-01 | Domaine Oct exact (×2^(d·s·x), 1e-9) ; Via V=(1−a)+a·v ; audio-rate exact ; raisons d'inactivité (audio non supporté, budget 16, suiveur) ; cycle macro refusé ; 64 max ; fusion : doublon ignoré |
| WT-01 | XY : aux points de grille = forme seule (bit à bit) ; Wrap continu dernier→premier ; Mirror ; Stepped sans fondu |
| WT-02 | Z=0 : bit à bit identique au chemin sans Z ; chaque Z actif : écart NMSE ≥ −20 dB ; Phase Warp/Fold : période conservée (énergie inharmonique ≤ −40 dB) |
| WT-03 | Raw vs Clean, note haute : énergie repliée Raw ≥ Clean + 10 dB |
| FM-01 | Opérateurs 3/4 en PM : bandes latérales |J_k(1)| à ±0,5 dB |
| FM-02 | LinFM/ExpFM : fréquence instantanée sous modulation DC conforme à 0,1 % ; TZFM traverse zéro ; LinFM bloque < 0 |
| FM-03 | Fixed/Offset/Coarse/Fine : fréquence à 0,1 cent |
| FM-04 | Mute : audio nul, modulation inchangée (bit à bit) ; Disable : aucune modulation |
| FM-05 | AC : composante continue de la sortie de modulation < 1e-3 après 1 s |
| CR-02 | Bus Raw/Normalized : formule exacte |
| CR-03 | Cross Audio : Mix = somme ; Crossfade extrémités = moteur seul ; Ring = produit ; AM = formule |
| CR-04 | Cross Reset : phases conservées ; historiques CROSS purgés |
| SYNC-01 | Hard sync interne : période WT = période maître (±1 échantillon) |
| DRN-01 | Drone maintient le gate après note-off ; Panic désarme |
| Q-01 | High : K doublé, latence déclarée = pic impulsionnel ; Eco/Normal = politique b5 |
| RT-G4 | 0 allocation pendant process sur tous les scénarios G4 |
| RNG-01 | PCG32 : vecteurs de référence (seed 42, seq 54) ; χ² uniformité 1e6 tirages p ∈ [0,001 ; 0,999] pour PCG32 et xoshiro128** ; choix : PCG32 si les deux passent et vitesse ≤ 2× |
| CHAOS-01 | Lorenz, Rössler, logistique : 1e7 pas à 100 Hz et 0,01 Hz sans NaN, bornés [−1,1], déterministes ; modèle retenu = premier qui passe tout, dans l'ordre Lorenz, Rössler, logistique |
| FLT-01 | F1 (res 0) : LP12/HP12 à fc = −3,01 ± 0,5 dB ; pente entre 4fc et 8fc : 12 ± 1,5 dB/oct (12 dB), 24 ± 3 (24 dB) ; BP : pic à fc ± 2 % ; Notch : ≤ −30 dB à fc (fc = 1 kHz, 48 kHz) |
| FLT-02 | F1/F2/F3/F4, 300 configurations aléatoires dont cutoff audio-rate 20 Hz–20 kHz et résonance max, entrée bruit ±1 : 0 NaN/Inf, |y| < 16 |
| FLT-03 | F2 : pic de résonance (feedback +0,9, damping 0) à fc ± 5 cents pour fc ∈ {40, 110, 440, 1000, 4000} Hz à 48 et 96 kHz |
| FLT-04 | F4 : fréquences des modes = modèle ± 1 cent (harmonique, f0 = 220 Hz) ; T60 du mode 1 = Decay ± 10 % (Decay = 1 s, damping 0) ; Freeze non armé : aucune persistance après le garde-fou ; armé : énergie conservée |
| FLT-05 | F3 : drive 0 dB, bias 0, niveau −40 dBFS : réponse = F1 de même mode ± 0,5 dB à fc et 4fc ; THD croissante avec le drive (0, 12, 24 dB) |
| FLT-06 | Excitation Note : réponse non nulle sans signal d'entrée ; Dual Mono + spread : L ≠ R ; Linked : L == R bit à bit ; Pre/Post : ordre effectif différent mesurable avec un processeur non linéaire |
| FLT-07 | Changement de type en cours de note : 0 NaN ; 0 allocation |
| DST-01 | N1 : fold 0, drive 0 dB, bias 0, sym 0 : NMSE ≤ −40 dB vs entrée (−20 dBFS) retardée d'un demi-échantillon (délai de groupe inhérent à l'ADAA1 ; amendement consigné avant toute mesure) ; THD croissante drive 0/12/24 dB. Comparaison anti-repliement (sinus 3 kHz, 48 kHz, drive 24 dB, fold 1) : naïf, ADAA1, ADAA1 + K×2 ; règle : candidat le moins coûteux avec énergie repliée ≤ −40 dB, sinon le plus bas ; rapporté |
| DST-02 | D1 : 16 bits, ratio 1, jitter 0 : NMSE ≤ −80 dB ; 4 bits sur sinus pleine échelle : SNR = 25,8 ± 3 dB ; ratio R, Step, Raw : sortie constante par paliers de R échantillons |
| DST-03 | Y1 : sinus −6 dBFS, seuil −20, ratio 4, knee 0 : sortie −16,5 ± 0,5 dBFS ; gate (−30 dB) sur −50 dBFS : atténuation ≥ 40 dB ; transient +1 : rapport crête attaque/sustain > transient 0 |
| DST-04 | S1 Ring 100 Hz sur 1 kHz : raies 900/1100, porteuse ≤ −40 dB ; Shift +100 Hz : raie 1100, image 900 ≤ −30 dB |
| DST-05 | N2 : 200 configurations, fb max, non armé : bornée, finie ; auto-oscillation seulement armée |
| DST-06 | D2 : Freeze armé : sortie périodique de période = taille ± 1 échantillon ; vitesse −1 : corrélation ≥ 0,9 avec le segment inversé |
| DST-07 | Y2 : profondeur +1 augmente le rapport attaque/sustain ; source Y2 dans [0,1] |
| DST-08 | S2 : latence = taille FFT, dry/wet alignés (impulsion, mix 0 : pic à L exactement ; mix 1 : pic du chemin traité ≈ L) ; latence de la chaîne déclarée à l'hôte ; Freeze armé : RMS stable ± 3 dB sur 2 s sans entrée |
| DST-09 | 6 emplacements, algorithmes et paramètres aléatoires, 4 Fs, 2 s, notes : 0 NaN, garde jamais déclenchée, 0 allocation |
| DST-10 | Mix 0 (algorithmes sans latence) : sortie identique bit à bit au contournement |
| ST-RND-01 | Randomize : même seed + compteur -> résultat identique ; compteur différent -> différent ; paramètres verrouillés inchangés ; intensité 0 -> état inchangé (bit à bit, compteur inchangé) ; résultat valide ; familles respectées (paramètres hors famille inchangés) |
| ST-RND-02 | Mutate : écart normalisé ≤ intensité × échelle du profil × 0,15 pour chaque paramètre continu ; déterministe ; verrous |
| ST-RND-03 | Transaction refusée (état invalide) : état et compteurs inchangés |
| UNDO-01 | 128 transactions max ; undo/redo restaurent exactement l'état (compteurs RNG compris) ; redo sans tirage ; nouvelle transaction après undo efface le redo ; doublon ignoré |
| EVO-01 | Evolve : décalages bornés (≤ intensité × échelle × 0,25) en Anchored et Drift ; Anchored : moyenne quadratique < Drift sur 60 s ; Reset -> décalages nuls ; Commit -> Base = Base + décalage, état valide ; hors liste blanche : inchangé |
| PRE-01 | Bibliothèque : presets usine valides et chargeables ; sauvegarde/rechargement exact (scope preset) ; favori persistant ; fichier corrompu refusé |
| WTA-01 | SHA-256 : vecteurs FIPS (« abc », chaîne vide) ; WAV float32 écrit puis relu : identique ; import de cycles 2048 : contenu brut exact ; cycle 1024 sinus : forme rééchantillonnée = sinus 2048 à 1e-5 près ; identifiant déterministe ; état avec banque intégrée : aller-retour exact, données altérées refusées (intégrité) |
| DST-09b | (enregistré après l'échec de DST-09, avant sa mesure) mêmes 48 configurations mais gains de sortie des emplacements ≤ 0 dB et makeup Y1 = 0 dB : 0 NaN, garde jamais déclenchée, 0 allocation. DST-09 reste rapporté tel quel |

### 3bis. Critères ARCH-G4.1 (enregistrés le 2026-11-27, avant toute mesure)
| ID | Critère et seuil |
|---|---|
| BAL-01 | Balance WT/FM au centre : sortie bit à bit identique à l'absence de balance ; à −1 : contribution FM nulle (sortie = WT seule, bit à bit) ; à +1 : WT nulle ; profondeurs de modulation, topologie et phases inchangées (rendu CROSS du moteur non audible identique) ; aucun gain > 1 |
| ENV-02 | Déclenchement Voice (défaut) = comportement 0.2.0 bit à bit ; Note : relance à chaque note-on même en Legato ; Gate : relance seulement à l'ouverture du gate même en Mono ; relance sans saut de niveau (|Δ| ≤ pas d'attaque) ; Sync : durées en temps (noires) — attaque de 1 noire à 120 bpm = 0,5 s ± 1 échantillon, à 90 bpm = 0,667 s ± 1 ; changement de BPM pris en compte au bloc suivant |
| SPEC-HR-01 | (spécification proposée, à arbitrer) Protection d'extrême niveau ELP : au-delà de +24 dBFS (|y| > 16) en sortie, saturation douce asymptotique vers +30 dBFS ; inactive en dessous (bit à bit) ; engagements comptés et signalés (télémétrie, mètre). Garde numérique +30 dBFS conservée |
| HR-01 | Sous +24 dBFS : sortie bit à bit identique au moteur sans ELP (REG-01 + 50 configurations aléatoires à gains ≤ 0 dB) |
| HR-02 | Protocoles G-15 et DST-09 inchangés, sous SPEC-HR-01 : 0 NaN, garde +30 dBFS jamais déclenchée, engagements ELP rapportés. Les FAIL historiques de G-15/DST-09 (commit c741fb8) restent consignés |
| N1-02 | N1 hybride ADAA1 + suréchantillonnage interne (Eco : ×1, Normal : ×2, High : ×4) : énergie repliée (3,1 kHz, drive 24, fold 1, 48 kHz) Normal ≥ 3 dB et High ≥ 6 dB meilleure que ADAA1 seul ; latence déclarée = latence mesurée (impulsion) ; dry/wet aligné (mix 0,5 sur impulsion : un seul pic) ; cas modéré (1 kHz, drive 12, fold 0,5) rapporté |
| WT-SM-01 | Morph spectral : aux points de grille = forme (NMSE ≤ −60 dB) ; deux formes de même spectre d'amplitude en opposition de phase : milieu spectral RMS ≥ 0,7 × RMS des extrémités (morph linéaire : ≤ 0,05) ; aucune allocation ; aucune FFT par échantillon |
| CAP-01 | Captures R2 : capture D2 (segment) et S2 (spectre) sans allocation RT ; aller-retour d'état exact (SHA-256 vérifié, altération refusée) ; restauration : contenu identique ; un preset ne rétablit jamais Arm/Freeze armé ni Drone ; état DAW : armements restaurés seulement avec l'autorisation d'instance |
| AUD-01 | Audit des commandes : chaque paramètre continu ou discret modifie la sortie dans au moins un contexte d'activation documenté, sinon il figure dans une liste d'exceptions justifiées |
| MT-01 | Microtuning : Custom, +50 cents sur La : fréquence × 2^(50/1200) ± 0,1 cent ; Equal : offsets ignorés ; A4 = 432 Hz respecté |
| FOL-01 | Suiveurs : source dans [0, ~1] proportionnelle au niveau de la chaîne mesurée ; route suiveur WT -> cutoff global active et efficace |
| ENV-03 | (clause BPM d'ENV-02, tolérance fixée à l'écriture du test, avant exécution) attaque synchronisée d'une noire, BPM 120 -> 60 à mi-attaque : fin à 36000 ± 128 échantillons, aucun pas > pente à 60 bpm |
| WT-ED-01/02w | Banques utilisateur : générateurs déterministes bornés 64×16 ; forme dessinée (DC nul, crête 1) ; remplacement par copie ; .sdwt aller-retour (même identifiant), altération refusée ; banque intégrée à l'état ; côté processeur : génération, édition, Undo/Redo |
| CHR-01, TOPO-01, MACRO-01, UI-01w, CAP-02w, PERF-03 | §9/§10 : Character neutre à 0 et déterministe ; 10 topologies valides/distinctes, CROSS conservé ; cycles de macros refusés, suiveur en amont inactif ; navigation/tailles/Inspector sans modification d'état, titres accessibles ; sauvegarde DAW d'un gel ; mesures Eco/Normal/High sans seuil |

Résultats ARCH-G4.1 (local MSVC, puis CI macOS arm64 — colonne CI de la matrice) : tous PASS sauf la clause DAW de CAP-01 (**non conforme** : la restauration DAW est fidèle, armements compris, car le critère antérieur G-21w exige un état DAW identique après rechargement ; arbitrage A-G41-03). G-15 et DST-09 : FAIL historiques au commit c741fb8 (consignés) ; PASS au commit courant **uniquement sous SPEC-HR-01 proposée** (si refusée : retour à FAIL).

### 3ter. Critères ARCH-G4.2 (enregistrés le 2026-11-27, avant toute mesure)
Texte intégral : `docs/V1-ARCH-G4.2-SPEC.md` (commits 87d1ed6 et 1ef1c67, antérieurs aux mesures) : ELP-01..07,
AUT-01..08, D2-01..05, ML-01w..07w, LB-01..07. Additifs consignés pendant l'exécution (§7 du document) : D2-04c et
AUT-02c, **complémentaires, définis après diagnostic**, qui ne remplacent pas les critères en échec ; CAP-02w : étape
« Autoriser » insérée (résultat G4.1 conservé). Échecs documentés (`tests/known_failures.txt`) : ELP-01 (plateau
numérique de tanh), D2-04 (moteur historique, fluctuation bornée), D2-04c (une fenêtre après coupure), AUT-02
clause (c) (S2 décroît par contrat). Résultats : rapport `docs/V1-ARCH-G4.2-REPORT.md`.

### 3quater. Critères ARCH-G4.3 (enregistrés le 2026-11-27, avant toute mesure)
Texte intégral : `docs/V1-ARCH-G4.3-SPEC.md` (commit 2a228d4) : TA-01..25 (+ TA-xxw), ELP-01R, D2R-01..11, R2-01..05,
ZC-01..18, LB-08..13, MIG-01..04 ; arbitrages et révisions fonctionnelles RF-01..04 (§1) ; additifs d'exécution (§11).
Échecs documentés ajoutés (`tests/known_failures.txt`, désormais bornés à leur portée `core:` / `wrapper:`) : R2-02,
ZC-05, ZC-13. Arbitrages ouverts : A-G43-01 (niveau de Harmonic Tilt), A-G43-02 (progressivité de Harmonic Shift).
Contrats : `docs/V1-AUTONOMY-CONTRACT.md`, `docs/V1-Z-TRANSFORMS-CONTRACT.md` ; procédure : `docs/V1-LOCAL-BUILD.md`.
Résultats : `docs/V1-ARCH-G4.3-REPORT.md`. Point de reprise MSVC : le suivi des dépendances d'en-têtes fonctionne dans
un répertoire de build recréé (`ninja -t deps` VALID) ; l'ancien `build-local` hérité de G4.2 produisait des objets
périmés. Tests : `tests/expected_results.txt` (inventaire) à compléter pour tout nouveau critère (INV-01 en CI).

### 3quinquies. Critères ARCH-G4.3-R1 (enregistrés le 2026-11-27, commit 58bf858, avant toute mesure)
Texte : `docs/V1-ARCH-G4.3-R1-SPEC.md` : HT-01..08 (Harmonic Tilt à énergie constante, A-G43-01), ZC-05R (Shift,
A-G43-02), ZC-13R (Scatter, A-G43-03), R2-02R (A-G43-04), ZA-01/02 (audit). Critère remplacé : ZC-15 (clause Tilt) par
HT-02/03. Nouvel échec documenté : R2-02R (a) (source Metal, résolution du gel S2). Défauts réels corrigés : Tilt
(niveau), Shift (continuité aux bornes et phase). Version 0.3.3. Rapport : `docs/V1-ARCH-G4.3-R1-REPORT.md`.

### 3sexies. DSP FINAL LOCK (2026-11-27) et ARCH-UI-04
DSP FINAL LOCK prononcé par le propriétaire avec acceptation des risques résiduels : `docs/V1-DSP-FINAL-LOCK.md`
(référence 0.3.3, SHA 0469c4d, build 263311915). R-26 Reason 14 : GO déclaré (14 critères, 7 instances, réserve D2
Repeat levée) ; Drone après réouverture collective : non consigné. R-27 Ableton Live : RETIRÉ (compatibilité non
certifiée). R-28, R-29, R-30 : NON EXÉCUTÉS jusqu'à leur terme, risques acceptés — aucun n'est PASS. Chemins du journal
TA corrigés (`~/Library/Labtechno/SUBDESTRUCKT/`). ARCH-UI-04 (interface SYMBIOTIC INDUSTRIAL) autorisée ; mission UI
seule, `source/` verrouillé.

### 3septies. Critères ARCH-UI-04 (enregistrés avant mesure, en-tête de `tests/wrapper/Ui04Tests.cpp`)
UI04-INV, COV, BIND, ALIAS, INACT, NAV, CLIP, PANIC, UNDO, AUTO, STATE, KEY, CORE, CON, OPEN, MULTI, RT, DSP, DECO, OPS
(inventaire `tests/expected_results.txt`). Additif d'exécution, défini après le diagnostic d'un crash et ajouté au
critère UI04-NAV sans en retirer de clause : aucune destruction de composant par la navigation ou la sélection.
Résultats : `docs/V1-ARCH-UI-04-REPORT.md` (20/20 PASS en local et en CI run 48). Version 0.4.0.

### 3octies. ARCH-UI-04-R1 (TRANSPORT AUTHORIZATION, anomalie R-28 ; critères enregistrés avant correction)
Audit physique Codex (0.4.0-b263320407) : NO GO sur R-28 (export sans PLAY puis annulation : signal à l'arrêt, D2 à
−11,8 dBFS). Critères : `tests/core/test_ui04r1_ta.cpp` (TA-R01..R13, R15, R16) et `tests/wrapper/TaR1Tests.cpp`
(TA-R01w, R13w, R14, R15w), commit `7d57d94` avant toute modification du code : 15 reproductions en ÉCHEC sur 0.4.0, 4
non-régressions PASS. Correction 0.4.1 : toutes PASS ; TA-15 clause (e) = critère remplacé (`tests/known_failures.txt`).
Contrat : `docs/V1-AUTONOMY-CONTRACT.md` §2 bis ; dérogation au DSP FINAL LOCK : `docs/V1-DSP-FINAL-LOCK.md` §6 ; rapport :
`docs/V1-ARCH-UI-04-R1-REPORT.md`. H-04 : cause établie par Codex (⌘Q déclenché par le raccourci Computer Use sur AZERTY,
fermeture normale, 0 crash) ; plus un blocage ; aucun correctif UI.
**Clôture (2026-11-28)** : contre-audit physique Codex sur 0.4.1 (build 263320802, Reason 14.1d100) — R-28 PASS dans le
périmètre exécuté ; PANIC pendant un export et sauvegarde pendant un export NON EXÉCUTÉS physiquement (dialogue modal),
couverts par TA-R06/TA-R14 ; ARCH-UI-04-R1 : GO technique avec réserves ; DSP FINAL LOCK révisé en 0.4.1. Rapport
indépendant et archive de preuves conservés localement par le propriétaire.

### 3nonies. ARCH-UI-04-R2-A (référentiel Design & Ergonomics, documentaire, 2026-11-28)
Entrées : revue artistique et ergonomique Codex (`docs/ARCH-UI-04-DA/`, commit `dbc21c8`, 6,7/10 descriptif) et 15
arbitrages du propriétaire DA-A01…DA-A15 (non rouverts). Livrable : `docs/ARCH-UI-04-R2/` (14 fichiers). Contenu :
invariants INV-01…12 ; nomenclature anglaise compacte, 2 213 occurrences, largeurs **estimées** à confirmer par mesure
JUCE (PT-02) ; contrats NEXUS, NEURAL CELL, Cybernetic Display, Plasma ; espaces adaptatifs avec étude géométrique
calculée sur le code 0.4.1 (PERFORMANCE sans défilement au nominal, défilement au minimum en 0.4.1) ; réseaux
NEURAL/TABLE ; enveloppe du Core ; inventaire de télémétrie (30 données, classes A/B/C/D) ; matrice d'acceptation
enregistrée **avant mesure** (`docs/ARCH-UI-04-R2/R2-ACCEPTANCE-MATRIX.md`) ; 19 décisions ouvertes. Aucun lot de
développement autorisé : R2-B attend la contre-expertise de l'architecte et la validation du propriétaire.
`git diff d2d08ac -- source/` vide.

### 3decies. ARCH-UI-04-R2-B1 (Component Lab, 2026-11-28)
Arbitrages du propriétaire : OD-01 B, OD-02 B, OD-06 B, OD-10 B (jamais sans reproduction préalable), OD-11 B, OD-17 A
(le Component Lab n'est pas une version ; première livraison intégrée R2-B en 0.5.0), OD-18 A. Corrections C01 à C06 de
la contre-expertise intégrées au référentiel (`docs/ARCH-UI-04-R2/R2-DECISION-REGISTER.md` §3 bis). **C06** :
`tests/known_failures.txt` compte dix entrées, soit huit échecs documentés et deux critères remplacés (ZC-15, TA-15),
et non neuf (erratum en `docs/V1-DSP-FINAL-LOCK.md` §7). Livrable : laboratoire isolé `lab/` (projet CMake séparé,
aucun fichier de production modifié), workflow `component-lab.yml` (macOS 15 VM, résultats versés par commit
[skip ci]), `docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/`. Résultats : PT-02 mesuré sur 2 213 occurrences × 2 plateformes,
0 dépassement ; PT-05 macOS 16/16 glyphes natifs, Windows 14/16 (→ substitué dans Verdana) ; plancher critique de 9 px
à 75 % atteint 26/26 ; contrastes 27/28 (libellé PANIC 0.4.1 blanc sur rouge 3,84:1 < 4,5:1, variante carbone
5,08:1 soumise) ; auto-tests du laboratoire LAB-G01..G13 13/13 PASS (composants R2 seulement). PB-01 et PB-02 ne sont
pas corrigés en production (protocoles de reproduction prêts). Validation artistique du propriétaire en attente.

### 3undecies. ARCH-UI-04-R2-B1-R1 (raffinement artistique final, 2026-11-28)
Arbitrages normatifs du propriétaire : LAB-A01 A (NEXUS conservés), LAB-A02 B (NEURAL CELL Machined Titanium),
LAB-A03 B (SYNAPTIC Machined Track), LAB-A04 C (Cybernetic HUD), LAB-A05 A (PLASMA conservé), LAB-A06 C (Symbiotic
Industrial Hybrid), LAB-A07.1 B (PANIC carbone sur rouge), LAB-A07.2 C (PLASMA vertical STANDARD 64 x 180 et COMPACT
48 x 150), LAB-A08 B (nomenclature P1 optimisée : AMOUNT et MODE sous CROSS AUDIO, KEEP FUND, OUT GAIN ; XAUDIO
abandonné). Livrable : `docs/ARCH-UI-04-R2/B1-R1-FINAL-ARTISTIC-REFINEMENT/`, nomenclature révisée
`docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-R1.csv` (la révision R2-A reste reproductible à l'identique). Critères AR-01 à
AR-15 ; AR-15 (validation artistique) est exclusivement humain.

### 3terdecies. R2-B2-00 GO et seuils GC-01 définitifs (2026-11-28, avant toute mesure)
Seuils **GS-01 à GS-07 définitifs** (DB2-07) et critères INT amendés, enregistrés avant toute mesure :
`B2-PRODUCTION-INTEGRATION/B2-00-ADDENDUM.md` §4 et §7. Matériel de référence : M3 de base (DB2-01).

### 3duodecies. AR-15 et ARCH-UI-04-R2-B2-00 (2026-11-28)
Décision humaine du propriétaire : **AR-15 PASS** (option A, validation artistique complète), **B1-R1 GO FINAL**, gel du
Design System R1, LAB-A01 à LAB-A08 validés, R2-B2 autorisé (`B1-R1-FINAL-ARTISTIC-REFINEMENT/OWNER-DECISION-AR15.md`).
La validation artistique ne vaut ni validation fonctionnelle du plugin intégré ni recette physique.
R2-B2-00 : critères **INT-01 à INT-21** enregistrés avant mesure (`B2-PRODUCTION-INTEGRATION/B2-ACCEPTANCE-CRITERIA.md`),
protocole GC-01 et seuils candidats GS-01 à GS-07 (à confirmer avant mesure, DB2-07), écarts EC-01 à EC-16, décisions
DB2-01 à DB2-07. Contradiction normative relevée et soumise (non tranchée) : matériel minimum M1 (mission) / M3 de base
(`V1-PLATFORM-REQUIREMENTS.md`, révision validée du 2026-11-25).

## 4. Matrice d'exigences P0/P1
Pondération ARCH-G4.1 (§14) : PASS = 1, IMPLEMENTED = 0,5, TODO/FAIL/BLOCKED = 0, sur les **50** exigences (BLOCKED compris). L'ancienne pondération ARCH-G4 (sur les lignes non bloquées) est conservée au rapport ARCH-G4.
| # | Exigence | Lot | Statut | CI |
|---|---|---|---|---|
| R01 | Registre stable extensible (C23–C27) | F | PASS (CI run 3) | — |
| R02 | État versionné v2 + migration 0.1.0-b5 | F | PASS (CI run 9) | — |
| R03 | Transactions/ownership hors RT (topologie, matrice, banque, courbes) | F | PASS (CI run 9) | — |
| R04 | Timeline MIDI sample-accurate, ordre C11 étendu (bend/CC/pression) | F | PASS (CI run 9) | — |
| R05 | Mono/Legato, priorités Last/Low/High, sustain | F | PASS (CI run 9) | — |
| R06 | Expressivité : bend Up/Down, pression, CC1/2/11/64, vélocité | F | PASS (CI run 9) | — |
| R07 | Accordage A4, microtuning 12 classes | F | PASS (G4.1 : MT-01) | run 33 |
| R08 | 5 enveloppes DAHDSR lin/exp, loop | F | PASS (G4.1 : ENV-01/02/03 (Trigger Voice/Note/Gate, Sync tempo)) | run 33 |
| R09 | 4 LFO (8 formes dont Custom), libre/tempo/phase ppq, retrigger | F | PASS (CI run 9) | — |
| R10 | Tail/Drone/Panic | F | PASS (G4.3 : Panic révoque systématiquement, purge des lignes d'alignement ; TA-09..12, TA-21/22 ; ARCH-UI-04-R1 : autorisation de rendu temporaire, état d'exécution remis à zéro au retour temps réel, TA-R01..R16 ; R-28 PASS physique dans le périmètre Reason 14 exécuté, Panic et sauvegarde pendant un export non exécutés physiquement) | UI-04-R1 |
| R11 | WT banques immuables XY 64×16, Clamp/Wrap/Mirror, Linear/Stepped | WT | PASS (CI run 9) | — |
| R12 | WT morph Spectral | WT | PASS (G4.1 : WT-SM-01) | run 33 |
| R13 | WT interpolation Linear/Cubic, Clean/Raw | WT | PASS (CI run 9) | — |
| R14 | WT phase free/retrigger/fixed/random, sync hard/soft (interne/OP) | WT | PASS (CI run 9) | — |
| R15 | WT Character | WT | PASS (G4.1 : CHR-01) | run 33 |
| R16 | Génération procédurale, import/export WAV, format portable, édition COW | WT | PASS (G4.1 : WTA-01, WT-ED-01, WT-ED-02w (générateur, éditeur, .sdwt, UI Banques)) | run 33 |
| R17 | Z deux slots A→B/B→A, Z Master, profondeurs/courbes, fondamentale, gain | WT | PASS (CI run 9) | — |
| R18 | Z : 3 transformations attestées | WT | PASS (CI run 9) | — |
| R19 | Z : 5 transformations du catalogue validé | WT | IMPLEMENTED (G4.3-R1 : Tilt corrigé HT-01..08, Shift ZC-05R, Scatter ZC-13R ; restriction d'ordre inter-domaines non acceptée ; écoute R-29) | G4.3-R1 |
| R20 | FM 4 opérateurs, PM/LinFM/TZFM/ExpFM par destinataire | FM | PASS (CI run 9) | — |
| R21 | 9 formes, ratio/fixed/offset/coarse/fine | FM | PASS (CI run 9) | — |
| R22 | Niveau audio/pan ≠ modulation, mute ≠ disable, AC/DC, Character + Master | FM | PASS (CI run 9) | — |
| R23 | Matrice 4×4 + 10 topologies préréglées | FM | PASS (G4.1 : TOPO-01) | run 33 |
| R24 | ExpFM benchmark | FM | PASS (PERF-02 : ExpFM = coût PM/Lin/TZ) | — |
| R25 | CROSS 55 routes B-T2 sticky, bus Raw/Normalized, directions | CR | PASS (CI run 9) | — |
| R26 | Sync bidirectionnel, Cross Audio Mix/Crossfade/Ring/AM, Cross Reset | CR | PASS (CI run 9) | — |
| R27 | Destruction N1/D1/Y1/S1 | DST | PASS (CI run 9) | — |
| R28 | Destruction N2/D2/Y2/S2 | DST | PASS (CI run 9) | — |
| R29 | 6 slots, chaînes locales/globale, dry/wet aligné, latence | DST | PASS (CI run 9) | — |
| R30 | Filtres F1/F2 | FLT | PASS (CI run 9) | — |
| R31 | Filtres F3/F4 | FLT | PASS (CI run 9) | — |
| R32 | Pre/Post, excitation, stéréo linked/dual | FLT | PASS (CI run 9) | — |
| R33 | Matrice globale 64, Via, courbes, rates, inactivité, macros | MOD | PASS (CI run 9) | — |
| R34 | RNG PCG32/xoshiro benchmark, chaos 3 modèles | MOD | PASS (CI run 9) | — |
| R35 | Suiveurs ×3, Y2 sources | MOD | PASS (G4.1 : FOL-01 (correctif setup), DST-07 (Y2)) | run 33 |
| R36 | Presets : bibliothèque, tags/favoris, import/export | STATE | PASS (CI run 9) | — |
| R37 | Randomize/Mutate/Evolve (verrous, intensité, seed, profils) | STATE | PASS (CI run 9) | — |
| R38 | Undo/Redo transactionnel (128, RNG) | STATE | PASS (CI run 9) | — |
| R39 | Quality Eco/Normal/High, transition contrôlée, latence | Q | PASS (CI run 9) | — |
| R40 | Limiteur musical optionnel, garde numérique | Q | PASS (G4.3 : SPEC-HR-01 adoptée, ELP-01R PASS ; ELP-01 FAIL historique conservé) | G4.3 |
| R41 | Benchmark CPU reproductible | Q | PASS (PERF-01 rapporté, sans seuil normatif) | — |
| R42 | UI Performance | UI | PASS (ARCH-UI-04 : UI04-COV/BIND/CORE/PANIC ; recette humaine H-01..H-09 à faire ; tâches UX01..18 non mesurées) | UI-04 |
| R43 | UI Sound Design (WT/FM/CROSS central/Process/Mod) | UI | PASS (ARCH-UI-04 : UI04-COV/BIND/INACT ; ordre Z effectif affiché) | UI-04 |
| R44 | UI Engineering (réseaux, matrice, banques, état, diagnostics) | UI | PASS (ARCH-UI-04 : UI04-OPS/DECO/BIND) | UI-04 |
| R45 | Inspector, moniteurs (scope/spectre/XY/FM), zoom/accessibilité | UI | IMPLEMENTED (G4.1 : UI-01w PASS ; ARCH-UI-04 : UI04-CLIP/CON/NAV/KEY PASS (zoom 75-150 %, 960x640) ; Retina réel non mesuré) | UI-04 |
| R46 | Wrapper VST3 : 684 paramètres, état v2, MIDI étendu, latence dynamique | PLG | PASS (G4.3 : 703 paramètres, format d'état 4, G-21w, G-22w, TA-xxw) | G4.3 |
| R47 | MIDI Learn global séparé des presets | PLG | PASS (G4.2 : ML-01w..07w (affectation, remplacement, profil isolé, presets, multi-instance, conflits, RT) ; recette physique R-11 à faire) | run 37 |
| R48 | Packaging 0.2.0 testé (sans diffusion publique) | PKG | PASS (G4.3 : paquets 0.3.2 en CI, NON publiés ; compilation locale LB-01..13, barrière exhaustive) | G4.3 |
| R49 | Manuel HTML édition G4 + assistant de recette | DOC | PASS (G4.3 : manuel 0.3.2, 29 tests de recette dont R-26..R-29, contrôles bloquants + Playwright) | G4.3 |
| R50 | Captures créatives Freeze/Glitch (R2) | STATE | IMPLEMENTED (G4.3-R1 : R2-02R (b)(c)(e) PASS, (a) FAIL documenté (Metal, résolution S2) ; WAV A/B ; écoute R-30 et recette physique à faire) | G4.3-R1 |

## 5. Journal
| Date | Lot | Commit | Résultat |
|---|---|---|---|
| 2026-11-26 | F1 registre/état/moteur généralisé | 079e94e | Local : REG-01 bit à bit 20/20 |
| 2026-11-26 | Tests fonctions + fold ADAA / sync | ec77650 | Local 41/41 |
| 2026-11-26 | Wrapper v2 + CI arch-g4 | bad063a | CI run 1 (45892274793) : FAIL G-21 hôte (intermittent) ; RTSan PASS |
| 2026-11-26 | Filtres F1–F4 | 110c490 | CI run 2 (45892837859) : SUCCESS complet (cœur, wrapper, hôte, pluginval 10, paquets, recette VM) |
| 2026-11-26 | Pont tolérant à l'écho hôte | d31a80c | CI run 3 (45893236546) : SUCCESS complet |
| 2026-11-26 | Destruction N1..S2, alignement latence, barrière CI | (voir git) | Local : DST 11/12 PASS, DST-09 FAIL documenté |
| 2026-11-26 | State : Randomize/Mutate/Evolve, Undo, presets ; G-15b/c | (voir git) | Local : 6/6 State PASS ; G-15 étendu FAIL documenté, G-15b/c PASS |

| 2026-11-26 | UI fonctionnelle | 764ce4f | CI run 7 : build-test-package SUCCESS (UI compilée macOS) ; RTSan FAIL (règle corrigée en 172a0d6) |
| 2026-11-26 | PERF, banques utilisateur, textes 0.2.0 | f640c5c..775bfdf | Local PASS ; CI en cours |
| 2026-11-26 | Fusion outillage du manuel (feature/arch-g3-user-manual) | 3a81d8a | — |
| 2026-11-26 | Manuel 0.2.0 + publication gh-pages (manual/0.2.0) | 573b60a / gh-pages 1631944 | build OK ; CI run 9 (f640c5c) SUCCESS complet (63/65, 2 FAIL documentés) |
| 2026-11-26 | Rapport final | (ce commit) | docs/V1-ARCH-G4-REPORT.md ; avancement 85,7 % |

| 2026-11-27 | G4.1 : balance, Trigger/Sync, SPEC-HR-01, état v3 | efeee24 | Local PASS ; CI : barrière en échec (liste d'échecs connus vide sous pipefail), corrigée en f1183b2 |
| 2026-11-27 | N1 hybride OS ; morph spectral ; captures R2 | c116aa4, 83f5d39, 22297e1 | Local PASS |
| 2026-11-27 | AUD-01 + correctifs suiveurs / modulation de sortie | 7a82590 | Local PASS (686 effectifs, 9 exceptions) |
| 2026-11-27 | UI ; banques utilisateur ; perf ; licence ; manuel 0.3.0 ; §9 | e9d6919..feafcb6 | voir rapport ARCH-G4.1 |

| 2026-11-27 | G4.2 : critères avant mesure | 87d1ed6, 1ef1c67 | — |
| 2026-11-27 | G4.2 : ELP, autorisation autonome, étude D2, MIDI Learn | 4912ea6 | Local : cœur sans régression (seuls ELP-01, D2-04, D2-04c documentés) ; wrapper : AUT-02 (c) documenté |
| 2026-11-27 | G4.2 : compilation locale sécurisée, harnais LB, version 0.3.1 | 49bfa0c | Local : LB-01..07 PASS ; CI : voir rapport G4.2 |
| 2026-11-27 | G4.2 : manuel 0.3.1, propositions Z, rapport | 0ab245e, 9d604e4 | Playwright 0.3.1 local : 46 PASS, 4 ignorés (b5) ; CI runs 35/36 : job macOS en échec à l'étape wrapper (code de sortie avant la barrière) |
| 2026-11-27 | G4.2 : verdict du wrapper à la barrière | 6ac4b83 | **CI run 37 (45951352395) SUCCESS complet** (pluginval 10, paquets, RTSan, LB, manuel) ; FAIL documentés seuls |
| 2026-11-27 | G4.3 : critères avant mesure, empreintes 0.3.1 (msvc-x64, clang-arm64) | 2a228d4 | CI run 39 SUCCESS (empreintes publiées en annotation) |
| 2026-11-27 | G4.3 : TRANSPORT AUTHORIZATION, purge Panic | 528a000 | Local : TA 24/24, TA-xxw 9/9 |
| 2026-11-27 | G4.3 : D2 Legacy/Regenerative, Z, R2, topologie par défaut, barrière exhaustive, LB-08..13, 0.3.2 | d7dcc22 | CI run 40 SUCCESS |
| 2026-11-27 | G4.3-R1 : spécification corrective avant mesure | 58bf858 | — |
| 2026-11-27 | G4.3-R1 : Tilt à énergie constante, continuité Shift, tests HT/ZC-05R/ZC-13R/R2-02R/ZA, 0.3.3 | 2985901 | — |
| 2026-11-27 | G4.3-R1 : manuel 0.3.3, campagne G431, documentationSha ; SHA livré 0469c4da6181eaa000bfc402b70b97e1dfef989b | 937ac6d, f1793ee, 0469c4d | run 44 SUCCESS |
| 2026-11-27 | G4.3 : manuel 0.3.2, contrats, rapport ; documentationSha | c7b8573, 97100b0 | run 41 : job manuel en échec (documentationSha vide ; titre S0.1 dynamique) ; **run 42 (45992835990) SUCCESS complet, SHA livré 97100b0** |
| 2026-11-27 | DSP FINAL LOCK : consignation, chemins du journal TA corrigés, contrat ARCH-UI-03 importé de main (9a07d76, sélectif) | (ce commit) | documentaire, aucun changement de code |
| 2026-11-27 | ARCH-UI-04 : Design System et trois espaces | 603abf8 | CI : build-test-package FAIL (suite wrapper interrompue : composants détruits pendant la navigation, corrigé en dafa6be) ; RTSan, LB, manuel SUCCESS |
| 2026-11-27 | ARCH-UI-04 : suite UI04 (20 critères), correctifs | 16e4559 | Local : 20/20 PASS |
| 2026-11-27 | ARCH-UI-04 : cycle de vie des composants (diagnostic ASan) | dafa6be | Local : wrapper 53 critères (AUT-02 seul, documenté), ASan 0 erreur |
| 2026-11-27 | ARCH-UI-04 : version 0.4.0, livraison, cartographie, captures, rapport | 8a61b12 | Local : barrière PASS (cœur 8 FAIL + wrapper 1 FAIL documentés, identiques à 0.3.3) |
| 2026-11-28 | ARCH-UI-04 : CI | dafa6be, 8a61b12 | run 47 SUCCESS ; **run 48 (46078289226) SUCCESS complet, SHA livré 8a61b124abc45449174dbd1f07e4b4f95f47a38e, 0.4.0-b48** (pluginval 10 avec UI, RTSan, paquets, LB, manuel) |
| 2026-11-28 | ARCH-UI-04-R1 : tests de reproduction R-28 avant correction | 7d57d94 | Local : 15 reproductions FAIL sur 0.4.0, non-régressions PASS ; CI : échec attendu de la barrière |
| 2026-11-28 | ARCH-UI-04-R1 : correction TRANSPORT AUTHORIZATION, 0.4.1, contrat, rapport | d2d08ac | Local : cœur 184 critères (9 FAIL documentés dont TA-15 remplacé), wrapper 57 (AUT-02 seul), barrière PASS, ASan 0 erreur |
| 2026-11-28 | ARCH-UI-04-R1 : CI | 7d57d94, d2d08ac | run 49 ÉCHEC attendu (reproductions sur macOS) ; **run 50 (46105804592) SUCCESS complet, SHA livré d2d08ac1bb6ed09ee3ea1b5e5d5d409c41b586af, 0.4.1-b50** (pluginval 10 avec UI, RTSan 0 violation, paquets, LB, manuel) |
| 2026-11-28 | ARCH-UI-04-R1 : clôture documentaire (contre-audit physique Codex) | 6286b49 | R-28 PASS physique (périmètre Reason 14 exécuté) ; GO technique avec réserves ; aucun changement de code ni de version |
| 2026-11-28 | ARCH-UI-04-DA : revue artistique et ergonomique indépendante (Codex) | dbc21c8 | documentaire ; 6,7/10 descriptif ; aucune modification de code |
| 2026-11-28 | ARCH-UI-04-R2-A : référentiel Design & Ergonomics (`docs/ARCH-UI-04-R2/`) | 04dd7cf | documentaire ; `git diff d2d08ac -- source/` vide ; aucun build, aucun test modifié ; GO documentaire, R2-B non autorisé |
| 2026-11-28 | ARCH-UI-04-R2-B1 : Component Lab isolé, captures et mesures Windows | a8a172b | aucun fichier de production modifié ; component-lab run 1 SUCCESS (macOS, résultats versés en 3aab1bd) |
| 2026-11-28 | ARCH-UI-04-R2-B1 : identifiants OD-01, variante PANIC, revue P1 | ab06b8c | component-lab run 2 (macOS) |
| 2026-11-28 | ARCH-UI-04-R2-B1 : documentation, arbitrages OD, corrections C01–C06 | 1530687 | documentaire ; `git diff d2d08ac -- source/` vide |
| 2026-11-28 | ARCH-UI-04-R2-B1 : résultats macOS (component-lab run 3) | 786c6ad | SHA des preuves finales B1 ; CI de production 51 et 53 SUCCESS |
| 2026-11-28 | ARCH-UI-04-R2-B1-R1 : raffinement artistique final (LAB-A01..A08), rendus et mesures Windows | e896dba | aucun fichier de production modifié |
| 2026-11-28 | ARCH-UI-04-R2-B1-R1 : résultats macOS (component-lab run 4) | 7c68a27 | commit du workflow |
| 2026-11-28 | ARCH-UI-04-R2-B1-R1 : planche 07 sans chevauchement ; planche-contact Windows (erratum B1) | 50cc8aa | laboratoire seulement |
| 2026-11-28 | ARCH-UI-04-R2-B1-R1 : documentation, validation P1, référentiel, tracker, rendus Windows | 7cbfa43 | CI de production 54 et 55 SUCCESS |
| 2026-11-28 | ARCH-UI-04-R2-B1-R1 : résultats macOS finals (component-lab run 5) | 17da3e6 | commit du workflow |
| 2026-11-28 | ARCH-UI-04-R2-B1-R1 : résultats CI consignés | 73b633b | documentaire, `[skip ci]` |
| 2026-11-28 | Décision du propriétaire : AR-15 PASS (option A), B1-R1 GO FINAL, Design System R1 gelé | e45ba7d | décision humaine consignée sans réécriture des preuves |
| 2026-11-28 | ARCH-UI-04-R2-B2-00 : audit préalable et contractualisation (dossier pour contre-expertise) | e45ba7d | documentaire ; aucune intégration de production ; `git diff d2d08ac -- source/` vide |
| 2026-11-28 | ARCH-UI-04-R2-B2-00 GO : addendum (DB2-01 à DB2-07, CE-B2-01 à CE-B2-05, GS définitifs, PL-06 amendé) | 9e0d5a9 | documentaire, `[skip ci]` ; R2-B2 exécution autorisée |
| 2026-11-28 | ARCH-UI-04-R2-B2-01 : banc de reproduction (18 scénarios permanents), workflow de preuves macOS, preuves Windows | b1502c8 | aucun fichier `source/`, `plugin/`, `ui/` modifié ; barrière PASS (75 critères, AUT-02 connu) ; CI de production 56 SUCCESS |
| 2026-11-28 | ARCH-UI-04-R2-B2-01 : preuves macOS (r2b2-evidence run 1) | 43ed70e | statuts identiques à Windows ; barrière PASS |
| 2026-11-28 | ARCH-UI-04-R2-B2-02 : banc GC-01, pair de test commun, ligne de base 0.4.1 (Windows), workflow GC-01 macOS | ad180b9 | aucun fichier `ui/` modifié ; CI de production 57 SUCCESS ; macOS run 2 (banc v1) versé en f2bb83e |
| 2026-11-28 | ARCH-UI-04-R2-B2-02 : banc GC-01 v2 (dépassement de traitement, autorelease pools), ligne de base Windows v2 | 44fc225 | aucun fichier `ui/` modifié ; macOS run 3 versé en d97e633 |
| 2026-11-28 | ARCH-UI-04-R2-B2-02 : banc GC-01 v3 (série mémoire sur 200 cycles), ligne de base Windows v3 | 0517426 | aucun fichier `ui/` modifié : **arbre de référence GC-01** ; ligne de base macOS run 4 versée en 0ddff52 |
| 2026-11-28 | CI de production 59 sur 0517426 : FAILURE du seul job manual-playwright (`test_s01_platform_unchanged` : `URLError [Errno 104] Connection reset by peer` en téléchargeant une page externe) ; build, tests, pluginval, RTSan, script local : SUCCESS ; aucun lien avec le commit (manual/ non modifié) | — | incident réseau consigné, non masqué |
| 2026-11-28 | ARCH-UI-04-R2-B2-02 : valeurs de référence GC-01 consignées (GC01/README) | 3f1c1f1 | documentaire ; **aucune CI déclenchée** : le message de commit citait le jeton de saut de CI, détecté par GitHub même dans une phrase (erreur de l'agent) |
| 2026-11-28 | Relance de la CI de production sur le code de référence inchangé (correction de l'erreur précédente) | c74745d | CI de production 60 **SUCCESS** (4 jobs, manual-playwright compris) : l'échec de la 59 était une défaillance externe |
| 2026-11-28 | DB2-08 validé (option A) ; PB-02a/d reclassés défauts fonctionnels majeurs (R2-C / R2-D, avant pré-BETA) ; DB2-09 préparé | 1ceb2d1 | documentaire |
| 2026-11-28 | ARCH-UI-04-R2-B2-02 (a) : module partagé `ui/r2/` (R2Style, R2Painters, R2Hud), laboratoire recâblé sur les peintres partagés, comparaison aux références R1 historiques (Windows 15/15 identiques, images entières), `component-lab.yml` ne réécrit plus les références | (ce commit) | `ui/` de production inchangé hors `ui/r2/` (compilé, non encore utilisé) ; `source/`, `plugin/` intacts |
| 2026-11-28 | ARCH-UI-04-R2-B2-02 (b) : registre d'identifiants (OD-01, ID-UNIQ), table des libellés générée (LBL-TABLE), habillage R1 des widgets JUCE génériques (DB2-06), captures de production | 25702f5 | barrière PASS (77 critères, AUT-02 connu) ; CI de production 63 SUCCESS |
| 2026-11-28 | ARCH-UI-04-R2-B2-02 : versement des preuves robuste ; GC-01 après fondation (run 7) ; **verdict B2-02 : GO technique** (GS-04 / GS-07 non concluants sur VM, physique M3) | 42a605e, (ce commit) | `B2-PRODUCTION-INTEGRATION/B2-02-REPORT.md` |

## 6. Prochaine action
ARCH-UI-04-R1 clôturé (GO technique avec réserves, 0.4.1, SHA `d2d08ac`). ARCH-UI-04-R2-A validé sur le plan
documentaire. ARCH-UI-04-R2-B1 : GO technique (preuves finales `786c6ad`). ARCH-UI-04-R2-B1-R1 : **GO FINAL** (AR-15 PASS,
décision du propriétaire du 2026-11-28 ; Design System R1 gelé). ARCH-UI-04-R2-B2 autorisé : **R2-B2-00** (audit et
contractualisation) publié dans `docs/ARCH-UI-04-R2/B2-PRODUCTION-INTEGRATION/`. En attente : contre-expertise de
ChatGPT sur R2-B2-00 : **obtenue** ; R2-B2-00 **GO**, DB2-01 à DB2-07 et CE-B2-01 à CE-B2-05 validés
(`B2-PRODUCTION-INTEGRATION/B2-00-ADDENDUM.md`). R2-B2 en exécution. **DB2-08 validé** (zone active PLASMA, B2-03 (2)) ; **DB2-09** (EC-19) à arbitrer avant clôture ;
PB-02a/d **défauts fonctionnels majeurs**, corrections R2-C / R2-D obligatoires avant pré-BETA. **B2-01** : PB-01 (double-clic) NON REPRODUIT ;
PB-01e (molette puis clic) REPRODUIT, correction en B2-03 (1) ; PB-02a/d REPRODUITS (Undo de session bloqué), corrections
R2-C / R2-D ; nouveau défaut PLASMA (saut au clic hors canal) soumis : **DB2-08** (STOP ciblé, B2-03 (2) seulement).
Suite : B2-02 (ligne de base GC-01, fondation).
R2-C, R2-D, R2-E, ARCH-UI-05 et ARCH-UI-06 non autorisés.
Recette humaine ARCH-UI-04 (`docs/V1-ARCH-UI-04-RECETTE.md`, H-09) toujours ouverte.

### Indicateurs d'avancement (2026-11-28)
- Exigences P0/P1 (pondération §4, automatique) : 47 PASS, 3 IMPLEMENTED (R19, R45, R50), 0 FAIL/BLOCKED → **97 %**
  (48,5/50). Ce taux mesure la couverture automatique, pas la recette physique.
- Couverture physique (Mac Studio M4 Max, Reason 14) : R-26 GO (0.3.3) ; R-28 PASS dans le périmètre exécuté (0.4.1) ;
  contrôles physiques d'interface partiels (automation WT X, isolation de deux instances, éditeur pendant
  l'enregistrement, PANIC en lecture). Non exécutés : R-29, R-30, PANIC et sauvegarde pendant un export, recette
  humaine et validation artistique ARCH-UI-04. Ableton Live : non certifié. M3 : non certifié.

### Backlog pré-BETA
- **PB-02a / PB-02d (EC-18) : défauts fonctionnels majeurs** — fermeture de l'éditeur pendant un glisser du Core ou d'une poignée d'enveloppe : geste hôte laissé ouvert, Undo de la session bloqué. Correction obligatoire en R2-C (Core) et R2-D (enveloppe) avant toute validation pré-BETA ; preuves `docs/ARCH-UI-04-R2/B2-PRODUCTION-INTEGRATION/PB/`.

| ID | Observation | Origine | État |
|---|---|---|---|
| PB-01 | Double-clic sur un NEXUS : deux gestes hôte imbriqués possibles (début, début, fin, fin) | audit statique H-04 (ARCH-UI-04-R1 §8) | sans impact démontré ; non corrigé |
| PB-02 | SYMBIOTIC CORE et éditeur d'enveloppe : geste non refermé si l'éditeur est détruit pendant un glisser | audit statique H-04 ; non démontré physiquement | sans impact démontré ; non corrigé |

## 7. Blocages et arbitrages à grouper
| ID | Décision manquante | Effet |
|---|---|---|
| B-01 | Catalogue Z | LEVÉ (G4.3) : catalogue de huit transformations validé par le propriétaire, cinq implémentées ; arbitrages A-G43-01 (niveau de Tilt) et A-G43-02 (progressivité de Shift) |
| B-02 | Sémantique WT/FM balance (AUD-A05) | Levé par ARCH-G4.1 : balance audio post-Amp, centre = 0.2.0 (BAL-01) ; conflit éventuel avec Cross Audio Crossfade documenté |
| B-03 | Règles de retrigger/loop/sync tempo des enveloppes | Implémenté provisoirement (Voice/Note/Gate, Sync en noires) ; à confirmer |
| B-05 | (proposition SPEC-HR-01 en attente) Plages de gain des étages (Out filtres/destruction jusqu'à +24 dB, makeup Y1 +36 dB) : en cumul extrême, la garde +30 dBFS se déclenche (G-15 étendu, DST-09 FAIL, rapportés) | Décider : conserver les plages (garde = comportement voulu) ou les réduire (ex. +12 dB) |
| B-04 | Licence JUCE 9 (AGPLv3/commerciale), canal de diffusion conforme, Developer ID/notarisation | Aucune publication binaire ; paquets construits et testés en CI sans publication |
