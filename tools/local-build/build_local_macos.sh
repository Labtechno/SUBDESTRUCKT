#!/bin/bash
# SUBDESTRUCKT — compilation LOCALE privée et sécurisée sur Mac Apple Silicon (recette Mac Studio M4 Max).
# ARCH-G4.1 §13, durci en ARCH-G4.2 §7 (LB-01..07 : docs/V1-ARCH-G4.2-SPEC.md) et ARCH-G4.3 §8 (LB-08..13 :
# docs/V1-ARCH-G4.3-SPEC.md) : ordre compilation -> tests -> signature ad hoc -> identité -> pluginval -> paquets
# construits à partir du bundle VALIDÉ (empreinte vérifiée jusque dans la charge utile de l'installateur) -> ZIP
# réextrait et vérifié ; barrière exhaustive partagée avec la CI (tools/check_results.sh).
#
# But : produire sur la machine de l'utilisateur, à partir des sources EXACTES d'un commit, une LIVRAISON DE RECETTE
# complète (SUBDESTRUCKT_Installer.pkg, SUBDESTRUCKT_Uninstaller.pkg, README_FR.txt, CHANGELOG.txt, SHA256SUMS.txt,
# ZIP versionné), sans qu'aucun binaire ne soit publié (dépôt, artefact CI, release) : rien ne sort de local-build/.
#
# Garanties :
#   - aucune installation automatique : l'utilisateur installe lui-même avec Installer.pkg (parcours du contrat de
#     packaging : contrôles, sauvegarde et restauration de la version présente, désinstallation par Uninstaller.pkg) ;
#     les dossiers ~/Library/Audio/Plug-Ins et /Library/Audio/Plug-Ins ne sont jamais modifiés (doublons signalés) ;
#   - arrêt immédiat à tout échec bloquant : dépôt modifié, commit inconnu, dépendance non conforme à son épinglage,
#     test obligatoire en échec ou interrompu, identité du bundle incohérente, pluginval (rigueur 10) en échec ;
#     aucun paquet ni livraison n'est produit après un échec ;
#   - compilation reproductible : sources exportées du commit (git archive) dans local-build/, dépendances lues dans le
#     workflow de CE commit (JUCE tag + SHA, pluginval version + SHA-256), arbre JUCE propre ;
#   - suppressions récursives bornées à local-build/ (refus de tout autre chemin).
#
# Licence : ce script NE CHOISIT AUCUNE LICENCE et n'accepte aucune condition à la place de l'utilisateur. JUCE est
# distribué sous double licence (AGPLv3 ou EULA JUCE 9, dont le niveau gratuit « Starter ») ; voir
# docs/V1-ARCH-G4.1-LICENCE.md. L'option --juce-licence=agplv3|starter|autre enregistre seulement le choix DÉCLARÉ par
# l'utilisateur dans le rapport local ; le script refuse de s'exécuter sans elle. Aucun compte, aucune dépense.
#
# Usage : tools/local-build/build_local_macos.sh --juce-licence=<choix> [--commit=<sha>]
# Prérequis : macOS 14+, arm64, Xcode Command Line Tools, cmake >= 3.22, ninja (brew install cmake ninja), git, curl.
# Codes de sortie : 0 livraison prête ; 2 usage ; 3 dépôt/commit/chemins ; 4 dépendances/outils ; 5 tests ; 6 bundle ;
#                   7 pluginval ; 8 paquets/livraison ; 9 collision (sortie existante : aucun écrasement).
set -euo pipefail
trap 'echo "ECHEC $(basename "$0") ligne $LINENO : $BASH_COMMAND" >&2' ERR

die() { local code="$1"; shift; echo "ARRET ($code) : $*" >&2; exit "$code"; }

LICENCE="" ; COMMIT=""
for a in "$@"; do
  case "$a" in
    --juce-licence=*) LICENCE="${a#*=}" ;;
    --commit=*) COMMIT="${a#*=}" ;;
    --install) die 2 "--install est supprime : aucune installation automatique. Installez la livraison avec SUBDESTRUCKT_Installer.pkg." ;;
    --pluginval|--packages) echo "note : $a est obsolete (pluginval et paquets sont toujours executes et bloquants)" ;;
    -h|--help) sed -n '2,30p' "$0"; exit 0 ;;
    *) die 2 "option inconnue : $a" ;;
  esac
