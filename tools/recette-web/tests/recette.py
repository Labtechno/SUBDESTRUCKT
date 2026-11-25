# LABTECHNO — recette automatisée de la page d'écoute (Playwright, moteur WebKit, émulation iPhone).
# Usage : python tests/recette.py <url_de_base> <dossier_sortie> [--repo <racine_dépôt>]
#   <url_de_base> : http://localhost:PORT/ (dist servi localement) ou l'URL GitHub Pages.
# IMPORTANT : WebKit Playwright n'est PAS Safari iOS. Ces tests sont une simulation (moteur WebKit + viewport,
# user-agent et tactile d'iPhone) ; ils ne remplacent pas un essai sur un vrai iPhone.
import hashlib
import io
import json
import os
import re
import subprocess
import sys
import time
import urllib.request
import zipfile

from playwright.sync_api import sync_playwright

BASE = sys.argv[1].rstrip("/") + "/"
OUT = sys.argv[2]
REPO = sys.argv[sys.argv.index("--repo") + 1] if "--repo" in sys.argv else None
os.makedirs(OUT, exist_ok=True)
results = []


CUR = "http"


def check(name, ok, detail=""):
    status = "N/A" if ok is None else ("OK" if ok else "ÉCHEC")
    results.append({"engine": CUR, "test": name, "status": status, "detail": detail})
    print(f"[{CUR:8s}] [{status}] {name}{' — ' + detail if detail else ''}", flush=True)


def get(url):
    with urllib.request.urlopen(url) as r:
        return r.read()


# ---------------------------------------------------------------- 1. intégrité des fichiers publiés (hors navigateur)
index = json.loads(get(BASE + "campaigns/index.json"))
entry = [c for c in index["campaigns"] if c["id"] == index["default"]][0]
manifest = json.loads(get(BASE + entry["manifest"]))
mbase = BASE + entry["manifest"].rsplit("/", 1)[0] + "/"
zipref = None
if REPO:
    src = manifest["audioSource"]
    blob = subprocess.run(["git", "-C", REPO, "show", f"{src['commit']}:{src['archive']}"], capture_output=True, check=True).stdout
    zipref = zipfile.ZipFile(io.BytesIO(blob))
n_ok = 0
for c in manifest["cases"]:
    for rid, a in c["audio"].items():
        data = get(mbase + a["file"])
        h = hashlib.sha256(data).hexdigest()
        same_zip = zipref is None or hashlib.sha256(zipref.read(os.path.basename(a["file"]))).hexdigest() == h
        n_ok += h == a["sha256"] and len(data) == a["bytes"] and same_zip
check("12 WAV servis = manifeste = archive du commit audio (SHA-256)", n_ok == 12, f"{n_ok}/12")

# ---------------------------------------------------------------- 2. confidentialité des ressources publiques
public = ["index.html", "css/labtechno.css", "js/app.js", "js/dom.js", "js/listening-engine.js", "js/questionnaire.js",
          "js/session-store.js", "campaigns/index.json", entry["manifest"]]
leaks = []
for p in public:
    t = get(BASE + p)
    for rx in (rb"\bT[12]\b", rb"lsd_vs_ref|nmse_vs_ref", rb"Graine du tirage"):
        if re.search(rx, t):
            leaks.append(f"{p}:{rx!r}")
for probe in ["campaigns/s01-final/KEY_NE_PAS_OUVRIR_AVANT_ECOUTE.md", "KEY_NE_PAS_OUVRIR_AVANT_ECOUTE.md", "campaigns/s01-final/KEY_final_metrics.csv"]:
    try:
        get(BASE + probe)
        leaks.append(f"{probe} accessible")
    except Exception:
        pass
check("Aucune clé X/Y ni métrique d'attribution dans les ressources publiques", not leaks, "; ".join(leaks) or f"{len(public)} fichiers + 3 sondes")

