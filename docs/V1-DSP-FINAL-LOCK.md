# SUBDESTRUCKT — DSP FINAL LOCK (0.3.3) — consignation avant ARCH-UI-04

Date : 2026-11-27. Décision du propriétaire : **DSP FINAL LOCK prononcé avec acceptation des risques résiduels**.
Ce document consigne les faits déclarés par le propriétaire et l'état des tests ; il ne déclare PASS aucun essai non
exécuté. Il précède toute modification UI d'ARCH-UI-04 et n'autorise aucune modification du DSP.

## 1. Référence verrouillée

| Élément | Valeur |
|---|---|
| Version | 0.3.3 (ARCH-G4.3-R1) |
| SHA source testé physiquement | `0469c4da6181eaa000bfc402b70b97e1dfef989b` |
| Build | 263311915 |
| Machine | Mac Studio M4 Max, macOS 26.5.2 |
| Hôte | Reason 14.1d100 (version 20 457), 44,1 kHz |
| HEAD de la branche au verrouillage | `004a98b88ce83254dddebca43716e80933323d84` (documentaire seul après 0469c4d) |
| CI de référence | run 44 (0469c4d) et run 45 (004a98b) SUCCESS, macOS 15 arm64 |

Périmètre verrouillé : `source/dsp/**`, `source/engine/**`, `source/core/**` (registre, codec d'état, topologie,
matrice, presets, actifs), causalité B-T2, TRANSPORT AUTHORIZATION, comportement des captures R2, ordre effectif des
transformations Z, liste et ordre des 703 StableID, format d'état version 4. Toute modification de ce périmètre exige
un rapport distinct et une autorisation préalable du propriétaire.

## 2. Recette physique 0.3.3

| ID | Statut | Détail |
|---|---|---|
| R-26 Reason 14 | **GO** (déclaré par le propriétaire) | 14 critères fonctionnels validés ; journal technique fourni par le propriétaire (non versionné dans le dépôt) |
| R-26 — périmètre | validé | sept instances simultanées ; Drone, D2 Legacy, S2, D2 Regenerative, Panic, sauvegarde, restauration, duplication, transport, coexistence, dans le périmètre testé |
| R-26 — réserve D2 Repeat | **levée** | sur confirmation du propriétaire |
| R-26 — Drone après réouverture collective | **non consignée** | la dernière confirmation spécifique du Drone après réouverture collective du projet n'a pas été consignée ; ce point n'est pas déclaré PASS |
| R-27 Ableton Live | **RETIRÉ** | décision explicite du propriétaire ; compatibilité Ableton Live **non certifiée** |
| R-28 exports hors ligne (TRANSPORT AUTHORIZATION) | **NON EXÉCUTÉ jusqu'à son terme** | risque accepté par le propriétaire |
| R-29 écoute Z (Tilt, Shift) | **NON EXÉCUTÉ jusqu'à son terme** | risque accepté par le propriétaire |
| R-30 écoute A/B des captures R2 | **NON EXÉCUTÉ jusqu'à son terme** | risque accepté par le propriétaire |

Aucun de R-28, R-29, R-30 n'est PASS. Le verrouillage repose sur l'acceptation de risque, pas sur leur réussite.

## 3. Échecs historiques et réserves conservés

Les échecs documentés de `tests/known_failures.txt` restent FAIL, non masqués, sans assouplissement de critère :
`core:ELP-01`, `core:D2-04`, `core:D2-04c`, `wrapper:AUT-02` (clause c), `core:R2-02`, `core:ZC-05`, `core:ZC-13`,
`core:ZC-15` (critère remplacé), `core:R2-02R` (clause a). Les FAIL historiques G-15 et DST-09 (commit c741fb8) restent
consignés (rapports ARCH-G4 / G4.1). Arbitrage non résolu conservé : restriction d'ordre inter-domaines des
transformations Z (`docs/V1-Z-TRANSFORMS-CONTRACT.md` §3 ter, **non validée par le propriétaire**), à représenter
fidèlement dans l'interface sans la considérer comme résolue. Licence JUCE (B-04) : non résolue.

## 4. Corrections documentaires (sans modification DSP)

Le code (`plugin/PluginProcessor.cpp`, `taFlagPath()` / `taLogPath()`) utilise
`juce::File::userApplicationDataDirectory`, qui vaut `~/Library` sur macOS. Les chemins effectifs sont donc :

- journal : `~/Library/Labtechno/SUBDESTRUCKT/ta_log.txt`
- fichier d'activation : `~/Library/Labtechno/SUBDESTRUCKT/ta_diagnostics.on`

La mention antérieure `~/Library/Application Support/Labtechno/SUBDESTRUCKT/` était erronée ; corrigée dans
`docs/V1-LOCAL-BUILD.md` §5 et `docs/V1-AUTONOMY-CONTRACT.md` §4. Les manuels déjà publiés (`manual/dist/0.3.3` et
antérieurs) sont des éditions figées et ne sont pas réécrits ; l'édition suivante du manuel doit porter le chemin
corrigé.

