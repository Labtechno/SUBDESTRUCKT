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
SUBDESTRUCKT ${BUILD_ID} — macOS Apple Silicon (arm64) — BUILD INTERNE ARCH-G4.3-R1
================================================================================

Version            : ${VERSION} (build ${BUILD}, identite ${BUILD_ID})
Commit source      : ${COMMIT}
Format             : plugin instrument VST3 uniquement, arm64 natif (pas d'AU, pas de standalone, pas d'Intel/Rosetta)
Construction       : GitHub Actions, runner macos-15 arm64 (machine virtuelle), ${RUN_URL}

STATUT : livraison interne candidate ARCH-G4.3-R1 (correction DSP finale, recette physique en attente). Ce n'est NI une beta NI une version
publique. DIFFUSION BINAIRE PUBLIQUE BLOQUEE (licence JUCE non arbitree, pas de signature Developer ID ni de notarisation).

1. Systemes et hotes testes
---------------------------
- Teste automatiquement sur : ${VM_OS}, machine virtuelle Apple Silicon de GitHub Actions (pas un Mac physique).
- Hotes utilises : hote VST3 de test sans interface (hebergement VST3 de JUCE 9.0.6) et pluginval 1.0.4.
- AUCUNE DAW n'a ete testee. Aucun test sur Mac physique (M4 Max, M3) n'a encore ete realise : recette utilisateur
  en attente (installation, decouverte dans la DAW, son, MIDI, CROSS, rechargement de projet, desinstallation).
