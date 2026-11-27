#!/bin/bash
# SUBDESTRUCKT — harnais de test de build_local_macos.sh (ARCH-G4.2, critères LB-01..07 : docs/V1-ARCH-G4.2-SPEC.md).
# Vérifie le FLUX DE CONTRÔLE du script (refus, arrêts, absence de livraison après échec, livraison complète, aucune
# écriture dans les dossiers de plugins, suppressions bornées) dans un dépôt git temporaire, avec les commandes macOS
# simulées (uname, cmake, codesign, plutil, lipo, pkgutil, curl, pluginval, paquets). La compilation réelle, pluginval
# et les paquets réels sont vérifiés par la CI macOS arm64 (G-23, G-24, PKG) et par la recette sur le Mac du
# propriétaire. Exécutable sous Linux (CI) et Git Bash. Sortie : lignes « RESULT LB-0x PASS|FAIL ... ».
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd -P)"
REPO="$(cd "$HERE/../.." && pwd -P)"
T="$(mktemp -d)"
trap 'rm -rf "$T"' EXIT
fails=0
result() {  # result <id> <0|1> <message>
  if [ "$2" = 1 ]; then echo "RESULT $1 PASS $3"; else echo "RESULT $1 FAIL $3"; fails=$((fails + 1)); fi
}
mkzip() {  # mkzip <zip> <dossier> <entrées...> (zip si disponible, sinon Python)
  local z="$1" d="$2"; shift 2
  if command -v zip >/dev/null 2>&1; then (cd "$d" && zip -X -q "$z" "$@"); else
    local py; py="$(command -v python3 2>/dev/null)"; "$py" -c "import sys" 2>/dev/null || py="py -3"
    (cd "$d" && $py -c "import sys,zipfile; z=zipfile.ZipFile(sys.argv[1],'w'); [z.write(f) for f in sys.argv[2:]]; z.close()" "$z" "$@")
  fi
}

# ---------------------------------------------------------------- commandes simulées
BIN="$T/bin"
mkdir -p "$BIN"
cat > "$BIN/uname" <<'EOF'
#!/bin/bash
case "${1-}" in -s) echo Darwin ;; -m) echo arm64 ;; *) echo Darwin ;; esac
EOF
cat > "$BIN/sysctl" <<'EOF'
#!/bin/bash
case "$*" in *brand*) echo "CPU simule" ;; *) echo 8 ;; esac
EOF
printf '#!/bin/bash\necho 15.0\n' > "$BIN/sw_vers"
printf '#!/bin/bash\necho "clang simule"\n' > "$BIN/xcrun"
printf '#!/bin/bash\necho 1.12.0\n' > "$BIN/ninja"
printf '#!/bin/bash\nexit 0\n' > "$BIN/codesign"
printf '#!/bin/bash\nexit 0\n' > "$BIN/pkgutil"
cat > "$BIN/plutil" <<'EOF'
#!/bin/bash
# plutil -extract <clé> raw -o - <fichier> : Info.plist simulé au format clé=valeur
[ "$1" = "-extract" ] || exit 1
grep "^$2=" "${@: -1}" 2>/dev/null | head -1 | cut -d= -f2-
EOF
cat > "$BIN/lipo" <<'EOF'
#!/bin/bash
echo "${FAKE_ARCH:-arm64}"
EOF
cat > "$BIN/curl" <<'EOF'
#!/bin/bash
out=""; while [ $# -gt 0 ]; do [ "$1" = "-o" ] && { out="$2"; shift; }; shift; done
echo "curl $out" >> "$FAKE_LOG"
cp "$FAKE_PV_ZIP" "$out"
EOF
cat > "$BIN/cmake" <<'EOF'
#!/bin/bash
case "$*" in *--version*) echo "cmake version 3.30.0 (simule)"; exit 0 ;; esac
if [ "$1" = "--build" ]; then
  B="$2"; . "$B/fake.env"
  mkdir -p "$B/subdestruckt_wrapper_tests_artefacts/Release" "$B/SUBDESTRUCKT_artefacts/Release/VST3/SUBDESTRUCKT.vst3/Contents/MacOS"
  cat > "$B/subdestruckt_core_tests" <<EOT
