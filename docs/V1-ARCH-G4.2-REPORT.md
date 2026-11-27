# SUBDESTRUCKT — ARCH-G4.2 — Rapport final

Branche : `feature/arch-g4-full-development` (une seule branche ; aucun merge vers `main`). Base ARCH-G4.1 :
`90e12f982688cd9513b8dd50e5457cd05b259591` ; référentiel documentaire `main` : `9a07d7654cfce951900cda9e8bec7303b8007a15`
(ARCH-UI-03 lu, **ARCH-UI-04 non commencé**). Version produit : **0.3.1** (build interne, aucun binaire diffusé).
Critères écrits avant mesure : `docs/V1-ARCH-G4.2-SPEC.md`. Suivi : `docs/V1-ARCH-G4-TRACKER.md` (§3ter, matrice, journal).

## 1. Verdict

**GO conditionnel** pour la recette privée sur le Mac du propriétaire (compilation locale sécurisée), sous trois
conditions : (1) choix par le propriétaire de la licence JUCE pour la compilation locale (D-LIC-1) ; (2) CI macOS arm64
verte sur le commit livré (§5) ; (3) acceptation, ou décision contraire, des échecs documentés (§7), en particulier
SPEC-HR-01 dont dépend le PASS de G-15 et DST-09. **NO-GO pour toute diffusion publique** : licence JUCE non choisie
(D-LIC-2), pas de Developer ID ni de notarisation, aucune recette DAW ni sur Mac physique.

| Indicateur | Valeur | Calcul |
|---|---|---|
| **Avancement fonctionnel** | **93,0 %** | (44 PASS + 0,5 × 5 IMPLEMENTED) / **50** exigences, BLOCKED compté 0 (ARCH-G4.1 : 92,0 %) |
| **Taux de validation effective** | **88,0 %** | 44 exigences PASS / 50 (ARCH-G4.1 : 86,0 %) |
| Répartition | 44 PASS · 5 IMPLEMENTED (R42–R45 UI, R50) · 1 BLOCKED (R19) · 0 TODO · 0 FAIL | |

## 2. Branche et SHA

Branche `feature/arch-g4-full-development`. Commits ARCH-G4.2 : 87d1ed6, 1ef1c67 (critères), 4912ea6 (ELP,
autonomie, D2, MIDI Learn), 49bfa0c (compilation locale, CI, 0.3.1), puis documentation (manuel 0.3.1, Z, rapport).
SHA exact livré : **`6ac4b838807460216294aa3cb3a26bb19479265d`** (dernier commit de code, manuel et CI), run CI 37 SUCCESS (§5) ; commits ultérieurs : rapport et tracker uniquement.

## 3. Modifications

- **ELP (SPEC-HR-01, toujours proposée)** : fonction isolée (`source/dsp/Elp.h`). Défaut réel trouvé par ELP-05 :
  en 0.3.0, la sortie *float* atteignait exactement 32,0 (+30,10 dBFS, niveau de la garde), la marge de 1e−9 étant
  absorbée par l'arrondi. Correctif : marge portée sur l'asymptote (R′ = 16·(1 − 1e−7)), pente exactement 1 au seuil,
  sortie < 32,0f. Identique bit à bit sous +24 dBFS (ELP-04). Contre-épreuve de test sans ELP (aucun paramètre).
- **Sécurité des états autonomes** (non-conformité DAW de CAP-01 corrigée) : quatre catégories séparées
  (configuration persistée, captures R2, état d'exécution, autorisation locale **non persistée**). Armement effectif
  = configuré ET autorisé (masque moteur sur les 9 armements, base intacte). Restauration externe (projet DAW,
  duplication, preset) portant un armement ou une capture → suspension et captures retenues ; Panic → suspension si
  un armement est configuré ; **Autoriser** (barre globale) → activation et restitution des captures. Sauvegarde
  d'une instance suspendue : captures conservées (aucune capture d'un tampon inactif). Drone : mécanisme
  `drone_restore` inchangé. **G-21w non modifié et PASS** (configuration persistée identique octet par octet).