done
case "$LICENCE" in
  agplv3|starter|autre) ;;
  *) die 2 "indiquez la licence JUCE que VOUS avez choisie : --juce-licence=agplv3|starter|autre (voir docs/V1-ARCH-G4.1-LICENCE.md)" ;;
esac
[ "$(uname -s)" = "Darwin" ] && [ "$(uname -m)" = "arm64" ] || die 4 "macOS arm64 requis"
for t in git cmake ninja xcrun shasum curl unzip codesign plutil lipo pkgutil; do
  command -v "$t" >/dev/null || die 4 "outil manquant : $t (brew install cmake ninja ; xcode-select --install)"
done

# suite partielle interdite (LB-11) : une variable de filtrage des tests rendrait la livraison non probante
[ -z "${SBD_ONLY+x}" ] || die 5 "SBD_ONLY=${SBD_ONLY} defini : suite de tests partielle, livraison refusee (unset SBD_ONLY)"
ROOT="$(cd "$(dirname "$0")/../.." && pwd -P)"
WORK="$ROOT/local-build"      # ignoré par git (.gitignore) : rien n'est commité ni publié
[ -L "$WORK" ] && die 3 "local-build est un lien symbolique : refus (les sorties doivent rester dans le depot)"
mkdir -p "$WORK"
WORK="$(cd "$WORK" && pwd -P)"
[ "$WORK" = "$ROOT/local-build" ] || die 3 "local-build resolu hors du depot ($WORK) : refus"
fresh() {  # fresh <chemin> : sortie neuve obligatoire (aucun écrasement, aucun lien symbolique)
  [ -L "$1" ] && die 3 "sortie symbolique refusee : $1"
  [ -e "$1" ] && die 9 "sortie existante (collision de version/build) : $1 ; aucun ecrasement"
  mkdir -p "$1"
}

# suppression récursive bornée (LB-07) : uniquement un sous-chemin strict de local-build/
# shellcheck source=lib_safe_rm.sh
. "$(cd "$(dirname "$0")" && pwd -P)/lib_safe_rm.sh"

# ---- 1. dépôt et commit (LB-02) : arbre propre, commit existant ; export des sources exactes du commit
git -C "$ROOT" rev-parse --is-inside-work-tree >/dev/null 2>&1 || die 3 "$ROOT n'est pas un depot git"
dirty="$(git -C "$ROOT" status --porcelain --untracked-files=all -- . ':(exclude)local-build' 2>/dev/null || true)"
[ -z "$dirty" ] || die 3 "depot modifie (compilation non reproductible) : validez ou retirez ces changements :
$dirty"
[ -n "$COMMIT" ] || COMMIT="HEAD"
COMMIT="$(git -C "$ROOT" rev-parse --verify --quiet "$COMMIT^{commit}")" || die 3 "commit inconnu : ${COMMIT}"
SHORT="${COMMIT:0:12}"
SRC="$WORK/src-$SHORT"
[ -e "$SRC" ] && safe_rm "$SRC"
mkdir -p "$SRC"
git -C "$ROOT" archive --format=tar "$COMMIT" | tar -x -C "$SRC"

WF="$SRC/.github/workflows/arch-g4.yml"
[ -f "$WF" ] || die 4 "workflow absent du commit $SHORT : epinglages introuvables"
pin() { sed -n "s/^ *$1: \"\(.*\)\"/\1/p" "$WF" | head -1; }
JUCE_TAG="$(pin JUCE_TAG)"; JUCE_SHA="$(pin JUCE_SHA)"; VERSION="$(pin VERSION)"; MIN_MACOS="$(pin MIN_MACOS)"
PV_VER="$(pin PLUGINVAL_VERSION)"; PV_SHA="$(pin PLUGINVAL_SHA256)"
for v in JUCE_TAG JUCE_SHA VERSION MIN_MACOS PV_VER PV_SHA; do [ -n "${!v}" ] || die 4 "epinglage $v absent du workflow du commit"; done
BUILD="$(date +%y%j%H%M)"     # numérique (CFBundleVersion, version de paquet) ; identité : $VERSION-b$BUILD
OUT="$WORK/out-$SHORT-$BUILD"
fresh "$OUT"
{
  echo "SUBDESTRUCKT compilation locale privee (non publiee)"
  echo "date        : $(date -u +%FT%TZ)"
  echo "commit      : $COMMIT (sources exportees : $SRC)"
  echo "version     : $VERSION (build $BUILD, identite $VERSION-b$BUILD)"
  echo "machine     : $(sysctl -n machdep.cpu.brand_string 2>/dev/null || echo n/a) ; macOS $(sw_vers -productVersion) ; $(sysctl -n hw.ncpu 2>/dev/null || echo n/a) coeurs"
  echo "outils      : $(cmake --version | head -1) ; ninja $(ninja --version) ; $(xcrun clang --version 2>/dev/null | head -1)"
  echo "licence JUCE declaree par l'utilisateur : $LICENCE"
} | tee "$OUT/machine.txt"

