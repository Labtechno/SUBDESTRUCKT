// SUBDESTRUCKT — manuel : navigation (sommaire, chapitre courant, précédent/suivant), recherche, agrandissement des
// figures, thème, glossaire (liens + infobulles accessibles). JavaScript natif, aucune dépendance, aucun réseau.
(() => {
  const $ = (s, r = document) => r.querySelector(s);
  const $$ = (s, r = document) => Array.from(r.querySelectorAll(s));
  const chapters = $$('section.chapter');

  // ---- thème (préférence locale, jamais requise)
  const root = document.documentElement;
  const themeBtn = $('#themeToggle');
  const safeGet = (k) => { try { return localStorage.getItem(k); } catch (e) { return null; } };
  const safeSet = (k, v) => { try { localStorage.setItem(k, v); } catch (e) { /* stockage indisponible */ } };
  const saved = safeGet('subdestruckt.manual.theme');
  if (saved) root.dataset.theme = saved;
  const syncTheme = () => {
    const dark = root.dataset.theme ? root.dataset.theme === 'dark' : !matchMedia('(prefers-color-scheme: light)').matches;
    themeBtn.setAttribute('aria-pressed', String(!dark));
    themeBtn.textContent = dark ? 'Mode clair' : 'Mode sombre';
  };
  themeBtn.addEventListener('click', () => {
    const dark = root.dataset.theme ? root.dataset.theme === 'dark' : !matchMedia('(prefers-color-scheme: light)').matches;
    root.dataset.theme = dark ? 'light' : 'dark';
    safeSet('subdestruckt.manual.theme', root.dataset.theme);
    syncTheme();
  });
  syncTheme();

  // ---- sommaire mobile
  const toc = $('nav.toc');
  const tocBtn = $('#tocToggle');
  tocBtn.addEventListener('click', () => {
    const open = !toc.classList.contains('open');
    toc.classList.toggle('open', open);
    tocBtn.setAttribute('aria-expanded', String(open));
    if (open) $('#searchInput').focus();
  });
  toc.addEventListener('click', (e) => {
    if (e.target.closest('a') && innerWidth <= 900) { toc.classList.remove('open'); tocBtn.setAttribute('aria-expanded', 'false'); }
  });

  // ---- chapitre courant
  const tocLinks = $$('nav.toc a[data-chapter]');
  const setCurrent = (id) => tocLinks.forEach((a) => a.setAttribute('aria-current', String(a.dataset.chapter === id)));
  if ('IntersectionObserver' in window) {
    const io = new IntersectionObserver((entries) => {
      entries.filter((e) => e.isIntersecting).forEach((e) => setCurrent(e.target.id));
    }, { rootMargin: '-40% 0px -55% 0px' });
    chapters.forEach((c) => io.observe(c));
  }

  // ---- recherche plein texte (index construit au chargement, aucune donnée envoyée)
  const index = [];
  chapters.forEach((ch) => {
    const chTitle = $('h1', ch).textContent;
    $$('h2, h3, p, li, td, dd', ch).forEach((el) => {
      if (!el.id) el.id = `s-${index.length}`;
      index.push({ el, chTitle, text: el.textContent.replace(/\s+/g, ' ').trim() });
    });
  });
  const norm = (s) => s.toLowerCase().normalize('NFD').replace(/[̀-ͯ]/g, '');
  const input = $('#searchInput');
  const results = $('#searchResults');
  const escapeHtml = (s) => s.replace(/[&<>"]/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]));
  input.addEventListener('input', () => {
    const q = norm(input.value.trim());
    results.innerHTML = '';
    if (q.length < 2) { $('#searchStatus').textContent = ''; return; }
    const hits = index.filter((i) => norm(i.text).includes(q)).slice(0, 30);
    $('#searchStatus').textContent = `${hits.length}${hits.length === 30 ? '+' : ''} résultat(s)`;
    hits.forEach((h) => {
      const t = h.text; const p = norm(t).indexOf(q);
      const a = Math.max(0, p - 40);
      const snip = (a > 0 ? '…' : '') + escapeHtml(t.slice(a, p)) + '<mark>' + escapeHtml(t.slice(p, p + q.length)) + '</mark>' +
        escapeHtml(t.slice(p + q.length, p + q.length + 60)) + '…';
      const li = document.createElement('li');
      li.innerHTML = `<a href="#${h.el.id}"><span class="mono" style="color:var(--dim)">${escapeHtml(h.chTitle)}</span><br>${snip}</a>`;
      results.appendChild(li);
    });
  });

  // ---- agrandissement des figures
  const lb = $('#lightbox');
  const lbImg = $('img', lb);
  const closeLb = () => { lb.removeAttribute('open'); lb.setAttribute('aria-hidden', 'true'); if (lb._from) lb._from.focus(); };
  $$('img.zoomable').forEach((img) => {
    img.tabIndex = 0;
    img.setAttribute('role', 'button');
    img.setAttribute('aria-label', `Agrandir : ${img.alt}`);
    const open = () => { lbImg.src = img.src; lbImg.alt = img.alt; lb.setAttribute('open', ''); lb.setAttribute('aria-hidden', 'false'); lb._from = img; $('.close', lb).focus(); };
    img.addEventListener('click', open);
    img.addEventListener('keydown', (e) => { if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); open(); } });
  });
  $('.close', lb).addEventListener('click', closeLb);
  lb.addEventListener('click', (e) => { if (e.target === lb) closeLb(); });
  document.addEventListener('keydown', (e) => { if (e.key === 'Escape' && lb.hasAttribute('open')) closeLb(); });

  // ---- glossaire : première occurrence de chaque terme par chapitre -> lien + infobulle
  const terms = window.SBD_GLOSSARY || [];
  const forbidden = 'a, h1, h2, h3, h4, code, figcaption, button, label, summary, .kv dt, th, .tip, svg, .glossary, select, option, textarea';
  chapters.forEach((ch) => {
    if (ch.id === 'glossaire') return;
    terms.forEach((t) => {
      const forms = [t.term, ...(t.aliases || [])];
      const walker = document.createTreeWalker(ch, NodeFilter.SHOW_TEXT, {
        acceptNode: (n) => (n.parentElement.closest(forbidden) ? NodeFilter.FILTER_REJECT : NodeFilter.FILTER_ACCEPT),
      });
      let node;
      while ((node = walker.nextNode())) {
        let found = null;
        for (const f of forms) {
          const re = new RegExp(`(^|[^\\p{L}\\d])(${f.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')})(?=$|[^\\p{L}\\d])`, 'u');
          const m = re.exec(node.data);
          if (m) { found = { start: m.index + m[1].length, len: m[2].length }; break; }
        }
        if (found) {
          const r = node.splitText(found.start);
          r.splitText(found.len);
          const a = document.createElement('a');
          a.className = 'gl'; a.href = `#gl-${t.id}`; a.dataset.term = t.id; a.textContent = r.data;
          a.setAttribute('aria-describedby', `tipdef-${t.id}`);
          r.replaceWith(a);
          break;
        }
      }
    });
  });
  // définitions cachées pour les lecteurs d'écran + infobulle visuelle
  const defs = document.createElement('div');
  defs.hidden = true;
  terms.forEach((t) => { const d = document.createElement('span'); d.id = `tipdef-${t.id}`; d.textContent = t.short; defs.appendChild(d); });
  document.body.appendChild(defs);
  const tip = document.createElement('div');
  tip.className = 'tip'; tip.setAttribute('role', 'tooltip'); tip.hidden = true;
  document.body.appendChild(tip);
  const show = (a) => {
    const t = terms.find((x) => x.id === a.dataset.term); if (!t) return;
    tip.textContent = `${t.term} — ${t.short}`; tip.hidden = false;
    const r = a.getBoundingClientRect();
    tip.style.left = `${Math.max(8, Math.min(scrollX + r.left, scrollX + innerWidth - 336))}px`;
    tip.style.top = `${scrollY + r.bottom + 6}px`;
  };
  document.addEventListener('mouseover', (e) => { const a = e.target.closest('a.gl'); if (a) show(a); });
  document.addEventListener('mouseout', (e) => { if (e.target.closest('a.gl')) tip.hidden = true; });
  document.addEventListener('focusin', (e) => { const a = e.target.closest('a.gl'); if (a) show(a); else tip.hidden = true; });
  document.addEventListener('keydown', (e) => { if (e.key === 'Escape') tip.hidden = true; });
})();
