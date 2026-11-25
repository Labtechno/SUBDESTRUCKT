#!/bin/bash
# SUBDESTRUCKT — tests des scripts d'installation/désinstallation dans un bac à sable (sans privilèges).
# Les scripts reçoivent le bac à sable comme volume cible ($3) ; la copie de la charge utile, normalement faite par
# l'Installer macOS, est simulée par ditto du bundle livré. Couvre les cas d'échec impossibles à provoquer proprement
# avec l'Installer réel (échec de vérification + restauration, plugin chargé par un processus).
# Usage : sandbox_tests.sh <bundle signé livré> <dossier scripts finaux (installer/, uninstaller/)>
set -u
BUNDLE_SRC="$1"; SCRIPTS="$2"
PASS=0; FAIL=0
S="$(mktemp -d)"; trap 'rm -rf "$S"; [ -n "${HOLD:-}" ] && kill "$HOLD" 2>/dev/null' EXIT
V="$S/vol"
VST3="$V/Library/Audio/Plug-Ins/VST3"
B="$VST3/SUBDESTRUCKT.vst3"
SUP="$V/Library/Application Support/Labtechno/SUBDESTRUCKT"
export SBD_TEST_SANDBOX=1

check() {  # check <id> <description> <commande de test...>
  local id="$1" desc="$2"; shift 2
  if "$@"; then PASS=$((PASS + 1)); echo "RESULT $id PASS $desc"; else FAIL=$((FAIL + 1)); echo "RESULT $id FAIL $desc"; fi
}
pre()   { "$SCRIPTS/installer/preinstall" /pkg / "$V" >>"$S/log" 2>&1; }
post()  { "$SCRIPTS/installer/postinstall" /pkg / "$V" >>"$S/log" 2>&1; }
unin()  { "$SCRIPTS/uninstaller/preinstall" /pkg / "$V" >>"$S/log" 2>&1; }
payload() { mkdir -p "$VST3"; rm -rf "$B"; ditto "$BUNDLE_SRC" "$B"; }
installed_ok() { [ -d "$B" ] && [ ! -L "$B" ] && [ -f "$SUP/install-manifest.txt" ] && [ ! -e "$SUP/installer-backup" ]; }
plistset() { /usr/libexec/PlistBuddy -c "Set :$2 $3" "$1/Contents/Info.plist"; }
treehash() { (cd "$1" && find . -type f -exec shasum -a 256 {} \; | sort | shasum -a 256 | cut -d' ' -f1); }
reset() { rm -rf "$V"; mkdir -p "$VST3"; }

# ---- installation initiale, réinstallation, mise à jour depuis une version antérieure (simulée)
reset
pre; r1=$?; payload; post; r2=$?
check SBX-01 "installation initiale : preinstall 0, postinstall 0, manifeste ecrit, aucune sauvegarde residuelle" \
  test "$r1" = 0 -a "$r2" = 0 -a -f "$SUP/install-manifest.txt" -a ! -e "$SUP/installer-backup"
pre; r1=$?; payload; post; r2=$?
check SBX-02 "reinstallation de la meme version" test "$r1" = 0 -a "$r2" = 0
plistset "$B" CFBundleShortVersionString 0.0.9
pre; r1=$?; payload; post; r2=$?
check SBX-03 "mise a jour depuis une version anterieure simulee (0.0.9) -> version courante installee" \
  test "$r1" = 0 -a "$r2" = 0 -a "$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' "$B/Contents/Info.plist")" != "0.0.9"

# ---- échec de vérification après copie : restauration de l'installation antérieure
reset; pre; payload; post   # installation valide de référence
h_before="$(treehash "$B")"
pre; payload; rm -f "$B/Contents/MacOS/SUBDESTRUCKT"; post; r=$?
check SBX-04 "echec de verification (binaire manquant) : postinstall en echec et installation anterieure restauree a l'identique" \
  test "$r" != 0 -a "$(treehash "$B")" = "$h_before"
reset; pre; payload; plistset "$B" CFBundleVersion 999999; post; r=$?
check SBX-05 "echec de verification sans installation anterieure : rien de partiel ne reste" test "$r" != 0 -a ! -e "$B"