#!/bin/bash
echo "RESULT G-01 PASS simule"
[ -n "\${FAKE_CORE_RESULTS-}" ] && printf '%b\n' "\$FAKE_CORE_RESULTS"
[ -n "\${FAKE_CORE_CRASH-}" ] && exit 139
echo "core_tests : 1 controles, 0 echecs"
exit \${FAKE_CORE_EXIT:-0}
EOT
  cat > "$B/subdestruckt_wrapper_tests_artefacts/Release/subdestruckt_wrapper_tests" <<EOT
#!/bin/bash
echo "RESULT G-21w PASS simule"
echo "wrapper_tests : 1 controles, 0 echecs"
EOT
  chmod +x "$B/subdestruckt_core_tests" "$B/subdestruckt_wrapper_tests_artefacts/Release/subdestruckt_wrapper_tests"
  P="$B/SUBDESTRUCKT_artefacts/Release/VST3/SUBDESTRUCKT.vst3/Contents"
  printf 'CFBundleIdentifier=%s\nCFBundleShortVersionString=%s\nCFBundleVersion=%s\nSUBDESTRUCKTCommit=%s\n' \
    "${FAKE_BUNDLE_ID:-com.labtechno.subdestruckt}" "$VERSION" "$BUILDNO" "$COMMITID" > "$P/Info.plist"
  echo binaire > "$P/MacOS/SUBDESTRUCKT"
  exit 0
fi
B=""; V=""; N=""; C=""
for a in "$@"; do
  case "$a" in -DSUBDESTRUCKT_BUILD_NUMBER=*) N="${a#*=}" ;; -DSUBDESTRUCKT_COMMIT=*) C="${a#*=}" ;; esac
