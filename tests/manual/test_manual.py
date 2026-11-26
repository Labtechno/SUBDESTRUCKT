"""Tests automatisés du manuel SUBDESTRUCKT 0.1.0-b5 et de l'assistant de recette ARCH-G3.

Lancement : python -m pytest tests/manual -q            (serveur local sur manual/dist)
            MANUAL_BASE=https://labtechno.github.io/SUBDESTRUCKT/manual/ python -m pytest tests/manual -q
Les tests « *_all » tournent sur Chromium bureau, WebKit (profil iPhone 13) et Chromium (émulation iPhone 13).
WebKit sous Windows/Linux n'est PAS Safari iOS : ces résultats ne remplacent pas un essai sur iPhone physique.
"""
import hashlib
import json
import re
import urllib.request
from pathlib import Path

import jsonschema
import pytest

ROOT = Path(__file__).resolve().parents[2]
S01_KEY = "labtechno.recette.SUBDESTRUCKT-S01-FINAL.v1"
KEY = "labtechno.subdestruckt.manual.arch-g3-recette.v1"
SCHEMA = json.loads((ROOT / "manual/schema/results.schema.json").read_text(encoding="utf-8"))


def goto(page, url):
    page.goto(url)
    page.wait_for_load_state("load")


# ---------------------------------------------------------------- manuel : structure et chargement
def test_manual_loads_without_errors_all(page, manual_url):
    goto(page, manual_url + "index.html")
    assert page.locator("section.chapter").count() == 16
    assert page.locator("nav.toc a[data-chapter]").count() == 16
    assert page.errors == []


def test_no_external_requests_all(page, manual_url):
    hosts = set()
    page.on("request", lambda r: hosts.add(re.match(r"^[a-z]+://[^/]+", r.url).group(0)) if r.url.startswith("http") else None)
    goto(page, manual_url + "index.html")
    goto(page, manual_url + "recette.html")
    base = re.match(r"^[a-z]+://[^/]+", manual_url).group(0)
    assert hosts == {base}, hosts


def test_one_h1_per_chapter_and_alt_texts(chromium_page, manual_url):
    pg = chromium_page
    goto(pg, manual_url + "index.html")
    for sec in pg.locator("section.chapter").all():
        assert sec.locator("h1").count() == 1
    assert pg.locator("img:not([alt])").count() == 0
    assert pg.locator("svg.diagram:not([role='img'])").count() == 0
    for svg in pg.locator("svg.diagram").all():
        assert svg.locator("title").count() == 1 and svg.locator("desc").count() == 1


def test_internal_anchors_resolve(chromium_page, manual_url):
    pg = chromium_page
    goto(pg, manual_url + "index.html")
    missing = pg.evaluate("""() => Array.from(document.querySelectorAll('a[href^="#"]')).map(a => a.getAttribute('href').slice(1))
        .filter(id => id && !document.getElementById(id))""")
    assert missing == []


def test_toc_navigation_and_current_chapter(chromium_page, manual_url):
    pg = chromium_page
    goto(pg, manual_url + "index.html")
    pg.click("nav.toc a[data-chapter='ch-08']")
    pg.wait_for_timeout(600)
    assert pg.get_attribute("nav.toc a[data-chapter='ch-08']", "aria-current") == "true"
    assert pg.locator("#ch-08 .chapter-nav a[rel=prev]").get_attribute("href") == "#ch-07"
    assert pg.locator("#ch-08 .chapter-nav a[rel=next]").get_attribute("href") == "#ch-09"


def test_mobile_toc_toggle(page, manual_url):
    if "iphone" not in page.browser_name:
        pytest.skip("sommaire superposé : affichage mobile uniquement")
    goto(page, manual_url + "index.html")
    assert not page.locator("nav.toc").is_visible()
    page.click("#tocToggle")
    assert page.locator("nav.toc").is_visible()
    page.click("nav.toc a[data-chapter='ch-13']")
    assert not page.locator("nav.toc").is_visible()


def test_search(chromium_page, manual_url):
    pg = chromium_page
    goto(pg, manual_url + "index.html")
    pg.fill("#searchInput", "cross master")
    n = pg.locator("#searchResults li").count()
    assert n > 3
    assert "résultat" in pg.text_content("#searchStatus")
    pg.fill("#searchInput", "zzzqqq")
    assert pg.locator("#searchResults li").count() == 0


