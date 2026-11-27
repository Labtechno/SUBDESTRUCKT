# SUBDESTRUCKT — ARCH-G4 / ARCH-G4.1 / ARCH-G4.2 — Suivi d'avancement (fichier central de reprise)

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
| R10 | Tail/Drone/Panic | F | PASS (G4.2 : Panic suspend aussi les états autonomes (AUT-05)) | run 37 |
| R11 | WT banques immuables XY 64×16, Clamp/Wrap/Mirror, Linear/Stepped | WT | PASS (CI run 9) | — |
| R12 | WT morph Spectral | WT | PASS (G4.1 : WT-SM-01) | run 33 |
| R13 | WT interpolation Linear/Cubic, Clean/Raw | WT | PASS (CI run 9) | — |
| R14 | WT phase free/retrigger/fixed/random, sync hard/soft (interne/OP) | WT | PASS (CI run 9) | — |
| R15 | WT Character | WT | PASS (G4.1 : CHR-01) | run 33 |
| R16 | Génération procédurale, import/export WAV, format portable, édition COW | WT | PASS (G4.1 : WTA-01, WT-ED-01, WT-ED-02w (générateur, éditeur, .sdwt, UI Banques)) | run 33 |
| R17 | Z deux slots A→B/B→A, Z Master, profondeurs/courbes, fondamentale, gain | WT | PASS (CI run 9) | — |
| R18 | Z : 3 transformations attestées | WT | PASS (CI run 9) | — |
| R19 | Z : 5 transformations historiques | WT | BLOCKED | — |
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
| R40 | Limiteur musical optionnel, garde numérique | Q | PASS (G4.2 : ELP / limiteur / garde distincts et comptés séparément (ELP-07)) | run 37 |
| R41 | Benchmark CPU reproductible | Q | PASS (PERF-01 rapporté, sans seuil normatif) | — |
| R42 | UI Performance | UI | IMPLEMENTED (captures réelles ; tâches UX01..18 non mesurées) | — |
| R43 | UI Sound Design (WT/FM/CROSS central/Process/Mod) | UI | IMPLEMENTED | — |
| R44 | UI Engineering (réseaux, matrice, banques, état, diagnostics) | UI | IMPLEMENTED | — |
| R45 | Inspector, moniteurs (scope/spectre/XY/FM), zoom/accessibilité | UI | IMPLEMENTED (G4.1 : UI-01w PASS (navigation, tailles, Inspector, titres accessibles) ; contrastes/DPI/zoom par niveau non mesurés) | run 33 |
| R46 | Wrapper VST3 : 684 paramètres, état v2, MIDI étendu, latence dynamique | PLG | PASS (G4.2 : G-21w inchangé et PASS ; G-22w) | run 37 |
| R47 | MIDI Learn global séparé des presets | PLG | PASS (G4.2 : ML-01w..07w (affectation, remplacement, profil isolé, presets, multi-instance, conflits, RT) ; recette physique R-11 à faire) | run 37 |
| R48 | Packaging 0.2.0 testé (sans diffusion publique) | PKG | PASS (G4.2 : paquets 0.3.1 construits et recette VM (CI) ; NON publiés ; compilation locale LB-01..07) | run 37 |
| R49 | Manuel HTML édition G4 + assistant de recette | DOC | PASS (G4.2 : manuel 0.3.1 (comportements modifiés) : contrôles bloquants + Playwright) | run 37 |
| R50 | Captures créatives Freeze/Glitch (R2) | STATE | IMPLEMENTED (G4.2 : non-conformité DAW corrigée (AUT-01, 03..08, AUT-02 a/b, CAP-02w PASS) ; AUT-02 clause (c) FAIL documenté (S2 décroît par contrat), AUT-02c PASS : PASS soumis à l'arbitrage A-G42-04) | run 37 |

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

## 6. Prochaine action
Arbitrages groupés du rapport ARCH-G4.2 (§8) : D-LIC-1 (licence JUCE pour la compilation locale), SPEC-HR-01
(et ELP-01), A-G42-01 (D2 Feedback), A-G42-04 (AUT-02 c), B-01 (Z). Puis recette physique 0.3.1 sur Mac Studio
M4 Max (compilation locale sécurisée, chapitre 2 du manuel 0.3.1). ARCH-UI-04 non commencé (non autorisé).

## 7. Blocages et arbitrages à grouper
| ID | Décision manquante | Effet |
|---|---|---|
| B-01 | Cinq transformations Z historiques (P0-C28/C29) | R19 BLOCKED ; 3 transformations développées ; G4.2 : recherche négative, 6 propositions NOUVELLES non implémentées (`docs/V1-ARCH-G4.2-Z-PROPOSALS.md`) |
| B-02 | Sémantique WT/FM balance (AUD-A05) | Levé par ARCH-G4.1 : balance audio post-Amp, centre = 0.2.0 (BAL-01) ; conflit éventuel avec Cross Audio Crossfade documenté |
| B-03 | Règles de retrigger/loop/sync tempo des enveloppes | Implémenté provisoirement (Voice/Note/Gate, Sync en noires) ; à confirmer |
| B-05 | (proposition SPEC-HR-01 en attente) Plages de gain des étages (Out filtres/destruction jusqu'à +24 dB, makeup Y1 +36 dB) : en cumul extrême, la garde +30 dBFS se déclenche (G-15 étendu, DST-09 FAIL, rapportés) | Décider : conserver les plages (garde = comportement voulu) ou les réduire (ex. +12 dB) |
| B-04 | Licence JUCE 9 (AGPLv3/commerciale), canal de diffusion conforme, Developer ID/notarisation | Aucune publication binaire ; paquets construits et testés en CI sans publication |