done
while [ $# -gt 0 ]; do [ "$1" = "-B" ] && B="$2"; [ "$1" = "-S" ] && S="$2"; shift; done
mkdir -p "$B"
V="$(sed -n 's/^ *VERSION: "\(.*\)"/\1/p' "$S/.github/workflows/arch-g4.yml" | head -1)"
printf 'VERSION=%s\nBUILDNO=%s\nCOMMITID=%s\n' "$V" "$N" "$C" > "$B/fake.env"
echo "configure $S" >> "$FAKE_LOG"
EOF
chmod +x "$BIN"/*

# pluginval simulé (zip réel, empreinte calculée)
mkdir -p "$T/pv/pluginval.app/Contents/MacOS"
cat > "$T/pv/pluginval.app/Contents/MacOS/pluginval" <<'EOF'
#!/bin/bash
echo "pluginval $*" >> "$FAKE_LOG"
exit "${FAKE_PV_EXIT:-0}"
EOF
chmod +x "$T/pv/pluginval.app/Contents/MacOS/pluginval"
mkzip "$T/pv.zip" "$T/pv" pluginval.app/Contents/MacOS/pluginval
PV_SHA="$(shasum -a 256 "$T/pv.zip" | awk '{print $1}')"

# ---------------------------------------------------------------- dépôt temporaire
R="$T/repo"
mkdir -p "$R/tools/local-build" "$R/.github/workflows" "$R/tests/host" "$R/packaging"
cp "$REPO/tools/local-build/build_local_macos.sh" "$REPO/tools/local-build/lib_safe_rm.sh" "$R/tools/local-build/"
cp "$REPO/tests/known_failures.txt" "$R/tests/"
echo "ids" > "$R/tests/host/expected_vst3_param_ids.txt"
printf '/local-build/\n' > "$R/.gitignore"
cat > "$R/packaging/build_packages.sh" <<'EOF'
#!/bin/bash
# paquets simulés : bundle « signé » copié, deux paquets
set -e
RAW="$1"; OUT="$2"; mkdir -p "$OUT/signed"; cp -R "$RAW" "$OUT/signed/SUBDESTRUCKT.vst3"
echo pkg > "$OUT/SUBDESTRUCKT_Installer.pkg"; echo pkg > "$OUT/SUBDESTRUCKT_Uninstaller.pkg"
exit "${FAKE_PKG_EXIT:-0}"
EOF
cat > "$R/packaging/make_delivery.sh" <<'EOF'
#!/bin/bash
set -e
PK="$1"; DL="$2"; NAME="SUBDESTRUCKT_v$3-b$4_macOS_arm64"; S="$DL/$NAME"; mkdir -p "$S"
cp "$PK/SUBDESTRUCKT_Installer.pkg" "$PK/SUBDESTRUCKT_Uninstaller.pkg" "$S/"
echo readme > "$S/README_FR.txt"; echo changelog > "$S/CHANGELOG.txt"
(cd "$S" && shasum -a 256 SUBDESTRUCKT_Installer.pkg SUBDESTRUCKT_Uninstaller.pkg README_FR.txt CHANGELOG.txt > SHA256SUMS.txt)
[ -n "${FAKE_BAD_SUMS-}" ] && echo alteration >> "$S/README_FR.txt"
. "$MKZIP_LIB"
mkzip "$DL/$NAME.zip" "$S" SUBDESTRUCKT_Installer.pkg SUBDESTRUCKT_Uninstaller.pkg README_FR.txt CHANGELOG.txt SHA256SUMS.txt
echo "$NAME" > "$DL/NAME"
EOF
declare -f mkzip > "$T/mkzip.sh"
export MKZIP_LIB="$T/mkzip.sh"
# JUCE simulé (dépôt git local au chemin attendu) ; épinglages du workflow alignés sur les simulations
mkdir -p "$R/local-build/JUCE-9.0.6"
(cd "$R/local-build/JUCE-9.0.6" && git init -q && git -c user.email=t@t -c user.name=t commit -q --allow-empty -m juce)
JSHA="$(git -C "$R/local-build/JUCE-9.0.6" rev-parse HEAD)"
cat > "$R/.github/workflows/arch-g4.yml" <<EOF
env:
  JUCE_TAG: "9.0.6"
  JUCE_SHA: "$JSHA"
  PLUGINVAL_VERSION: "v1.0.4"
  PLUGINVAL_SHA256: "$PV_SHA"
  MIN_MACOS: "14.0"
  VERSION: "9.9.9"
EOF
(cd "$R" && git init -q && git add -A && git -c user.email=t@t -c user.name=t commit -q -m base)

# dossiers de plugins simulés (utilisateur et système) : doivent rester intacts
H="$T/home"
SYSV="$T/system/Library/Audio/Plug-Ins/VST3"
mkdir -p "$H/Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3/Contents" "$SYSV/SUBDESTRUCKT.vst3/Contents"
echo "CFBundleShortVersionString=0.2.0" > "$H/Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3/Contents/Info.plist"
echo "CFBundleShortVersionString=0.3.0" > "$SYSV/SUBDESTRUCKT.vst3/Contents/Info.plist"
snapshot() { (cd "$T" && find home system -type f -exec shasum -a 256 {} + | sort); }
SNAP0="$(snapshot)"

export FAKE_LOG="$T/fake.log" FAKE_PV_ZIP="$T/pv.zip"
run() {  # run <env...> -- <args...> : exécute le script ; code dans RC, sortie dans OUTT
  local envs=()
  while [ $# -gt 0 ] && [ "$1" != "--" ]; do envs+=("$1"); shift; done
  shift
  : > "$FAKE_LOG"
  OUTT="$(cd "$R" && env HOME="$H" SBD_SYSTEM_VST3_DIR="$SYSV" PATH="$BIN:$PATH" "${envs[@]}" bash tools/local-build/build_local_macos.sh "$@" 2>&1)"
  RC=$?
}
clean() { (cd "$R/local-build" && find . -mindepth 1 -maxdepth 1 ! -name 'JUCE-9.0.6' -exec rm -rf {} +); }
none() { ! ls -d "$R/local-build/$1"* >/dev/null 2>&1; }

# ---------------------------------------------------------------- LB-01 : licence obligatoire, installation supprimée
run X=1 --; a=$RC; none build- && b1=1 || b1=0
run X=1 -- --juce-licence=gratuite; b=$RC
run X=1 -- --juce-licence=starter --install; c=$RC
result LB-01 "$([ $a = 2 ] && [ $b = 2 ] && [ $c = 2 ] && [ $b1 = 1 ] && echo 1 || echo 0)" "sans licence : code $a ; licence hors liste : code $b ; --install : code $c (2 attendus, rien construit $b1)"
clean

# ---------------------------------------------------------------- LB-02 : dépôt modifié, commit inconnu
echo modif >> "$R/tests/known_failures.txt"
run X=1 -- --juce-licence=agplv3; a=$RC; none build- && n1=1 || n1=0
(cd "$R" && git checkout -q -- tests/known_failures.txt)
echo x > "$R/nouveau.txt"
run X=1 -- --juce-licence=agplv3; b=$RC; none build- && n2=1 || n2=0
rm -f "$R/nouveau.txt"
run X=1 -- --juce-licence=agplv3 --commit=0123456789abcdef0123456789abcdef01234567; c=$RC
result LB-02 "$([ $a = 3 ] && [ $b = 3 ] && [ $c = 3 ] && [ $n1 = 1 ] && [ $n2 = 1 ] && echo 1 || echo 0)" "fichier suivi modifie : code $a ; fichier non suivi : code $b ; commit inconnu : code $c (3 attendus) ; aucune compilation $n1/$n2"
clean

# ---------------------------------------------------------------- LB-03 : tests obligatoires bloquants
run FAKE_CORE_RESULTS="RESULT X-99 FAIL simule" FAKE_CORE_EXIT=1 -- --juce-licence=agplv3; a=$RC
p1=0; none packages- && none delivery- && ! grep -q pluginval "$FAKE_LOG" && p1=1; clean
run FAKE_CORE_CRASH=1 -- --juce-licence=agplv3; b=$RC
p2=0; none packages- && none delivery- && p2=1; clean
known="$( (grep -v '^#' "$REPO/tests/known_failures.txt" || true) | awk 'NF {print $1; exit}')"
if [ -n "$known" ]; then
  run FAKE_CORE_RESULTS="RESULT $known FAIL echec connu documente" FAKE_CORE_EXIT=1 -- --juce-licence=agplv3; c=$RC
else c=0; fi
clean
result LB-03 "$([ $a = 5 ] && [ $b = 5 ] && [ $p1 = 1 ] && [ $p2 = 1 ] && [ $c = 0 ] && echo 1 || echo 0)" "critere FAIL non documente : code $a, ni pluginval ni paquets ni livraison $p1 ; tests interrompus : code $b, aucune livraison $p2 ; echec connu documente ($known) seul : code $c (0 : non bloquant, consigne)"

# ---------------------------------------------------------------- LB-04 : pluginval obligatoire et bloquant
run FAKE_PV_EXIT=1 -- --juce-licence=agplv3; a=$RC; d1=0; none delivery- && d1=1
clean
mkdir -p "$R/local-build/tools"; echo altere > "$R/local-build/tools/pluginval-v1.0.4.zip"
run X=1 -- --juce-licence=agplv3; b=$RC; d2=0; none delivery- && [ ! -e "$R/local-build/tools/pluginval-v1.0.4.zip" ] && d2=1
clean
result LB-04 "$([ $a = 7 ] && [ $d1 = 1 ] && [ $b = 4 ] && [ $d2 = 1 ] && echo 1 || echo 0)" "pluginval en echec : code $a, aucune livraison $d1 ; empreinte pluginval alteree : code $b, fichier supprime et aucune livraison $d2"

# ---------------------------------------------------------------- LB-05 : identité du bundle et JUCE épinglé
run FAKE_BUNDLE_ID=com.autre.plugin -- --juce-licence=agplv3; a=$RC; clean
run FAKE_ARCH="x86_64 arm64" -- --juce-licence=agplv3; b=$RC; clean
echo sale > "$R/local-build/JUCE-9.0.6/modif.txt"
run X=1 -- --juce-licence=agplv3; c=$RC
rm -f "$R/local-build/JUCE-9.0.6/modif.txt"; clean
(cd "$R/local-build/JUCE-9.0.6" && git -c user.email=t@t -c user.name=t commit -q --allow-empty -m autre)
run X=1 -- --juce-licence=agplv3; d=$RC
(cd "$R/local-build/JUCE-9.0.6" && git reset -q --hard "$JSHA"); clean
nd=0; none delivery- && nd=1
result LB-05 "$([ $a = 6 ] && [ $b = 6 ] && [ $c = 4 ] && [ $d = 4 ] && [ $nd = 1 ] && echo 1 || echo 0)" "identifiant de bundle incoherent : code $a ; architecture non arm64 : code $b ; arbre JUCE modifie : code $c ; SHA JUCE different de l'epinglage : code $d (6/6/4/4 attendus) ; aucune livraison $nd"

# ---------------------------------------------------------------- LB-06 : livraison complète, dossiers de plugins intacts
run X=1 -- --juce-licence=starter; a=$RC
Z="$(ls "$R"/local-build/delivery-*/*.zip 2>/dev/null | head -1)"
listed="$( [ -n "$Z" ] && unzip -Z1 "$Z" | sort | tr '\n' ' ')"
want="CHANGELOG.txt README_FR.txt SHA256SUMS.txt SUBDESTRUCKT_Installer.pkg SUBDESTRUCKT_Uninstaller.pkg "
sums=0; [ -n "$Z" ] && (cd "$(dirname "$Z")/$(basename "$Z" .zip)" && shasum -a 256 -c SHA256SUMS.txt >/dev/null 2>&1) && sums=1
intact=0; [ "$(snapshot)" = "$SNAP0" ] && intact=1
dup=0; echo "$OUTT" | grep -q "ATTENTION DOUBLON" && dup=1
ver=0; case "$Z" in *SUBDESTRUCKT_v9.9.9-b*_macOS_arm64.zip) ver=1 ;; esac
clean
run FAKE_BAD_SUMS=1 -- --juce-licence=starter; b=$RC; nb=0; none delivery- && nb=1
clean
result LB-06 "$([ $a = 0 ] && [ "$listed" = "$want" ] && [ $sums = 1 ] && [ $intact = 1 ] && [ $dup = 1 ] && [ $ver = 1 ] && [ $b = 8 ] && [ $nb = 1 ] && echo 1 || echo 0)" "succes : code $a ; ZIP versionne $ver contenant [$listed] ; SHA256SUMS verifie $sums ; dossiers VST3 utilisateur et systeme intacts $intact ; doublon signale $dup ; SHA256SUMS incoherent : code $b, livraison retiree $nb"

