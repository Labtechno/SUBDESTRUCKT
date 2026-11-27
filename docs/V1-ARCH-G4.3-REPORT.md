# SUBDESTRUCKT — ARCH-G4.3 — DSP FINAL LOCK — Rapport final

Branche unique `feature/arch-g4-full-development` (aucun merge vers `main`). Base : `a9cde4ba8f2f506cf2472158832a6ea54816febb`
(ARCH-G4.2 final) ; référentiel documentaire `main` : `9a07d7654cfce951900cda9e8bec7303b8007a15`. Version interne
**0.3.2** (aucune collision : pas d'étiquette, pas d'édition 0.3.2 préexistante), format d'état **4**. Critères écrits avant
mesure : `docs/V1-ARCH-G4.3-SPEC.md` (commit 2a228d4, antérieur à toute mesure ; §11 : additifs consignés pendant
l'exécution). Suivi : `docs/V1-ARCH-G4-TRACKER.md`. ARCH-UI-04, Design System final et Symbiotic Core 3D : non commencés.

## 1. Verdicts

| Verdict | Statut | Motif |
|---|---|---|
| **A. Clôture de l'implémentation DSP automatisée** | **ATTEINTE, avec réserves documentées** | Tous les chantiers implémentés (TRANSPORT AUTHORIZATION, captures R2, ELP adoptée, D2 LEGACY/REGENERATIVE, catalogue Z de huit transformations, compilation locale durcie) ; CI macOS arm64 verte au SHA livré (§11) ; 3 nouveaux échecs documentés, critères non assouplis (R2-02, ZC-05, ZC-13 c) ; 2 arbitrages DSP ouverts (A-G43-01 niveau de Harmonic Tilt, A-G43-02 progressivité de Harmonic Shift) |
| **B. Autorisation de recette privée sur Mac physique** | **GO conditionnel** | Conditions : (1) le propriétaire confirme lui-même son éligibilité et accepte la licence JUCE Starter (aucune acceptation par l'agent) ; (2) compilation locale au SHA livré (§12) ; (3) garder Gain Compensation activé avec Harmonic Tilt brillant (A-G43-01) |
| **C. DSP FINAL LOCK définitif** | **NON — EN ATTENTE DE RECETTE PHYSIQUE** | TRANSPORT AUTHORIZATION doit être validée dans Reason 14 et Ableton Live, exports compris (§13–15) ; une CI verte ne vaut pas validation physique |

| Indicateur | Valeur | Calcul |
|---|---|---|
| **Avancement fonctionnel** | **94,0 %** | (44 PASS + 0,5 × 6 IMPLEMENTED) / **50** ; BLOCKED/FAIL/TODO = 0 (ARCH-G4.2 : 93,0 %) |
| **Validation effective** | **88,0 %** | 44 PASS / 50 (inchangé) |
| Répartition | 44 PASS · 6 IMPLEMENTED (R19, R42–R45, R50) · 0 BLOCKED · 0 FAIL · 0 TODO | R19 : BLOCKED → IMPLEMENTED |
| Portée des preuves | automatisée : cœur (155 critères), wrapper (33), hôte VST3, pluginval 10, RTSan, paquets en VM, harnais LB, manuel | physique : **aucune** (aucune DAW, aucun Mac physique, aucun contrôleur) |

## 2. Branche et SHA

Commits ARCH-G4.3 : 2a228d4 (spécification, empreintes 0.3.1), 528a000 (TRANSPORT AUTHORIZATION), d7dcc22 (D2, Z,
R2, compilation locale, barrière, 0.3.2), puis le commit de livraison (manuel 0.3.2, documentation, mesure TA-20).
**SHA livré : voir §11** (dernier commit de code et de manuel, CI verte) ; les commits ultérieurs éventuels ne touchent
que ce rapport et le tracker.

## 3. Arbitrages intégrés

| Arbitrage | Intégration | Preuve |
|---|---|---|
| SPEC-HR-01 adoptée, plateau accepté | ELP inchangée ; ELP-01 reste FAIL historique ; ELP-01R prospectif | ELP-01R PASS ; ELP-02..07, G-15, G-15b/c, DST-09/09b, HR-01/02 PASS |
| D2 C1 adoptée, compatibilité historique | `dst.<slot>.d2.fb_mode` Legacy / Regenerative ; défaut Regenerative ; format < 4 → Legacy | D2R-01..10 PASS ; D2R-11 : 28 exemples du manuel identiques octet par octet à 0.3.1 |
| Autorisation par le transport, jamais persistée | AutonomyGate (§4) | TA-01..25, TA-xxw PASS |
| Panic : révocation systématique | RF-01 | TA-09..12, TA-22, TA-22w PASS |
| Captures R2 avec décroissance | trajectoires spectrales | R2-01, 03, 04, 05 PASS ; R2-02 FAIL documenté |
| Catalogue Z de huit transformations | Tilt, Mask, Focus, Shift, Scatter + Shape (append-only) | ZC-01..04, 06..12, 14..17 PASS ; ZC-05, ZC-13 FAIL documentés ; ZC-18 : 10 exemples |
| JUCE Starter sous réserve | aucune acceptation par l'agent ; script inchangé (choix déclaré) | `docs/V1-ARCH-G4.1-LICENCE.md` |

Révisions fonctionnelles (définies avant mesure, spec §1) : RF-01 Panic systématique ; RF-02 autorisation par
transport/rendu ; RF-03 Drone restauré soumis à l'autorisation ; RF-04 D2 Legacy pour tout état antérieur au format 4
(appliquée aussi à la clause « autres au défaut » de ST-02).

## 4. Architecture TRANSPORT AUTHORIZATION

Contrat : `docs/V1-AUTONOMY-CONTRACT.md`. Machine d'états `AutonomyGate` évaluée par le thread audio **une fois par bloc
hôte**, sans allocation ni verrou : (1) suspension (génération de chargement) ; (2) transport INCONNU / ARRÊT / LECTURE
(± ENR, hors temps réel) — INCONNU ne vaut jamais ARRÊT ; (3) candidats EXPLICITE, TRANSPORT (ARRÊT→LECTURE), RECORD
(punch-in), RENDER (hors temps réel en lecture) ; (4) un bloc contenant All Sound Off n'accorde rien ; (5) Panic révoque et
verrouille jusqu'à un ARRÊT réel ou une autorisation explicite. Commandes du thread de contrôle atomiques (génération
visée par Autoriser : commande périmée si une restauration plus récente arrive). Captures préparées hors RT, **retenues**
par le moteur, appliquées une fois par autorisation (et après re-préparation) aux seuls emplacements armés. Drone effectif
= configuré ET autorisé. `authorizeAutonomy()` n'est plus qu'une commande atomique (plus de mutex ni d'allocation).
Classification JUCE 9.0.6 auditée : `setNonRealtime(processMode == kOffline)` à chaque `process` ; contexte VST3 absent
→ temps 0/0 non fini → INCONNU. Journal d'événements SPSC + `ta_log.txt` sur fichier d'activation ; état dans
Engineering › Diagnostics. Clic d'armement ou de Drone dans l'éditeur = autorisation explicite.

Défaut réel corrigé au passage : **Panic** ne purgeait pas les lignes d'alignement de latence (S2/N1) ; l'audio antérieur
ressortait après la rampe (TA-09 premier passage : 1,26e−4 ; corrigé : 0).

## 5. Résultats automatisés et cas limites (local MSVC puis CI macOS arm64, qui fait foi)

| Famille | Résultat | Points saillants |
|---|---|---|
| TA moteur (24) | **24/24 PASS** | STOP→PLAY bit à bit = autorisation explicite au même bloc ; boucle/seek/tempo sans effet ; Panic prioritaire dans les 6 ordres et aux décalages 0/300 ; INCONNU ≠ ARRÊT ; PLAY déjà actif non autorisé ; rendu hors temps réel autorisé sans lecture préalable ; re-préparation 96 kHz/64, High, Eco : conservée ; journal ordonné, 0 perte ; coût 6,3 ns par bloc hôte |
| TA wrapper (9) | **9/9 PASS** | playhead simulé, INCONNU (playhead nul, position absente, temps non fini), export hors temps réel juste après ouverture sans éditeur ni thread message, état identique autorisé/suspendu, `drone_restore`, CC120/CC123, éditeur ×10, clics Drone et armement, 0 allocation |
| R2 (5) | 4 PASS, **R2-02 FAIL** | trajectoires S2 (Decay 0,3–1, 44,1/48/96 kHz) ≤ 0,26 dB par bande, pente à 3,6 % du contrat ; restitution par STOP→PLAY et par rendu ; Panic puis réautorisation : spectre = capture (≤ 1e−7) |
| ELP-01R | PASS | identité, symétrie, monotonie non stricte jusqu'à DBL_MAX, continuité, borne < 32.0f, non-finis à la garde |
| D2R (10 + manuel) | **PASS** | LEGACY = empreintes 0.3.1 (10/10) ; fb 0 : modes identiques 30/30 ; Position 0 : −14 / −8,7 / −3,8 dB ; gel inchangé ; transitions par bloc ; 90/90 cas du protocole de stabilité (décroissance ≤ −49 dB) |
| Z (17 + exemples) | 15 PASS, **ZC-05, ZC-13 FAIL** | neutralité bit à bit ; existant = empreintes 0.3.1 (14/14) ; continuité à 0⁺ ≤ −68 dB ; aliasing ≤ −67,7 dB ; fondamentale exacte ; gain ±0,000 dB hors butée ; ordre respecté ; déterminisme entre processus et entre compilations propres |
| Compilation locale | LB-01..13 PASS, INV-01 PASS | pluginval avant paquets ; charge utile = bundle validé ; ZIP réextrait ; barrière exhaustive ; SBD_ONLY, symlink, collision refusés |
| Non-régression | PASS | REG-01, REG-ID (703, 695 inchangés), G-22w (VST3), G-21w, AUT-01..08, CAP-02w, ST-01..03, AUD-01 (703), MIDI Learn, Randomize/Mutate/Evolve, Undo, PERF-01..03 |

Cas limites étudiés : instance créée, projet chargé, duplication et restauration **pendant la lecture** (non autorisés
jusqu'à ARRÊT→LECTURE : TA-14, TA-04 b) ; premier bloc en LECTURE ; preroll (une LECTURE observée après ARRÊT :
autorisée) ; changement de fréquence et de taille de bloc, désactivation/réactivation, Quality (TA-16) ; disparition et
retour du playhead (TA-13). Ambiguïtés non résolubles par l'API : contrat §5.

## 6. Compatibilité des anciens projets

695 StableID et identifiants VST3 historiques inchangés (REG-ID, G-22w), 8 ajoutés en fin (703). Format d'état 4 ;
états v1/v2/v3 lus ; D2 en Legacy ; rendu des états v3 (empreintes 0.3.1, Z existant et D2 à Feedback > 0) identique
bit à bit sur msvc-x64 et clang-arm64 (ZC-03, D2R-03). Un état 0.3.2 est refusé par 0.3.1 (version future). Énumérations
Z étendues en fin (indices 0–3 inchangés). Drone : `drone_restore=1` restaure la configuration, désormais suspendue
jusqu'à l'autorisation (RF-03) ; `drone_restore=0` inchangé. Presets d'usine et exemples du manuel : Legacy explicite.
Cartographie ARCH-UI-03 actualisée (703/703 + actions) : `docs/V1-ARCH-UI-03-STABLEID-MAP.md` (copie de `main` + ajouts).

## 7. Les huit transformations Z

Contrat : `docs/V1-Z-TRANSFORMS-CONTRACT.md`. Z2 Fold, Z3 Warp, Z8 Mutation inchangées (empreintes 0.3.1). Z1 Tilt,
Z4 Mask, Z5 Focus, Z6 Shift, Z7 Scatter : domaine spectral (coefficients harmoniques de la forme morphée, FFT inverse
toutes les 128 échantillons, fondu, bande appliquée après transformation, profil mip pour la continuité).

| Critère | Résultat |
|---|---|
| Distinction Tilt/Mask/Focus (v = 1) | 78,1 / 115,0 / 86,8 dB ; Tilt monotone, Mask minima nuls aux multiples de p, Focus maximal au rang 11 (C = 11,31) |
| Propriétés exactes (référence indépendante) | Tilt h^(vλ), Mask (1 − v) aux multiples, Shift entier exact (k = 1, 2, 5, 16, −3, −8), Scatter amplitudes inchangées |
| Morph XY × Quality | finis ; continuité ≤ −63,8 dB ; aliasing ≤ −67,7 dB |
| Modulation rapide | LFO 20 Hz profondeur/Shape + Z Master par CROSS audio : crête ×1,00, saut ×1,14 (≤ 1,5) |
| **ZC-05 FAIL** | Shift : distance non monotone sur Metal seulement (coïncidences de partiels d'un spectre creux) ; Saw et les 4 autres : monotones — arbitrage A-G43-02 |
| **ZC-13 FAIL (c)** | Scatter : facteur de crête +0,91 dB (< 1) sur dent de scie σ = 0 ; (a) FM via CROSS −12,2 dB et (b) après Fold 23 bandes PASS. Pertinence : le facteur de crête mesure la seule forme d'onde ; l'intérêt musical de Scatter est sa réponse aux non-linéarités et à la FM, démontrée par (a) et (b) ; le critère (c) est conservé comme indicateur de forme, FAIL maintenu |
| **ZC-16 PASS, défaut signalé** | sortie finie, garde 0 ; **mais** la crête vient de Harmonic Tilt brillant : avant ELP +40,9 dBFS (Metal), +46,8 dBFS (Basic) ; Tilt seul v 1 σ 1 sans compensation +78,9 dBFS ; avec compensation forcée +30,3 dBFS ; sans Tilt −1,1 dBFS. L'ELP borne la sortie mais ne doit pas tenir lieu de correction : arbitrage A-G43-01 (options : Tilt à énergie constante, borne par rang, butée élargie) |
| Exemples (ZC-18) | e23–e32 (2 par transformation, moteur réel, chapitre 6) ; e33/e34 D2 Legacy/Regenerative |

## 8. Mesures CPU et mémoire (Windows MSVC : ne certifient ni M3 ni M4 Max)

| Mesure | Valeur |
|---|---|
| Moteur, ns/échantillon P50/P95 (48 kHz) | Z off 281/330 ; Fold 316/353 ; Shift+Tilt statique 269/303 ; profondeur modulée 877/1 080 ; XY modulé 1 070/~1 120 ; Scatter modulé ~550 |
| Par resynthèse spectrale | Tilt 10,9 µs ; Mask 28,0 ; Focus 26,2 ; Shift 40,2 ; Scatter 14,1 ; combinaison XY 4 formes 33,5 ; FFT 2048 18,7 ; copie du cache 0,12 |
| Cadence | ≤ 1 resynthèse / 128 échantillons (2 352–2 664 pour 384 000) ; avant correction de la topologie par défaut des moteurs de test : 13 906 (WT évalué 6 fois par échantillon, mesure invalide) |
| Autorisation | 6,3 ns par bloc hôte (0,009 % d'un bloc de 256 échantillons) |
| Mémoire ajoutée | Z : 48 Ko par instance ; captures retenues : au plus 6 captures (≤ 2,05 s stéréo chacune) partagées avec l'état |
| PERF-01..03 | inchangés en structure (facteurs temps réel de la machine de test) ; mesures M4 Max : `perf_this_machine.txt` de la compilation locale |

## 9. Échecs historiques conservés et nouveaux échecs (aucun critère assoupli)

| ID | Statut | Détail |
|---|---|---|
| ELP-01 | FAIL historique | plateau de tanh (ELP-01R prospectif PASS) |
| D2-04, D2-04c | FAIL historiques | moteur LEGACY non modifié ; protocole D2R-10 : 90/90 |
| AUT-02 (c) | FAIL historique | S2 décroît par contrat ; AUT-02c et R2-01/03/05 PASS |
| G-15, DST-09 | FAIL historiques (c741fb8) conservés | PASS actuels, désormais **non conditionnels** (SPEC-HR-01 adoptée) |
| **R2-02** | FAIL nouveau | restitution S2 entre fréquences : biais −2,5/−3 dB corrigé (énergie totale −0,35/+0,48 dB), écarts résiduels ≤ 2,06 dB dans des bandes à −16…−24 dB (grille de bins plus grossière à 44,1 kHz ×2 et 96 kHz) |
| **ZC-05** | FAIL nouveau | §7 |
| **ZC-13 (c)** | FAIL nouveau | §7 |
| Premiers passages consignés | corrigés | TA-08 (mesure), TA-09 (défaut Panic), TA-21/22 (configuration), TA-23w (scénario), R2-02 (normalisation), R2-05 (mesure), ZC-13 b (lecture hors limites du test), ZC-17 (topologie par défaut), D2R-02 (filtre) : spec §11 |

## 10. Matrice R01–R50 (PASS = 1, IMPLEMENTED = 0,5, autres = 0)

Inchangées depuis G4.2 et PASS : R01–R09, R11–R17, R20–R26, R29–R39, R41, R47. Évolutions ARCH-G4.3 :

| # | Exigence | Statut | Preuve G4.3 |
|---|---|---|---|
| R10 | Tail/Drone/Panic | PASS | TA-09..12, TA-21/22 ; purge Panic des lignes d'alignement |
| R18 | Z : transformations attestées | PASS | ZC-03 (empreintes 0.3.1) |
| R19 | Z : cinq transformations du catalogue | **IMPLEMENTED** (était BLOCKED) | 5 implémentées ; ZC-05, ZC-13 FAIL ; A-G43-01/02 ouverts |
| R27–R28 | Destruction (dont D2) | PASS | D2R-01..10 |
| R40 | Limiteur, garde, ELP | PASS | ELP adoptée, ELP-01R |
| R42–R45 | UI | IMPLEMENTED | inchangé (seuls ajouts : Feedback Mode D2, Shape Z, état d'autorisation) |
| R46 | Wrapper VST3 | PASS | 703 paramètres, format 4, G-21w/G-22w, pluginval 10 |
| R48 | Packaging | PASS | paquets 0.3.2 en CI (non publiés), LB-01..13 |
| R49 | Manuel | PASS | manuel 0.3.2 (29 tests de recette), Playwright |
| R50 | Captures R2 | **IMPLEMENTED** | AUT, TA, R2-01/03/04/05 PASS ; R2-02 FAIL ; recette physique R-21/R-25/R-28 restante |

## 11. CI

Voir l'addendum en fin de rapport (run de validation du SHA livré). Run 40 (`d7dcc22`, code complet hors manuel 0.3.2) :
https://github.com/Labtechno/SUBDESTRUCKT/actions/runs/45988842150 .

## 12. Compilation et installation privées

`docs/V1-LOCAL-BUILD.md`. Résumé : dépôt propre au SHA livré ; `unset SBD_ONLY` ;
`tools/local-build/build_local_macos.sh --juce-licence=<votre choix>` ; retirer un doublon utilisateur signalé ; ouvrir
vous-même `SUBDESTRUCKT_Installer.pkg` ; désinstaller par `SUBDESTRUCKT_Uninstaller.pkg`. Aucune installation
automatique, aucun fichier à publier.

## 13. Protocole Reason 14 et Ableton Live (R-26, R-27 du manuel 0.3.2)

Projet de recette à 5 pistes (Drone + note tenue, D2 gelé armé, S2 gelé armé, Feedback D2 Regenerative, preset non
autonome) ; journal activé (`ta_diagnostics.on`). Étapes 1–14 : ouverture à l'arrêt (silence) ; PLAY (autorisation,
source transport) ; RECORD ; STOP (conservée) ; boucle ; seek ; Panic pendant PLAY (silence maintenu) ; STOP puis PLAY
(retour) ; preset pendant PLAY ; duplication pendant PLAY (copie muette jusqu'à STOP→PLAY) ; insertion pendant PLAY ;
éditeur fermé/rouvert ; sauvegarde/réouverture ; plusieurs instances. Relever la version exacte de l'hôte, joindre
`ta_log.txt`.

## 14. Protocole d'exports (R-28)

Étapes 15–20 : export immédiatement après ouverture (sans lecture ni Autoriser) ; hors temps réel et temps réel ; bounce,
rendu de piste, Freeze de piste ; région décalée ; comparaison lecture/export **par bande et par présence des états
autonomes**, sans exiger l'égalité bit à bit (décroissance S2, phase et preroll dépendent du point de départ ; protocole
R2 : énergie par bande ±0,5 dB à temps écoulé égal) ; annulation puis lecture. Attendu : chaque export contient le
Drone, les gels restitués et le Feedback ; `ta_log.txt` montre « hors temps reel » et la source « rendu hors temps reel ».
Si un hôte ne signale ni lecture ni rendu : limitation documentée (contrat §5), alternative Autoriser avant l'export.

## 15. Tests physiques restant à exécuter

R-26 (Reason 14), R-27 (Ableton Live), R-28 (exports), R-29 (écoute Z et D2), R-11 (MIDI Learn, contrôleur), R-13,
R-16, R-21, R-25 (états autonomes), R-01/R-02/R-17/R-18 (compilation, installation, désinstallation) ; mesures
M4 Max (`perf_this_machine.txt`) ; M3 physique avant toute Beta publique.

## 16. Confirmations

- **Git** : une seule branche, aucun sous-agent, aucun worktree parallèle, **aucune fusion vers `main`** (toujours
  `9a07d76`).
- **Licence** : aucune licence acceptée, aucun compte créé ; Starter retenue sous réserve de l'éligibilité et de
  l'acceptation explicite du propriétaire. Note JUCE : une licence commerciale JUCE peut être requise selon l'usage.
- **Publication** : aucun binaire ni paquet publié (release, dépôt, artefact CI : journaux uniquement) ; manuel 0.3.2
  construit et testé dans le dépôt, **non publié sur gh-pages**.
- **Dépenses** : aucune.
- **ARCH-UI-04** : non commencé ; nécessite l'autorisation du propriétaire.
