#!/usr/bin/env python3
"""SUBDESTRUCKT — générateur du manuel utilisateur (HTML statique, sans dépendance externe).

Usage : python manual/build.py [--version 0.1.0-b5] [--out manual/dist]

Assemble pour chaque version :  dist/<version>/index.html (manuel), dist/<version>/recette.html (assistant),
dist/<version>/version.json (manifeste), dist/<version>/schema/results.schema.json, ressources copiées.
Génère dist/index.html (liste des versions). Effectue des contrôles de cohérence bloquants :
  - plages / unités / défauts / lissages des paramètres = registre de la version (0.1.0-b5 : oracle figé
    tests/regression/b5/Parameters.cpp ; 0.2.0 : source/core/ParamTable.inc, 684 paramètres) ;
  - tout fichier référencé (src, href locaux, audio) existe ; aucun marqueur {{…}} résiduel ;
  - identifiants HTML uniques ; aucune ressource externe (http(s) hors liens de téléchargement listés) ;
  - aucune adresse électronique ; aucun jeton ;
  - tests R-01..R-15 complets et ordonnés ; exemples audio présents avec leur SHA-256.
"""
import argparse
import hashlib
import html
import json
import re
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parent
ALLOWED_EXTERNAL = (
    "https://github.com/Labtechno/SUBDESTRUCKT/raw/b49a09e3efd6fe52fdb75a23c5b12dd794f696e8/deliveries/0.1.0-b5/",
)
esc = html.escape


def fail(msg):
    print(f"ECHEC : {msg}")
    sys.exit(1)


def sha256(p):
    return hashlib.sha256(Path(p).read_bytes()).hexdigest()


def fmt_num(v):
    s = f"{v:g}"
    return s.replace(".", ",").replace("-", "−")


def parse_param_registry_g4():
    text = (REPO / "source/core/ParamTable.inc").read_text(encoding="utf-8")
    rx = re.compile(r'SBD_PARAM\((\w+), "([^"]+)", "([^"]+)", ParamType::(\w+), "([^"]*)", (-?[\d.e+-]+)f, (-?[\d.e+-]+)f, (-?[\d.e+-]+)f, ([\d.]+)f, (true|false), (\d+), Domain::(\w+), [^,]+, (true|false), [^,]+, [^,]+, [^,]+, "([^"]*)", "([^"]*)", (\d+)\)')
    out = {}
    for m in rx.finditer(text):
        _, sid, name, typ, unit, lo, hi, d, sm, auto, ver, dom, rnd, choices, group, stage = m.groups()
        out[sid] = dict(name=name, unit=unit, type=typ, min=float(lo), max=float(hi), default=float(d), smoothingMs=float(sm),
                        automatable=auto == "true", version=int(ver), choices=choices.split("|") if choices else [], group=group)
    if len(out) < 600:
        fail(f"registre G4 : {len(out)} entrées lues")
    return out


def param_table_g4(reg):
    rows, group = [], None
    for sid, r in reg.items():
        if r["group"] != group:
            group = r["group"]
            rows.append(f'<tr class="grp"><th colspan="6" scope="colgroup">{esc(group)}</th></tr>')
        if r["type"] == "Choice":
            rng = " / ".join(r["choices"])
            dft = r["choices"][int(r["default"])]
        elif r["type"] == "Bool":
            rng, dft = "Off / On", "On" if r["default"] >= 0.5 else "Off"
        else:
            rng, dft = f'{fmt_num(r["min"])} à {fmt_num(r["max"])}', fmt_num(r["default"])
        rows.append(f'<tr><th scope="row">{esc(r["name"])}</th><td><code>{esc(sid)}</code></td><td>{esc(rng)}</td>'
                    f'<td>{esc(r["unit"] or "—")}</td><td>{esc(dft)}</td><td>{"oui" if r["automatable"] else "non (structurel)"}</td></tr>')
    return ('<div class="table-wrap"><table class="params"><caption>Registre complet : ' + str(len(reg)) + ' paramètres, générés depuis '
            'source/core/ParamTable.inc (identifiants stables)</caption><thead><tr><th scope="col">Paramètre</th><th scope="col">StableID</th>'
            '<th scope="col">Plage ou choix</th><th scope="col">Unité</th><th scope="col">Défaut</th><th scope="col">Automatisable</th></tr></thead><tbody>'
            + "".join(rows) + '</tbody></table></div>')