# ---------------------------------------------------------------- LB-07 : suppressions bornées
WORK="$T/repo/local-build"; ROOT="$T/repo"; HOME_SAVE="$HOME"
. "$REPO/tools/local-build/lib_safe_rm.sh"
mkdir -p "$WORK/a/b"
refused=0
for p in "" / "$HOME" "$ROOT" "$WORK" "$WORK/" "$T" "$WORK/../repo" "/etc/x"; do
  ( safe_rm "$p" ) 2>/dev/null; [ $? = 8 ] && refused=$((refused + 1))
done
( safe_rm "$WORK/a" ) 2>/dev/null; ok=$?
removed=0; [ $ok = 0 ] && [ ! -e "$WORK/a" ] && [ -d "$WORK/JUCE-9.0.6" ] && [ -d "$ROOT/tools" ] && removed=1
rm=0; grep -q 'rm -rf' "$REPO/tools/local-build/build_local_macos.sh" && rm=1
result LB-07 "$([ $refused = 9 ] && [ $removed = 1 ] && [ $rm = 0 ] && echo 1 || echo 0)" "9/9 chemins hors perimetre refuses ($refused) ; sous-chemin de local-build/ supprime, reste intact $removed ; aucune suppression recursive directe dans le script hors safe_rm ($([ $rm = 0 ] && echo oui || echo NON))"

echo "test_build_local : $fails echec(s)"
[ "$fails" = 0 ]