- **D2 Glitch Buffer** : analyse (§6) et sémantique candidate C1 « régénération dans la boucle » implémentée comme
  option interne de test (non exposée, non persistée, défaut historique) ; **non adoptée** (A-G42-01).
- **Transformations Z** : recherche négative ; 6 propositions **nouvelles**, non implémentées
  (`docs/V1-ARCH-G4.2-Z-PROPOSALS.md`). R19 reste BLOCKED.
- **MIDI Learn** : une CC ↔ un paramètre (remplacement, retrait de l'ancienne CC) ; CC réservées (0, 6, 32, 38, 64,
  96–101, 120–127) et cibles refusées (non automatisables, 9 armements) ; profil en lecture-modification-écriture et
  relecture périodique (plusieurs instances sans perte) ; chemin de profil isolé en test ; service du thread message
  appelable (tests). Chemin audio : atomiques uniquement.
- **Compilation locale macOS** (`tools/local-build/build_local_macos.sh`, réécrit) : sources exportées du commit
  (`git archive`), dépôt propre exigé, épinglages lus dans le workflow du commit (JUCE tag + SHA et arbre propre,
  pluginval SHA-256), tests obligatoires avec la même barrière que la CI, identité du bundle (identifiant, version,
  build, commit, arm64, signature ad hoc), pluginval rigueur 10 bloquant, livraison complète vérifiée (Installer.pkg,
  Uninstaller.pkg, README_FR, CHANGELOG, SHA256SUMS vérifié, ZIP versionné), **aucune installation** (`--install`
  supprimé), doublons VST3 utilisateur/système signalés sans modification, suppressions bornées à `local-build/`
  (`lib_safe_rm.sh`). Défaut corrigé : le numéro de build `localAAAAMMJJ…` de 0.3.0 n'était pas numérique (non conforme
  au format numérique de CFBundleVersion ; jamais exécuté jusqu'ici) ; désormais numérique (`AAJJJhhmm`).