# ---- éléments étrangers : bundle tiers, lien symbolique
reset; mkdir -p "$B/Contents"; cp "$BUNDLE_SRC/Contents/Info.plist" "$B/Contents/"; plistset "$B" CFBundleIdentifier com.other.vendor.plugin
h="$(treehash "$B")"; pre; r=$?
check SBX-06 "installateur : bundle tiers a l'emplacement -> refus, element intact" test "$r" = 17 -a "$(treehash "$B")" = "$h"
unin; r=$?
check SBX-07 "desinstallateur : bundle tiers -> refus, element intact" test "$r" = 32 -a "$(treehash "$B")" = "$h"
reset; mkdir -p "$S/ailleurs"; ditto "$BUNDLE_SRC" "$S/ailleurs/SUBDESTRUCKT.vst3"; ln -s "$S/ailleurs/SUBDESTRUCKT.vst3" "$B"
h="$(treehash "$S/ailleurs/SUBDESTRUCKT.vst3")"; pre; r1=$?; unin; r2=$?
check SBX-08 "lien symbolique a l'emplacement : installateur et desinstallateur refusent, cible non suivie et intacte" \
  test "$r1" = 16 -a "$r2" = 31 -a -L "$B" -a "$(treehash "$S/ailleurs/SUBDESTRUCKT.vst3")" = "$h"

# ---- plugin chargé par un processus (DAW ouverte simulée par un descripteur ouvert sur le binaire)
reset; pre; payload; post
tail -f "$B/Contents/MacOS/SUBDESTRUCKT" >/dev/null 2>&1 & HOLD=$!
sleep 1
pre; r1=$?; unin; r2=$?
kill "$HOLD" 2>/dev/null; wait "$HOLD" 2>/dev/null; HOLD=""
check SBX-09 "plugin en cours d'utilisation : installateur et desinstallateur refusent sans rien modifier" \
  test "$r1" = 13 -a "$r2" = 34 -a -d "$B"

# ---- désinstallation : ciblée, données personnelles et autres plugins conservés
reset; pre; payload; post
mkdir -p "$VST3/AutrePlugin.vst3/Contents" && echo autre > "$VST3/AutrePlugin.vst3/Contents/marker"
mkdir -p "$V/Users/u/Library/Application Support/SUBDESTRUCKT" && echo preset > "$V/Users/u/Library/Application Support/SUBDESTRUCKT/mon_preset.txt"
mkdir -p "$V/Users/u/Music/Projets" && echo projet > "$V/Users/u/Music/Projets/projet.als"
mkdir -p "$V/Library/Application Support/Labtechno/AutreProduit" && echo keep > "$V/Library/Application Support/Labtechno/AutreProduit/keep.txt"
unin; r=$?
check SBX-10 "desinstallation : bundle et manifeste retires, dossier produit vide retire, autres plugins, donnees personnelles, projets et dossier Labtechno partage conserves" \
  test "$r" = 0 -a ! -e "$B" -a ! -e "$SUP" -a -f "$VST3/AutrePlugin.vst3/Contents/marker" \
  -a -f "$V/Users/u/Library/Application Support/SUBDESTRUCKT/mon_preset.txt" -a -f "$V/Users/u/Music/Projets/projet.als" \
  -a -f "$V/Library/Application Support/Labtechno/AutreProduit/keep.txt"
unin; r=$?
check SBX-11 "desinstallation sans plugin present : refus explicite (30), rien supprime" \
  test "$r" = 30 -a -f "$VST3/AutrePlugin.vst3/Contents/marker"
pre; payload; post; r=$?
check SBX-12 "reinstallation apres desinstallation, donnees conservees accessibles" \
  test "$r" = 0 -a -f "$V/Users/u/Library/Application Support/SUBDESTRUCKT/mon_preset.txt"

# ---- ancien désinstallateur face à une version plus récente
plistset "$B" CFBundleShortVersionString 9.9.9
unin; r=$?
check SBX-13 "desinstallateur plus ancien que la version installee (9.9.9) : refus explicite (33), bundle intact" test "$r" = 33 -a -d "$B"

echo "--- journal des scripts ---"; cat "$S/log"
echo "sandbox_tests : $PASS PASS, $FAIL FAIL"
[ "$FAIL" = 0 ]
