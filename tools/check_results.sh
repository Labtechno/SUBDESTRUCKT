#!/bin/bash
# SUBDESTRUCKT — barrière exhaustive des résultats de tests (ARCH-G4.3, LB-10 ; partagée par la CI et la compilation
# locale). Usage :
#   tools/check_results.sh <inventaire> <echecs_connus> <portée>=<journal>:<code_de_sortie> [...]
#   ex. tools/check_results.sh tests/expected_results.txt tests/known_failures.txt core=out/core_tests.log:1 wrapper=out/wrapper_tests.log:1
# Refus (code 1) si :
#   - une variable de filtrage des tests (SBD_ONLY) est présente : suite partielle ;
#   - un journal est absent, sans ligne de synthèse (« <portée>_tests : N controles, M echecs, K criteres rapportes ») :
#     suite interrompue, crash ou timeout ; ou code de sortie >= 128 (signal) ;
#   - K différent du nombre de lignes RESULT du journal ;
#   - code de sortie incohérent : 0 avec M > 0 ou un critère FAIL ; non nul avec M = 0 ;
#   - un identifiant de l'inventaire absent, présent plusieurs fois, ou un identifiant hors inventaire ;
#   - un critère FAIL absent de la liste des échecs connus pour SA portée (« portée:ID ») ;
# Information (non bloquante) : un échec connu qui passe désormais.
set -uo pipefail
[ $# -ge 3 ] || { echo "usage : $0 <inventaire> <echecs_connus> <portee>=<journal>:<code> ..." >&2; exit 2; }
INV="$1"; KNOWN="$2"; shift 2
bad=0
err() { echo "BARRIERE : $*"; bad=1; }
[ -z "${SBD_ONLY+x}" ] || err "SBD_ONLY=${SBD_ONLY} present : suite partielle, refusee"
[ -f "$INV" ] || { echo "BARRIERE : inventaire absent ($INV)"; exit 1; }
[ -f "$KNOWN" ] || { echo "BARRIERE : liste des echecs connus absente ($KNOWN)"; exit 1; }
known="$(grep -v '^#' "$KNOWN" | awk 'NF {print $1}')"
for spec in "$@"; do
  scope="${spec%%=*}"; rest="${spec#*=}"; log="${rest%:*}"; code="${rest##*:}"
  if [ ! -f "$log" ]; then err "$scope : journal absent ($log)"; continue; fi
  case "$code" in ''|*[!0-9]*) err "$scope : code de sortie illisible ('$code')"; continue ;; esac
  summary="$(grep -E "^${scope}_tests : [0-9]+ controles, [0-9]+ echecs, [0-9]+ criteres rapportes" "$log" | tail -1)"
  if [ -z "$summary" ]; then err "$scope : ligne de synthese absente (suite interrompue, crash ou timeout ; code $code)"; continue; fi
  [ "$code" -lt 128 ] || err "$scope : terminaison par signal (code $code)"
  m="$(echo "$summary" | awk '{print $5}')"; k="$(echo "$summary" | awk '{print $7}')"
  n="$(grep -c '^RESULT ' "$log" || true)"
  [ "$n" = "$k" ] || err "$scope : $n lignes RESULT pour $k criteres annonces (sortie tronquee ou incoherente)"
  nfail="$(grep -c '^RESULT [^ ]* FAIL ' "$log" || true)"
  if [ "$code" = 0 ] && { [ "$m" != 0 ] || [ "$nfail" != 0 ]; }; then err "$scope : code 0 avec $m controle(s) en echec et $nfail critere(s) FAIL"; fi
  if [ "$code" != 0 ] && [ "$m" = 0 ]; then err "$scope : code $code sans controle en echec (interruption probable)"; fi
  # inventaire : chaque identifiant attendu exactement une fois, aucun identifiant inattendu
  ids="$(grep '^RESULT ' "$log" | awk '{print $2}' | sort)"
  dups="$(echo "$ids" | uniq -d | tr '\n' ' ')"
  [ -z "${dups// /}" ] || err "$scope : resultats dupliques : $dups"
  want="$(awk -v s="$scope" '$1 == s {print $2}' "$INV" | sort -u)"
  missing="$(comm -23 <(echo "$want") <(echo "$ids" | sort -u) | tr '\n' ' ')"
  extra="$(comm -13 <(echo "$want") <(echo "$ids" | sort -u) | tr '\n' ' ')"
  [ -z "${missing// /}" ] || err "$scope : resultats obligatoires absents (suite partielle) : $missing"
  [ -z "${extra// /}" ] || err "$scope : resultats hors inventaire (inventaire a mettre a jour) : $extra"
  # échecs : exceptions bornées à leur portée
  while read -r line; do
    [ -n "$line" ] || continue
    id="$(echo "$line" | awk '{print $2}')"
    if echo "$known" | grep -qx -- "$scope:$id"; then echo "echec connu (documente) : $scope:$id"
    else err "ECHEC NON ATTENDU ($scope) : ${line:0:400}"; fi
  done < <(grep '^RESULT [^ ]* FAIL ' "$log" || true)
  for kf in $(echo "$known" | grep "^$scope:" | cut -d: -f2-); do
    grep -q "^RESULT $kf PASS " "$log" && echo "information : l'echec connu $scope:$kf passe desormais (liste a revoir)"
  done
  echo "$scope : $n criteres ($nfail FAIL), synthese '$summary', code $code"
done
if [ "$bad" = 0 ]; then echo "BARRIERE : PASS"; else echo "BARRIERE : REFUS"; fi
exit "$bad"