# ---- 2. dépendances épinglées (LB-05) : JUCE tag + SHA, arbre propre
JUCE="$WORK/JUCE-$JUCE_TAG"
if [ ! -d "$JUCE/.git" ]; then git clone --quiet --depth 1 --branch "$JUCE_TAG" https://github.com/juce-framework/JUCE.git "$JUCE"; fi
[ "$(git -C "$JUCE" rev-parse HEAD)" = "$JUCE_SHA" ] || die 4 "JUCE : SHA $(git -C "$JUCE" rev-parse HEAD) different de l'epinglage $JUCE_SHA"
[ -z "$(git -C "$JUCE" status --porcelain --untracked-files=all)" ] || die 4 "JUCE : arbre modifie ($JUCE) : supprimez ce dossier pour un clone propre"
echo "JUCE $JUCE_TAG @ $JUCE_SHA verifie (arbre propre)" | tee -a "$OUT/machine.txt"

# ---- 3. compilation
BLD="$WORK/build-$SHORT"
cmake -S "$SRC" -B "$BLD" -G Ninja -DCMAKE_BUILD_TYPE=Release -DSUBDESTRUCKT_JUCE_DIR="$JUCE" \
  -DSUBDESTRUCKT_BUILD_NUMBER="$BUILD" -DSUBDESTRUCKT_COMMIT="$COMMIT" > "$OUT/configure.log" 2>&1 || die 4 "configuration CMake en echec ($OUT/configure.log)"
cmake --build "$BLD" > "$OUT/build.log" 2>&1 || die 4 "compilation en echec ($OUT/build.log)"

# ---- 4. tests obligatoires et bloquants (LB-03, LB-10) : mêmes exécutables et même barrière que la CI
set +e
"$BLD/subdestruckt_core_tests" > "$OUT/core_tests.log" 2>&1; core=$?
(cd "$SRC" && "$BLD/subdestruckt_wrapper_tests_artefacts/Release/subdestruckt_wrapper_tests" tests/host/expected_vst3_param_ids.txt "$OUT/ui.png") > "$OUT/wrapper_tests.log" 2>&1; wrap=$?
bash "$SRC/tools/check_results.sh" "$SRC/tests/expected_results.txt" "$SRC/tests/known_failures.txt" \
  "core=$OUT/core_tests.log:$core" "wrapper=$OUT/wrapper_tests.log:$wrap" > "$OUT/gate.log" 2>&1; gate=$?
set -e
cat "$OUT/gate.log"
grep -h "^RESULT " "$OUT/core_tests.log" "$OUT/wrapper_tests.log" > "$OUT/results.txt" || true
echo "criteres : $(grep -c ' PASS ' "$OUT/results.txt" || true) PASS, $(grep -c ' FAIL ' "$OUT/results.txt" || true) FAIL"
[ "$gate" = 0 ] || die 5 "barriere des tests refusee ($OUT/gate.log) : aucune livraison produite"
grep -E "^RESULT (PERF-0|ZC-17)" "$OUT/results.txt" > "$OUT/perf_this_machine.txt" || true

# ---- 5. bundle de recette : préparé et signé ad hoc, identité vérifiée (LB-05, LB-08)
RAW="$BLD/SUBDESTRUCKT_artefacts/Release/VST3/SUBDESTRUCKT.vst3"
[ -d "$RAW" ] || die 6 "bundle absent : $RAW"
VALD="$WORK/validated-$SHORT-$BUILD"
fresh "$VALD"
bash "$SRC/packaging/build_packages.sh" --prepare-only "$RAW" "$VALD" "$VERSION" "$BUILD" "$COMMIT" "$MIN_MACOS" > "$OUT/prepare.log" 2>&1 \
  || die 6 "preparation du bundle en echec ($OUT/prepare.log)"