## 5. Contrôle de non-modification

Pour toute mission postérieure (ARCH-UI-04 et suivantes) : `git diff 0469c4d -- source/` doit être vide, la liste
`tests/core/expected_param_ids.txt` inchangée, et les rendus de référence (REG-01, goldens `tests/core/golden_g43_*`)
identiques. Le rapport ARCH-UI-04 publie ce contrôle.

## 6. Dérogation ciblée ARCH-UI-04-R1 (0.4.1) — TRANSPORT AUTHORIZATION, anomalie R-28

Autorisée par le propriétaire (« GO ARCH-UI-04-R1 ») pour la seule correction d'autorisation autonome. Aucune
modification des algorithmes de synthèse, des transformations Z, des algorithmes des chaînes de destruction et de
filtrage, des 703 StableID ni du format d'état 4. Chaque modification de `source/` est justifiée par un test qui
échouait sur la baseline 0.4.0 (`tests/core/test_ui04r1_ta.cpp`, commit de reproduction `7d57d94`) :

| Fichier | Modification | Justification démontrée |
|---|---|---|
| `source/engine/AutonomyGate.h/.cpp` | autorisation temporaire de RENDU distincte de l'autorisation normale ; fin au premier bloc temps réel ; historique temps réel et verrou Panic figés pendant le rendu ; première observation après le rendu sans arête ; Panic et suspension retirent aussi l'autorisation de rendu ; vue « normale » pour le thread de contrôle ; événement de journal « fin de l'autorisation de rendu » (ajouté en fin d'énumération, non persisté) | TA-R01..R03, R05..R11, R13 : autorisation « transport » permanente obtenue par un export, conservée après le rendu ; ARRÊT pendant le rendu levant le verrou Panic (TA-R06) |
| `source/engine/Engine.h/.cpp` | `resetAfterRender()` au retour en temps réel sans autorisation normale (voix, lignes, chaînes, lignes d'alignement remises à zéro par les primitives existantes de fin de rampe Panic) ; accesseur `autonomyNormallyAuthorized()` | contre-épreuve : machine d'états corrigée SANS cette remise à zéro → restitution résiduelle après la fin du rendu (D2 Legacy 6,9e-3, D2 Regenerative 8,8e-3, S2 9,1e-2, N2 1,9e-2, Drone 5,0e-2 RMS sur la première seconde ; TA-R01, R02, R16 en échec) |
| `plugin/PluginProcessor.cpp` (hors `source/`) | capture automatique à la sauvegarde et capture demandée conditionnées par l'autorisation NORMALE | TA-R14 : la sauvegarde DAW pendant ou après un export réécrivait les captures persistées |

Coût mesuré de la machine d'autorisation (Windows MSVC, TA-20) : 5,8 → 7,8 ns par bloc hôte (0,011 % d'un bloc de
256 échantillons). Aucune allocation, aucun verrou, aucun accès disque ajoutés sur le thread audio (TA-R15, TA-R15w,
TA-20 ; RTSan en CI). Tous les verdicts historiques sont inchangés, sauf TA-15 clause (e), critère remplacé
(`tests/known_failures.txt`).

## 7. Révision du DSP FINAL LOCK : 0.4.1 (2026-11-28)

La référence verrouillée devient **0.4.1**, SHA `d2d08ac1bb6ed09ee3ea1b5e5d5d409c41b586af` (CI run 50 SUCCESS ; build
physique 263320802, Reason 14.1d100, Mac Studio M4 Max). Synthèse, transformations Z, chaînes, 703 StableID et format
d'état 4 identiques à 0.3.3 ; seule différence : TRANSPORT AUTHORIZATION (§6). Contre-audit physique indépendant de Codex (Mac Studio M4 Max, Reason 14.1d100, build physique 263320802, SHA
`d2d08ac1bb6ed09ee3ea1b5e5d5d409c41b586af`, CI run 50 SUCCESS). Rapport
indépendant et archive de preuves conservés localement par le propriétaire.

| Point | État |
|---|---|
| R-26 Reason 14 | GO (0.3.3) |
| R-27 Ableton Live | retiré ; compatibilité non certifiée |
| R-28 exports | **PASS physique dans le périmètre Reason 14 exécuté (0.4.1)** ; PANIC pendant un export et sauvegarde pendant un export NON EXÉCUTÉS physiquement (dialogue modal), couverts automatiquement (TA-R06, TA-R14) |
| R-29, R-30 | non exécutés, risques acceptés (réserves maintenues) |
| Échecs historiques | neuf, documentés (`tests/known_failures.txt`), dont TA-15 clause (e) critère remplacé |
| Limites d'API VST3 | `docs/V1-AUTONOMY-CONTRACT.md` §5 |

Contrôle de non-modification pour les missions suivantes : `git diff d2d08ac -- source/` vide (remplace la référence
`0469c4d` du §5).
