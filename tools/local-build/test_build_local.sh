#!/bin/bash
# SUBDESTRUCKT — harnais de test de build_local_macos.sh (ARCH-G4.2, LB-01..07 : docs/V1-ARCH-G4.2-SPEC.md ; ARCH-G4.3,
# LB-08..13 : docs/V1-ARCH-G4.3-SPEC.md §8).
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
cat > "$BIN/date" <<'EOF'
#!/bin/bash
# numéro de build figé (collisions, LB-12) ; autres formats : date réelle
if [ "${1-}" = "+%y%j%H%M" ]; then echo "${FAKE_BUILDNO:-261231200}"; exit 0; fi
exec /usr/bin/date "$@"
EOF
cat > "$BIN/pkgutil" <<'EOF'
#!/bin/bash
# pkgutil --expand-full <pkg> <dest> : charge utile simulée (copie du bundle déposé par build_packages simulé)
if [ "${1-}" = "--expand-full" ]; then
  mkdir -p "$3/SUBDESTRUCKT.pkg/Payload/Library/Audio/Plug-Ins/VST3"
  cp -R "$(dirname "$2")/.payload/SUBDESTRUCKT.vst3" "$3/SUBDESTRUCKT.pkg/Payload/Library/Audio/Plug-Ins/VST3/"
fi
exit 0
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
  cat > "$B/subdestruckt_core_tests" <<'EOT'
