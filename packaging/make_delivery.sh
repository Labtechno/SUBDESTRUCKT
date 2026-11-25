#!/bin/bash
# SUBDESTRUCKT — assemblage de la livraison : README_FR.txt, CHANGELOG.txt, SHA256SUMS.txt, ZIP versionné.
# Usage : make_delivery.sh <dossier_paquets> <dossier_livraison> <version> <build> <commit> <macos_min> <resultats.txt> <url_run> <macos_vm>
set -euo pipefail
trap 'echo "ECHEC $(basename "$0") ligne $LINENO : $BASH_COMMAND" >&2' ERR
PK="$1"; DL="$2"; VERSION="$3"; BUILD="$4"; COMMIT="$5"; MIN_MACOS="$6"; RESULTS="$7"; RUN_URL="$8"; VM_OS="$9"
BUILD_ID="$VERSION-b$BUILD"
NAME="SUBDESTRUCKT_v${BUILD_ID}_macOS_arm64"
STAGE="$DL/$NAME"
rm -rf "$STAGE" && mkdir -p "$STAGE"
cp "$PK/SUBDESTRUCKT_Installer.pkg" "$PK/SUBDESTRUCKT_Uninstaller.pkg" "$STAGE/"

n_pass=$(grep -c "^RESULT .* PASS " "$RESULTS" || true)
n_fail=$(grep -c "^RESULT .* FAIL " "$RESULTS" || true)

cat > "$STAGE/README_FR.txt" <<EOF
SUBDESTRUCKT ${BUILD_ID} — macOS Apple Silicon (arm64) — BUILD INTERNE ARCH-G3
================================================================================

Version            : ${VERSION} (build ${BUILD}, identite ${BUILD_ID})
Commit source      : ${COMMIT}
Format             : plugin instrument VST3 uniquement, arm64 natif (pas d'AU, pas de standalone, pas d'Intel/Rosetta)
Construction       : GitHub Actions, runner macos-15 arm64 (machine virtuelle), ${RUN_URL}

STATUT : livraison interne candidate de la vertical slice ARCH-G3. Ce n'est NI une beta NI une version publique.

1. Systemes et hotes testes
---------------------------
- Teste automatiquement sur : ${VM_OS}, machine virtuelle Apple Silicon de GitHub Actions (pas un Mac physique).
- Hotes utilises : hote VST3 de test sans interface (hebergement VST3 de JUCE 9.0.6) et pluginval 1.0.4.
- AUCUNE DAW n'a ete testee. Aucun test sur Mac physique (M4 Max, M3) n'a encore ete realise : recette utilisateur
  en attente (installation, decouverte dans la DAW, son, MIDI, CROSS, rechargement de projet, desinstallation).
- Resultats automatiques de cette build : ${n_pass} criteres PASS, ${n_fail} ECHEC (detail : docs/V1-ARCH-G3-REPORT.md
  et journal du run CI).
- Cible materielle : Apple M3 minimum (non encore certifiee). M1/M2 hors support officiel, sans blocage.
- macOS ${MIN_MACOS} minimum DECLARE (valeur de compilation, non validee ; seul macOS de la VM CI a ete teste).

2. Installation
---------------
1. Fermez toute DAW qui utilise SUBDESTRUCKT.
2. Ouvrez SUBDESTRUCKT_Installer.pkg. Le paquet n'est pas signe : macOS indique qu'il provient d'un developpeur non
   identifie. Ouvrez Reglages Systeme > Confidentialite et securite, puis « Ouvrir quand meme » pour CE paquet.
   Ne desactivez jamais Gatekeeper globalement.
3. Saisissez votre mot de passe administrateur : le plugin est installe pour tous les utilisateurs dans
   /Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3
4. Relancez la DAW et relancez l'analyse des plugins VST3 si necessaire.
L'installateur refuse d'agir (sans rien modifier) si : le Mac n'est pas Apple Silicon, macOS est anterieur a
${MIN_MACOS}, l'emplacement est occupe par un element non identifie ou un lien symbolique, ou le plugin est charge par
une DAW ouverte. En cas d'echec de verification apres copie, l'installation anterieure est restauree.

3. Mise a jour
--------------
Installez simplement la nouvelle version par-dessus : la version presente est detectee et sauvegardee pendant
l'installation. Un retour a une version plus ancienne est signale par un avertissement : les etats enregistres par
une version plus recente peuvent etre refuses par une version plus ancienne (jamais lus partiellement).