- Resultats automatiques de cette build : ${n_pass} criteres PASS, ${n_fail} ECHEC (detail : docs/V1-ARCH-G4-REPORT.md
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
Les etats du plugin sont enregistres dans vos projets DAW. Donnees personnelles creees par le plugin (jamais
supprimees par le desinstallateur) : presets utilisateur et bibliotheque (~/Library/Application Support/Labtechno/
SUBDESTRUCKT/Presets) et correspondances MIDI Learn (~/Library/Application Support/Labtechno/SUBDESTRUCKT/midi_learn.txt).
L'installateur et le desinstallateur ne touchent ni aux projets, ni aux presets, ni aux preferences, ni aux autres
plugins. Seul /Library/Application Support/Labtechno/SUBDESTRUCKT/install-manifest.txt est cree par l'installation
(et retire par la desinstallation).

6. Contenu de la build et limitations connues
---------------------------------------------
Contenu (fonctions implementees et testees automatiquement ; recette utilisateur sur Mac a venir) :
- Voix mono Mono/Legato, priorites derniere/basse/haute, sustain, pitch bend Up/Down, pression, CC1/2/11/64,
  accordage A4 et microtuning 12 classes, Drone (desarme par defaut, restauration sur autorisation), Panic.
- 5 enveloppes DAHDSR lineaires/exponentielles (Loop), 4 LFO (8 formes, tempo, phase), aleatoires, chaos (Lorenz).
- Wavetable XYZ : banques 64x16 (7 banques usine + import WAV), Clamp/Wrap/Mirror, Linear/Stepped, Clean/Raw,
  Linear/Cubic, phases, sync dure/douce, Character, Z deux emplacements (Harmonic Fold, Phase Warp, Spectral Mutation).
- FM 4 operateurs PM/LinFM/TZFM/ExpFM, 9 formes, ratio/fixe/offset, matrice 4x4, 10 topologies, AC/DC, mute/disable.
- CROSS : 55 routes avec contrat causal B-T2 sticky, bus FM, directions, Cross Audio Mix/Crossfade/Ring/AM, Cross Reset.
- Filter & Resonator F1-F4 (3 emplacements) et Destruction N1/D1/Y1/S1/N2/D2/Y2/S2 (6 emplacements), Pre/Post.
- Matrice globale 64 connexions (Via, courbes, audio-rate 16, macros M1-M4), Randomize/Mutate/Evolve, Undo/Redo,
  presets usine/utilisateur, Quality Eco/Normal/High, limiteur musical optionnel, UI Performance/Sound Design/Engineering.
Limitations : 5 transformations Z historiques non implementees (catalogue non retrouve) ; morph spectral non
implemente ; enveloppes non synchronisables au tempo (regles non arbitrees) ; balance WT/FM de facade non exposee
(semantique non arbitree) ; editeur graphique de formes absent ; repliement eleve de N1 en repli extreme (utiliser
Quality High) ; la garde numerique +30 dBFS peut s'engager avec des gains cumules extremes ; aucune promesse
d'absence de clic lors des changements structurels ; performances non certifiees sur M3/M4 ; graphisme provisoire.
La latence declaree depend de la configuration (decimation ; + taille FFT si S2 est actif).

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

${BUILD_ID} (commit ${COMMIT}) — correction DSP ARCH-G4.3-R1 (build interne, non diffusee)
-------------------------------------------------------------------------------------------
Wavetable Z
- Harmonic Tilt redistribue l'energie au lieu de l'amplifier : energie de la bande lue constante (meme sans Gain
  Compensation) et elevation de crete bornee a +6 dB par etage (reduction de gain continue, sans ecretage). Le son
  sombre/brillant est conserve ; le NIVEAU de Tilt change par rapport a 0.3.2 (plus de crete a +30..+79 dBFS).
- Harmonic Shift : balayage conserve ; extinction progressive aux bornes du spectre et phase interpolee continument
  (supprime des sauts lorsque la profondeur traverse certaines valeurs). Spectre d'amplitude inchange.
- Phase Scatter inchange.
Captures R2 : restitution entre frequences d'echantillonnage inchangee (recette A/B complementaire).
Compatibilite : 703 parametres, format d'etat 4, projets 0.3.x lus ; seuls les sons utilisant Tilt ou Shift changent.

0.3.2 — developpement integre ARCH-G4.3 (build interne, non diffusee)
---------------------------------------------------------------------
TRANSPORT AUTHORIZATION (a valider dans Reason 14 et Ableton Live, exports compris)
- Un projet ouvert a l'arret garde ses etats autonomes suspendus ; le lancement de la LECTURE (ou de
  l'enregistrement) les autorise automatiquement ; un export hors temps reel aussi. Bouton Autoriser conserve.
- PANIC (et CC120) revoque toujours l'autorisation ; elle ne revient qu'apres un ARRET puis une LECTURE, ou Autoriser.
- Le Drone restaure par un projet (drone_restore) est lui aussi soumis a cette autorisation.
- Captures R2 retenues et restituees exactement a l'autorisation ; restitution S2 entre frequences d'echantillonnage
  corrigee (niveau conserve).
- Panic purge aussi les lignes d'alignement de latence (residu audible corrige).
D2 Glitch Buffer : mode de Feedback par emplacement LEGACY / REGENERATIVE
- Nouvelles instances : REGENERATIVE (Feedback audible a Position 0) ; projets et presets anterieurs : LEGACY
  (rendu historique inchange). Format d'etat 4 (les versions anterieures refusent ces etats).
Wavetable Z : catalogue de huit transformations
- Ajouts : Harmonic Tilt, Harmonic Mask, Harmonic Focus, Harmonic Shift, Phase Scatter, et un reglage Shape par
  emplacement Z. Fold, Warp et Mutation inchanges. Harmonic Tilt brillant sans compensation de gain pouvait produire
  des niveaux tres eleves (corrige en 0.3.3).
Parametres : 703 (695 historiques inchanges + 8 ajoutes en fin) ; identifiants VST3 historiques inchanges.

0.3.1 — developpement integre ARCH-G4.2 (build interne, non diffusee)
---------------------------------------------------------------------
Securite des etats autonomes
- Projet DAW, duplication d'instance ou preset portant un armement autonome ou une capture : etats autonomes
  SUSPENDUS (rien ne joue seul) jusqu'au bouton Autoriser ; reglages et captures restaures a l'identique.
- PANIC suspend aussi les etats autonomes armes (Autoriser les relance) ; l'autorisation n'est jamais enregistree.
MIDI Learn
- Une commande <-> un reglage ; armements, reglages non automatisables et CC reservees (0, 6, 32, 38, 64, 96-101,
  120-127) refuses ; profil partage entre instances sans perte d'affectation.
Protection d'extreme niveau (SPEC-HR-01, toujours proposee)
- Marge portee sur l'asymptote : sortie float strictement < +30,10 dBFS (atteignait la garde en 0.3.0) ; identique
  bit a bit sous +24 dBFS.
Compilation locale
- Script durci : arret au premier echec, pluginval obligatoire, livraison complete verifiee, aucune installation.
Inchange
- Son, parametres (695 StableID), identifiants VST3, format d'etat 3 ; D2 Feedback a Position 0 inchange (etude).


0.3.0 — developpement integre ARCH-G4.1 (build interne, non diffusee)
---------------------------------------------------------------------
Ajouts
- Balance WT/FM (Performance) ; morph spectral X/Y ; editeur de formes, generateur procedural, bibliotheque .sdwt.
- Enveloppes : declenchement Voice/Note/Gate, synchronisation au tempo (regles provisoires).
- Captures Glitch D2 / Freeze S2 dans l'etat (integrite SHA-256), sauvegarde DAW autonome.
- N1 : suréchantillonnage interne par Quality (Eco x1, Normal x2, High x4) avec latence declaree et compensee.
- Protection d'extreme niveau SPEC-HR-01 (proposee) au-dela de +24 dBFS, comptee et signalee (ELP).
- 695 parametres hote (11 ajoutes en fin de registre, format d'etat 3).
Corrections
- Suiveurs d'enveloppe : temps Attack/Release desormais appliques (suiveurs instantanes auparavant).
- Modulation de Output Gain et du plafond du limiteur desormais effective.
Changements de son volontaires (projets 0.2.0)
- N1 en Quality Normal/High (choisir Eco pour le N1 de 0.2.0) ; routes depuis les suiveurs ; modulation de sortie.
Securite des etats
- Un preset externe n'arme jamais un comportement autonome (armements remis a zero et signales).
Compatibilite
- Format d'etat 3 ; etats 1 et 2 migres (b5 : rendu identique, REG-01). Un etat 3 est refuse par 0.2.0 et 0.1.0-b5.

0.2.0 — developpement integre ARCH-G4 (build interne, non diffusee)
------------------------------------------------------------------
Ajouts
- Moteurs complets P0/P1 (voir README_FR.txt section 6) : WT XYZ, FM 4 operateurs, CROSS 55 routes B-T2,
  filtres F1-F4, destruction N1..S2, matrice globale 64, macros, Randomize/Mutate/Evolve, Undo/Redo, presets, Quality.
- 684 parametres hote (les 22 parametres de 0.1.0-b5 conservent leur identite et leur correspondance).
- UI fonctionnelle Performance / Sound Design / Engineering, Destination Inspector, moniteurs reels.
Regressions connues
- Aucune sur le perimetre 0.1.0-b5 : rendu bit a bit identique a l'oracle 0.1.0-b5 (test REG-01).
Compatibilite des etats et presets
- Format d'etat 2. Les etats 0.1.0-b5 (format 1) sont migres explicitement (rendu identique).
- Un etat ecrit par cette version (format 2) est REFUSE par 0.1.0-b5 (aucune lecture partielle).
Retour arriere
- Revenir a 0.1.0-b5 rend illisibles les projets enregistres avec cette version : conservez une copie.

0.1.0-b5 — premiere livraison installable (vertical slice ARCH-G3, build interne)
EOF

(cd "$STAGE" && shasum -a 256 SUBDESTRUCKT_Installer.pkg SUBDESTRUCKT_Uninstaller.pkg README_FR.txt CHANGELOG.txt > SHA256SUMS.txt)
(cd "$STAGE" && shasum -a 256 -c SHA256SUMS.txt)
rm -f "$DL/$NAME.zip"
(cd "$STAGE" && zip -X -q "../$NAME.zip" SUBDESTRUCKT_Installer.pkg SUBDESTRUCKT_Uninstaller.pkg README_FR.txt CHANGELOG.txt SHA256SUMS.txt)
(cd "$DL" && shasum -a 256 "$NAME.zip" > "$NAME.zip.sha256")
unzip -l "$DL/$NAME.zip"
cat "$DL/$NAME.zip.sha256"
echo "$NAME" > "$DL/NAME"