def test_collapsible_sections(chromium_page, manual_url):
    pg = chromium_page
    goto(pg, manual_url + "index.html")
    d = pg.locator("details.adv").first
    assert d.get_attribute("open") is None
    d.locator("summary").click()
    assert d.get_attribute("open") is not None


def test_lightbox_zoom(page, manual_url):
    goto(page, manual_url + "index.html#ch-04")
    img = page.locator("#ui-annotated img.zoomable")
    img.scroll_into_view_if_needed()
    img.click(position={"x": 8, "y": 30})  # hors repères
    assert page.locator("#lightbox").get_attribute("open") is not None
    page.keyboard.press("Escape")
    assert page.locator("#lightbox").get_attribute("open") is None


def test_annotated_screenshot_is_real_and_has_15_pins(chromium_page, manual_url):
    pg = chromium_page
    goto(pg, manual_url + "index.html#ch-04")
    assert pg.locator("#ui-annotated .pin").count() == 15
    assert pg.locator("ol.legend li").count() == 15
    data = urllib.request.urlopen(manual_url + "assets/ui_diagnostic_macos.png").read()
    assert hashlib.sha256(data).hexdigest() == "69e1c9b26003d1399c95f699ad805f11452a90754c0a55c74ffb2f2face89ede"
    # chaque repère reste à l'intérieur de l'image
    inside = pg.evaluate("""() => { const r = document.querySelector('#ui-annotated .pins').getBoundingClientRect();
      return Array.from(document.querySelectorAll('#ui-annotated .pin')).every(p => { const b = p.getBoundingClientRect();
        const cx = b.left + b.width / 2, cy = b.top + b.height / 2; return cx >= r.left && cx <= r.right && cy >= r.top && cy <= r.bottom; }); }""")
    assert inside


def test_all_22_parameters_documented(chromium_page, manual_url):
    pg = chromium_page
    goto(pg, manual_url + "index.html")
    reg = re.findall(r'\{"([a-z0-9_.]+)", "([^"]+)", "([^"]+)", (-?[\d.]+)f, (-?[\d.]+)f, (-?[\d.]+)f',
                     (ROOT / "source/core/Parameters.cpp").read_text(encoding="utf-8"))
    assert len(reg) == 22
    for sid, *_ in reg:
        assert pg.locator(f"[id='p-{sid}']").count() == 1, sid
    assert pg.locator("table.params tbody tr:not(.grp)").count() == 22


def test_glossary_links_and_tooltips(page, manual_url):
    goto(page, manual_url + "index.html")
    links = page.locator("a.gl")
    assert links.count() > 40
    first = links.first
    target = first.get_attribute("href")[1:]
    assert page.locator(f"[id='{target}']").count() == 1
    desc = first.get_attribute("aria-describedby")
    assert page.locator(f"[id='{desc}']").text_content().strip() != ""
    assert page.locator("#glossaire a.gl").count() == 0
    terms = page.locator(".glossary dt").all_text_contents()
    keys = [t.lower().replace("é", "e") for t in terms]
    assert keys == sorted(keys), "glossaire non alphabétique"


def test_theme_toggle_persists(chromium_page, manual_url):
    pg = chromium_page
    goto(pg, manual_url + "index.html")
    before = pg.evaluate("getComputedStyle(document.body).backgroundColor")
    pg.click("#themeToggle")
    after = pg.evaluate("getComputedStyle(document.body).backgroundColor")
    assert before != after
    pg.reload()
    assert pg.evaluate("getComputedStyle(document.body).backgroundColor") == after


def test_no_horizontal_overflow_all(page, manual_url):
    for p in ("index.html", "recette.html"):
        goto(page, manual_url + p)
        sw, cw = page.evaluate("[document.documentElement.scrollWidth, document.documentElement.clientWidth]")
        assert sw <= cw + 1, (p, sw, cw)


def test_touch_targets_44px_all(page, manual_url):
    if "iphone" not in page.browser_name:
        pytest.skip("cible tactile : profils iPhone")
    for p in ("index.html", "recette.html"):
        goto(page, manual_url + p)
        small = page.evaluate("""() => Array.from(document.querySelectorAll('button, .btn, input[type=text], input[type=search], textarea, details.trouble > summary, .st span, nav.toc li a'))
          .filter(e => e.offsetParent !== null).map(e => [e.id || e.className || e.tagName, e.getBoundingClientRect().height])
          .filter(([, h]) => h < 43.5)""")
        assert small == [], (p, small[:5])


