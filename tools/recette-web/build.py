# LABTECHNO — construction du site statique de recette (GitHub Pages).
# Usage (depuis la racine du dépôt) : py -3 -I tools/recette-web/build.py <dist_dir> [--key <fichier_clé_local>]
#
# 1. Copie tools/recette-web/src/ dans <dist_dir> (+ .nojekyll).
# 2. Pour chaque campagne de campaigns/index.json : lit l'archive audio au commit indiqué par le manifeste
#    (`git show <commit>:<archive>`), en extrait les fichiers référencés SANS les modifier, et vérifie taille et SHA-256
#    par rapport au manifeste. Vérifie aussi que chaque WAV ne contient que les chunks fmt et data (aucune métadonnée).
# 3. Contrôle de confidentialité sur tout <dist_dir> : aucun fichier KEY*, aucune mention T1/T2, aucun nom de métrique
#    de la clé ; si --key est fourni, aucune ligne de la clé locale (graine, attribution) n'apparaît dans le site.
# Code de sortie 0 seulement si tout est conforme.
import hashlib
import io
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "src")
FORBIDDEN = [
    (re.compile(rb"\bT[12]\b"), "référence temporelle T1/T2"),
    (re.compile(rb"lsd_vs_ref|nmse_vs_ref|render_sr|latence compens", re.I), "colonne de métrique de la clé"),
    (re.compile(rb"Graine du tirage|NE PAS OUVRIR avant", re.I), "contenu du fichier clé"),
]


def sha(b):
    return hashlib.sha256(b).hexdigest()


def wav_chunks(b):
    assert b[:4] == b"RIFF" and b[8:12] == b"WAVE"
    pos, ids = 12, []
    while pos + 8 <= len(b):
        cid, size = b[pos:pos + 4], struct.unpack_from("<I", b, pos + 4)[0]
        ids.append(cid.decode("latin-1"))
        pos += 8 + size + (size & 1)
    return ids


def main():
    args = sys.argv[1:]
    if not args:
        print(__doc__ or "usage: build.py <dist> [--key fichier]")
        return 2
    dist = os.path.abspath(args[0])
    key_path = args[args.index("--key") + 1] if "--key" in args else None
    if os.path.exists(dist):
        shutil.rmtree(dist)
    shutil.copytree(SRC, dist)
    open(os.path.join(dist, ".nojekyll"), "wb").close()

    report, ok = [], True
    index = json.load(open(os.path.join(SRC, "campaigns", "index.json"), encoding="utf-8"))
    for camp in index["campaigns"]:
        mpath = os.path.join(SRC, camp["manifest"])
        m = json.load(open(mpath, encoding="utf-8"))
        src = m["audioSource"]
        blob = subprocess.run(["git", "show", f"{src['commit']}:{src['archive']}"], capture_output=True, check=True).stdout
        report.append(f"campagne {m['campaignId']} : archive {src['archive']} @ {src['commit'][:12]} "
                      f"({len(blob)} octets, SHA-256 {sha(blob)})")
        z = zipfile.ZipFile(io.BytesIO(blob))
        cdir = os.path.dirname(os.path.join(dist, camp["manifest"]))
        for c in m["cases"]:
            for rid, a in c["audio"].items():
                name = os.path.basename(a["file"])
                data = z.read(name)
                good = len(data) == a["bytes"] and sha(data) == a["sha256"]
                chunks = wav_chunks(data)
                meta_ok = chunks == ["fmt ", "data"]
                ok &= good and meta_ok
                out = os.path.join(cdir, a["file"])
                os.makedirs(os.path.dirname(out), exist_ok=True)
                with open(out, "wb") as f:
                    f.write(data)
                report.append(f"  {name:14s} {len(data)} o  sha256 {'OK' if good else 'ÉCHEC'}  chunks {','.join(chunks)}"
                              f"{'' if meta_ok else '  ÉCHEC métadonnées'}")

    # ---- confidentialité
    key_lines = []
    if key_path:
        for line in open(key_path, encoding="utf-8").read().splitlines():
            line = line.strip()
            for tok in re.findall(r"`([0-9a-f]{16})`", line):  # graine
                key_lines.append(tok.encode())
            if line.startswith("| C") and ".wav" in line:  # lignes d'attribution
                key_lines.append(line.encode("utf-8"))
    leaks = []
    for root, _, files in os.walk(dist):
        for fn in files:
            p = os.path.join(root, fn)
            rel = os.path.relpath(p, dist).replace("\\", "/")
            if fn.upper().startswith("KEY"):
                leaks.append(f"{rel} : fichier de clé")
            data = open(p, "rb").read()
            if fn.endswith(".wav"):
                continue  # audio PCM brut : vérifié ci-dessus (fmt + data uniquement)
            for rx, why in FORBIDDEN:
                for mt in rx.finditer(data):
                    leaks.append(f"{rel} : {why} ({mt.group(0)[:40]!r})")
            for kl in key_lines:
                if kl in data:
                    leaks.append(f"{rel} : contenu de la clé locale")
    ok &= not leaks
    report.append(f"confidentialité : {'OK' if not leaks else 'ÉCHEC'} "
                  f"({'avec' if key_path else 'sans'} comparaison à la clé locale, {len(key_lines)} motifs)")
    report.extend("  " + x for x in leaks)
    files = sum(len(f) for _, _, f in os.walk(dist))
    report.append(f"site : {dist} ({files} fichiers)")
    report.append("RÉSULTAT : " + ("OK" if ok else "ÉCHEC"))
    sys.stdout.reconfigure(encoding="utf-8")
    print("\n".join(report))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
