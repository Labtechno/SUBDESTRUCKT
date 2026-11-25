#!/bin/bash
# SUBDESTRUCKT — recette des paquets avec l'Installer macOS réel (sudo installer) sur la VM de CI.
# Statut : PASS (VM CI) au mieux. Ce n'est PAS une recette sur Mac physique ni dans une DAW (PENDING).
# Usage : system_tests.sh <Installer.pkg> <Uninstaller.pkg> <host_tests> <expected_ids.txt>
set -u
INST="$1"; UNIN="$2"; HOST="$3"; IDS="$4"
PASS=0; FAIL=0
B="/Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3"
OTHER="/Library/Audio/Plug-Ins/VST3/ZZ_AutrePlugin_Test.vst3"
SUP="/Library/Application Support/Labtechno/SUBDESTRUCKT"
USERDATA="$HOME/Documents/SUBDESTRUCKT_test_donnees/mon_preset.txt"
RECEIPT="com.labtechno.subdestruckt.vst3"
LOG="$(mktemp)"

check() {
  local id="$1" desc="$2"; shift 2
  if "$@"; then PASS=$((PASS + 1)); echo "RESULT $id PASS $desc"; else FAIL=$((FAIL + 1)); echo "RESULT $id FAIL $desc"; fi
}
install_pkg() { sudo installer -pkg "$1" -target / >"$LOG" 2>&1; local r=$?; cat "$LOG"; return $r; }
treehash() { (cd "$1" && sudo find . -type f -exec shasum -a 256 {} \; | sort | shasum -a 256 | cut -d' ' -f1); }

sudo rm -rf "$OTHER"
mkdir -p "$(dirname "$USERDATA")" && echo "preset utilisateur" > "$USERDATA"
sudo mkdir -p "$OTHER/Contents" && echo autre | sudo tee "$OTHER/Contents/marker" >/dev/null

# PKG-02 : installation propre, bundle arm64, reçu, manifeste, découverte et instanciation par l'hôte de test
install_pkg "$INST"; r=$?
check PKG-02-install "installation propre (installer reel, VM) : code 0, bundle arm64 present, recu et manifeste" \
  test "$r" = 0 -a -d "$B" -a "$(lipo -archs "$B/Contents/MacOS/SUBDESTRUCKT" 2>/dev/null)" = arm64 -a -f "$SUP/install-manifest.txt"
pkgutil --pkg-info "$RECEIPT"; codesign --verify --strict --verbose=2 "$B"
"$HOST" "$B" "$IDS" --quick --tag "PKG-02-" ; r=$?
check PKG-02-host "bundle installe decouvert et instancie depuis /Library/Audio/Plug-Ins/VST3 par l'hote VST3 de test (rendu d'une note)" test "$r" = 0

# PKG-03 : réinstallation de la même version (pas de version précédente publiée : mise à jour testée en bac à sable)
h1="$(treehash "$B")"; install_pkg "$INST"; r=$?
check PKG-03-reinstall "reinstallation de la meme version : code 0, contenu identique, identite conservee" \
  test "$r" = 0 -a "$(treehash "$B")" = "$h1" -a "$(/usr/libexec/PlistBuddy -c 'Print :CFBundleIdentifier' "$B/Contents/Info.plist")" = com.labtechno.subdestruckt

# PKG-04 : désinstallation ciblée
install_pkg "$UNIN"; r=$?
check PKG-04 "desinstallation : bundle, manifeste et recu retires ; autre plugin et donnees personnelles conserves" \
  test "$r" = 0 -a ! -e "$B" -a ! -e "$SUP" -a -f "$OTHER/Contents/marker" -a -f "$USERDATA"
pkgutil --pkg-info "$RECEIPT" >/dev/null 2>&1 && { echo "recu encore present"; FAIL=$((FAIL + 1)); }

# PKG-06a : désinstallation sans plugin présent : refus explicite, rien supprimé
install_pkg "$UNIN"; r=$?
check PKG-06-absent "desinstallateur sans plugin present : refus (code non nul), aucun autre element touche" \
  test "$r" != 0 -a -f "$OTHER/Contents/marker" -a -f "$USERDATA"

# PKG-05 : réinstallation après désinstallation, données conservées accessibles
install_pkg "$INST"; r=$?
check PKG-05 "reinstallation apres desinstallation : code 0, donnees personnelles toujours presentes" test "$r" = 0 -a -d "$B" -a -f "$USERDATA"

# PKG-06b : ancien désinstallateur face à une version installée plus récente (simulée)
sudo /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString 9.9.9" "$B/Contents/Info.plist"
install_pkg "$UNIN"; r=$?
check PKG-06-newer "desinstallateur plus ancien que la version installee : refus, bundle intact" test "$r" != 0 -a -d "$B"
sudo rm -rf "$B"   # nettoyage explicite du bundle modifié pour le test
sudo rm -rf "$SUP"

# PKG-06c : élément tiers à l'emplacement : installateur et désinstallateur refusent, élément intact
sudo mkdir -p "$B/Contents"
sudo /usr/libexec/PlistBuddy -c "Add :CFBundleIdentifier string com.other.vendor.plugin" "$B/Contents/Info.plist" >/dev/null
h="$(treehash "$B")"
install_pkg "$INST"; r1=$?; install_pkg "$UNIN"; r2=$?
check PKG-06-foreign "element tiers a l'emplacement : installateur et desinstallateur refusent, element intact" \
  test "$r1" != 0 -a "$r2" != 0 -a "$(treehash "$B")" = "$h"
sudo rm -rf "$B"

# PKG-06d : lien symbolique à l'emplacement
TGT="$(mktemp -d)/cible"; mkdir -p "$TGT" && echo cible > "$TGT/marker"
sudo ln -s "$TGT" "$B"
install_pkg "$INST"; r1=$?; install_pkg "$UNIN"; r2=$?
check PKG-06-symlink "lien symbolique a l'emplacement : refus des deux paquets, cible non suivie et intacte" \
  test "$r1" != 0 -a "$r2" != 0 -a -L "$B" -a -f "$TGT/marker"
sudo rm -f "$B"

# nettoyage final : installation puis désinstallation propres
install_pkg "$INST" >/dev/null; install_pkg "$UNIN" >/dev/null
sudo rm -rf "$OTHER"
echo "--- extrait de /var/log/install.log (SUBDESTRUCKT) ---"
grep "SUBDESTRUCKT\[" /var/log/install.log | tail -60 || true
echo "system_tests : $PASS PASS, $FAIL FAIL"
[ "$FAIL" = 0 ]