4. Desinstallation
------------------
Ouvrez SUBDESTRUCKT_Uninstaller.pkg (meme procedure d'ouverture). Il retire uniquement le bundle SUBDESTRUCKT
identifie, son manifeste d'installation et son recu d'installation. Il refuse d'agir si le plugin est absent, si
l'element present n'est pas SUBDESTRUCKT, si la version installee est plus recente que lui, ou si une DAW utilise le
plugin. Journal : /var/log/install.log (lignes « SUBDESTRUCKT[ »).

5. Donnees conservees
---------------------
Aucune donnee personnelle n'est creee par cette build. Les etats du plugin sont enregistres dans vos projets DAW.
L'installateur et le desinstallateur ne touchent ni aux projets, ni aux presets, ni aux preferences, ni aux autres
plugins. Seul /Library/Application Support/Labtechno/SUBDESTRUCKT/install-manifest.txt est cree par l'installation
(et retire par la desinstallation).

6. Contenu de la tranche et limitations connues
-----------------------------------------------
Contenu : MIDI monophonique (priorite derniere note), un oscillateur wavetable minimal (dent de scie limitee en bande),
deux operateurs FM en modulation de phase, CROSS bidirectionnel avec contrat causal B-T2 (retard 1/48000 s sur les
routes Delayed, Thiran1 si fractionnaire), oversampling 2x permanent a 44,1 kHz, ampli ADSR stereo, preset initial,
UI de diagnostic (non definitive).
Limitations : pas de pitch bend, d'aftertouch ni de CC hors 120/123 ; pas de LinFM/TZFM/ExpFM, de filtres, de
destruction, d'axe Z, de matrice globale ni de banques ; topologie modifiable uniquement depuis l'UI de diagnostic
(pas d'undo/redo) ; aucune promesse d'absence de clic lors des changements de topologie ; aucune performance CPU
mesuree ni certifiee sur M3/M4 ; latence declaree 32 echantillons a 44,1 kHz (0 ailleurs).
Le wrapper VST3 de JUCE prend son verrou interne de rappel audio autour du traitement (comportement de JUCE).

7. Signature et notarisation
----------------------------
- Bundle VST3 : signature AD HOC uniquement (pas de certificat Developer ID).
- Paquets : NON signes. Notarisation Apple : NON effectuee.
- Consequence : livraison interne uniquement ; distribution publique BLOQUEE tant que la signature Developer ID et la
  notarisation ne sont pas disponibles.

8. Licences
-----------
JUCE 9.0.6 : AGPLv3 ou licence commerciale JUCE 9 — choix NON arbitre. VST3 SDK 3.8.0 : MIT. Voir
THIRD_PARTY_NOTICES.txt dans le bundle (Contents/Resources).
EOF

cat > "$STAGE/CHANGELOG.txt" <<EOF
SUBDESTRUCKT — journal des modifications
========================================

${BUILD_ID} (commit ${COMMIT}) — premiere livraison installable (vertical slice ARCH-G3, build interne)
------------------------------------------------------------------------------------------------------
Ajouts
- Plugin instrument VST3 arm64 natif : MIDI mono, WT minimal, 2 operateurs FM (PM), CROSS bidirectionnel B-T2,
  ampli ADSR stereo, preset initial, UI de diagnostic.
- Graphe FM/CROSS avec classification causale sticky et persistante (routes Immediate/Delayed, StableRouteID).
- Etat versionne (format 1) : valeurs Base par StableID, topologie (identifiants, classes) ; validation complete au
  chargement, refus des etats invalides ou d'une version future.
- Installateur et desinstallateur .pkg (installation systeme, desinstallation ciblee, donnees conservees).
Regressions connues
- Aucune (premiere version).
Compatibilite des etats et presets
- Format d'etat 1. Premiere version : aucun etat anterieur a migrer.
- Un etat ecrit par une version future (format > 1) est refuse explicitement par cette version.
Retour arriere
- Sans objet (aucune version anterieure livree).
EOF

(cd "$STAGE" && shasum -a 256 SUBDESTRUCKT_Installer.pkg SUBDESTRUCKT_Uninstaller.pkg README_FR.txt CHANGELOG.txt > SHA256SUMS.txt)
(cd "$STAGE" && shasum -a 256 -c SHA256SUMS.txt)
rm -f "$DL/$NAME.zip"
(cd "$STAGE" && zip -X -q "../$NAME.zip" SUBDESTRUCKT_Installer.pkg SUBDESTRUCKT_Uninstaller.pkg README_FR.txt CHANGELOG.txt SHA256SUMS.txt)
(cd "$DL" && shasum -a 256 "$NAME.zip" > "$NAME.zip.sha256")
unzip -l "$DL/$NAME.zip"
cat "$DL/$NAME.zip.sha256"
echo "$NAME" > "$DL/NAME"
