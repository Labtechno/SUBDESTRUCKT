#!/bin/bash
# SUBDESTRUCKT — fonctions communes des scripts d'installation et de désinstallation (V1-PACKAGING-RELEASE-CONTRACT).
# Tous les chemins sont dérivés du volume cible ($3 fourni par l'Installer macOS), ce qui permet de tester les
# scripts dans un bac à sable sans privilèges. Aucun glob de suppression, aucun chemin partagé supprimé.

SBD_EXPECTED_ID="com.labtechno.subdestruckt"
SBD_RECEIPT_ID="com.labtechno.subdestruckt.vst3"
SBD_VERSION="@VERSION@"        # remplacé à la construction du paquet
SBD_BUILD="@BUILD@"
SBD_BUILD_ID="@BUILD_ID@"
SBD_COMMIT="@COMMIT@"
SBD_MIN_MACOS="@MIN_MACOS@"

sbd_init_paths() {
  local vol="${1:-/}"
  SBD_ROOT="${vol%/}"
  SBD_VST3_DIR="$SBD_ROOT/Library/Audio/Plug-Ins/VST3"
  SBD_BUNDLE="$SBD_VST3_DIR/SUBDESTRUCKT.vst3"
  SBD_BINARY="$SBD_BUNDLE/Contents/MacOS/SUBDESTRUCKT"
  SBD_SUPPORT_PARENT="$SBD_ROOT/Library/Application Support/Labtechno"
  SBD_SUPPORT="$SBD_SUPPORT_PARENT/SUBDESTRUCKT"
  SBD_MANIFEST="$SBD_SUPPORT/install-manifest.txt"
  SBD_BACKUP="$SBD_SUPPORT/installer-backup/SUBDESTRUCKT.vst3"
}

sbd_log() { echo "SUBDESTRUCKT[$SBD_SCRIPT]: $*"; }

# Lecture d'une clé d'Info.plist ; vide si absente.
sbd_plist() { /usr/libexec/PlistBuddy -c "Print :$2" "$1/Contents/Info.plist" 2>/dev/null; }

# Comparaison de versions numériques pointées : affiche -1, 0 ou 1.
sbd_vercmp() {
  local IFS=.
  local -a a=($1) b=($2)
  local i n=${#a[@]}
  [ ${#b[@]} -gt "$n" ] && n=${#b[@]}
  for ((i = 0; i < n; i++)); do
    local x=${a[i]:-0} y=${b[i]:-0}
    x=${x//[^0-9]/}; y=${y//[^0-9]/}
    x=${x:-0}; y=${y:-0}
    if ((10#$x > 10#$y)); then echo 1; return; fi
    if ((10#$x < 10#$y)); then echo -1; return; fi
  done
  echo 0
}

# Le bundle installé est-il plus récent que ce paquet ? (version courte, puis numéro de build)
sbd_installed_is_newer() {
  local v b c
  v=$(sbd_plist "$SBD_BUNDLE" CFBundleShortVersionString)
  b=$(sbd_plist "$SBD_BUNDLE" CFBundleVersion)
  c=$(sbd_vercmp "${v:-0}" "$SBD_VERSION")
  [ "$c" = "1" ] && return 0
  [ "$c" = "0" ] && [ "$(sbd_vercmp "${b:-0}" "$SBD_BUILD")" = "1" ] && return 0
  return 1
}

# Le binaire du plugin est-il chargé par un processus (DAW ouverte) ?
sbd_in_use() {
  [ -e "$SBD_BINARY" ] || return 1
  /usr/sbin/lsof -t "$SBD_BINARY" >/dev/null 2>&1
}

# Contrôle d'identité de l'élément présent à l'emplacement du bundle. Code : 0 absent, 1 notre bundle,
# 2 lien symbolique, 3 élément non identifié (fichier, bundle tiers).
sbd_identify() {
  if [ -L "$SBD_BUNDLE" ]; then return 2; fi
  if [ ! -e "$SBD_BUNDLE" ]; then return 0; fi
  if [ ! -d "$SBD_BUNDLE" ] || [ -L "$SBD_BUNDLE/Contents" ]; then return 3; fi
  if [ "$(sbd_plist "$SBD_BUNDLE" CFBundleIdentifier)" = "$SBD_EXPECTED_ID" ]; then return 1; fi
  return 3
}