def test_print_stylesheet(chromium_page, manual_url):
    pg = chromium_page
    goto(pg, manual_url + "index.html")
    pg.emulate_media(media="print")
    assert not pg.locator("nav.toc").is_visible()
    assert not pg.locator(".topbar").is_visible()
    assert pg.locator("#ch-16").is_visible()


# ---------------------------------------------------------------- audio
def test_audio_files_match_manifest(manual_url):
    man = json.loads(urllib.request.urlopen(manual_url + "version.json").read())
    assert man["buildSourceSha"] == "186fb8644ae9518e3cb5116f4368b45dcea0d430"
    assert man["documentationSha"] == "8d2334250994758f2182b18c3a47dd059cb68495"
    assert len(man["audioExamples"]) == 15
    for e in man["audioExamples"]:
        data = urllib.request.urlopen(manual_url + e["file"]).read()
        assert data[:4] == b"RIFF" and data[8:12] == b"WAVE"
        assert hashlib.sha256(data).hexdigest() == e["sha256"]


def test_no_autoplay_and_no_audio_download_before_click_all(page, manual_url):
    audio_reqs = []
    page.on("request", lambda r: audio_reqs.append(r.url) if r.url.endswith(".wav") else None)
    goto(page, manual_url + "index.html")
    page.wait_for_timeout(800)
    assert audio_reqs == []
    assert page.evaluate("window.SBD_AUDIO.playing()") is None


def test_exclusive_playback_and_ab_switch(chromium_page, manual_url):
    pg = chromium_page
    goto(pg, manual_url + "index.html#ch-07")
    ab = pg.locator("#ch-07 .ab").filter(has=pg.locator("button.play >> nth=1")).first
    a, b = ab.locator("button.play").nth(0), ab.locator("button.play").nth(1)
    a.click()
    pg.wait_for_timeout(1200)
    assert a.get_attribute("aria-pressed") == "true"
    b.click()
    pg.wait_for_timeout(200)
    assert a.get_attribute("aria-pressed") == "false" and b.get_attribute("aria-pressed") == "true"
    t = pg.evaluate("""() => { const btns = Array.from(document.querySelectorAll('button.play')).filter(x => x._audio && !x._audio.paused);
       return btns.map(x => x._audio.currentTime); }""")
    assert len(t) == 1 and t[0] > 0.8, t  # un seul son, repris au même instant
    other = pg.locator("#ch-12 button.play").first
    other.click()
    pg.wait_for_timeout(200)
    assert pg.evaluate("Array.from(document.querySelectorAll('button.play')).filter(x => x._audio && !x._audio.paused).length") == 1
    other.click()
    assert pg.evaluate("window.SBD_AUDIO.playing()") is None


def test_demos_render_and_react(chromium_page, manual_url):
    pg = chromium_page
    goto(pg, manual_url + "index.html#ch-05")
    for kind in ("sine", "phase", "harmonics", "adsr", "pm"):
        demo = pg.locator(f".demo[data-demo={kind}]")
        demo.scroll_into_view_if_needed()
        pg.wait_for_timeout(100)
        nonblank = demo.locator("canvas").first.evaluate("""c => { const d = c.getContext('2d').getImageData(0, 0, c.width, c.height).data;
            let n = 0; for (let i = 3; i < d.length; i += 4) if (d[i]) n++; return n; }""")
        assert nonblank > 100, kind
    rng = pg.locator(".demo[data-demo=harmonics] input[name=n]")
    rng.fill("8")
    assert pg.locator(".demo[data-demo=harmonics] [data-out=n]").text_content() == "8"


# ---------------------------------------------------------------- assistant de recette
def fresh_recette(page, manual_url):
    goto(page, manual_url + "recette.html")
    page.evaluate(f"localStorage.removeItem('{KEY}')")
    page.reload()
    page.wait_for_load_state("load")


