# SUBDESTRUCKT — suppression récursive bornée (LB-07), partagée par build_local_macos.sh et son harnais de test.
# safe_rm <chemin> : supprime uniquement un sous-chemin strict de $WORK (dossier local-build/ du dépôt) ; refuse un
# chemin vide, /, $HOME, la racine du dépôt, $WORK lui-même, tout chemin hors de $WORK et tout chemin contenant « .. ».
# Variables requises : WORK (absolu, résolu), ROOT. Refus : message sur stderr et code 8 (fonction die si définie).
safe_rm() {
  local p="${1-}"
  _sr_refuse() {
    if declare -F die >/dev/null; then die 8 "$1"; fi
    echo "ARRET (8) : $1" >&2
    return 8
  }
  [ -n "$p" ] || { _sr_refuse "suppression refusee : chemin vide"; return 8; }
  [ -n "${WORK-}" ] || { _sr_refuse "suppression refusee : WORK non defini"; return 8; }
  case "$p" in *..*) _sr_refuse "suppression refusee (chemin relatif) : $p"; return 8 ;; esac
  case "$p" in /|"${HOME-}"|"${HOME-}"/|"${ROOT-/}"|"${ROOT-/}"/|"$WORK"|"$WORK"/) _sr_refuse "suppression refusee : $p"; return 8 ;; esac
  case "$p" in "$WORK"/?*) ;; *) _sr_refuse "suppression refusee hors de local-build/ : $p"; return 8 ;; esac
  rm -rf -- "$p"
}
