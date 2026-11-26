# SUBDESTRUCKT — ARCH-G4 — Rapport consolidé (développement séquentiel accéléré)

Date : 2026-11-26. Branche `feature/arch-g4-full-development` (base `8d23342` + docs `main` `58a8022`), aucun merge vers
`main`, aucune diffusion binaire. Suivi détaillé, décisions et seuils préenregistrés : `docs/V1-ARCH-G4-TRACKER.md`.

## 1. Synthèse
- **Avancement fonctionnel : 85,7 %** = (37 PASS + 0,5 × 10 IMPLEMENTED) / 49 exigences non bloquées de la matrice R01–R50
  (tracker §4) ; 2 TODO (R12 morph spectral, R50 captures créatives R2), 1 BLOCKED (R19, cinq transformations Z).
- **Préparation à une diffusion publique : non prête (0/6 conditions)** — licence JUCE non arbitrée, pas de Developer
  ID ni de notarisation, recette physique M4 Max non faite, M3 physique non testé, tâches UX non mesurées, graphisme
  non validé. Distinct de l'avancement fonctionnel.
- **Dernier run CI complet : run 9** (`f640c5c`), <https://github.com/Labtechno/SUBDESTRUCKT/actions/runs/45898414191> :
  build arm64, cœur, wrapper, hôte VST3, pluginval 1.0.4 rigueur 10 (UI incluse, 4 Fs × blocs 32–512), bundle
  `0.2.0-b9`, paquets et recette VM, RTSan : **SUCCESS** ; 65 critères rapportés, 63 PASS, 2 FAIL documentés (§4).
  Runs 13/14 (banques utilisateur, textes 0.2.0, manuel) en cours à la rédaction.

## 2. Fonctionnalités
**Terminées et testées (PASS)** : registre 684 paramètres append-only (22 StableID v1 inchangés) ; état v2 + migration
v1 (rendu bit à bit = 0.1.0-b5) ; transactions/publication lock-free ; MIDI sample-accurate (bend Up/Down, CC, pression,
sustain, ordre C11) ; Mono/Legato et priorités ; 5 enveloppes DAHDSR lin/exp/Loop ; 4 LFO (8 formes, tempo, ppq) ;
Tail/Drone/Panic ; WT XY 64×16 (Clamp/Wrap/Mirror, Linear/Stepped, Clean/Raw, Linear/Cubic, phases, sync) ; Z deux
emplacements (Harmonic Fold ADAA, Phase Warp, Spectral Mutation) ; FM 4 opérateurs PM/LinFM/TZFM/ExpFM, 9 formes,
ratio/fixe/offset, mute ≠ disable, AC/DC ; CROSS 55 routes B-T2 sticky, bus Raw/Normalized, directions, Cross Audio,
Cross Reset ; Filter F1–F4 ; Destruction N1–S2 (6 emplacements, Pre/Post, latence alignée) ; matrice 64 (Via, courbes,
16 audio-rate, inactivité motivée, macros) ; RNG PCG32 / chaos Lorenz retenus après benchmark ; presets usine et
utilisateur ; Randomize/Mutate/Evolve ; Undo/Redo 128 ; Quality ; limiteur musical ; benchmark CPU ; wrapper VST3 ;
paquets 0.2.0 ; manuel 0.2.0.
**Implémentées, recette à faire (IMPLEMENTED)** : accordage/microtuning, Character WT, topologies FM, suiveurs/Y2,
import/export WAV des banques (WTA-01 PASS local), UI Performance / Sound Design / Engineering + Inspector + moniteurs,
zoom/accessibilité de base, MIDI Learn global.
**Non implémentées** : morph spectral (R12), captures créatives Freeze/Glitch persistées (R50), éditeur graphique et
générateur de formes dans l'UI (partie de R16), sync tempo des enveloppes (règle non arbitrée).
**Bloquées** : cinq transformations Z historiques (R19) — non inventées.

## 3. Tests et non-régressions
- 70 critères du cœur (G-xx, REG, ST, MIDI, ENV, LFO, MOD, WT, FM, CR, Q, RNG, CHAOS, FLT, DST, ST-RND, UNDO, EVO,
  PRE, WTA, PERF) + 6 wrapper + 5 hôte + pluginval + 8 paquets.
- **Non-régression 0.1.0-b5** : oracle figé `tests/regression/b5` ; REG-01 20/20 rendus **identiques bit à bit**
  (5 scénarios × 4 Fs) ; ST-02 : 40 états v1 du codec b5 migrés, rendu bit à bit identique ; G-15b (espace v1) PASS.