def test_recette_initial_state_all(page, manual_url):
    fresh_recette(page, manual_url)
    assert page.locator(".tcard").count() == 15
    ids = page.locator(".tcard").evaluate_all("els => els.map(e => e.id)")
    assert ids == [f"R-{i:02d}" for i in range(1, 16)]
    assert page.locator("input[type=radio][value=PENDING]:checked").count() == 15
    assert page.locator("textarea[id^=obs-]").evaluate_all("els => els.every(e => e.value === '')")
    assert page.locator("#cfg-machine").input_value() == ""
    assert page.locator("#validateBtn").is_disabled()
    assert page.locator("h2").all_text_contents()[1:5] == ["Installation", "Validation musicale", "Validation technique", "Validation des paquets"]
    assert page.errors == []


def test_recette_storage_key_distinct_from_s01(chromium_page, manual_url):
    pg = chromium_page
    fresh_recette(pg, manual_url)
    pg.evaluate(f"localStorage.setItem('{S01_KEY}', 'S01-SENTINEL')")
    pg.check("#st-R-01-PASS", force=True)
    keys = pg.evaluate("Object.keys(localStorage)")
    assert KEY in keys and KEY != S01_KEY
    assert pg.evaluate(f"localStorage.getItem('{S01_KEY}')") == "S01-SENTINEL"
    pg.once("dialog", lambda d: d.accept())
    pg.click("#resetBtn")
    pg.wait_for_load_state("load")
    assert pg.evaluate(f"localStorage.getItem('{S01_KEY}')") == "S01-SENTINEL"


def set_status(pg, tid, st):
    pg.locator(f"label[for='st-{tid}-{st.replace('/', '')}']").click()


def test_recette_fail_never_hidden_and_progress_distinct(chromium_page, manual_url):
    pg = chromium_page
    fresh_recette(pg, manual_url)
    set_status(pg, "R-01", "PASS")
    set_status(pg, "R-02", "FAIL")
    assert "2 / 15" in pg.text_content("#progressText")
    assert "pas une validation" in pg.text_content("#progressText")
    assert "FAIL" in pg.text_content("#globalStatus")
    assert pg.locator("#failList").is_visible() and "R-02" in pg.text_content("#failList")
    for i in range(3, 16):
        set_status(pg, f"R-{i:02d}", "PASS")
    assert "avec FAIL" in pg.text_content("#globalStatus")
    assert pg.locator("#failList").is_visible()


def test_recette_na_requires_justification(chromium_page, manual_url):
    pg = chromium_page
    fresh_recette(pg, manual_url)
    for i in range(1, 15):
        set_status(pg, f"R-{i:02d}", "PASS")
    set_status(pg, "R-15", "N/A")
    assert pg.locator("#na-R-15").is_visible()
    assert pg.locator("#validateBtn").is_disabled()
    assert "justification manquante" in pg.text_content("#b-R-15")
    pg.fill("#na-R-15", "Désinstallateur non relancé : machine partagée.")
    assert pg.locator("#validateBtn").is_enabled()


def test_recette_persistence_correction_validation_lock(chromium_page, manual_url):
    pg = chromium_page
    fresh_recette(pg, manual_url)
    pg.fill("#cfg-machine", "Mac Studio M4 Max")
    pg.check("#sr-44100")
    set_status(pg, "R-03", "FAIL")
    pg.fill("#obs-R-03", "Plugin absent de la liste.")
    pg.fill("#ev-R-03", "capture_R03.png")
    pg.reload()
    pg.wait_for_load_state("load")
    assert pg.locator("#cfg-machine").input_value() == "Mac Studio M4 Max"
    assert pg.locator("#sr-44100").is_checked()
    assert pg.locator("#st-R-03-FAIL").is_checked()
    assert pg.locator("#obs-R-03").input_value() == "Plugin absent de la liste."
    # correction d'une réponse
    set_status(pg, "R-03", "PASS")
    for i in range(1, 16):
        if i != 3:
            set_status(pg, f"R-{i:02d}", "PASS")
    pg.once("dialog", lambda d: d.dismiss())
    pg.click("#validateBtn")
    assert not pg.evaluate("window.__sbdRecette.doc.finalValidation.validated")
    pg.once("dialog", lambda d: d.accept())
    pg.click("#validateBtn")
    doc = pg.evaluate("window.__sbdRecette.doc")
    assert doc["finalValidation"]["validated"] and doc["finalValidation"]["statusAtValidation"] == "terminée — aucun FAIL"
    assert pg.locator("#obs-R-01").is_disabled() and pg.locator("#cfg-machine").is_disabled()
    pg.once("dialog", lambda d: d.accept())
    pg.click("#reopenBtn")
    assert pg.locator("#obs-R-01").is_enabled()