def parse_param_registry(kind="b5"):
    if kind == "g4":
        return parse_param_registry_g4()
    text = (REPO / "tests/regression/b5/Parameters.cpp").read_text(encoding="utf-8")
    rx = re.compile(r'\{"([a-z0-9_.]+)", "([^"]+)", "([^"]+)", (-?[\d.]+)f, (-?[\d.]+)f, (-?[\d.]+)f, ([\d.]+)f, (true|false)')
    out = {}
    for m in rx.finditer(text):
        sid, name, unit, lo, hi, d, sm, auto = m.groups()
        out[sid] = dict(name=name, unit=unit, min=float(lo), max=float(hi), default=float(d), smoothingMs=float(sm), automatable=auto == "true")
    if len(out) != 22:
        fail(f"registre des paramètres : {len(out)} entrées lues (22 attendues)")
    return out


UNIT_FR = {"s": "s", "lin": "—", "dB": "dB", "x f0": "× note", "rad": "rad"}


def param_table(reg, pdoc):
    ids = [i for g in pdoc["groups"] for i in g["ids"]]
    if sorted(ids) != sorted(reg) or sorted(pdoc["params"]) != sorted(reg):
        fail("parameters.json ne couvre pas exactement les 22 paramètres du registre")
    rows = []
    for g in pdoc["groups"]:
        rows.append(f'<tr class="grp"><th colspan="6" scope="colgroup">{esc(g["title"])}</th></tr>')
        for sid in g["ids"]:
            r = reg[sid]
            sm = f'{fmt_num(r["smoothingMs"])} ms' if r["smoothingMs"] > 0 else "immédiat"
            rows.append(f'<tr><th scope="row"><a href="#p-{sid}">{esc(r["name"])}</a></th><td>{fmt_num(r["min"])} à {fmt_num(r["max"])}</td>'
                        f'<td>{esc(UNIT_FR[r["unit"]])}</td><td>{fmt_num(r["default"])}</td><td>{sm}</td><td>{"oui" if r["automatable"] else "non"}</td></tr>')
    table = ('<div class="table-wrap"><table class="params"><caption>Tableau récapitulatif des 22 paramètres</caption>'
             '<thead><tr><th scope="col">Paramètre</th><th scope="col">Plage</th><th scope="col">Unité</th><th scope="col">Défaut</th>'
             '<th scope="col">Lissage</th><th scope="col">Automatisable</th></tr></thead><tbody>' + "".join(rows) + '</tbody></table></div>')
    cards = []
    for g in pdoc["groups"]:
        cards.append(f'<h3>{esc(g["title"])}</h3>')
        for sid in g["ids"]:
            r, d = reg[sid], pdoc["params"][sid]
            cards.append(
                f'<details class="adv param" id="p-{sid}"><summary>{esc(r["name"])} <code>{sid}</code></summary><dl class="kv">'
                f'<dt>Fonction</dt><dd>{esc(d["function"])}</dd><dt>Intérêt musical</dt><dd>{esc(d["interest"])}</dd>'
                f'<dt>Plage</dt><dd>{fmt_num(r["min"])} à {fmt_num(r["max"])} {esc(r["unit"])}</dd><dt>Défaut</dt><dd>{fmt_num(r["default"])} {esc(r["unit"])}</dd>'
                f'<dt>Comportement</dt><dd>{esc(d["behaviour"])}</dd><dt>Effet sonore</dt><dd>{esc(d["sound"])}</dd>'
                f'<dt>Automatisable</dt><dd>{"oui" if r["automatable"] else "non"}</dd><dt>Interactions</dt><dd>{esc(d["interactions"])}</dd></dl></details>')
    return table + "<h3>Fiches détaillées</h3><p>Ouvrez une fiche pour lire la fonction, l'intérêt musical, le comportement et les interactions de chaque paramètre.</p>" + "".join(cards)


