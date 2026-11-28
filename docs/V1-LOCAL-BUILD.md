# SUBDESTRUCKT — Compilation et installation privées sur Mac (0.4.1, ARCH-UI-04-R1)

Procédure de recette **privée** sur le Mac du propriétaire (Mac Studio M4 Max, VST3 arm64). Aucune diffusion : les
fichiers produits restent dans `local-build/` (ignoré par git) et ne doivent pas être publiés. Script :
`tools/local-build/build_local_macos.sh` (critères LB-01..13, harnais `tools/local-build/test_build_local.sh`).

## 1. Prérequis (gratuits)

macOS 14+, Apple Silicon ; `xcode-select --install` ; `brew install cmake ninja` ; git, curl. Aucun compte, aucune
dépense. Licence JUCE : **à choisir et accepter par vous-même** (option retenue sous réserve : Starter, après
vérification de votre éligibilité ; voir `docs/V1-ARCH-G4.1-LICENCE.md`). Le script n'accepte aucune condition à votre
place : il enregistre seulement le choix que vous déclarez.

## 2. Compiler

    git clone https://github.com/Labtechno/SUBDESTRUCKT.git && cd SUBDESTRUCKT
    git checkout <SHA communiqué>        # git status doit être vide
    unset SBD_ONLY                       # aucune suite partielle n'est livrable
    tools/local-build/build_local_macos.sh --juce-licence=<agplv3|starter|autre>

Le script : exporte les sources exactes du commit ; vérifie JUCE (tag + SHA, arbre propre) et pluginval (SHA-256)
d'après le workflow du commit ; compile ; exécute les suites cœur et wrapper **complètes** et applique la barrière
exhaustive (`tools/check_results.sh` : inventaire `tests/expected_results.txt`, échecs connus bornés à leur portée
`tests/known_failures.txt`) ; prépare et signe ad hoc le bundle, vérifie son identité ; exécute pluginval (rigueur 10) sur
**ce** bundle ; construit les paquets à partir de ce bundle sans le modifier et vérifie que la charge utile de
l'installateur lui est identique (empreinte d'arbre) ; assemble la livraison, réextrait le ZIP et vérifie chaque fichier
contre `SHA256SUMS.txt`. Il s'arrête au premier échec (codes : 2 usage, 3 dépôt/commit/chemins, 4 dépendances, 5 tests,
6 bundle, 7 pluginval, 8 paquets/livraison, 9 collision) et ne laisse alors aucune livraison présentée comme valide. Il
n'écrase jamais une sortie existante et refuse un `local-build/` symbolique.

## 3. Installer, désinstaller

1. Si le script signale un doublon, retirez `~/Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3` (ancienne option
   `--install`, supprimée depuis 0.3.1). Le script ne touche jamais à vos dossiers de plugins.
2. Ouvrez vous-même `local-build/delivery-…/SUBDESTRUCKT_v0.4.1-b…/SUBDESTRUCKT_Installer.pkg` (paquet non signé :
   Réglages Système › Confidentialité et sécurité › « Ouvrir quand même » pour CE paquet).
3. Désinstallation : `SUBDESTRUCKT_Uninstaller.pkg`.

## 4. Mesures de cette machine

`local-build/out-…/perf_this_machine.txt` (PERF-01..03, ZC-17) : seules ces mesures valent pour votre Mac ; les mesures
CI (machine virtuelle) et Windows ne certifient ni M3 ni M4 Max.

## 5. Journal de recette TRANSPORT AUTHORIZATION

Créez le fichier vide `~/Library/Labtechno/SUBDESTRUCKT/ta_diagnostics.on` ; chaque instance écrit alors ses
événements (transport observé, hors temps réel, autorisations et leur source, suspensions, révocations, captures
appliquées) dans `~/Library/Labtechno/SUBDESTRUCKT/ta_log.txt` (chemin corrigé le 2026-11-27 :
`userApplicationDataDirectory` = `~/Library` sur macOS ; voir `docs/V1-DSP-FINAL-LOCK.md` §4). Supprimez le fichier d'activation après la recette.

## 6. Recette 0.3.3 (ARCH-G4.3-R1)

Ordre conseillé après installation : R-01..R-05 (installation, version 0.3.3 affichée), R-26 Reason 14, R-27 Ableton
Live, R-28 exports hors ligne (TRANSPORT AUTHORIZATION), R-29 écoute Z (Tilt avec et sans Gain Compensation, Shift),
R-30 écoute A/B des captures R2 (`docs/r1-audio/`). Résultats : bouton d'export du manuel 0.3.3
(`SUBDESTRUCKT_ARCH_G43R1_RESULTS.json`). Un hôte simulé ne remplace pas ces essais.

## 7. Recette 0.4.0 (ARCH-UI-04)

Moteur DSP identique à 0.3.3 (DSP FINAL LOCK). Recette humaine courte, groupée : `docs/V1-ARCH-UI-04-RECETTE.md`
(H-01..H-09, ~25 min, Reason 14.1). La section 6 est conservée pour l'historique 0.3.3 (R-27 retiré, R-28..R-30 non
exécutés, risques acceptés : `docs/V1-DSP-FINAL-LOCK.md`).

## 8. Validation physique 0.4.1 (ARCH-UI-04-R1, R-28)

Correction des exports hors temps réel (TRANSPORT AUTHORIZATION). Protocole de validation confié à Codex :
`docs/V1-ARCH-UI-04-R1-REPORT.md` §9 (export avant PLAY puis annulation, export complet, ARRÊT → PLAY, Panic pendant
l'export, autorisation préalable conservée, journal `ta_log.txt`). R-28 n'est pas déclaré PASS avant ce retour.