#!/bin/bash
# résultats simulés : G-01 et K-01 (inventaire) ; variables FAKE_* : échec, doublon, absence, crash, troncature
lines=("RESULT G-01 PASS simule")
[ -z "${FAKE_DROP_K01-}" ] && lines+=("RESULT K-01 ${FAKE_K01:-PASS} simule")
[ -n "${FAKE_DUP-}" ] && lines+=("RESULT G-01 PASS doublon")
[ -n "${FAKE_CORE_RESULTS-}" ] && lines+=("$FAKE_CORE_RESULTS")
m=0
for l in "${lines[@]}"; do echo "$l"; case "$l" in *" FAIL "*) m=$((m + 1)) ;; esac; done
[ -n "${FAKE_CORE_CRASH-}" ] && exit 139
k=${#lines[@]}
[ -n "${FAKE_TRUNC-}" ] && k=$((k + 1))
echo "core_tests : 9 controles, $m echecs, $k criteres rapportes"
if [ -n "${FAKE_CORE_EXIT-}" ]; then exit "$FAKE_CORE_EXIT"; fi
[ "$m" = 0 ] || exit 1
exit 0
EOT
  cat > "$B/subdestruckt_wrapper_tests_artefacts/Release/subdestruckt_wrapper_tests" <<'EOT'
#!/bin/bash
echo "RESULT G-21w PASS simule"
m=0
if [ -n "${FAKE_W_K01-}" ]; then echo "RESULT K-01 FAIL hors portee"; m=1; fi
echo "wrapper_tests : 3 controles, $m echecs, $((1 + m)) criteres rapportes"
[ "$m" = 0 ]
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
mkdir -p "$R/tools"
cp "$REPO/tools/check_results.sh" "$R/tools/"
printf '# simule\ncore:K-01 echec connu simule (portee core)\n' > "$R/tests/known_failures.txt"
printf 'core G-01\ncore K-01\nwrapper G-21w\n' > "$R/tests/expected_results.txt"
echo "ids" > "$R/tests/host/expected_vst3_param_ids.txt"
printf '/local-build/\n' > "$R/.gitignore"
cat > "$R/packaging/build_packages.sh" <<'EOF'
#!/bin/bash
# paquets simulés : --prepare-only (bundle « signé ») ; --prepared (paquets à partir du bundle validé, charge utile)
set -e
MODE=full; case "$1" in --prepare-only) MODE=prepare; shift ;; --prepared) MODE=prepared; shift ;; esac
RAW="$1"; OUT="$2"; mkdir -p "$OUT/signed"; rm -rf "$OUT/signed/SUBDESTRUCKT.vst3"; cp -R "$RAW" "$OUT/signed/SUBDESTRUCKT.vst3"
echo "packages $MODE" >> "$FAKE_LOG"
[ "$MODE" = prepare ] && exit 0
mkdir -p "$OUT/.payload"; cp -R "$RAW" "$OUT/.payload/SUBDESTRUCKT.vst3"
[ -n "${FAKE_PAYLOAD_ALTER-}" ] && echo altere >> "$OUT/.payload/SUBDESTRUCKT.vst3/Contents/MacOS/SUBDESTRUCKT"
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
if [ -n "${FAKE_ZIP_ALTER-}" ]; then  # ZIP construit depuis une copie altérée : seul le contrôle du ZIP réextrait le voit
  Z="$DL/zsrc"; mkdir -p "$Z"; cp "$S"/* "$Z/"; echo alteration >> "$Z/CHANGELOG.txt"
  mkzip "$DL/$NAME.zip" "$Z" SUBDESTRUCKT_Installer.pkg SUBDESTRUCKT_Uninstaller.pkg README_FR.txt CHANGELOG.txt SHA256SUMS.txt; rm -rf "$Z"
else
  mkzip "$DL/$NAME.zip" "$S" SUBDESTRUCKT_Installer.pkg SUBDESTRUCKT_Uninstaller.pkg README_FR.txt CHANGELOG.txt SHA256SUMS.txt
fi
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
known="K-01"
run FAKE_K01=FAIL -- --juce-licence=agplv3; c=$RC
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

# ---------------------------------------------------------------- LB-08 : ordre et bundle empaqueté = bundle validé
run X=1 -- --juce-licence=starter; a=$RC
seq="$(grep -E '^(packages|pluginval)' "$FAKE_LOG" | awk '{ if ($1 == "packages") print $1 "-" $2; else print $1 }' | tr '\n' ' ')"
order=0; [ "$seq" = "packages-prepare pluginval packages-prepared " ] && order=1
same=0; echo "$OUTT" | grep -q "charge utile de l'installateur = bundle valide" && same=1
clean
run FAKE_PV_EXIT=1 -- --juce-licence=starter; b=$RC; np=0; none packages- && none validated- && none delivery- && np=1; clean
run FAKE_PAYLOAD_ALTER=1 -- --juce-licence=starter; c=$RC; nd=0; none packages- && none delivery- && nd=1; clean
result LB-08 "$([ $a = 0 ] && [ $order = 1 ] && [ $same = 1 ] && [ $b = 7 ] && [ $np = 1 ] && [ $c = 8 ] && [ $nd = 1 ] && echo 1 || echo 0)" "succes : code $a, ordre [$seq] $order, charge utile = bundle valide $same ; pluginval en echec : code $b, aucun paquet ni bundle valide ni livraison $np ; charge utile alteree : code $c, paquets et livraison retires $nd"

# ---------------------------------------------------------------- LB-09 : échec après paquets, aucune livraison présentée
run FAKE_ZIP_ALTER=1 -- --juce-licence=starter; a=$RC; n1=0; none delivery- && none packages- && n1=1; clean
run FAKE_BAD_SUMS=1 -- --juce-licence=starter; b=$RC; n2=0; none delivery- && none packages- && n2=1; clean
result LB-09 "$([ $a = 8 ] && [ $n1 = 1 ] && [ $b = 8 ] && [ $n2 = 1 ] && echo 1 || echo 0)" "ZIP final non conforme (fichier altere seulement dans le ZIP) : code $a, livraison et paquets retires $n1 ; SHA256SUMS incoherent : code $b, retires $n2"

# ---------------------------------------------------------------- LB-10 : barrière exhaustive
codes=""
for envv in FAKE_DROP_K01=1 FAKE_DUP=1 FAKE_CORE_CRASH=1 FAKE_TRUNC=1 FAKE_CORE_EXIT=1 "FAKE_K01=FAIL FAKE_CORE_EXIT=0" FAKE_W_K01=1 "FAKE_CORE_RESULTS=RESULT G-01b PASS hors inventaire" "FAKE_CORE_RESULTS=RESULT X-99 FAIL inattendu"; do
  # shellcheck disable=SC2086
  if [[ "$envv" == FAKE_CORE_RESULTS=* ]]; then run "$envv" -- --juce-licence=agplv3; else run $envv -- --juce-licence=agplv3; fi
  codes="$codes $RC"; none packages- || codes="$codes!"; clean
done
run FAKE_K01=FAIL -- --juce-licence=agplv3; k=$RC; clean
all5=1; for c in $codes; do [ "$c" = 5 ] || all5=0; done
result LB-10 "$([ $all5 = 1 ] && [ $k = 0 ] && echo 1 || echo 0)" "resultat absent, doublon, crash, synthese incoherente (troncature), code non nul sans echec, code 0 avec FAIL, echec connu hors de sa portee, resultat hors inventaire, FAIL inattendu : codes$codes (5 attendus, aucun paquet) ; echec connu dans sa portee : code $k (0)"

# ---------------------------------------------------------------- LB-11 : variable de filtrage
run SBD_ONLY=dsp -- --juce-licence=agplv3; a=$RC; n=0; none build- && none packages- && n=1; clean
gb=0; (SBD_ONLY=x bash "$REPO/tools/check_results.sh" "$R/tests/expected_results.txt" "$R/tests/known_failures.txt" x=/nonexistent:0 >/dev/null 2>&1) || gb=1
result LB-11 "$([ $a = 5 ] && [ $n = 1 ] && [ $gb = 1 ] && echo 1 || echo 0)" "SBD_ONLY defini : code $a, rien construit $n ; barriere seule : refus $gb"

# ---------------------------------------------------------------- LB-12 : symlinks et collisions
run X=1 -- --juce-licence=agplv3; first=$RC
run X=1 -- --juce-licence=agplv3; coll=$RC
kept=0; [ -n "$(ls "$R"/local-build/delivery-*/*.zip 2>/dev/null)" ] && kept=1
clean
mv "$R/local-build" "$T/lb-real"; ln -s "$T/lb-real" "$R/local-build" 2>/dev/null
if [ -L "$R/local-build" ]; then
  run X=1 -- --juce-licence=agplv3; sym=$RC; symnote="code $sym (3)"
  rm -f "$R/local-build"
else  # Git Bash (MSYS) : ln -s copie au lieu de lier ; le controle est effectif sous Linux (CI) et macOS
  sym=3; symnote="non verifiable sur cette plate-forme (liens symboliques non pris en charge par ln -s)"
  rm -rf "$R/local-build"
fi
mv "$T/lb-real" "$R/local-build"
result LB-12 "$([ $first = 0 ] && [ $coll = 9 ] && [ $kept = 1 ] && [ $sym = 3 ] && echo 1 || echo 0)" "meme version et meme build : 1er code $first, 2e code $coll (9 : aucun ecrasement), livraison existante conservee $kept ; local-build symbolique : $symnote ; ZIP reextrait et verifie : LB-09"

# ---------------------------------------------------------------- LB-13 : non-régression LB-01..07
result LB-13 "$([ "$fails" = 0 ] && echo 1 || echo 0)" "LB-01..12 : $fails echec(s) avant ce controle"

echo "test_build_local : $fails echec(s)"
[ "$fails" = 0 ]