B="$VALD/signed/SUBDESTRUCKT.vst3"
pl() { plutil -extract "$1" raw -o - "$B/Contents/Info.plist" 2>/dev/null || true; }
bad=""
[ "$(pl CFBundleIdentifier)" = "com.labtechno.subdestruckt" ] || bad="$bad identifiant=$(pl CFBundleIdentifier)"
[ "$(pl CFBundleShortVersionString)" = "$VERSION" ] || bad="$bad version=$(pl CFBundleShortVersionString)"
[ "$(pl CFBundleVersion)" = "$BUILD" ] || bad="$bad build=$(pl CFBundleVersion)"
[ "$(pl SUBDESTRUCKTCommit)" = "$COMMIT" ] || bad="$bad commit=$(pl SUBDESTRUCKTCommit)"
[ "$(lipo -archs "$B/Contents/MacOS/SUBDESTRUCKT" 2>/dev/null || true)" = "arm64" ] || bad="$bad architectures=$(lipo -archs "$B/Contents/MacOS/SUBDESTRUCKT" 2>/dev/null || echo '?')"
codesign --verify --strict "$B" >/dev/null 2>&1 || bad="$bad signature-ad-hoc-invalide"
[ -z "$bad" ] || { safe_rm "$VALD"; die 6 "identite du bundle incoherente :$bad"; }
treehash() { (cd "$1" && find . -type f -print | LC_ALL=C sort | while read -r f; do printf '%s ' "$f"; shasum -a 256 < "$f" | awk '{print $1}'; done) | shasum -a 256 | awk '{print $1}'; }
HVAL="$(treehash "$B")"
echo "bundle de recette : com.labtechno.subdestruckt $VERSION build $BUILD, commit $SHORT, arm64, signe ad hoc ; empreinte d'arbre $HVAL" | tee -a "$OUT/machine.txt"

# ---- 6. pluginval obligatoire et bloquant sur CE bundle (LB-04, LB-08), empreinte de l'outil vérifiée
PVD="$WORK/tools/pluginval-$PV_VER"
PVZ="$WORK/tools/pluginval-$PV_VER.zip"
mkdir -p "$WORK/tools"
[ -f "$PVZ" ] || curl -sfL -o "$PVZ" "https://github.com/Tracktion/pluginval/releases/download/$PV_VER/pluginval_macOS.zip" || die 4 "telechargement de pluginval impossible"
echo "$PV_SHA  $PVZ" | shasum -a 256 -c - >/dev/null 2>&1 || { safe_rm "$PVZ"; die 4 "pluginval : empreinte SHA-256 inattendue (fichier supprime)"; }
[ -e "$PVD" ] && safe_rm "$PVD"
unzip -q -o "$PVZ" -d "$PVD"
set +e
"$PVD/pluginval.app/Contents/MacOS/pluginval" --strictness-level 10 --timeout-ms 900000 --validate "$B" > "$OUT/pluginval.log" 2>&1; pv=$?
set -e
[ "$pv" = 0 ] || { safe_rm "$VALD"; die 7 "pluginval $PV_VER rigueur 10 en echec (code $pv, $OUT/pluginval.log) : aucun paquet ni livraison"; }
[ "$(treehash "$B")" = "$HVAL" ] || { safe_rm "$VALD"; die 7 "bundle modifie pendant pluginval : refus"; }
echo "pluginval $PV_VER rigueur 10 : PASS (bundle $HVAL)" | tee -a "$OUT/machine.txt"

# ---- 7. paquets construits à partir du bundle validé, sans modification (LB-08)
PKG="$WORK/packages-$SHORT-$BUILD"
DL="$WORK/delivery-$SHORT-$BUILD"
fresh "$PKG"
[ -e "$DL" ] && die 9 "sortie existante (collision de version/build) : $DL ; aucun ecrasement"
fail_pkg() { safe_rm "$PKG"; [ -e "$DL" ] && safe_rm "$DL"; die 8 "$1"; }
bash "$SRC/packaging/build_packages.sh" --prepared "$B" "$PKG" "$VERSION" "$BUILD" "$COMMIT" "$MIN_MACOS" > "$OUT/packaging.log" 2>&1 \
  || fail_pkg "construction des paquets en echec ($OUT/packaging.log)"
[ "$(treehash "$PKG/signed/SUBDESTRUCKT.vst3")" = "$HVAL" ] || fail_pkg "bundle empaquete different du bundle valide"
XP="$PKG/expanded"
pkgutil --expand-full "$PKG/SUBDESTRUCKT_Installer.pkg" "$XP" >/dev/null 2>&1 || fail_pkg "Installer.pkg illisible (pkgutil --expand-full)"
PB="$(find "$XP" -type d -name SUBDESTRUCKT.vst3 -prune | head -1)"
[ -n "$PB" ] && [ "$(treehash "$PB")" = "$HVAL" ] || fail_pkg "charge utile de Installer.pkg differente du bundle valide"
safe_rm "$XP"
echo "paquets : charge utile de l'installateur = bundle valide ($HVAL)" | tee -a "$OUT/machine.txt"

