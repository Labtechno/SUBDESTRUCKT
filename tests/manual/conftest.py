"""Fixtures Playwright du manuel SUBDESTRUCKT.

MANUAL_BASE : URL de base des manuels (défaut : serveur local sur manual/dist), ex. https://labtechno.github.io/SUBDESTRUCKT/manual/
S01_URL     : plateforme d'écoute S0.1 à vérifier en non-régression (facultatif, tests ignorés si absent).
"""
import functools
import http.server
import os
import threading
from pathlib import Path

import pytest
from playwright.sync_api import sync_playwright

DIST = Path(__file__).resolve().parents[2] / "manual" / "dist"
VERSION = os.environ.get("MANUAL_VERSION", "0.1.0-b5")  # édition testée (voir manual/versions.json)


class _Quiet(http.server.SimpleHTTPRequestHandler):
    """Serveur statique avec requêtes Range (comme GitHub Pages) : indispensable au positionnement dans un WAV."""

    def log_message(self, *a):
        pass

    def send_head(self):
        rng = self.headers.get("Range")
        path = self.translate_path(self.path)
        if not rng or not os.path.isfile(path):
            return super().send_head()
        size = os.path.getsize(path)
        a, _, b = rng.replace("bytes=", "").partition("-")
        start = int(a) if a else size - int(b)
        end = int(b) if (a and b) else size - 1
        end = min(end, size - 1)
        f = open(path, "rb")
        f.seek(start)
        self.send_response(206)
        self.send_header("Content-Type", self.guess_type(path))
        self.send_header("Accept-Ranges", "bytes")
        self.send_header("Content-Range", f"bytes {start}-{end}/{size}")
        self.send_header("Content-Length", str(end - start + 1))
        self.end_headers()
        self._remaining = end - start + 1
        return f

    def copyfile(self, source, outputfile):
        n = getattr(self, "_remaining", None)
        if n is None:
            return super().copyfile(source, outputfile)
        outputfile.write(source.read(n))


@pytest.fixture(scope="session")
def base_url():
    env = os.environ.get("MANUAL_BASE")
    if env:
        yield env.rstrip("/") + "/"
        return
    srv = http.server.ThreadingHTTPServer(("127.0.0.1", 0), functools.partial(_Quiet, directory=str(DIST)))
    threading.Thread(target=srv.serve_forever, daemon=True).start()
    yield f"http://127.0.0.1:{srv.server_address[1]}/"
    srv.shutdown()


@pytest.fixture(scope="session")
def manual_url(base_url):
    return base_url + VERSION + "/"


@pytest.fixture(scope="session")
def pw():
    with sync_playwright() as p:
        yield p


BROWSERS = {
    "chromium-desktop": ("chromium", dict(viewport={"width": 1280, "height": 900})),
    "webkit-iphone": ("webkit", "iPhone 13"),
    "chromium-iphone-emul": ("chromium", "iPhone 13"),
}


@pytest.fixture(scope="session", params=list(BROWSERS))
def browser_ctx(request, pw):
    engine, spec = BROWSERS[request.param]
    args = dict(pw.devices[spec]) if isinstance(spec, str) else dict(spec)
    if engine == "chromium" and isinstance(spec, str):
        args.pop("default_browser_type", None)
    b = getattr(pw, engine).launch()
    yield request.param, b, args
    b.close()


@pytest.fixture()
def page(browser_ctx):
    name, b, args = browser_ctx
    ctx = b.new_context(**args, accept_downloads=True)
    pg = ctx.new_page()
    pg.errors = []
    pg.on("pageerror", lambda e: pg.errors.append(str(e)))
    pg.on("console", lambda m: pg.errors.append(m.text) if m.type == "error" else None)
    pg.browser_name = name
    yield pg
    ctx.close()


@pytest.fixture()
def chromium_page(pw):
    b = pw.chromium.launch()
    ctx = b.new_context(viewport={"width": 1280, "height": 900}, accept_downloads=True)
    pg = ctx.new_page()
    yield pg
    b.close()
