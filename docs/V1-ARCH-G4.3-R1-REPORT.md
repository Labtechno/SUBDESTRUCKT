# SUBDESTRUCKT — ARCH-G4.3-R1 — DSP FINAL CORRECTION — Rapport final

Branche unique `feature/arch-g4-full-development` (aucun merge vers `main`, toujours `9a07d76`). Base vérifiée :
`cbca78db9c1b60a0400c5df1e4c415d000548ca1` (ARCH-G4.3 final, CI run 43 SUCCESS). Version interne **0.3.3** (sans collision),
format d'état 4 inchangé, 703 StableID inchangés. Critères écrits avant mesure : `docs/V1-ARCH-G4.3-R1-SPEC.md` (commit
58bf858, antérieur à toute mesure et à toute modification du code). ARCH-UI-04 : non commencé.

## 1. Verdicts

| Verdict | Statut | Motif |
|---|---|---|
| **A. Correction DSP automatisée** | **PASS** | Harmonic Tilt corrigé (HT-01..08 PASS, plus aucun engagement ELP dans les scénarios historiques) ; Harmonic Shift : deux défauts réels de continuité démontrés puis corrigés (ZC-05R PASS) ; Phase Scatter clos (ZC-13R PASS) ; non-régression complète ; CI macOS run 44 verte (§14). Réserve documentée : R2-02R (a) FAIL (source Metal, résolution du gel S2), énergie toutes bandes conservée |
| **B. Recette privée sur Mac physique** | **GO conditionnel** | Conditions : acceptation de la licence JUCE Starter par le propriétaire lui-même (éligibilité à confirmer ; aucune acceptation par l'agent) ; compilation locale au SHA livré |
| **C. DSP FINAL LOCK définitif** | **EN ATTENTE** | recette physique Reason 14 et Ableton Live (TRANSPORT AUTHORIZATION, exports R-26..R-28), écoute A/B R2 (R-30) et Z (R-29) ; décision du propriétaire sur la restriction d'ordre inter-domaines Z (§9) |

| Indicateur | Valeur | Calcul |
|---|---|---|
| Avancement fonctionnel | **94,0 %** (inchangé) | (44 PASS + 0,5 × 6 IMPLEMENTED) / 50 |
| Validation effective | **88,0 %** | 44 / 50 |
| R19 | IMPLEMENTED | catalogue complet, Tilt corrigé, mais restriction d'ordre inter-domaines non acceptée explicitement (§9) |
| R50 | IMPLEMENTED | écoute A/B et recette physique non validées ; R2-02R (a) FAIL |

## 2. SHA

Commits R1 : `58bf858` (spécification, avant mesure) ; `2985901` (correction DSP, tests, rendus de recette, 0.3.3) ;
`937ac6d` (manuel 0.3.3, audio rendu à `2985901`, contrats, procédure) ; `f1793ee` (identifiant de campagne conforme
au schéma) ; `0469c4d` (documentationSha). **SHA livré : `0469c4da6181eaa000bfc402b70b97e1dfef989b`**
— CI run 44 SUCCESS. Les commits ultérieurs ne touchent que ce rapport et le tracker.

## 3. Correction mathématique de Harmonic Tilt

`source/dsp/ZSpectral.h` (`tiltStage`), contrat `docs/V1-Z-TRANSFORMS-CONTRACT.md` §3 bis. Pour chaque étage Tilt, sur
D = {h ≥ 2} (Keep Fundamental) ou {h ≥ 1}, avec bₕ le gain de bande lue (profil mip / coupure / Raw) :

1. c′ₕ = h^(v·λ(σ))·cₕ (pondération inchangée : caractère sombre/brillant conservé) ;
2. s = √(Σ bₕ²|cₕ|² / Σ bₕ²|c′ₕ|²), c″ₕ = s·c′ₕ — **toujours**, indépendamment de Gain Compensation ; énergie d'entrée
   nulle ou négligeable (ε = 1e−24) : identité ;
3. contrôle de crête : si crête(c″) > κ·crête(c), κ = 2 (+6,02 dB), rangs de D × α = (κ·pᵢₙ − p₀)/(pₒᵤₜ − p₀),
   p₀ = amplitude de la fondamentale protégée (0 sinon). La crête du cycle étant convexe en α, la sécante garantit la
   borne en une seule évaluation ; aucune saturation, aucun écrêtage ;
4. Gain Compensation (autres transformations) vise l'énergie d'origine × Π α² : elle ne réintroduit jamais une crête
   retirée. Les crêtes sont mesurées par FFT inverse du cycle à bande lue (tampon préalloué, 2 FFT par étage).

## 4. Mesures avant / après (avant ELP, après le gain de sortie)

| Scénario | 0.3.2 (ARCH-G4.3) | 0.3.3 (R1) | ELP 0.3.3 |
|---|---|---|---|
| Protocole ZC-16, Metal | +40,9 dBFS | −2,38 dBFS | 0 |
| Protocole ZC-16, Basic | +46,8 dBFS | +10,50 dBFS | 0 |
| Compensation forcée (Metal) | +30,3 dBFS | −2,38 dBFS | 0 |
| Tilt brillant seul (v 1, σ 1, sans compensation) | +78,9 dBFS | −1,00 dBFS | 0 |
| Scénario isolé de référence (Metal, note 45) | — | −6,62 dBFS (Z off −7,68 dBFS : +1,06 dB) | 0 |

Le +10,5 dBFS du protocole Basic vient d'autres transformations aléatoirement combinées (sous le seuil ELP +24,08 dBFS).

## 5. Résultats HT-01 à HT-08

| ID | Résultat | Mesure |
|---|---|---|
| HT-01 | PASS | Depth 0 / Z Master 0, fondamentale 0/1, compensation 0/1, emplacements A/B : 8/8 bit à bit |
| HT-02 | PASS | 3 200 cas : \|Eₒᵤₜ/Eᵢₙ − 1\| ≤ 7,6e−15 sans réduction ; 171 cas avec réduction de crête : Eₒᵤₜ ≤ Eᵢₙ |
| HT-03 | PASS | ON : c₁ exact ; OFF : facteur commun (0 écart) |
| HT-04 | PASS | nul (sortie nulle exacte), 1e−150, sinus, scie, Metal, dense aléatoire, rang 1024, H = 1/2 : finis, aucune énergie créée |
| HT-05 | PASS | (a) crête ≤ 2 × entrée sur 3 200 cas (pire 2,0000) ; (b) +1,06 dB vs Z off, 0 ELP ; (c) §4 |
| HT-06 | PASS | Tilt + chacune des 7 autres et Tilt + Tilt, deux ordres, σ ±1 (32 configurations) : 0 ELP, crêtes dans l'enveloppe |
| HT-07 | PASS | Depth, Shape, Z Master (LFO 0,5 et 20 Hz), CROSS audio : sauts ≤ 0,26 (statique 0,34), fenêtres 5 ms ≤ ×1,22, 0 ELP |
| HT-08 | mesure | §12 |

## 6. Harmonic Shift — clôture prospective (A-G43-02)

ZC-05 reste FAIL historique ; il liste désormais aussi Tilt sur Saw à σ = 1 : la normalisation d'énergie déplace
l'énergie hors de la fenêtre de mesure h ≤ 64 (effet de la métrique, pas un défaut). **ZC-05R**, mesuré AVANT toute
modification de Shift : **FAIL** — (d) discontinuité quand le rang source franchit 1 (saut de c₁ à 0), (e) un
décalage de 1e−6 supprimait la fondamentale, et saut de 0,09 × |c| au point où la moyenne complexe de deux rangs de
phases opposées s'annule (dent de scie). **Correction minimale** (balayage, déplacements entiers et amplitudes inchangés) :
rangs virtuels nuls en 0 et 1025, phase interpolée sur l'arc le plus court. Après : **ZC-05R PASS** (saut résiduel
6,2e−9 × |c|, LFO 20 Hz : saut 0,46 vs 0,47 statique). Effet sonore : phases des déplacements fractionnaires modifiées
(spectre d'amplitude identique) ; exemples e29/e30 re-rendus.

## 7. Phase Scatter — clôture prospective (A-G43-03)

Algorithme inchangé. ZC-13 (c) FAIL historique conservé. **ZC-13R PASS** : déterministe et indépendant de la graine,
amplitudes inchangées (1,9e−15 dB), forme temporelle −1,3 dB (≥ −20), FM via CROSS −12,2 dB (≥ −30), après Fold 23
bandes et après N1 20 bandes (≥ 3), LFO 20 Hz conforme, neutralité exacte ; facteur de crête +0,49 dB (descriptif).

## 8. Captures R2 — complément (A-G43-04)

R2-02 FAIL historique conservé (correctif G4.3 conservé). **R2-02R** (24 couples Fc → Fr, 44,1 / 48 / 88,2 / 96 kHz, scie
et Metal) : (b) écart pondéré sur bandes résolubles pire 0,47 dB (≤ 0,5) PASS ; (c) bandes résolubles fortes pire
0,42 dB (≤ 1) PASS ; (e) Fr = Fc exact PASS ; **(a) énergie totale 200 Hz – 0,45 Fs : FAIL pour Metal** (9 couples, pire
3,37 dB ; scie : ≤ 0,79 dB). Diagnostic : la fondamentale Metal (165 Hz, hors plage d'analyse) s'étale sur la grille
grossière (43–47 Hz par bin) et déborde dans les bandes 250–315 Hz ; **énergie toutes bandes conservée à ±0,42 dB**
dans tous les couples. Bandes non résolubles listées (250 Hz : 1,2–1,3 bin, jusqu'à +8,5 dB ; 315 Hz à −42 dB). WAV A/B
(capture 48 kHz restituée à 44,1 / 48 / 88,2 / 96 kHz, même gain) : `docs/r1-audio/`. Validation d'écoute : R-30
(propriétaire). R50 reste IMPLEMENTED.

## 9. Audit des restrictions Z

**ZA-01 — cadence** : resynthèse ≤ 1 / 128 échantillons moteur (vérifiée par compteur), fondu linéaire sur la période
suivante :

| Fs | Eco / Normal | High |
|---|---|---|
| 44,1 kHz (K = 2) | 1,45 ms (689 Hz) ; réaction ≤ 2,90 ms | 0,73 ms (1 378 Hz) |
| 48 kHz | 2,67 ms (375 Hz) ; ≤ 5,33 ms | 1,33 ms (750 Hz) |
| 88,2 kHz | 1,45 ms (689 Hz) | 0,73 ms |
| 96 kHz | 1,33 ms (750 Hz) | 0,67 ms (1 500 Hz) |

Échelon de profondeur (Mask, 48 kHz, fenêtre d'une période) : 90 % du changement d'énergie en 11 ms, dominé par le
lissage du paramètre (rampe de 20 ms) plus ≤ 5,3 ms de resynthèse. Modulation sinusoïdale de la profondeur (bandes
latérales h1..5 relatives aux raies) : Mask −15,0 / −15,5 / −25,5 dB à 10 / 50 / 200 Hz ; Fold +0,2 / +0,2 / −1,0 dB.
Les bandes latérales de Mask chutent de 10 dB au-delà de la demi-cadence (187 Hz à 48 kHz) alors que celles de Fold
(par échantillon) restent stables : la modulation rapide des transformations spectrales est atténuée et repliée. Rendus : `docs/r1-audio/za01_cross_audio_tilt.wav` / `_fold.wav` (même modulation
audio CROSS, même gain). Conséquence sonore : les transformations spectrales répondent aux modulations lentes et
moyennes, pas aux modulations audio (pas de « FM de timbre »).

**ZA-02 — ordre A/B** (matrice mesurée, A>B vs B>A) :

```
B →      Fold Warp Mut  Tilt Mask Foc  Shift Scat
Fold      D    =    =    =    =    =    =    =
Warp      =    D    D    =    =    =    =    =
Mut       =    D    D    =    =    =    =    =
Tilt      =    =    =    =    D    D    D    =
Mask      =    =    =    L    =    =    D    =
Focus     =    =    =    D    =    =    D    =
Shift     =    =    =    D    D    D    D    D
Scatter   =    =    =    =    =    =    D    =
```
`=` identique dans les deux ordres, `L` niveau seul, `D` différent. Toutes les paires inter-domaines sont `=` : l'ordre
y est fixe (spectral → phase → lecture → Fold). Le contrat P0-C30 (« ordre inversable ») n'est donc tenu qu'entre
transformations d'un même domaine : **restriction non validée par le propriétaire** (pistes et coûts :
`docs/V1-Z-TRANSFORMS-CONTRACT.md` §3 ter). Paires commutatives par nature : Tilt/Tilt, Mask/Mask, Focus/Mask,
Scatter/(Tilt, Mask, Focus).

## 10. TRANSPORT AUTHORIZATION — non-régression

Mécanisme inchangé. TA moteur 24/24, TA-xxw 9/9, AUT-01..08 inchangés, purge Panic, aucune persistance, captures hors
RT, autorisation indépendante de l'éditeur. Limites (contrat `docs/V1-AUTONOMY-CONTRACT.md` §5) : hôte sans
traitement à l'arrêt ; premier bloc en LECTURE ; export sans information de transport ; duplication/restauration pendant
la lecture. Autoriser n'est **pas** une réponse définitive à l'exigence d'export sans intervention : la validation
physique Reason 14 / Ableton Live (R-26..R-28) est requise.

## 11. Compatibilité et migrations

703 StableID et identifiants VST3 inchangés (REG-ID, G-22w) ; format d'état 4 ; états v1–v3 lus ; D2 Legacy/
Regenerative inchangés ; empreintes 0.3.1 (ZC-03, D2R-03) identiques. **Changement volontaire** : rendu de Harmonic
Tilt (niveau) et de Harmonic Shift (phase des déplacements fractionnaires) par rapport à 0.3.2 ; aucun preset d'usine
n'utilise Tilt ni Shift. Exemples du manuel : les 40 fichiers 0.3.3 sont rendus par `subdestruckt_manual_render`
au commit de code `2985901` (SHA inscrit dans le manifeste audio) ; comparés octet par octet à 0.3.2, **3 diffèrent** :
e24 (Tilt brillant), e29, e30 (Shift) ; e23 (Tilt sombre, Gain Compensation par défaut activée) est identique après la normalisation propre à chaque exemple (gainDb) et la
quantification 24 bits : la compensation conservait déjà l'énergie en 0.3.2 et la garde de crête n'intervient pas ;
les 36 autres sont identiques. Aucun mode Legacy pour l'ancien Tilt (défaut corrigé).

## 12. CPU et mémoire (Windows MSVC, même machine ; ne certifie ni M3 ni M4 Max)

| Scénario (ns/échantillon P50/P95) | 0.3.2 (`cbca78d` reconstruit) | 0.3.3 |
|---|---|---|
| Z off | 276/284 | 290/302 |
| Shift + Tilt statique | 262/270 | 265/273 |
| Shift + Tilt, profondeur modulée | 870/899 | **1 342/1 412 (+54 %)** |
| Tilt seul, profondeur modulée | — | 862/901 |

Écart significatif documenté : deux FFT inverses de 2048 points par étage Tilt et par resynthèse (mesure de crête), au
plus toutes les 128 échantillons ; aucune augmentation de la cadence. Mémoire : +32 Ko par instance (tampon de crête).

## 13. Matrice R01–R50

Inchangée depuis ARCH-G4.3 : 44 PASS, 6 IMPLEMENTED (R19, R42–R45, R50) → 94,0 % ; validation 88,0 %. R19 reste
IMPLEMENTED (restriction d'ordre inter-domaines non acceptée ; recette d'écoute R-29) ; R50 IMPLEMENTED (R2-02R (a),
R-30, recette physique).

## 14. CI

Run 44 (https://github.com/Labtechno/SUBDESTRUCKT/actions/runs/46038461205), SHA `0469c4d`, **SUCCESS** sur les
4 jobs : `build-test-package` (macOS arm64 : cœur et wrapper complets, barrière exhaustive PASS — seuls les échecs
connus documentés, dont le nouveau R2-02R et le critère remplacé ZC-15 —, golden arm64, bundle 0.3.3-b44 arm64 signé
ad hoc, pluginval v1.0.4 rigueur 10 aux 4 fréquences × 5 tailles de bloc PASS, hôte VST3, paquets, bac à sable
d'installation, livraison réextraite et vérifiée, artefacts limités aux journaux) ; `RealtimeSanitizer` (G-18 0
allocation, G-18rt 0 violation, contrôle négatif détecté) ; `local-build-script` (LB-01..13 PASS) ; `manual-playwright`
(éditions 0.1.0-b5 à 0.3.3 : 0.3.3 46 passés, 4 ignorés). Localement (Windows MSVC) : cœur 169 critères, 16 contrôles en
échec tous rattachés aux échecs connus, barrière PASS ; wrapper 33 critères (AUT-02 seul) ; LB 13/13 ; Playwright
0.3.2 et 0.3.3 verts.

## 15. Compilation et installation

`docs/V1-LOCAL-BUILD.md` (inchangé, version 0.3.3) : dépôt propre au SHA livré, `unset SBD_ONLY`,
`tools/local-build/build_local_macos.sh --juce-licence=<votre choix>`, ouverture manuelle de `SUBDESTRUCKT_Installer.pkg`,
désinstallation par `SUBDESTRUCKT_Uninstaller.pkg`. LB-01..13 PASS (harnais local et CI Linux).

## 16. Protocole Reason 14 et Ableton Live

Manuel 0.3.3 : R-26 (Reason 14), R-27 (Ableton Live), R-28 (exports), R-29 (écoute Z : niveau de Tilt avec et sans
compensation, continuité de Shift), R-30 (écoute A/B des captures R2 entre fréquences). Journal de recette :
`ta_diagnostics.on` → `ta_log.txt`.

## 17. Réserves résiduelles

1. Recette physique (TRANSPORT AUTHORIZATION, exports) non exécutée.
2. Restriction d'ordre inter-domaines Z (§9) : décision du propriétaire requise.
3. Cadence de resynthèse spectrale (§9) : limitation documentée, non arbitrée.
4. R2-02R (a) : résolution du gel S2 aux fréquences élevées (graves étalés) ; écoute R-30.
5. Coût des modulations de Tilt (+54 % sur Shift + Tilt modulé).
6. Échecs historiques conservés : ELP-01, D2-04, D2-04c, AUT-02 (c), R2-02, ZC-05, ZC-13 (c) ; critère remplacé :
   ZC-15 (clause Tilt) par HT-02/HT-03 ; nouvel échec documenté : R2-02R (a).

## 18. Confirmations

Une branche, aucun sous-agent ni worktree parallèle, **aucune fusion vers `main`** ; aucune licence acceptée, aucun
compte ; **aucun binaire ni paquet publié** (artefacts CI : journaux) ; manuel 0.3.3 non publié sur gh-pages ; aucune
dépense ; ARCH-UI-04 non commencé.