- Allocations RT : 0 (G-18, RT-G4, G-18w) ; RTSan : PASS (run 9).
- Seuils jamais abaissés ; un amendement consigné **avant mesure** (DST-01 : référence retardée d'un demi-échantillon,
  délai inhérent à l'ADAA1). Critères complémentaires ajoutés après un échec, rapportés séparément (G-15b/c, DST-09b).

## 4. Échecs et limitations mesurés
| ID | Résultat | Cause | Suite |
|---|---|---|---|
| G-15 (étendu) | FAIL 1/200 | garde +30 dBFS (9 éch.) avec gains cumulés extrêmes, 0 NaN | arbitrage B-05 ; G-15b/c PASS |
| DST-09 | FAIL 4/48 | idem (Out +24 dB × 6, makeup +36 dB) | arbitrage B-05 ; DST-09b PASS |
| DST-01 (comparaison) | rapporté | repli N1 extrême (drive 24, fold 1) : naïf −0,7 dB, ADAA1 −2,5 dB, ADAA1+K×2 −6,1 dB d'énergie repliée ; aucun ≤ −40 dB | ADAA1 retenu, Quality High recommandé ; OS interne N1 = dette |
| G-21 hôte (run 1) | FAIL intermittent | écho normalisé de l'hôte ; corrigé (pont tolérant), PASS runs 2–9 | — |
Autres limitations : aucune promesse click-free ; MIDI Learn appliqué à ~33 ms ; perfs non certifiées M3/M4 ;
graphisme provisoire ; tests Playwright du manuel non exécutés ; rendus audio du manuel produits sous Windows (MSVC).

## 5. Performances mesurées (PERF-01/02, sans seuil, PC de développement Windows, mono-instance, un cœur)
| Fs / Quality | Init (b5) | Stress maximal (5 osc, 55 routes, 64 mod dont 16 audio, 6 FX dont S2 4096, 3 filtres dont F4 16 modes) |
|---|---|---|
| 44,1 k Normal / High | 0,029 / 0,055 | 0,159 / 0,313 |
| 48 k Normal / High | 0,014 / 0,031 | 0,085 / 0,173 |
| 96 k Normal / High | 0,028 / 0,061 | 0,176 / 0,343 |
Facteur temps réel (durée de calcul / durée audio). ExpFM : 0,0165 vs PM 0,0163, Lin 0,0158, TZ 0,0158 (48 kHz,
16 routes). Le run CI exécute le même benchmark sur VM arm64 (journal `core_tests.log`). Aucune certification M3.

## 6. Livrables
- Code : 22 commits sur la branche (de `891885f` à la tête), dont `079e94e` (fondations), `110c490` (filtres), lot
  destruction, `ef3c458` (état), `764ce4f` (UI), `f640c5c` (benchmark), `77f589b` (banques), `573b60a` (manuel).
- Paquets : `SUBDESTRUCKT_v0.2.0-b9_macOS_arm64.zip` construit et testé en CI (Installer/Uninstaller, README_FR,
  CHANGELOG, SHA256SUMS) — **non publié** (B-04) ; l'artefact CI ne contient que des journaux.
- Manuel 0.2.0 : <https://labtechno.github.io/SUBDESTRUCKT/manual/0.2.0/> — assistant de recette R-01..R-18
  (sauvegarde locale, export JSON `SUBDESTRUCKT_ARCH_G4_RESULTS.json`, iPhone) :
  <https://labtechno.github.io/SUBDESTRUCKT/manual/0.2.0/recette.html> ; édition 0.1.0-b5 et plateforme S0.1 inchangées.
- Captures réelles de l'UI : `manual/src/0.2.0/assets/ui_*.png` (test automatisé de l'éditeur, rendu Windows) et
  `out/ui_snapshot*.png` des runs CI macOS.

## 7. Arbitrages restant nécessaires (liste unique)
1. **B-04 Licence et canal** : AGPLv3 ou licence commerciale JUCE 9 ; canal privé de remise de la build 0.2.0-b9 pour
   la recette ; Developer ID/notarisation (aucun engagement financier pris).
2. **B-01 Transformations Z** : retrouver le catalogue historique ou ouvrir un QCM Z dédié (5 transformations).
3. **B-05 Plages de gain** des étages (+24 dB Out, +36 dB makeup) : conserver (garde = comportement voulu) ou réduire.
4. **B-02 Balance WT/FM** de la façade Performance : fonction précise à piloter.
5. **B-03 Enveloppes** : règles de retrigger/sync tempo.
6. **Budget CPU produit** et seuils M3 (aucun seuil normatif ; PERF rapporté).
7. **Graphisme et tâches UX** : planifier les tests UX01–UX18 avant le gel graphique.
8. Recette physique M4 Max (assistant R-01..R-18), puis M3 physique avant toute Beta.