# ---------------------------------------------------------------- 3. navigateur
def browser_suite(pw, engine):
    global CUR
    CUR = engine
    dev = pw.devices["iPhone 13"]
    browser = getattr(pw, engine).launch()
    ctx = browser.new_context(**dev, accept_downloads=True)
    if engine == "chromium":
        ctx.grant_permissions(["clipboard-read", "clipboard-write"])
    page = ctx.new_page()
    errors = []
    page.on("pageerror", lambda e: errors.append(str(e)))
    page.on("console", lambda m: errors.append(m.text) if m.type == "error" else None)
    ev = lambda js: page.evaluate(js)

    page.goto(BASE, wait_until="load")
    page.wait_for_function("() => window.__recette && window.__recette.engine.integrity.size === 12", timeout=30000)
    page.wait_for_selector("text=12/12 fichiers vérifiés", timeout=30000)
    integ = ev("() => [...window.__recette.engine.integrity.values()].filter(r => r.ok).length")
    check("Chargement des 12 fichiers audio + SHA-256 vérifié dans le navigateur", integ == 12, f"{integ}/12, contexte {ev('() => window.__recette.engine.sampleRate')} Hz")
    check("Aucune lecture automatique au chargement", ev("() => !window.__recette.engine.playing && window.__recette.engine.liveSources === 0"))
    page.screenshot(path=os.path.join(OUT, f"{engine}_iphone13_accueil.png"), full_page=True)

    # ---- lecture, bascule, exclusivité
    has_audio = ev("() => typeof (window.AudioContext || window.webkitAudioContext) === 'function'")
    if not has_audio:
        check("Lecture, bascule, exclusivité, pause/reprise, boucle", None, "NON EXÉCUTABLE : Web Audio absent de ce moteur")
        page.goto(BASE + "#/cas/C1")
        page.wait_for_selector("h1:has-text('C1')")
    else:
        page.tap("button.primary:has-text('Commencer')")
        page.wait_for_selector(".rbtn[data-id=REF]:not([disabled])")
        check("Page de cas sans lecture automatique", ev("() => !window.__recette.engine.playing && window.__recette.engine.liveSources === 0"))
        page.tap(".rbtn[data-id=REF]")
        time.sleep(0.6)
        st = ev("() => ({s: window.__recette.engine.ctx.state, p: window.__recette.engine.position('C1'), a: window.__recette.engine.active && window.__recette.engine.active.id})")
        check("Lecture REF (contexte audio actif, position qui avance)", st["s"] == "running" and st["p"] > 0.3 and st["a"] == "REF", json.dumps(st))
        p0 = ev("() => window.__recette.engine.position('C1')")
        page.tap(".rbtn[data-id=X]")
        p1 = ev("() => window.__recette.engine.position('C1')")
        time.sleep(0.1)
        live = ev("() => window.__recette.engine.liveSources")
        lit = ev("() => document.querySelectorAll('.rbtn.live').length")
        check("Bascule REF → X à la même position temporelle", abs(p1 - p0) < 0.12 and ev("() => window.__recette.engine.active.id") == "X", f"avant {p0:.3f} s, après {p1:.3f} s")
        check("Lecture exclusive après bascule (1 source vivante, 1 bouton actif)", live == 1 and lit == 1, f"sources {live}, boutons {lit}")
        for rid in ["Y", "REF", "X", "Y", "REF"]:
            page.tap(f".rbtn[data-id={rid}]")
        time.sleep(0.1)
        live = ev("() => window.__recette.engine.liveSources")
        check("Exclusivité après 5 bascules rapides", live == 1 and ev("() => window.__recette.engine.active.id") == "REF", f"sources vivantes {live}")
        page.tap(".btn.play")
        time.sleep(0.05)
        pp = ev("() => window.__recette.engine.position('C1')")
        time.sleep(0.5)
        pp2 = ev("() => window.__recette.engine.position('C1')")
        check("Pause : position figée", not ev("() => window.__recette.engine.playing") and abs(pp2 - pp) < 1e-6, f"{pp:.3f} s")
        time.sleep(0.1)
        check("Pause : plus aucune source vivante", ev("() => window.__recette.engine.liveSources") == 0)
        page.tap(".btn.play")
        pr = ev("() => window.__recette.engine.position('C1')")
        check("Reprise au même emplacement", abs(pr - pp) < 0.05 and ev("() => window.__recette.engine.playing"), f"{pp:.3f} → {pr:.3f} s")
        page.tap(".case-nav .btn.primary")
        page.wait_for_selector("h1:has-text('C2')")
        time.sleep(0.1)
        check("Navigation suivant : C2 affiché, lecture arrêtée", not ev("() => window.__recette.engine.playing") and ev("() => window.__recette.engine.liveSources") == 0)
        page.tap(".case-nav .btn:not(.primary)")
        page.wait_for_selector("h1:has-text('C1')")
        pc = ev("() => window.__recette.engine.position('C1')")
        check("Navigation précédent : position de C1 conservée", abs(pc - pr) < 0.2 and pc > 0, f"{pc:.3f} s")
        page.screenshot(path=os.path.join(OUT, f"{engine}_iphone13_cas_lecture.png"))

        # ---- boucle
        ev("() => window.__recette.engine.seek('C1', 2.6)")
        page.tap(".rbtn[data-id=X]")
        time.sleep(0.9)
        check("Sans boucle : arrêt en fin de fichier", not ev("() => window.__recette.engine.playing"))
        page.tap(".transport2 .btn >> text=Boucle")
        ev("() => window.__recette.engine.seek('C1', 2.6)")
        page.tap(".rbtn[data-id=X]")
        time.sleep(0.9)
        lp = ev("() => window.__recette.engine.position('C1')")
        check("Avec boucle : lecture continue après la fin", ev("() => window.__recette.engine.playing") and lp < 1.0, f"position {lp:.3f} s")
        page.tap(".btn.play")
        page.tap(".transport2 .btn >> text=Boucle")

    # ---- questionnaire + sauvegarde immédiate
    key = ev("() => window.__recette.store.key")
    page.tap("[data-q=audibleDifference] .seg-opt >> text=Oui")
    saved = json.loads(ev(f"() => localStorage.getItem('{key}')"))
    check("Sauvegarde immédiate d'une réponse (localStorage)", saved["cases"]["C1"]["audibleDifference"] == "yes")
    page.tap("[data-q=closestToRef] .seg-opt >> text=X")
    page.tap("[data-q=artisticPreference] .seg-opt >> text=Aucune préférence")
    page.fill("[data-q=observations] textarea", "Essai automatisé — aigus")
    saved = json.loads(ev(f"() => localStorage.getItem('{key}')"))
    check("Sauvegarde du texte à la frappe", saved["cases"]["C1"]["observations"] == "Essai automatisé — aigus" and saved["answeredCases"] == 1)
    page.tap("[data-q=closestToRef] .seg-opt >> text=Y")
    saved = json.loads(ev(f"() => localStorage.getItem('{key}')"))
    check("Modification d'une réponse avant validation", saved["cases"]["C1"]["closestToRef"] == "Y")
    page.screenshot(path=os.path.join(OUT, f"{engine}_iphone13_cas_questions.png"), full_page=True)

    page.reload(wait_until="load")
    page.wait_for_selector("[data-q=closestToRef]")
    restored = ev("""() => ({d: document.querySelector('[data-q=audibleDifference] input[value=yes]').checked,
                             c: document.querySelector('[data-q=closestToRef] input[value=Y]').checked,
                             o: document.querySelector('[data-q=observations] textarea').value})""")
    check("Restauration après rechargement", restored == {"d": True, "c": True, "o": "Essai automatisé — aigus"}, json.dumps(restored, ensure_ascii=False))
    sid = ev("() => window.__recette.store.doc.sessionId")
    page.close()
    page = ctx.new_page()
    ev = lambda js: page.evaluate(js)
    page.goto(BASE, wait_until="load")
    page.wait_for_function("() => window.__recette && window.__recette.store")
    check("Restauration après fermeture de l'onglet (même session)", ev("() => window.__recette.store.doc.sessionId") == sid
          and ev("() => window.__recette.store.doc.cases.C1.closestToRef") == "Y")

    # ---- compléter les autres cas
    for cid in ["C2", "C3", "C5"]:
        page.goto(BASE + f"#/cas/{cid}")
        page.wait_for_selector(f"h1:has-text('{cid}')")
        page.tap("[data-q=audibleDifference] .seg-opt >> text=Incertain")
        page.tap("[data-q=closestToRef] .seg-opt >> text=Aucune différence discernable")
        page.tap("[data-q=artisticPreference] .seg-opt >> text=X")
    check("4 cas complétés → statut complete", ev("() => window.__recette.store.doc.status") == "complete")

    # ---- résultats : export, copie, validation, réinitialisation
    page.goto(BASE + "#/resultats")
    page.wait_for_selector("textarea.json")
    live_json = json.loads(page.input_value("textarea.json"))
    check("JSON affiché en texte = document sauvegardé", live_json == json.loads(ev(f"() => localStorage.getItem('{key}')")))
    with page.expect_download() as dl:
        page.tap("text=Exporter le JSON")
    d = dl.value
    path = os.path.join(OUT, d.suggested_filename)
    path = os.path.join(OUT, f"{engine}_" + d.suggested_filename)
    d.save_as(path)
    doc = json.load(open(path, encoding="utf-8"))
    need = ["schemaVersion", "campaignId", "audioCommit", "sessionId", "startedAt", "updatedAt", "status", "cases", "listeningContext", "validated"]
    fields_ok = all(k in doc for k in need) and set(doc["cases"]) == {"C1", "C2", "C3", "C5"} \
        and doc["audioCommit"] == "3edc528f79500941b8a748f747eedf4f704db521" and doc["status"] == "complete"
    check("Export S01_LISTENING_RESULTS.json valide", d.suggested_filename == "S01_LISTENING_RESULTS.json" and fields_ok,
          f"{d.suggested_filename}, {len(open(path, 'rb').read())} octets")
    page.tap("text=Copier mes résultats")
    page.wait_for_function("() => /copi|Copie/.test(document.querySelector('.toast.show')?.textContent || '')")
    toast = page.inner_text(".toast")
    clip = None
    try:
        clip = ev("() => navigator.clipboard.readText()")
    except Exception as e:
        clip = f"lecture presse-papiers impossible ({type(e).__name__})"
    copied_ok = isinstance(clip, str) and clip.startswith("{") and json.loads(clip)["sessionId"] == doc["sessionId"]
    check("Copie des résultats", "copiés" in toast, f"message « {toast} » ; relecture : {'contenu identique' if copied_ok else clip}")
    page.screenshot(path=os.path.join(OUT, f"{engine}_iphone13_resultats.png"), full_page=True)

    page.once("dialog", lambda dg: dg.accept())
    page.tap("text=Valider mes réponses")
    page.wait_for_selector("text=Validé le")
    page.goto(BASE + "#/cas/C1")
    page.wait_for_selector("[data-q=audibleDifference]")
    locked = ev("() => [...document.querySelectorAll('.questions input, .questions textarea')].every(i => i.disabled)")
    before = ev("() => window.__recette.store.doc.cases.C1.audibleDifference")
    ev("() => window.__recette.store.setAnswer('C1', 'audibleDifference', 'no')")
    check("Validation finale : réponses verrouillées", locked and ev("() => window.__recette.store.doc.cases.C1.audibleDifference") == before
          and ev("() => window.__recette.store.doc.validated"))

    page.goto(BASE + "#/resultats")
    page.wait_for_selector("text=Réinitialiser")
    page.once("dialog", lambda dg: dg.dismiss())
    page.tap("text=Réinitialiser")
    check("Réinitialisation annulée : réponses conservées", ev("() => window.__recette.store.doc.validated"))
    page.once("dialog", lambda dg: dg.accept())
    page.tap("text=Réinitialiser")
    page.wait_for_selector("button.primary:has-text('Commencer')")
    fresh = json.loads(ev(f"() => localStorage.getItem('{key}')"))
    check("Réinitialisation confirmée : nouvelle session vide", fresh["sessionId"] != sid and fresh["answeredCases"] == 0 and not fresh["validated"])

    errs = [e for e in errors if 'Web Audio' not in e]
    check("Aucune erreur JavaScript", not errs, "; ".join(errs[:5]))
    ctx.close()

    browser.close()


