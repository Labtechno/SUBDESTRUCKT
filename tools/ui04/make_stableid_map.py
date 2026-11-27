#!/usr/bin/env python3
"""SUBDESTRUCKT ARCH-UI-04 : cartographie StableID -> interface, produite depuis l'artefact du test UI04-COV.

Usage : python tools/ui04/make_stableid_map.py <ui04_map.tsv> <sortie.md> <sha_source>
L'entrée est écrite par subdestruckt_wrapper_tests (3e argument) : chaque ligne provient de l'exploration réelle de
l'éditeur compilé (commande liée visible trouvée), jamais d'une affectation déclarative.
"""
import collections
import sys


def main() -> int:
    src, out, sha = sys.argv[1], sys.argv[2], sys.argv[3]
    rows = []
    with open(src, encoding="utf-8") as f:
        header = f.readline().rstrip("\n").split("\t")
        for line in f:
            rows.append(dict(zip(header, line.rstrip("\n").split("\t"))))
    missing = [r for r in rows if r["access"] == "NON ATTEINT"]
    by_space = collections.Counter(r["access"].split(" / ")[0] for r in rows if r["access"] != "NON ATTEINT")
    contextual = sum(1 for r in rows if " ; " in r["access"])
    with open(out, "w", encoding="utf-8", newline="\n") as o:
        o.write("# ARCH-UI-04 — Cartographie StableID (preuve d'implémentation)\n\n")
        o.write(f"Source : `{sha}`, registre compilé (`source/core/ParamTable.inc`). Générée par "
                "`tools/ui04/make_stableid_map.py` depuis l'artefact du test **UI04-COV** "
                "(`subdestruckt_wrapper_tests`, 3e argument) : chaque ligne indique le premier chemin de navigation où "
                "une commande **liée et visible** du StableID a été trouvée dans l'éditeur réel, avec les sélections "
                "d'algorithme ou de mode nécessaires (« ; »). Le test UI04-BIND édite ensuite chaque StableID par cette "
                "commande et vérifie la valeur Base hôte. Remplace la cartographie candidate ARCH-UI-03 (695 IDs, "
                "affectations documentaires).\n\n")
        o.write(f"Total : **{len(rows)}** StableID ; atteints : **{len(rows) - len(missing)}** ; non atteints : "
                f"**{len(missing)}** ; accès contextuel (algorithme/mode requis) : {contextual}.\n\n")
        o.write("| Espace d'accès principal | StableID |\n|---|---|\n")
        for k, v in sorted(by_space.items()):
            o.write(f"| {k} | {v} |\n")
        o.write("\n`états` = nombre d'états de navigation explorés où une commande du StableID est visible "
                "(alias et contextes compatibles compris).\n\n")
        o.write("| # | StableID | Nom registre | Type | Accès vérifié (contexte ; sélections) | états |\n|---|---|---|---|---|---|\n")
        for r in rows:
            acc = r["access"].replace("|", "/")
            o.write(f"| {r['index']} | `{r['stableId']}` | {r['name']} | {r['type']} | {acc} | {r['visibleStates']} |\n")
    print(f"{len(rows)} lignes, {len(missing)} non atteintes -> {out}")
    return 1 if missing or len(rows) != 703 else 0


if __name__ == "__main__":
    sys.exit(main())