def audio_block(ex_by_id, ids, label):
    btns = []
    for k, i in enumerate(ids):
        if i not in ex_by_id:
            fail(f"exemple audio inconnu : {i}")
        e = ex_by_id[i]
        tag = "AB"[k] + " · " if len(ids) == 2 else ""
        btns.append(f'<button type="button" class="btn play" data-src="audio/{e["file"]}" aria-pressed="false"><span class="state" aria-hidden="true">▶</span> {esc(tag)}{esc(e["title"])}</button>')
    meta = " · ".join(f'{ex_by_id[i]["seconds"]:.1f} s' for i in ids)
    return (f'<div class="ab" role="group" aria-label="{esc(label)}"><h4>{esc(label)}</h4><div class="players">{"".join(btns)}</div>'
            f'<p class="meta">Rendu du moteur réel {esc(CURRENT["version"])} · 48 kHz · normalisé à −20 dBFS RMS · {meta} · détails au chapitre {CURRENT["audioChapter"]}</p></div>')


def audio_catalog(examples, audio_dir):
    out = ['<div class="table-wrap"><table><caption>Catalogue des rendus (paramètres complets dans <a href="audio/examples.json">examples.json</a>)</caption>'
           '<thead><tr><th scope="col">Exemple</th><th scope="col">Note</th><th scope="col">Durée</th><th scope="col">Relâchement</th>'
           '<th scope="col">Gain de normalisation</th><th scope="col">Crête</th><th scope="col">SHA-256 (début)</th></tr></thead><tbody>']
    for e in examples:
        h = sha256(audio_dir / e["file"])
        e["sha256"] = h
        out.append(f'<tr><th scope="row">{esc(e["id"])}</th><td>{e["note"]}</td><td>{e["seconds"]:.1f} s</td><td>{e["noteOffAt"]:.2f} s</td>'
                   f'<td>{fmt_num(round(e["gainDb"], 2))} dB</td><td>{fmt_num(round(e["peakDbfs"], 1))} dBFS</td><td><code>{h[:16]}</code></td></tr>')
    out.append("</tbody></table></div>")
    for e in examples:
        diffs = {k: v for k, v in e["params"].items() if v != DEFAULTS.get(k)}
        dtxt = ", ".join(f"{k} = {fmt_num(v)}" for k, v in diffs.items()) or "aucune (réglages par défaut)"
        out.append(f'<h3 id="ex-{e["id"]}">{esc(e["id"])} — {esc(e["title"])}</h3>')
        out.append(f'<p>Différences avec Preset init : <code>{esc(dtxt)}</code>.' + (f' Automatisation : {esc(e["automation"])}.' if e.get("automation") else "") +
                   f' Note {e["note"]}, vélocité {e["velocity"]}, 48 kHz, K = 1.</p>')
        out.append(audio_block({e["id"]: e}, [e["id"]], f'Écouter {e["id"]}'))
    return "".join(out)


DEFAULTS = {}
CURRENT = {"version": "0.1.0-b5", "audioChapter": 12}