def responsive_suite(pw):
    global CUR
    CUR = "webkit"
    browser = pw.webkit.launch()
    # ---- responsive : plusieurs iPhone, aucun débordement horizontal
    for name in ["iPhone SE", "iPhone 13", "iPhone 15 Pro Max", "iPhone 13 landscape"]:
        c2 = browser.new_context(**pw.devices[name])
        pg = c2.new_page()
        overflow = []
        for h in ["#/accueil", "#/cas/C1", "#/resultats"]:
            pg.goto(BASE + h)
            pg.wait_for_selector("h1")
            pg.wait_for_timeout(300)
            sw = pg.evaluate("() => [document.documentElement.scrollWidth, window.innerWidth]")
            if sw[0] > sw[1]:
                overflow.append(f"{h} {sw[0]}>{sw[1]}")
            if h == "#/cas/C1":
                pg.screenshot(path=os.path.join(OUT, f"{name.replace(' ', '_')}_cas.png"))
        vp = pw.devices[name]["viewport"]
        check(f"Affichage {name} ({vp['width']}×{vp['height']}) sans débordement horizontal", not overflow, "; ".join(overflow))
        c2.close()

    browser.close()


with sync_playwright() as pw:
    browser_suite(pw, "webkit")
    browser_suite(pw, "chromium")
    responsive_suite(pw)

json.dump(results, open(os.path.join(OUT, "recette_results.json"), "w", encoding="utf-8"), ensure_ascii=False, indent=2)
n_ok = sum(r["status"] == "OK" for r in results)
n_na = sum(r["status"] == "N/A" for r in results)
n_ko = sum(r["status"] == "ÉCHEC" for r in results)
print(f"\n{n_ok} OK, {n_ko} ÉCHEC, {n_na} non exécutables, sur {len(results)} contrôles")
sys.exit(0 if n_ko == 0 else 1)
