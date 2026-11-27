#!/bin/bash
# SUBDESTRUCKT — compilation LOCALE privée sur Mac Apple Silicon (recette Mac Studio M4 Max), ARCH-G4.1 §13.
#
# But : obtenir un VST3 de recette sans qu'aucun binaire ne soit publié (dépôt public, artefact CI, release) : le
# binaire est produit sur la machine de l'utilisateur, à partir des sources du commit choisi, et n'en sort pas.
#
# Licence : ce script NE CHOISIT AUCUNE LICENCE et n'accepte aucune condition à la place de l'utilisateur. JUCE est
# distribué sous double licence (AGPLv3 ou EULA JUCE 9, dont le niveau gratuit « Starter »). Le téléchargement et
# l'utilisation de JUCE supposent que l'utilisateur ait lui-même choisi l'une de ces licences (voir
# docs/V1-ARCH-G4.1-LICENCE.md). Le script exige donc l'option --juce-licence=agplv3|starter|autre, qui ne fait
# qu'enregistrer le choix déclaré par l'utilisateur dans le rapport local ; il refuse de s'exécuter sans elle.
#
# Usage : tools/local-build/build_local_macos.sh --juce-licence=<choix> [--commit=<sha>] [--install] [--pluginval] [--packages]
#   --install    copie le bundle (signé ad hoc) dans ~/Library/Audio/Plug-Ins/VST3 (utilisateur courant, sans sudo)
#   --pluginval  télécharge pluginval v1.0.4 (empreinte vérifiée) et valide en rigueur 10
#   --packages   construit Installer.pkg / Uninstaller.pkg et la livraison ZIP (README_FR, CHANGELOG, SHA256SUMS) en local
# Prérequis : macOS 14+, arm64, Xcode Command Line Tools, cmake >= 3.22, ninja (brew install cmake ninja), git.
# Aucune dépense, aucun compte requis par ce script.
set -euo pipefail
trap 'echo "ECHEC $(basename "$0") ligne $LINENO : $BASH_COMMAND" >&2' ERR

LICENCE="" ; COMMIT="" ; INSTALL=0 ; PLUGINVAL=0 ; PACKAGES=0
for a in "$@"; do
  case "$a" in
    --juce-licence=*) LICENCE="${a#*=}" ;;
    --commit=*) COMMIT="${a#*=}" ;;
    --install) INSTALL=1 ;;
    --pluginval) PLUGINVAL=1 ;;
    --packages) PACKAGES=1 ;;
    -h|--help) sed -n '2,22p' "$0"; exit 0 ;;
    *) echo "option inconnue : $a" >&2; exit 2 ;;
  esac
done
case "$LICENCE" in
  agplv3|starter|autre) ;;
  *) echo "Indiquez la licence JUCE que VOUS avez choisie : --juce-licence=agplv3|starter|autre (voir docs/V1-ARCH-G4.1-LICENCE.md)." >&2; exit 2 ;;
esac
[ "$(uname -s)" = "Darwin" ] && [ "$(uname -m)" = "arm64" ] || { echo "macOS arm64 requis" >&2; exit 1; }
for t in git cmake ninja xcrun shasum; do command -v "$t" >/dev/null || { echo "outil manquant : $t (brew install cmake ninja ; xcode-select --install)" >&2; exit 1; }; done

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"
[ -n "$COMMIT" ] && git checkout --quiet "$COMMIT"
COMMIT="$(git rev-parse HEAD)"
WF=.github/workflows/arch-g4.yml
JUCE_TAG="$(sed -n 's/^ *JUCE_TAG: "\(.*\)"/\1/p' "$WF")"
JUCE_SHA="$(sed -n 's/^ *JUCE_SHA: "\(.*\)"/\1/p' "$WF")"
VERSION="$(sed -n 's/^ *VERSION: "\(.*\)"/\1/p' "$WF")"
MIN_MACOS="$(sed -n 's/^ *MIN_MACOS: "\(.*\)"/\1/p' "$WF")"
PV_VER="$(sed -n 's/^ *PLUGINVAL_VERSION: "\(.*\)"/\1/p' "$WF")"
PV_SHA="$(sed -n 's/^ *PLUGINVAL_SHA256: "\(.*\)"/\1/p' "$WF")"
BUILD="local$(date +%Y%m%d%H%M)"
WORK="$ROOT/local-build"            # ignoré par git (.gitignore) : rien n'est commité ni publié
OUT="$WORK/out"
mkdir -p "$OUT"
{
  echo "SUBDESTRUCKT compilation locale privee"
  echo "date        : $(date -u +%FT%TZ)"
  echo "commit      : $COMMIT"
  echo "version     : $VERSION (build $BUILD)"
  echo "machine     : $(sysctl -n machdep.cpu.brand_string) ; macOS $(sw_vers -productVersion) ; $(sysctl -n hw.ncpu) coeurs"
  echo "licence JUCE declaree par l'utilisateur : $LICENCE"
} | tee "$OUT/machine.txt"

