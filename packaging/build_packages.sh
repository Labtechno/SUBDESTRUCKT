#!/bin/bash
# SUBDESTRUCKT — construction de SUBDESTRUCKT_Installer.pkg et SUBDESTRUCKT_Uninstaller.pkg (macOS, outils Apple).
# Usage : packaging/build_packages.sh <bundle.vst3> <dossier_sortie> <version> <build> <commit> <macos_min>
# Sorties : <dossier_sortie>/SUBDESTRUCKT_Installer.pkg, SUBDESTRUCKT_Uninstaller.pkg, signed/SUBDESTRUCKT.vst3 (bundle
# livré, signé ad hoc), scripts/installer et scripts/uninstaller (scripts finaux, pour les tests en bac à sable).
# Aucun certificat : bundle signé ad hoc, paquets non signés, aucune notarisation (build interne).
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
SRC_BUNDLE="$1"; OUT="$2"; VERSION="$3"; BUILD="$4"; COMMIT="$5"; MIN_MACOS="$6"
BUILD_ID="$VERSION-b$BUILD"
PKG_VERSION="$VERSION.$BUILD"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

subst() {  # remplace les champs @...@ (version, build, commit, macOS minimal)
  sed -e "s|@VERSION@|$VERSION|g" -e "s|@BUILD@|$BUILD|g" -e "s|@BUILD_ID@|$BUILD_ID|g" -e "s|@COMMIT@|$COMMIT|g" \
      -e "s|@MIN_MACOS@|$MIN_MACOS|g" -e "s|@PKG_VERSION@|$PKG_VERSION|g" "$1" > "$2"
}
plist() { /usr/libexec/PlistBuddy -c "Print :$2" "$1/Contents/Info.plist"; }

# ---- 1. bundle livré : contrôles d'identité, mentions tierces, signature ad hoc
ROOT="$WORK/root"
B="$ROOT/Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3"
mkdir -p "$(dirname "$B")"
ditto "$SRC_BUNDLE" "$B"
cp "$HERE/THIRD_PARTY_NOTICES.txt" "$B/Contents/Resources/THIRD_PARTY_NOTICES.txt"
expect() {  # expect <clé> <valeur attendue>
  local got
  got="$(plist "$B" "$1" 2>/dev/null || true)"
  [ "$got" = "$2" ] || { echo "ECHEC Info.plist : $1 = '$got' (attendu '$2')"; exit 1; }
}
expect CFBundleIdentifier "com.labtechno.subdestruckt"
expect CFBundleShortVersionString "$VERSION"
expect CFBundleVersion "$BUILD"
expect SUBDESTRUCKTBuild "$BUILD_ID"
expect SUBDESTRUCKTCommit "$COMMIT"
archs="$(lipo -archs "$B/Contents/MacOS/SUBDESTRUCKT")"
[ "$archs" = "arm64" ] || { echo "ECHEC binaire : architectures '$archs' (attendu arm64 uniquement)"; exit 1; }
codesign --force --sign - --timestamp=none "$B"
codesign --verify --strict --verbose=2 "$B"
rm -rf "$OUT/signed" && mkdir -p "$OUT/signed" && ditto "$B" "$OUT/signed/SUBDESTRUCKT.vst3"

# ---- 2. scripts finaux (champs remplacés)
for kind in installer uninstaller; do
  d="$OUT/scripts/$kind"
  rm -rf "$d" && mkdir -p "$d"
  subst "$HERE/scripts/common.sh" "$d/common.sh"
  for f in "$HERE/scripts/$kind"/*; do subst "$f" "$d/$(basename "$f")"; done
  chmod 755 "$d"/*
done

# ---- 3. paquets composants
pkgbuild --analyze --root "$ROOT" "$WORK/component.plist"
/usr/libexec/PlistBuddy -c "Set :0:BundleIsRelocatable false" "$WORK/component.plist"   # jamais relocalisé ailleurs
/usr/libexec/PlistBuddy -c "Set :0:BundleIsVersionChecked false" "$WORK/component.plist" # le retour arrière est averti par la Distribution
/usr/libexec/PlistBuddy -c "Set :0:BundleOverwriteAction upgrade" "$WORK/component.plist"
/usr/libexec/PlistBuddy -c "Set :0:BundleHasStrictIdentifier true" "$WORK/component.plist"
mkdir -p "$WORK/pkgs"
pkgbuild --root "$ROOT" --component-plist "$WORK/component.plist" --identifier com.labtechno.subdestruckt.vst3 \
  --version "$PKG_VERSION" --install-location / --scripts "$OUT/scripts/installer" \
  "$WORK/pkgs/subdestruckt-vst3-component.pkg"
pkgbuild --nopayload --identifier com.labtechno.subdestruckt.uninstaller --version "$PKG_VERSION" \
  --scripts "$OUT/scripts/uninstaller" "$WORK/pkgs/subdestruckt-uninstall-component.pkg"

# ---- 4. paquets produits (Distribution + textes français)
mkdir -p "$WORK/resources"
for f in "$HERE/resources"/*.html; do subst "$f" "$WORK/resources/$(basename "$f")"; done
subst "$HERE/distribution/installer.xml.in" "$WORK/installer.xml"
subst "$HERE/distribution/uninstaller.xml.in" "$WORK/uninstaller.xml"
productbuild --distribution "$WORK/installer.xml" --package-path "$WORK/pkgs" --resources "$WORK/resources" \
  "$OUT/SUBDESTRUCKT_Installer.pkg"
productbuild --distribution "$WORK/uninstaller.xml" --package-path "$WORK/pkgs" --resources "$WORK/resources" \
  "$OUT/SUBDESTRUCKT_Uninstaller.pkg"

# ---- 5. contrôles des paquets produits
pkgutil --payload-files "$OUT/SUBDESTRUCKT_Installer.pkg" > "$OUT/installer_payload.txt"
grep -q "Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3/Contents/MacOS/SUBDESTRUCKT" "$OUT/installer_payload.txt"
if pkgutil --payload-files "$OUT/SUBDESTRUCKT_Uninstaller.pkg" 2>/dev/null | grep -q "SUBDESTRUCKT.vst3"; then
  echo "le desinstallateur ne doit contenir aucune charge utile"; exit 1
fi
{ pkgutil --check-signature "$OUT/SUBDESTRUCKT_Installer.pkg" || true; } > "$OUT/signature_status.txt" 2>&1
{ pkgutil --check-signature "$OUT/SUBDESTRUCKT_Uninstaller.pkg" || true; } >> "$OUT/signature_status.txt" 2>&1
{ codesign -dv --verbose=2 "$OUT/signed/SUBDESTRUCKT.vst3" 2>&1 || true; } >> "$OUT/signature_status.txt"
echo "paquets construits : $BUILD_ID ($COMMIT)"