# ---- 8. livraison complète et vérifiée (LB-06, LB-09, LB-12)
echo "RESULT LOCAL-PLUGINVAL PASS pluginval $PV_VER rigueur 10 (compilation locale, bundle $HVAL)" >> "$OUT/results.txt"
bash "$SRC/packaging/make_delivery.sh" "$PKG" "$DL" "$VERSION" "$BUILD" "$COMMIT" "$MIN_MACOS" "$OUT/results.txt" \
  "compilation locale privee (non publiee)" "macOS $(sw_vers -productVersion) (machine locale)" > "$OUT/delivery.log" 2>&1 \
  || fail_pkg "assemblage de la livraison en echec ($OUT/delivery.log)"
NAME="$(cat "$DL/NAME")"
ZIP="$DL/$NAME.zip"
expected="CHANGELOG.txt README_FR.txt SHA256SUMS.txt SUBDESTRUCKT_Installer.pkg SUBDESTRUCKT_Uninstaller.pkg"
listed="$(unzip -Z1 "$ZIP" 2>/dev/null | sort | tr '\n' ' ' | sed 's/ $//')"
[ "$listed" = "$expected" ] || fail_pkg "contenu du ZIP inattendu : '$listed' (attendu '$expected')"
(cd "$DL/$NAME" && shasum -a 256 -c SHA256SUMS.txt >/dev/null 2>&1) || fail_pkg "SHA256SUMS.txt non verifie"
for f in SUBDESTRUCKT_Installer.pkg SUBDESTRUCKT_Uninstaller.pkg README_FR.txt CHANGELOG.txt SHA256SUMS.txt; do
  [ -s "$DL/$NAME/$f" ] || fail_pkg "element de livraison absent ou vide : $f"
done
# ZIP final réextrait : ce sont SES fichiers qui sont vérifiés (et non les fichiers d'origine)
ZX="$DL/zip-check"
mkdir -p "$ZX"
unzip -q "$ZIP" -d "$ZX" || fail_pkg "ZIP illisible"
(cd "$ZX" && shasum -a 256 -c SHA256SUMS.txt >/dev/null 2>&1) || fail_pkg "fichiers du ZIP final non conformes a SHA256SUMS.txt"
cmp -s "$ZX/SUBDESTRUCKT_Installer.pkg" "$PKG/SUBDESTRUCKT_Installer.pkg" || fail_pkg "Installer.pkg du ZIP different du paquet verifie"
safe_rm "$ZX"
safe_rm "$VALD"

# ---- 9. installations existantes : constat uniquement, jamais modifiées (doublons utilisateur / système)
found=0
SYS_VST3="${SBD_SYSTEM_VST3_DIR:-/Library/Audio/Plug-Ins/VST3}"  # surcharge réservée au harnais de test (lecture seule)
for d in "$HOME/Library/Audio/Plug-Ins/VST3" "$SYS_VST3"; do
  if [ -e "$d/SUBDESTRUCKT.vst3" ]; then
    found=$((found + 1))
    v="$(plutil -extract CFBundleShortVersionString raw -o - "$d/SUBDESTRUCKT.vst3/Contents/Info.plist" 2>/dev/null || echo '?')"
    echo "installation existante (non modifiee) : $d/SUBDESTRUCKT.vst3 version $v"
  fi
done
[ "$found" -lt 2 ] || echo "ATTENTION DOUBLON : SUBDESTRUCKT est present dans le dossier VST3 utilisateur ET systeme ; la DAW peut charger l'un ou l'autre. Retirez la copie utilisateur (~/Library/...) avant la recette ; l'installateur ne gere que /Library."

echo
echo "LIVRAISON DE RECETTE PRETE (non publiee) : $ZIP"
echo "Installation : ouvrez $DL/$NAME/SUBDESTRUCKT_Installer.pkg (aucune installation automatique par ce script)."
echo "Desinstallation : SUBDESTRUCKT_Uninstaller.pkg. Rapport local : $OUT"
echo "Ne publiez pas ces fichiers : diffusion bloquee tant que la licence JUCE n'est pas arbitree (docs/V1-ARCH-G4.1-LICENCE.md)."
