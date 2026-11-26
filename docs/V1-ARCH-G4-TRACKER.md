# SUBDESTRUCKT — ARCH-G4 — Suivi d'avancement (fichier central de reprise)

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
| DST-09b | (enregistré après l'échec de DST-09, avant sa mesure) mêmes 48 configurations mais gains de sortie des emplacements ≤ 0 dB et makeup Y1 = 0 dB : 0 NaN, garde jamais déclenchée, 0 allocation. DST-09 reste rapporté tel quel |

## 4. Matrice d'exigences P0/P1
Pondération : chaque ligne vaut 1 ; avancement fonctionnel = (IMPLEMENTED×0,5 + PASS×1) / lignes non BLOCKED.
| # | Exigence | Lot | Statut | CI |
|---|---|---|---|---|
| R01 | Registre stable extensible (C23–C27) | F | PASS (local) | — |
| R02 | État versionné v2 + migration 0.1.0-b5 | F | IMPLEMENTED | — |
| R03 | Transactions/ownership hors RT (topologie, matrice, banque, courbes) | F | IMPLEMENTED | — |
| R04 | Timeline MIDI sample-accurate, ordre C11 étendu (bend/CC/pression) | F | IMPLEMENTED | — |
| R05 | Mono/Legato, priorités Last/Low/High, sustain | F | IMPLEMENTED | — |
| R06 | Expressivité : bend Up/Down, pression, CC1/2/11/64, vélocité | F | IMPLEMENTED | — |
| R07 | Accordage A4, microtuning 12 classes | F | IMPLEMENTED | — |
| R08 | 5 enveloppes DAHDSR lin/exp, loop | F | IMPLEMENTED | — |
| R09 | 4 LFO (8 formes dont Custom), libre/tempo/phase ppq, retrigger | F | IMPLEMENTED | — |
| R10 | Tail/Drone/Panic | F | IMPLEMENTED | — |
| R11 | WT banques immuables XY 64×16, Clamp/Wrap/Mirror, Linear/Stepped | WT | IMPLEMENTED | — |
| R12 | WT morph Spectral | WT | TODO | — |
| R13 | WT interpolation Linear/Cubic, Clean/Raw | WT | IMPLEMENTED | — |
| R14 | WT phase free/retrigger/fixed/random, sync hard/soft (interne/OP) | WT | IMPLEMENTED | — |
| R15 | WT Character | WT | IMPLEMENTED | — |
| R16 | Génération procédurale, import/export WAV, format portable, édition COW | WT | TODO | — |
| R17 | Z deux slots A→B/B→A, Z Master, profondeurs/courbes, fondamentale, gain | WT | IMPLEMENTED | — |
| R18 | Z : 3 transformations attestées | WT | IMPLEMENTED | — |
| R19 | Z : 5 transformations historiques | WT | BLOCKED | — |
| R20 | FM 4 opérateurs, PM/LinFM/TZFM/ExpFM par destinataire | FM | IMPLEMENTED | — |
| R21 | 9 formes, ratio/fixed/offset/coarse/fine | FM | IMPLEMENTED | — |
| R22 | Niveau audio/pan ≠ modulation, mute ≠ disable, AC/DC, Character + Master | FM | IMPLEMENTED | — |
| R23 | Matrice 4×4 + 10 topologies préréglées | FM | IMPLEMENTED | — |
| R24 | ExpFM benchmark | FM | TODO | — |
| R25 | CROSS 55 routes B-T2 sticky, bus Raw/Normalized, directions | CR | IMPLEMENTED | — |
| R26 | Sync bidirectionnel, Cross Audio Mix/Crossfade/Ring/AM, Cross Reset | CR | IMPLEMENTED | — |
| R27 | Destruction N1/D1/Y1/S1 | DST | TODO | — |
| R28 | Destruction N2/D2/Y2/S2 | DST | TODO | — |
| R29 | 6 slots, chaînes locales/globale, dry/wet aligné, latence | DST | IN PROGRESS | — |
| R30 | Filtres F1/F2 | FLT | TODO | — |
| R31 | Filtres F3/F4 | FLT | TODO | — |
| R32 | Pre/Post, excitation, stéréo linked/dual | FLT | IN PROGRESS | — |
| R33 | Matrice globale 64, Via, courbes, rates, inactivité, macros | MOD | IMPLEMENTED | — |
| R34 | RNG PCG32/xoshiro benchmark, chaos 3 modèles | MOD | IMPLEMENTED | — |
| R35 | Suiveurs ×3, Y2 sources | MOD | IMPLEMENTED | — |
| R36 | Presets : bibliothèque, tags/favoris, import/export | STATE | TODO | — |
| R37 | Randomize/Mutate/Evolve (verrous, intensité, seed, profils) | STATE | TODO | — |
| R38 | Undo/Redo transactionnel (128, RNG) | STATE | TODO | — |
| R39 | Quality Eco/Normal/High, transition contrôlée, latence | Q | IMPLEMENTED | — |
| R40 | Limiteur musical optionnel, garde numérique | Q | IMPLEMENTED | — |
| R41 | Benchmark CPU reproductible | Q | TODO | — |
| R42 | UI Performance | UI | TODO | — |
| R43 | UI Sound Design (WT/FM/CROSS central/Process/Mod) | UI | TODO | — |
| R44 | UI Engineering (réseaux, matrice, banques, état, diagnostics) | UI | TODO | — |
| R45 | Inspector, moniteurs (scope/spectre/XY/FM), zoom/accessibilité | UI | TODO | — |
| R46 | Wrapper VST3 : 684 paramètres, état v2, MIDI étendu, latence dynamique | PLG | TODO | — |
| R47 | MIDI Learn global séparé des presets | PLG | TODO | — |
| R48 | Packaging 0.2.0 testé (sans diffusion publique) | PKG | TODO | — |
| R49 | Manuel HTML édition G4 + assistant de recette | DOC | TODO | — |
| R50 | Captures créatives Freeze/Glitch (R2) | STATE | TODO | — |

## 5. Journal
| Date | Lot | Commit | Résultat |
|---|---|---|---|
| 2026-11-26 | F1 registre/état/moteur généralisé | (ce commit) | Local MSVC : 25/25 critères PASS dont REG-01 bit à bit (20/20) ; CI non exécutée |

## 6. Prochaine action
Tests G4 des fonctions implémentées (ST, MIDI, ENV, LFO, MOD, WT, FM, CR, Q, RNG, CHAOS), puis filtres, destruction,
wrapper, workflow CI ARCH-G4.

## 7. Blocages et arbitrages à grouper
| ID | Décision manquante | Effet |
|---|---|---|
| B-01 | Cinq transformations Z historiques (P0-C28/C29) | R19 BLOCKED ; 3 transformations développées |
| B-02 | Sémantique WT/FM balance de la façade Performance (AUD-A05) | Commande non exposée tant que non arbitrée |
| B-03 | Règles de retrigger/loop/sync tempo des enveloppes « à préciser » | Implémenté : free + loop A-H-D ; sync tempo des enveloppes non implémentée |
| B-04 | Licence JUCE 9 (AGPLv3/commerciale), canal de diffusion conforme, Developer ID/notarisation | Aucune publication binaire ; paquets construits et testés en CI sans publication |