# JUCE épinglé (tag + SHA vérifié), comme la CI
if [ ! -d "$WORK/JUCE/.git" ]; then git clone --quiet --depth 1 --branch "$JUCE_TAG" https://github.com/juce-framework/JUCE.git "$WORK/JUCE"; fi
test "$(git -C "$WORK/JUCE" rev-parse HEAD)" = "$JUCE_SHA" || { echo "JUCE : SHA inattendu" >&2; exit 1; }
echo "JUCE $JUCE_TAG @ $JUCE_SHA verifie" | tee -a "$OUT/machine.txt"

cmake -S . -B "$WORK/build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DSUBDESTRUCKT_JUCE_DIR="$WORK/JUCE" \
  -DSUBDESTRUCKT_BUILD_NUMBER="$BUILD" -DSUBDESTRUCKT_COMMIT="$COMMIT" > "$OUT/configure.log"
cmake --build "$WORK/build" 2>&1 | tee "$OUT/build.log" | tail -3

# Tests (mêmes exécutables que la CI) ; les mesures PERF-01..03 de ce rapport sont celles de CETTE machine
"$WORK/build/subdestruckt_core_tests" > "$OUT/core_tests.log" 2>&1 || true
"$WORK/build/subdestruckt_wrapper_tests_artefacts/Release/subdestruckt_wrapper_tests" tests/host/expected_vst3_param_ids.txt "$OUT/ui.png" > "$OUT/wrapper_tests.log" 2>&1 || true
grep -h "^RESULT " "$OUT"/*_tests.log > "$OUT/results.txt" || true
echo "criteres : $(grep -c ' PASS ' "$OUT/results.txt" || true) PASS, $(grep -c ' FAIL ' "$OUT/results.txt" || true) FAIL"
grep " FAIL " "$OUT/results.txt" || true
grep -E "^RESULT PERF-0" "$OUT/results.txt" > "$OUT/perf_this_machine.txt" || true

RAW="$WORK/build/SUBDESTRUCKT_artefacts/Release/VST3/SUBDESTRUCKT.vst3"
SIGNED="$WORK/SUBDESTRUCKT.vst3"
rm -rf "$SIGNED" && cp -R "$RAW" "$SIGNED"
codesign --force --deep -s - "$SIGNED"     # signature ad hoc locale (aucun certificat, aucune notarisation)
codesign --verify --strict "$SIGNED" && echo "bundle signe ad hoc : $SIGNED"

if [ "$PLUGINVAL" = 1 ]; then
  curl -sfL -o "$WORK/pluginval.zip" "https://github.com/Tracktion/pluginval/releases/download/$PV_VER/pluginval_macOS.zip"
  echo "$PV_SHA  $WORK/pluginval.zip" | shasum -a 256 -c -
  unzip -q -o "$WORK/pluginval.zip" -d "$WORK/pluginval"
  "$WORK/pluginval/pluginval.app/Contents/MacOS/pluginval" --strictness-level 10 --timeout-ms 900000 --validate "$SIGNED" > "$OUT/pluginval.log" 2>&1 \
    && echo "pluginval rigueur 10 : PASS" || echo "pluginval rigueur 10 : FAIL (voir $OUT/pluginval.log)"
fi

if [ "$PACKAGES" = 1 ]; then
  bash packaging/build_packages.sh "$RAW" "$WORK/packages" "$VERSION" "$BUILD" "$COMMIT" "$MIN_MACOS"
  bash packaging/make_delivery.sh "$WORK/packages" "$WORK/delivery" "$VERSION" "$BUILD" "$COMMIT" "$MIN_MACOS" "$OUT/results.txt" "compilation locale" "$(sw_vers -productVersion)"
  echo "livraison locale : $WORK/delivery (ne pas publier : voir docs/V1-ARCH-G4.1-LICENCE.md)"
fi

if [ "$INSTALL" = 1 ]; then
  DEST="$HOME/Library/Audio/Plug-Ins/VST3"
  mkdir -p "$DEST"
  rm -rf "$DEST/SUBDESTRUCKT.vst3"
  cp -R "$SIGNED" "$DEST/"
  echo "installe pour l'utilisateur courant : $DEST/SUBDESTRUCKT.vst3 (desinstallation : supprimer ce dossier)"
fi
echo "rapport local : $OUT"