- **CI** : job `local-build-script` (harnais LB, Ubuntu gratuit) ; artefacts limités aux journaux (l'exécutable du
  contrôle négatif RTSan n'est plus publié) ; VERSION 0.3.1.
- **Tests** : `tests/core/test_g42.cpp` (ELP, AUT-01, D2), tests wrapper AUT-02..08, ML-01w..07w, CAP-02w mis à jour,
  filtre `SBD_ONLY` pour les exécutions ciblées (la CI exécute toujours tout).
- **Documentation** : manuel 0.3.1 (comportements modifiés seulement ; 25 tests de recette dont R-25 nouveau ;
  glossaire corrigé), README/CHANGELOG 0.3.1, tracker, ce rapport. Éditions 0.1.0-b5, 0.2.0, 0.3.0 et S0.1 inchangées.

## 4. Résultats des tests (critères ARCH-G4.2 ; local MSVC, puis CI macOS arm64 qui fait foi, §5)

| ID | Résultat | Mesure |
|---|---|---|
| ELP-01 | **FAIL** (documenté) | identité exacte sur 10⁶ points, impaire, < 32 jusqu'à DBL_MAX : oui ; **stricte croissance non tenue en double précision** : plateau de tanh dès \|y\| = 262,3 (+48,4 dBFS d'entrée) ; fonction non décroissante |
| ELP-02 | PASS | \|f(16+ε) − (16+ε)\| ≤ ε²/16 (pire rapport 0,002) ; pente au seuil 0,999999991 ; 1-lipschitzienne (10⁵ paires) |
| ELP-03 | PASS | f(10⁶) = 31,999998 ; f(DBL_MAX) = 31,9999984 (+30,1030 dBFS) |
| ELP-04 | PASS | 30 configurations à crête dans ]1, 16[ : identiques bit à bit avec et sans ELP, 0 engagement |
| ELP-05 | PASS (après correctif) | 24 configurations extrêmes, sauts de gain par bloc, 4 Fs : crête 31,9999981 (< 32), garde 0, non-finis 0, 4 935 842 engagements, télémétrie cohérente avec les échantillons > 16 (0 écart). Avant correctif : crête 32,0 (FAIL, consigné) |
| ELP-06 | PASS | protocole DST-09 : avec ELP garde 0 ; **sans ELP garde 638 487** : le PASS de G-15/DST-09 dépend de SPEC-HR-01 |
| ELP-07 | PASS | limiteur −6 dB sous +24 dBFS : limiteur 749, ELP 0, garde 0 ; au-delà : ELP 95 170, limiteur 3 763, garde 0, crête = plafond ; télémétrie = compteurs internes |
| AUT-01 | PASS | 9 armements : suspendu ≡ désarmé bit à bit 9/9 ; autorisé ≡ moteur armé historique 9/9 ; armement audible dans le scénario 6/9 ; 0 allocation RT |
| AUT-02 | **FAIL** (clause c, documenté) | (a) état relu identique octet par octet ; (b) 3 s sans note : énergie 0, suspendu ; après Autoriser : gel rejoué ; (c) nouvelles captures : D2 identique (écart 0), **S2 différent (écart 54,5)** : le spectre gelé S2 décroît par contrat (S2 Decay = 0,5) |
| AUT-02c | PASS (complémentaire, défini après diagnostic) | S2 Decay = 1 : captures D2 et S2 identiques après restauration suspendue puis Autoriser |
| AUT-03 | PASS | copie suspendue (énergie 0) ; source autorisée, rendu identique à son jumeau |
| AUT-04 | PASS | preset armé avec captures : armements remis à zéro, captures conservées, suspendu, silence ; changement de preset pendant un gel : 1,5·10³ → 2·10⁻¹⁵ |
| AUT-05 | PASS | Panic : autorisation retirée, Drone désarmé, configuration et « dirty » inchangés, silence ; Autoriser : gel restauré |
| AUT-06 | PASS | 2 cycles fermeture/réouverture : état identique octet par octet, toujours suspendu |
| AUT-07 | PASS | état v2 armé migré : suspendu, armement conservé ; Drone `drone_restore` 0/1 inchangé |
| AUT-08 | PASS | bouton Autoriser visible si suspendu ; navigation des 3 espaces sans effet |
| CAP-02w | PASS (étape Autoriser insérée) | suspendu et silencieux avant autorisation ; gel rejoué après Autoriser |
| D2-01 | PASS | historique, Position 0 : écart fb 0,25/0,5/1 vs fb 0 = −344,8 dB (inaudible) |
| D2-02 | PASS | candidat C1 : −14,0 / −8,7 / −3,8 dB (audible, croissant) |
| D2-03 | PASS | fb 0 : candidat ≡ historique bit à bit 30/30 ; gel actif avec fb > 0 : 30/30 |
| D2-04 | **FAIL** (documenté) | seul cas : **historique** vitesse 1 : RMS 50–60 s / 40–50 s = 1,147 (> 1,05) ; candidat C1 : 10/10 ≤ 1,021 ; crêtes ≤ 2,82 |
| D2-04c | **FAIL** (complémentaire, documenté) | sans excitation : une hausse (H v1, canal R, 60–70 s : 1,179 > 1,127), décroissance monotone ensuite, ≤ −50,3 dB à 170–180 s ; 300 s stéréo : finis, crête 2,946 |
| D2-05 | PASS | déterministe ; sauts par bloc : finis ; 0 allocation |
| ML-01w..07w | PASS (7/7) | CC20 → wt.x 0/64/127 exacts ; remplacement et retrait ; profil isolé relu, lignes invalides ignorées ; aucune affectation dans état/preset ; multi-instance sans perte ; CC réservées et cibles refusées, dernière écriture gagnante ; 512 CC/bloc : 0 allocation |
| LB-01..07 | PASS (7/7) | refus sans licence / `--install` (2) ; dépôt modifié, commit inconnu (3) ; test FAIL ou interrompu (5), sans pluginval ni livraison ; échec connu non bloquant ; pluginval FAIL (7) ou empreinte altérée (4) ; bundle incohérent (6), JUCE modifié ou SHA différent (4) ; livraison complète, SHA256SUMS vérifié, dossiers VST3 intacts, doublon signalé ; 9/9 chemins hors périmètre refusés |

Non-régression (local, suite complète) : cœur 99 critères, **seuls ELP-01, D2-04, D2-04c en échec (documentés)** ;
REG-01 (oracle 0.1.0-b5, 20/20 bit à bit), G-15, G-15b/c, DST-09/09b, HR-01/02, CAP-01, AUD-01 (695 paramètres)
PASS. Wrapper : G-18w (0 allocation), G-06w, G-21w, G-22w (695 StableID, identifiants VST3), G-19w, G-16w, WT-ED-02w,
UI-01w PASS ; seul AUT-02 (clause c) en échec documenté. Manuel 0.3.1 : 28 exemples audio rendus au commit 49bfa0c,
**identiques octet par octet à 0.3.0** ; Playwright local 46 PASS, 4 ignorés (tests propres à b5).

## 5. CI (macOS arm64 : build, tests, paquets, pluginval, RTSan ; Ubuntu : harnais LB, manuel)

**Run 37 — SUCCESS** (tous les jobs) : https://github.com/Labtechno/SUBDESTRUCKT/actions/runs/45951352395 , commit
`6ac4b83` (code et manuel identiques au commit livré ; les commits suivants ne modifient que ce rapport et le tracker).

| Job | Résultat |
|---|---|
| build-test-package (macos-15 arm64, VM) | SUCCESS : compilation Release arm64 ; suites cœur et wrapper complètes ; barrière : seuls FAIL = ELP-01, D2-04, D2-04c, AUT-02 (documentés) ; paquets 0.3.1 Installer/Uninstaller ; bundle livré G-24 PASS ; hôte VST3 G-19h/G-20/G-21/G-22 PASS ; **pluginval 1.0.4 rigueur 10 PASS** (tests d'interface inclus, 44,1/48/88,2/96 kHz, blocs 32–512) ; bac à sable et recette VM des paquets ; livraison assemblée **non publiée** |
| RealtimeSanitizer (macOS) | SUCCESS : 0 violation dans Engine::process sur l'ensemble des tests du cœur (masque d'autorisation inclus) ; contrôle négatif détecté |
| local-build-script (ubuntu) | SUCCESS : LB-01..07 PASS |
| manual-playwright (ubuntu) | SUCCESS : éditions 0.1.0-b5, 0.2.0, 0.3.0 et 0.3.1 construites et testées (Chromium, WebKit profil iPhone) |

Runs 35 et 36 : échec du job macOS (voir §7, cause corrigée en 6ac4b83). Mesures CI = machine virtuelle : aucune
certification de performance M3 ou M4 Max.

## 6. Matrice R01–R50 (règle : PASS = 1, IMPLEMENTED = 0,5, FAIL/BLOCKED/TODO = 0, sur 50)

| # | Exigence | Statut | Preuve |
|---|---|---|---|
| R01 | Registre stable extensible | PASS | REG-ID (695 StableID figés, 22 v1 identiques à l'oracle) |
| R02 | État versionné + migration | PASS | ST-01/02/03 (format 3 ; v1/v2 migrés ; version future refusée) |
| R03 | Transactions hors RT | PASS | G-01..G-04, RT-G4 (0 allocation) |
| R04 | Timeline MIDI sample-accurate | PASS | MIDI-01..04 |
| R05 | Mono/Legato, priorités, sustain | PASS | MIDI-0x, ENV-02 |
| R06 | Expressivité MIDI | PASS | MIDI-0x |
| R07 | Accordage A4, microtuning | PASS | MT-01 (+50 ct exact à 0,1 ct ; A4 432) |
| R08 | 5 enveloppes DAHDSR, loop, **Trigger, Sync** | PASS | ENV-01, ENV-02, ENV-03 |
| R09 | 4 LFO | PASS | LFO-01 |
| R10 | Tail/Drone/Panic | PASS | CR/SYNC/DRN/Q ; G4.2 : Panic suspend les états autonomes armés (AUT-05) |
| R11 | WT banques XY 64×16, bornes, Linear/Stepped | PASS | WT-01/02/03 |
| R12 | **WT morph Spectral** | PASS | WT-SM-01 |
| R13 | Interpolation, Clean/Raw | PASS | WT-0x |
| R14 | Phase, sync | PASS | WT-0x, SYNC |
| R15 | WT Character | PASS | CHR-01 |
| R16 | **Génération procédurale, import/export, format portable, édition COW** | PASS | WTA-01, WT-ED-01, WT-ED-02w |
| R17 | Z deux slots | PASS | WT-0x |
| R18 | Z : 3 transformations | PASS | WT-0x |
| R19 | Z : 5 transformations historiques | **BLOCKED** | catalogue non retrouvé (B-01) ; non inventées |
| R20–R22 | FM 4 opérateurs, formes, niveaux/Character | PASS | FM-01..04, CHR-01 |
| R23 | Matrice 4×4 + 10 topologies | PASS | TOPO-01 (45/45 rendus distincts, CROSS conservé) |
| R24 | ExpFM benchmark | PASS | PERF-02 |
| R25–R26 | CROSS B-T2, Cross Audio, Cross Reset | PASS | G-0x, CR |
| R27–R29 | Destruction N1..S2, 6 slots, latence | PASS | DST-01..10, N1-02 |
| R30–R32 | Filtres F1..F4, Pre/Post, excitation, stéréo | PASS | FLT-01..07 |
| R33 | Matrice 64, Via, macros, inactivité | PASS | MOD-01, MACRO-01 |
| R34 | RNG, chaos | PASS | RNG-01, CHAOS-01 |
| R35 | Suiveurs ×3, Y2 | PASS | FOL-01 (après correctif), DST-07 |
| R36–R38 | Presets, Randomize/Mutate/Evolve, Undo | PASS | PRE-01, ST-RND-0x, EVO-01, UNDO-01 |
| R39 | Quality Eco/Normal/High | PASS | Q, N1-02, PERF-03 |
| R40 | Limiteur, garde numérique | PASS | G-15c, HR-01 ; G4.2 : ELP-07 (ELP, limiteur, garde distincts, comptés séparément) |
| R41 | Benchmark CPU reproductible | PASS | PERF-01/02/03 (sans seuil) |
| R42 | UI Performance | IMPLEMENTED | balance ajoutée ; tâches d'ergonomie non mesurées |
| R43 | UI Sound Design | IMPLEMENTED | Capturer gels, Trigger/Sync ; tâches non mesurées |
| R44 | UI Engineering | IMPLEMENTED | éditeur de formes ; tâches non mesurées |
| R45 | Inspector, moniteurs, zoom, accessibilité | IMPLEMENTED | UI-01w PASS ; contrastes/DPI/zoom par niveau non mesurés |
| R46 | Wrapper VST3 (695 paramètres, état v3) | PASS | G-21w (protocole inchangé), G-22w, hôte, pluginval rigueur 10 |
| R47 | MIDI Learn global | **PASS** (G4.2) | ML-01w..07w ; recette physique (contrôleur, DAW) R-11 à faire |
| R48 | Packaging 0.3.1 testé sans diffusion | PASS | CI : Installer/Uninstaller, bac à sable, recette VM ; non publiés ; LB-01..07 |
| R49 | Manuel HTML + recette | PASS | manuel 0.3.1 (25 tests de recette) : contrôles bloquants, Playwright |
| R50 | Captures Freeze/Glitch R2 | IMPLEMENTED | non-conformité DAW **corrigée** (AUT-01, AUT-02 a/b, AUT-03..08, CAP-02w PASS) ; AUT-02 (c) FAIL documenté, AUT-02c PASS ; PASS soumis à A-G42-04 |

## 7. Régressions, échecs et limitations (aucun masqué)

| Élément | Statut | Détail |
|---|---|---|
| Régressions | aucune | REG-01 bit à bit ; exemples audio du manuel identiques à 0.3.0 ; 695 StableID et identifiants VST3 inchangés (G-22w) ; format d'état 3 inchangé ; états v1/v2 migrés (AUT-07, ST-0x) |
| Changement de comportement volontaire | documenté | projets/duplications/presets avec armement ou capture : suspendus jusqu'à Autoriser ; Panic suspend les armements (manuel 0.3.1, CHANGELOG) |
| ELP-01 | FAIL documenté | stricte croissance non tenue en double précision au-delà de +48,4 dBFS d'entrée (plateau ; non décroissante). Critère non assoupli. Options : accepter, ou variante algébrique (forme différente au-delà de +24 dBFS) |
| ELP-05 (avant correctif) | FAIL consigné, corrigé | sortie float = 32,0 en 0.3.0 (marge absorbée par l'arrondi) |
| G-15, DST-09 | FAIL historiques (c741fb8) conservés ; PASS **conditionnel** | dépendent de SPEC-HR-01 (contre-épreuve ELP-06 : garde 638 487 sans ELP) |
| AUT-02 (c) | FAIL documenté | identité S2 non atteignable avec un gel S2 qui décroît (Decay < 1) ; critère mal posé, non modifié ; AUT-02c PASS |
| D2-04 / D2-04c | FAIL documentés | moteur historique : fluctuation bornée (RMS 10 s 0,83–1,65 sur 300 s, sans tendance, crête < 4, décroissance vers 0 sans entrée) ; protocole à deux fenêtres insuffisant pour H ; aucune correction (son historique conservé) ; candidat C1 PASS aux deux |
| CI runs 35 (49bfa0c) et 36 (9d604e4) | **échec, cause corrigée** | job macOS arrêté à l'étape « Tests du wrapper » : son code de sortie non nul (AUT-02 c, échec documenté) faisait échouer l'étape avant la barrière des échecs connus, contrairement au cœur. Correctif 6ac4b83 : code consigné, verdict à la barrière ; un échec non documenté ou une interruption du wrapper restent bloquants. RTSan, harnais LB et manuel : verts sur ces deux runs |
| Wrapper, hôte simulé mono-thread | limite de test | la capture automatique de `getStateInformation` attend un bloc audio (50 ms) ; les tests réalisent la capture explicitement (comme CAP-02w) |
| Glossaire des manuels 0.2.0 et 0.3.0 publiés | défaut connu | définitions héritées de 0.1.0-b5, fausses depuis 0.2.0 (« trois oscillateurs », « WT à une seule forme », PM seule) ; corrigé en 0.3.1 ; éditions publiées non modifiées |
| Performances | non certifiées | aucune nouvelle mesure ; DSP par défaut inchangé ; M3/M4 Max : `perf_this_machine.txt` de la compilation locale |
| UI (R42–R45) | IMPLEMENTED | ARCH-UI-04 non commencé ; seul ajout : bouton Autoriser |
| Recette physique | non exécutée | aucune DAW, aucun contrôleur MIDI, aucun Mac physique : procédure §9 préparée, **non exécutée** |

## 8. Arbitrages nécessaires

| ID | Décision | Options et conséquences |
|---|---|---|
| D-LIC-1 | Licence JUCE pour la compilation locale privée | AGPLv3 ou Starter (gratuites) ; à déclarer au script (`--juce-licence`) |
| D-LIC-2 | Licence et canal de diffusion publique | inchangé (rapport G4.1, étude de licence) |
| SPEC-HR-01 | Adopter la protection > +24 dBFS | adoptée : PASS G-15/DST-09 confirmés ; refusée : retour à FAIL. Plus ELP-01 : accepter le plateau numérique ou variante algébrique |
| A-G42-01 | D2 Feedback à Position = 0 | (H) conserver l'historique (inaudible à Position 0) ; (C1) régénération dans la boucle (audible, fb = 0 identique, change le son des projets fb > 0) — adoption : paramètre de mode append-only ou défaut versionné ; (C2) réancrage glissant du segment (écho de période Size ; change aussi fb = 0) — non implémentée |
| A-G42-02 | Autorisation persistée ? | aujourd'hui jamais persistée (redemandée à chaque ouverture) ; option : préférence d'instance du projet (comme `drone_restore`) |
| A-G42-03 | Panic sans armement | aujourd'hui l'autorisation n'est retirée que si un armement est configuré ; option : toujours |
| A-G42-04 | AUT-02 (c) | accepter AUT-02c comme preuve de restitution exacte (R50 → PASS) ou exiger un autre protocole |
| B-01 | Catalogue Z | options A–D (`docs/V1-ARCH-G4.2-Z-PROPOSALS.md`) ; R19 BLOCKED en attendant |
| B-03, A-G41-05 | Enveloppes, balance | inchangés (provisoires) |

## 9. Procédure de recette Mac sécurisée (préparée, NON exécutée par l'agent)

1. Mac Apple Silicon (M4 Max de recette), macOS 14+. Outils gratuits : `xcode-select --install`, `brew install cmake ninja`.
2. Cloner le dépôt, `git checkout` du SHA livré (§5), `git status` vide.
3. Choisir **vous-même** la licence JUCE (D-LIC-1), puis :
   `tools/local-build/build_local_macos.sh --juce-licence=<agplv3|starter|autre>`.
4. Le script s'arrête au premier échec (codes 2 à 8). En cas de succès : livraison dans `local-build/delivery-…`
   (ZIP, Installer.pkg, Uninstaller.pkg, README_FR, CHANGELOG, SHA256SUMS vérifié) et rapport `local-build/out-…`
   (`machine.txt`, `results.txt`, `perf_this_machine.txt` = mesures de **cette** machine).
5. Retirer un éventuel doublon signalé (`~/Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3`, ancienne option `--install`).
6. Installer en ouvrant `SUBDESTRUCKT_Installer.pkg` (contrôles, sauvegarde/restauration de la version présente).
7. Dérouler la recette du manuel 0.3.1 (25 tests, assistant `recette.html`, export JSON), notamment R-11 (MIDI Learn,
   contrôleur réel), R-13, R-16, R-21, R-25 (états autonomes).
8. Désinstaller avec `SUBDESTRUCKT_Uninstaller.pkg` (R-17), réinstaller (R-18). Ne publier aucun fichier produit.

## 10. Confirmations

- **Aucune fusion vers `main`** ; une seule branche ; aucun sous-agent, aucun worktree parallèle.
- **Aucune publication de binaire** : paquets construits et testés en CI puis non publiés ; artefacts CI limités aux
  journaux (l'exécutable du contrôle négatif RTSan, publié dans les artefacts jusqu'au run 34, ne l'est plus).
- **Aucune dépense**, aucun compte créé, aucune licence choisie ni condition acceptée au nom du propriétaire.
- Manuel 0.3.1 : construit et testé dans le dépôt ; **non publié sur gh-pages** (publication à la demande du propriétaire).
- ARCH-UI-04, Design System JUCE final et Symbiotic Core 3D : **non commencés**.