def exercises_html(exs, ex_by_id):
    out = []
    for x in exs:
        aud = ""
        if x.get("audio"):
            aud = audio_block(ex_by_id, [x["audio"]], f'Référence sonore de l\'exercice {x["n"]}')
        if x.get("ab"):
            aud = audio_block(ex_by_id, x["ab"], f'Avant / après — exercice {x["n"]}')
        out.append(
            f'<article class="exercise" id="ex{x["n"]}"><h2>Exercice {x["n"]} — {esc(x["title"])}</h2>'
            f'<div class="meta"><span class="chip">{esc(x["level"])}</span><span class="chip">{esc(x["duration"])}</span>'
            + "".join(f'<span class="chip">{esc(c)}</span>' for c in x["concepts"]) + '</div>'
            f'<dl><dt>Objectif</dt><dd>{esc(x["goal"])}</dd><dt>Réglage de départ</dt><dd>{esc(x["start"])}</dd>'
            f'<dt>Étapes</dt><dd><ol>' + "".join(f"<li>{esc(s)}</li>" for s in x["steps"]) + '</ol></dd>'
            f'<dt>Valeurs</dt><dd><code>{esc(x["values"])}</code></dd><dt>Résultat attendu</dt><dd>{esc(x["expected"])}</dd>'
            f'<dt>À écouter</dt><dd>{esc(x["listen"])}</dd><dt>Erreurs fréquentes</dt><dd>{esc(x["mistakes"])}</dd>'
            f'<dt>Variante</dt><dd>{esc(x["variation"])}</dd></dl>{aud}</article>')
    return "".join(out)


def recette_summary(tests):
    by = {t["id"]: t for t in tests["tests"]}
    out = []
    for g in tests["groups"]:
        out.append(f'<h3>{esc(g["title"])}</h3><div class="table-wrap"><table><thead><tr><th scope="col">Test</th><th scope="col">Objet</th><th scope="col">Critère officiel</th></tr></thead><tbody>')
        for tid in g["tests"]:
            t = by[tid]
            out.append(f'<tr><th scope="row"><a href="recette.html#{tid}">{tid}</a></th><td>{esc(t["title"])}</td><td>{esc(t["official"]["expected"])}</td></tr>')
        out.append("</tbody></table></div>")
    return "".join(out)


def glossary_html(gl):
    items = sorted(gl, key=lambda t: t["term"].lower().replace("é", "e"))
    out, letter = ['<dl class="glossary">'], None
    for t in items:
        L = t["term"][0].upper().replace("É", "E")
        if L != letter:
            letter = L
            out.append(f'<div class="letter" aria-hidden="true">{L}</div>')
        al = f' <span class="d">({esc(", ".join(t["aliases"]))})</span>' if t["aliases"] else ""
        out.append(f'<dt id="gl-{t["id"]}">{esc(t["term"])}{al}</dt><dd>{esc(t["long"])} <a href="#{t["see"]}">Voir le chapitre</a>.</dd>')
    out.append("</dl>")
    return "".join(out)


def check_output(html_text, outdir, name):
    if "{{" in html_text:
        fail(f"{name} : marqueur résiduel {re.search(r'{{[^}]*}}', html_text).group(0)}")
    if re.search(r"[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[a-z]{2,}", html_text):
        fail(f"{name} : adresse électronique détectée")
    if re.search(r"gh[pousr]_[A-Za-z0-9]{20,}|github_pat_", html_text):
        fail(f"{name} : jeton détecté")
    ids = re.findall(r'\sid="([^"]+)"', html_text)
    dup = {i for i in ids if ids.count(i) > 1}
    if dup:
        fail(f"{name} : identifiants dupliqués {sorted(dup)[:8]}")
    idset = set(ids)
    for attr, ref in re.findall(r'\s(src|href|data-src)="([^"]+)"', html_text):
        if ref.startswith("#"):
            if ref[1:] and ref[1:] not in idset:
                fail(f"{name} : ancre interne absente {ref}")
            continue
        if ref.startswith("http"):
            if not ref.startswith(ALLOWED_EXTERNAL):
                fail(f"{name} : ressource externe non autorisée {ref}")
            continue
        path = ref.split("#")[0]
        if path in ("../", ""):
            continue
        if not (outdir / path).exists():
            fail(f"{name} : fichier référencé absent {ref}")