def test_recette_reset_requires_confirmation(chromium_page, manual_url):
    pg = chromium_page
    fresh_recette(pg, manual_url)
    set_status(pg, "R-01", "PASS")
    pg.once("dialog", lambda d: d.dismiss())
    pg.click("#resetBtn")
    assert pg.locator("#st-R-01-PASS").is_checked()
    pg.once("dialog", lambda d: d.accept())
    pg.click("#resetBtn")
    pg.wait_for_load_state("load")
    assert pg.locator("input[type=radio][value=PENDING]:checked").count() == 15


def test_recette_export_json_matches_schema(chromium_page, manual_url):
    pg = chromium_page
    fresh_recette(pg, manual_url)
    set_status(pg, "R-01", "PASS")
    set_status(pg, "R-09", "N/A")
    pg.fill("#na-R-09", "PDC non contrôlable dans cette DAW.")
    with pg.expect_download() as dl:
        pg.click("#exportBtn")
    d = dl.value
    assert d.suggested_filename == "SUBDESTRUCKT_ARCH_G3_RESULTS.json"
    doc = json.loads(Path(d.path()).read_text(encoding="utf-8"))
    jsonschema.validate(doc, SCHEMA)
    assert doc["buildSha"] == "186fb8644ae9518e3cb5116f4368b45dcea0d430"
    assert doc["counts"] == {"total": 15, "filled": 2, "pass": 1, "fail": 0, "pending": 13, "na": 1, "naWithoutJustification": 0}
    assert doc["results"]["R-09"]["naJustification"].startswith("PDC")
    assert json.loads(pg.input_value("#json")) == doc


def test_recette_copy_to_clipboard(pw, manual_url):
    b = pw.chromium.launch()
    ctx = b.new_context(permissions=["clipboard-read", "clipboard-write"])
    pg = ctx.new_page()
    fresh_recette(pg, manual_url)
    set_status(pg, "R-01", "FAIL")
    pg.click("#copyBtn")
    pg.wait_for_timeout(200)
    txt = pg.evaluate("navigator.clipboard.readText()")
    assert json.loads(txt)["results"]["R-01"]["status"] == "FAIL"
    assert "copiés" in pg.text_content("#toast") or "sélectionné" in pg.text_content("#toast")
    b.close()


def test_recette_storage_unavailable_fallback(pw, manual_url):
    b = pw.chromium.launch()
    ctx = b.new_context()
    ctx.add_init_script("Object.defineProperty(window, 'localStorage', { get() { throw new Error('bloqué'); } });")
    pg = ctx.new_page()
    goto(pg, manual_url + "recette.html")
    assert "indisponible" in pg.text_content("#saveState")
    set_status(pg, "R-01", "PASS")
    assert json.loads(pg.input_value("#json"))["results"]["R-01"]["status"] == "PASS"
    b.close()


# ---------------------------------------------------------------- versions et non-régression S0.1
def test_versions_index(chromium_page, base_url):
    pg = chromium_page
    goto(pg, base_url + "index.html" if base_url.startswith("http://127.") else base_url)
    assert pg.locator("a[href='0.1.0-b5/']").count() == 1


S01_EXPECTED = Path(__file__).with_name("s01_expected.json")


@pytest.mark.skipif(not S01_EXPECTED.exists(), reason="empreintes S0.1 non capturées")
def test_s01_platform_unchanged():
    exp = json.loads(S01_EXPECTED.read_text(encoding="utf-8"))
    for path, h in exp["files"].items():
        data = urllib.request.urlopen(exp["base"] + path).read()
        assert hashlib.sha256(data).hexdigest() == h, path


@pytest.mark.skipif(not S01_EXPECTED.exists(), reason="empreintes S0.1 non capturées")
def test_s01_platform_still_works(pw):
    exp = json.loads(S01_EXPECTED.read_text(encoding="utf-8"))
    b = pw.chromium.launch()
    pg = b.new_page()
    errs = []
    pg.on("pageerror", lambda e: errs.append(str(e)))
    pg.goto(exp["base"])
    pg.wait_for_load_state("load")
    assert errs == []
    assert pg.title() == exp["title"]
    b.close()