def build_version(ver, out_root, meta):
    src = ROOT / "src" / ver
    out = out_root / ver
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    for d in ("css", "js", "assets", "audio", "recette"):
        shutil.copytree(src / d, out / d)
    kind = meta.get("registry", "b5")
    reg = parse_param_registry(kind)
    CURRENT.update(version=ver, audioChapter=meta.get("audioChapter", 12))
    DEFAULTS.clear()
    DEFAULTS.update({k: v["default"] for k, v in reg.items()})
    pdoc = json.loads((src / "parameters.json").read_text(encoding="utf-8")) if kind == "b5" else None
    gl = json.loads((src / "glossary.json").read_text(encoding="utf-8"))
    tests = json.loads((src / "recette/tests.json").read_text(encoding="utf-8"))
    exs = json.loads((src / "exercises.json").read_text(encoding="utf-8"))
    exdoc = json.loads((src / "audio/examples.json").read_text(encoding="utf-8"))
    ex_by_id = {e["id"]: e for e in exdoc["examples"]}
    if exdoc["engineSourceCommit"] != meta["buildSourceSha"]:
        fail("examples.json : commit moteur différent du commit source de la build")
    for e in exdoc["examples"]:
        for k, v in e["params"].items():
            if k not in reg or not (reg[k]["min"] <= v <= reg[k]["max"]):
                fail(f"exemple {e['id']} : paramètre {k}={v} hors registre")
    ids = [t["id"] for t in tests["tests"]]
    nrec = meta.get("recetteCount", 15)
    if ids != [f"R-{i:02d}" for i in range(1, nrec + 1)] or sorted(sum((g["tests"] for g in tests["groups"]), [])) != ids:
        fail(f"tests.json : R-01..R-{nrec:02d} incomplets ou mal groupés")
    if len(exs) < 8:
        fail("moins de 8 exercices")

    chapters = sorted((src / "chapters").glob("*.html"))
    nch = meta.get("chapterCount", 16)
    if len(chapters) != nch:
        fail(f"{len(chapters)} chapitres ({nch} attendus)")
    sections = []
    toc = []
    for path in chapters:
        t = path.read_text(encoding="utf-8")
        cid = re.search(r'<section class="chapter" id="([^"]+)"', t).group(1)
        h1 = re.search(r"<h1>(.*?)</h1>", t).group(1)
        num = int(path.name[:2])
        toc.append((cid, num, re.sub("<[^>]+>", "", h1)))
        sections.append([cid, t])
    for k, (cid, t) in enumerate(sections):
        prev = f'<a href="#{toc[k-1][0]}" rel="prev">← {toc[k-1][1]}. {esc(toc[k-1][2])}</a>' if k else "<span></span>"
        nxt = f'<a href="#{toc[k+1][0]}" rel="next">{toc[k+1][1]}. {esc(toc[k+1][2])} →</a>' if k + 1 < len(sections) else "<span></span>"
        t = t.replace("</section>", f'<nav class="chapter-nav" aria-label="Chapitres précédent et suivant">{prev}{nxt}</nav></section>', 1)
        sections[k][1] = t
    body = "\n".join(t for _, t in sections)
    body = body.replace("{{PARAM_TABLE}}", param_table(reg, pdoc) if kind == "b5" else param_table_g4(reg))
    body = body.replace("{{AUDIO_CATALOG}}", audio_catalog(exdoc["examples"], src / "audio"))
    body = body.replace("{{EXERCISES}}", exercises_html(exs, ex_by_id))
    body = body.replace("{{RECETTE_SUMMARY}}", recette_summary(tests))
    body = body.replace("{{GLOSSARY}}", glossary_html(gl))
    body = re.sub(r"{{AUDIO:([a-z0-9_]+)}}", lambda m: audio_block(ex_by_id, [m.group(1)], ex_by_id[m.group(1)]["title"]), body)
    body = re.sub(r"{{AB:([a-z0-9_]+)\|([a-z0-9_]+)\|([^}]+)}}", lambda m: audio_block(ex_by_id, [m.group(1), m.group(2)], m.group(3)), body)

    toc_html = "".join(f'<li><a href="#{cid}" data-chapter="{cid}"><span class="num">{n:02d}</span>{esc(title)}</a></li>' for cid, n, title in toc)
    glossary_js = json.dumps([{k: t[k] for k in ("id", "term", "aliases", "short")} for t in gl], ensure_ascii=False)
    tpl = (ROOT / "templates/manual.html").read_text(encoding="utf-8")
    page = (tpl.replace("{{VERSION}}", ver).replace("{{BUILD_SHA}}", meta["buildSourceSha"]).replace("{{DOC_SHA}}", meta["documentationSha"])
            .replace("{{TOC}}", toc_html).replace("{{CHAPTERS}}", body).replace("{{GLOSSARY_JSON}}", glossary_js.replace("</", "<\\/")))

    rcfg = dict(campaignId=tests["campaignId"], version=ver, buildSha=meta["buildSourceSha"], docSha=meta["documentationSha"],
                storageKey=meta.get("recetteStorageKey"),
                exportFileName=meta.get("exportFileName", "SUBDESTRUCKT_ARCH_G3_RESULTS.json"), groups=tests["groups"], tests=tests["tests"])
    rtpl = (ROOT / "templates/recette.html").read_text(encoding="utf-8")
    rpage = (rtpl.replace("{{VERSION}}", ver).replace("{{BUILD_SHA}}", meta["buildSourceSha"])
             .replace("{{RECETTE_CONFIG}}", json.dumps(rcfg, ensure_ascii=False).replace("</", "<\\/")))
    (out / "recette.html").write_text(rpage, encoding="utf-8")
    (out / "index.html").write_text(page, encoding="utf-8")

    (out / "schema").mkdir()
    shutil.copy(ROOT / "schema/results.schema.json", out / "schema/results.schema.json")
    files = sorted(p for p in out.rglob("*") if p.is_file())
    manifest = dict(meta, version=ver, generator="manual/build.py",
                    audioExamples=[dict(id=e["id"], file=f'audio/{e["file"]}', sha256=e["sha256"]) for e in exdoc["examples"]],
                    screenshot=dict(file=meta.get("screenshot", "assets/ui_diagnostic_macos.png"), sha256=sha256(out / meta.get("screenshot", "assets/ui_diagnostic_macos.png"))),
                    files={str(p.relative_to(out)).replace("\\", "/"): sha256(p) for p in files})
    (out / "version.json").write_text(json.dumps(manifest, ensure_ascii=False, indent=2), encoding="utf-8")
    check_output(page, out, "index.html")
    check_output(rpage, out, "recette.html")
    print(f"{ver} : {len(files) + 1} fichiers, {len(page) // 1024} Kio de HTML")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=str(ROOT / "dist"))
    a = ap.parse_args()
    out_root = Path(a.out)
    versions = json.loads((ROOT / "versions.json").read_text(encoding="utf-8"))
    out_root.mkdir(parents=True, exist_ok=True)
    for v in versions["versions"]:
        build_version(v["version"], out_root, v)
    items = "".join(f'<li><a href="{v["version"]}/">SUBDESTRUCKT {v["version"]}</a> — {esc(v["label"])} '
                    f'(<a href="{v["version"]}/recette.html">assistant de recette</a>)</li>' for v in versions["versions"])
    idx = (ROOT / "templates/versions.html").read_text(encoding="utf-8").replace("{{ITEMS}}", items)
    check_output(idx, out_root, "manual/index.html")
    (out_root / "index.html").write_text(idx, encoding="utf-8")
    print("OK")


if __name__ == "__main__":
    main()
