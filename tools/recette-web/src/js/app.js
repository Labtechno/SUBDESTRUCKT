// LABTECHNO — application de recette perceptive : assemble manifeste, moteur d'écoute, questionnaire et sauvegarde.
// Une nouvelle campagne = un dossier campaigns/<id>/ (manifest.json + audio/) et une entrée dans campaigns/index.json.
// Sélection : ?c=<id> (sinon la campagne par défaut de l'index).

import { el, clear } from './dom.js';
import { ListeningEngine } from './listening-engine.js';
import { renderQuestions, answerLabel } from './questionnaire.js';
import { SessionStore } from './session-store.js';

const app = document.getElementById('app');
const $progress = document.getElementById('progress-count');
const $save = document.getElementById('save-status');

let M, base, engine, store;
let audioReady = false;
let audioDone = 0;
let audioTotal = 0;
let raf = 0;
let currentView = null; // {name, caseIndex, update()}

// ------------------------------------------------------------------------------------------------ démarrage
async function boot() {
  try {
    const idx = await (await fetch('campaigns/index.json', { cache: 'no-cache' })).json();
    const want = new URLSearchParams(location.search).get('c') || idx.default;
    const entry = idx.campaigns.find((c) => c.id === want) || idx.campaigns.find((c) => c.id === idx.default);
    M = await (await fetch(entry.manifest, { cache: 'no-cache' })).json();
    base = entry.manifest.replace(/manifest\.json$/, '');
  } catch (e) {
    clear(app).append(el('div', { class: 'panel error' }, 'Impossible de charger la campagne : ', String(e)));
    return;
  }
  document.title = `${M.title} — LABTECHNO`;
  store = new SessionStore(M);
  engine = new ListeningEngine({ switchFadeMs: M.player?.switchFadeMs ?? 5, loop: M.player?.loopDefault ?? false });
  window.__recette = { manifest: M, engine, store }; // inspection et tests automatisés (aucune donnée secrète)
  store.addEventListener('change', updateChrome);
  engine.addEventListener('state', () => currentView && currentView.update && currentView.update());
  window.addEventListener('hashchange', route);
  document.addEventListener('keydown', onKey);
  updateChrome();
  route();
  loadAudio();
}

function loadAudio() {
  const items = [];
  for (const c of M.cases) for (const r of M.renders) {
    const a = c.audio[r.id];
    items.push({ group: c.id, id: r.id, url: base + a.file, sha256: a.sha256, bytes: a.bytes, file: a.file });
  }
  audioTotal = items.length;
  engine.load(items, (done) => { audioDone = done; currentView && currentView.update && currentView.update(); })
    .then((results) => {
      audioReady = true;
      const files = {};
      items.forEach((it, i) => { files[it.file.split('/').pop()] = results[i].ok; });
      const ok = results.filter((r) => r.ok).length;
      const playable = results.filter((r) => r.playable).length;
      store.setIntegrity({ checkedAt: new Date().toISOString(), algorithm: 'SHA-256', verified: ok, playable, total: items.length, allOk: ok === items.length, files });
      store.setClient({ audioSampleRate: engine.sampleRate });
      if (currentView && currentView.update) currentView.update();
    });
}

// ------------------------------------------------------------------------------------------------ routage
function route() {
  cancelAnimationFrame(raf);
  const h = location.hash.replace(/^#\/?/, '');
  const [name, arg] = h.split('/');
  if (name === 'cas') {
    const i = M.cases.findIndex((c) => c.id === arg);
    if (i >= 0) { engine.pause(); currentView = { name: 'case', caseIndex: i }; return render(); }
  }
  if (name === 'resultats') { engine.pause(); currentView = { name: 'results' }; return render(); }
  engine.pause();
  currentView = { name: 'intro' };
  render();
}

function go(hash) { if (location.hash === hash) route(); else location.hash = hash; }

function render() {
  clear(app);
  const v = currentView;
  if (v.name === 'case') renderCase(v);
  else if (v.name === 'results') renderResults(v);
  else renderIntro(v);
  window.scrollTo(0, 0);
}

function firstIncomplete() {
  const c = M.cases.find((x) => !store.caseComplete(x.id));
  return c ? c.id : null;
}

// ------------------------------------------------------------------------------------------------ chrome
function updateChrome() {
  const d = store.doc;
  $progress.textContent = `${d.answeredCases}/${d.totalCases}`;
  $progress.title = `${d.answeredCases} cas complétés sur ${d.totalCases}`;
  if (!store.available) { $save.textContent = 'Stockage indisponible'; $save.className = 'save warn'; }
  else if (store.lastSaveOk === false) { $save.textContent = 'Échec de sauvegarde'; $save.className = 'save warn'; }
  else if (store.lastSavedAt) {
    $save.textContent = `Sauvegardé ${store.lastSavedAt.toLocaleTimeString('fr-FR')}`;
    $save.className = 'save ok';
  }
  if (currentView && currentView.onStore) currentView.onStore();
}

let toastTimer = 0;
function toast(msg, kind = 'ok') {
  let t = document.getElementById('toast');
  if (!t) { t = el('div', { id: 'toast', role: 'status' }); document.body.append(t); }
  t.textContent = msg;
  t.className = `toast show ${kind}`;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => { t.className = 'toast'; }, 2600);
}

function storageBanner() {
  if (store.available) return null;
  return el('div', { class: 'panel warn' },
    el('strong', {}, 'Sauvegarde locale indisponible. '),
    'Safari bloque le stockage (navigation privée ?). Tes réponses ne survivront pas à la fermeture de la page : ',
    'exporte ou copie tes résultats avant de quitter.');
}

// ------------------------------------------------------------------------------------------------ accueil
function audioStatusPanel() {
  const panel = el('div', { class: 'panel audio-status' });
  const update = () => {
    clear(panel);
    const results = [...engine.integrity.values()];
    const ok = results.filter((r) => r.ok).length;
    const bad = results.filter((r) => !r.ok);
    const unplayable = results.filter((r) => r.ok && !r.playable);
    const head = !audioReady
      ? el('div', { class: 'row' }, el('span', { class: 'label' }, 'Audio'), el('span', {}, `Chargement et vérification ${audioDone}/${audioTotal}…`))
      : el('div', { class: 'row' }, el('span', { class: 'label' }, 'Audio'),
        el('span', { class: bad.length ? 'bad' : 'good' }, bad.length ? `${bad.length} fichier(s) non conformes` : `${ok}/${audioTotal} fichiers vérifiés (SHA-256)`));
    panel.append(head, el('div', { class: 'meter' }, el('div', { class: 'meter-fill', style: `width:${audioTotal ? (100 * audioDone) / audioTotal : 0}%` })));
    if (audioReady && unplayable.length) {
      panel.append(el('p', { class: 'bad small' }, `Lecture impossible : ${unplayable[0].error}. Utilise Safari à jour sur iPhone.`));
    }
    if (audioReady) {
      const det = el('details', {}, el('summary', {}, 'Détail des empreintes'));
      const ul = el('ul', { class: 'hashes' });
      for (const c of M.cases) for (const r of M.renders) {
        const res = engine.integrity.get(`${c.id}/${r.id}`);
        const name = c.audio[r.id].file.split('/').pop();
        ul.append(el('li', {}, el('span', { class: res && res.ok ? 'good' : 'bad' }, res && res.ok ? '✓ ' : '✗ '), el('code', {}, name),
          el('br'), el('code', { class: 'dim' }, c.audio[r.id].sha256), res && !res.ok ? el('div', { class: 'bad' }, res.error || '') : null));
      }
      det.append(ul, el('p', { class: 'dim small' }, `Commit audio : ${M.audioSource.commit}`));
      panel.append(det);
      if (engine.sampleRate && engine.sampleRate !== 48000) {
        panel.append(el('p', { class: 'warn small' }, `Contexte audio à ${engine.sampleRate} Hz : le navigateur rééchantillonne la lecture.`));
      }
    }
  };
  update();
  return { panel, update };
}

function renderIntro(v) {
  const resume = Object.values(store.doc.cases).some((a) => a.updatedAt);
  const status = audioStatusPanel();
  v.update = status.update;
  const ctxQ = renderQuestions(M.contextQuestions || [], store.doc.listeningContext, {
    disabled: store.doc.validated, onChange: (id, val) => store.setContext(id, val),
  });
  const next = firstIncomplete();
  app.append(
    el('section', { class: 'intro' },
      el('p', { class: 'kicker' }, 'Recette perceptive'),
      el('h1', {}, M.title),
      el('p', { class: 'sub' }, M.subtitle),
      storageBanner(),
      ...M.intro.map((t) => el('p', {}, t)),
      el('h2', {}, 'Avant de commencer'),
      el('ul', { class: 'checklist' }, ...M.checklist.map((t) => el('li', {}, t))),
      el('h2', {}, 'Contexte d\'écoute'),
      ctxQ,
      status.panel,
      el('div', { class: 'actions' },
        el('button', { class: 'btn primary', type: 'button', onclick: () => go(`#/cas/${next || M.cases[0].id}`) },
          store.doc.validated ? 'Revoir les cas' : resume ? 'Reprendre' : 'Commencer'),
        el('button', { class: 'btn', type: 'button', onclick: () => go('#/resultats') }, 'Résultats')),
      store.restored && resume ? el('p', { class: 'dim small' }, `Session restaurée (début ${new Date(store.doc.startedAt).toLocaleString('fr-FR')}).`) : null,
    ),
  );
}

// ------------------------------------------------------------------------------------------------ cas
function fmt(t) { return t.toFixed(2).replace('.', ','); }

function renderCase(v) {
  const c = M.cases[v.caseIndex];
  const n = M.cases.length;
  const answers = store.doc.cases[c.id];
  const locked = store.doc.validated;

  const renderBtns = M.renders.map((r) => el('button', {
    class: 'rbtn', type: 'button', 'data-id': r.id, 'aria-pressed': 'false',
    onclick: () => { if (engine.has(c.id, r.id)) engine.select(c.id, r.id); },
  }, el('span', { class: 'rlabel' }, r.label), el('span', { class: 'rhint' }, r.hint || '')));
  const playBtn = el('button', { class: 'btn play', type: 'button', 'aria-label': 'Lecture', onclick: () => engine.toggle(c.id) }, '▶');
  const scrub = el('input', { class: 'scrub', type: 'range', min: '0', max: '1000', step: '1', value: '0', 'aria-label': 'Position' });
  const time = el('span', { class: 'time' }, '0,00 / 0,00 s');
  const loopBtn = el('button', { class: 'btn small-btn', type: 'button', 'aria-pressed': String(engine.loop), onclick: () => engine.setLoop(!engine.loop) }, 'Boucle');
  const zeroBtn = el('button', { class: 'btn small-btn', type: 'button', onclick: () => engine.seek(c.id, 0) }, '⟲ Début');
  let dragging = false;
  scrub.addEventListener('input', () => { dragging = true; const d = engine.duration(c.id); time.textContent = `${fmt((scrub.value / 1000) * d)} / ${fmt(d)} s`; });
  scrub.addEventListener('change', () => { dragging = false; engine.seek(c.id, (scrub.value / 1000) * engine.duration(c.id)); });

  const player = el('div', { class: 'player' },
    el('div', { class: 'renders' }, ...renderBtns),
    el('div', { class: 'transport' }, playBtn, el('div', { class: 'scrub-wrap' }, scrub, time)),
    el('div', { class: 'transport2' }, loopBtn, zeroBtn, el('span', { class: 'pstate dim' }, '')));

  const q = renderQuestions(M.questions, answers, { disabled: locked, onChange: (id, val) => store.setAnswer(c.id, id, val) });
  const done = el('p', { class: 'case-done small' });
  const isLast = v.caseIndex === n - 1;
  const nav = el('nav', { class: 'case-nav' },
    el('button', { class: 'btn', type: 'button', onclick: () => go(v.caseIndex > 0 ? `#/cas/${M.cases[v.caseIndex - 1].id}` : '#/accueil') },
      v.caseIndex > 0 ? `← ${M.cases[v.caseIndex - 1].id}` : '← Accueil'),
    el('button', { class: 'btn primary', type: 'button', onclick: () => go(isLast ? '#/resultats' : `#/cas/${M.cases[v.caseIndex + 1].id}`) },
      isLast ? 'Résultats →' : `${M.cases[v.caseIndex + 1].id} →`));

  app.append(el('section', { class: 'case' },
    el('p', { class: 'kicker' }, `Cas ${v.caseIndex + 1}/${n}`),
    el('h1', {}, `${c.id} — ${c.title}`),
    el('p', { class: 'sub' }, c.description),
    storageBanner(),
    player,
    locked ? el('p', { class: 'panel info small' }, 'Réponses validées : modification impossible.') : null,
    q, done, nav));

  const tick = () => {
    const s = engine.state(c.id);
    const d = s.duration;
    if (!dragging) {
      scrub.value = d ? String(Math.round((s.position / d) * 1000)) : '0';
      time.textContent = `${fmt(s.position)} / ${fmt(d)} s`;
    }
    if (s.playing) raf = requestAnimationFrame(tick);
  };
  v.update = () => {
    const s = engine.state(c.id);
    for (const b of renderBtns) {
      const id = b.dataset.id;
      const avail = engine.has(c.id, id);
      b.disabled = !avail;
      b.setAttribute('aria-pressed', String(s.selected === id));
      b.classList.toggle('live', s.playing && s.activeId === id);
    }
    playBtn.textContent = s.playing ? '❚❚' : '▶';
    playBtn.setAttribute('aria-label', s.playing ? 'Pause' : 'Lecture');
    playBtn.disabled = !audioReady || !engine.has(c.id, s.selected || 'REF');
    loopBtn.setAttribute('aria-pressed', String(s.loop));
    player.querySelector('.pstate').textContent = !audioReady ? `Chargement ${audioDone}/${audioTotal}…`
      : s.playing ? `Lecture : ${s.activeId}` : s.selected ? `Pause : ${s.selected}` : 'Choisis REF, X ou Y';
    cancelAnimationFrame(raf);
    tick();
  };
  v.onStore = () => {
    const ok = store.caseComplete(c.id);
    done.textContent = ok ? '✓ Cas complet' : 'Réponds aux trois questions pour compléter ce cas.';
    done.className = `case-done small ${ok ? 'good' : 'dim'}`;
  };
  v.update();
  v.onStore();
}

// ------------------------------------------------------------------------------------------------ résultats
async function copyResults(fallback) {
  const text = store.json();
  try {
    if (navigator.clipboard && window.isSecureContext) { await navigator.clipboard.writeText(text); return true; }
  } catch (e) { /* repli ci-dessous */ }
  const ta = el('textarea', { class: 'offscreen', 'aria-hidden': 'true' });
  ta.value = text;
  document.body.append(ta);
  ta.focus();
  ta.select();
  ta.setSelectionRange(0, text.length);
  let ok = false;
  try { ok = document.execCommand('copy'); } catch (e) { ok = false; }
  ta.remove();
  if (!ok && fallback) { fallback.focus(); fallback.select(); }
  return ok;
}

function exportResults() {
  const blob = new Blob([store.json()], { type: 'application/json' });
  const url = URL.createObjectURL(blob);
  const a = el('a', { href: url, download: M.exportFileName });
  document.body.append(a);
  a.click();
  a.remove();
  setTimeout(() => URL.revokeObjectURL(url), 30000);
}

async function shareResults() {
  const file = new File([store.json()], M.exportFileName, { type: 'application/json' });
  try {
    if (navigator.canShare && navigator.canShare({ files: [file] })) await navigator.share({ files: [file], title: M.exportFileName });
    else await navigator.share({ title: M.exportFileName, text: store.json() });
  } catch (e) {
    if (e && e.name !== 'AbortError') toast('Partage impossible : utilise Exporter ou Copier.', 'warn');
  }
}

function renderResults(v) {
  const d = store.doc;
  const table = el('table', { class: 'summary' });
  const head = el('tr', {}, el('th', {}, 'Cas'), ...M.questions.filter((q) => q.type !== 'text').map((q) => el('th', {}, q.label)));
  table.append(el('thead', {}, head));
  const tb = el('tbody');
  for (const c of M.cases) {
    const a = d.cases[c.id];
    tb.append(el('tr', { class: store.caseComplete(c.id) ? '' : 'incomplete' },
      el('td', {}, el('a', { href: `#/cas/${c.id}` }, c.id)),
      ...M.questions.filter((q) => q.type !== 'text').map((q) => el('td', {}, answerLabel(q, a[q.id])))));
  }
  table.append(tb);

  const json = el('textarea', { class: 'json', readonly: 'readonly', rows: '14', 'aria-label': 'Résultats JSON', spellcheck: 'false' });
  json.value = store.json();
  const statusLine = el('p', { class: 'status-line' });
  const validateBtn = el('button', { class: 'btn primary', type: 'button', onclick: () => {
    if (store.doc.validated) return;
    if (window.confirm('Valider définitivement tes réponses ?\nElles ne pourront plus être modifiées sur cet appareil.')) {
      store.validate();
      render();
      toast('Réponses validées. Exporte ou copie tes résultats.');
    }
  } }, 'Valider mes réponses');
  const actions = el('div', { class: 'actions wrap' },
    el('button', { class: 'btn', type: 'button', onclick: () => { exportResults(); toast(`${M.exportFileName} exporté`); } }, 'Exporter le JSON'),
    navigator.share ? el('button', { class: 'btn', type: 'button', onclick: shareResults }, 'Partager…') : null,
    el('button', { class: 'btn', type: 'button', onclick: async () => {
      const ok = await copyResults(json);
      toast(ok ? 'Résultats copiés dans le presse-papiers' : 'Copie automatique impossible : texte sélectionné ci-dessous', ok ? 'ok' : 'warn');
    } }, 'Copier mes résultats'));
  const resetBtn = el('button', { class: 'btn danger', type: 'button', onclick: () => {
    if (window.confirm('Effacer toutes les réponses enregistrées sur cet appareil et recommencer ?\nCette action est irréversible.')) {
      store.reset();
      engine.pause();
      engine.positions.clear();
      engine.selected.clear();
      go('#/accueil');
      toast('Session réinitialisée');
    }
  } }, 'Réinitialiser');

  app.append(el('section', { class: 'results' },
    el('p', { class: 'kicker' }, 'Résultats'),
    el('h1', {}, 'Synthèse'),
    storageBanner(),
    statusLine,
    el('div', { class: 'table-wrap' }, table),
    d.validated ? null : validateBtn,
    el('h2', {}, 'Transmettre'),
    el('p', { class: 'dim small' }, 'Aucun envoi automatique : la page est statique. Exporte le fichier, partage-le, ou copie le texte et colle-le dans un message.'),
    actions,
    el('label', { class: 'small dim', for: 'json-out' }, `Résultats (JSON, mis à jour à chaque modification)`),
    json,
    el('h2', {}, 'Clé X/Y'),
    el('p', { class: 'dim small' }, M.keyCommitment.note),
    el('ul', { class: 'hashes' }, ...M.keyCommitment.files.map((f) => el('li', {}, el('code', {}, f.name), el('br'), el('code', { class: 'dim' }, f.sha256)))),
    el('h2', {}, 'Session'),
    el('p', { class: 'dim small' }, `Identifiant ${d.sessionId} · début ${new Date(d.startedAt).toLocaleString('fr-FR')}`),
    resetBtn,
  ));
  json.id = 'json-out';
  v.onStore = () => {
    json.value = store.json();
    const dd = store.doc;
    statusLine.textContent = dd.validated
      ? `Validé le ${new Date(dd.validatedAt).toLocaleString('fr-FR')} · ${dd.answeredCases}/${dd.totalCases} cas`
      : dd.status === 'complete' ? `Complet · ${dd.answeredCases}/${dd.totalCases} cas · non validé`
        : `Incomplet · ${dd.answeredCases}/${dd.totalCases} cas complétés`;
    statusLine.className = `status-line ${dd.status === 'complete' ? 'good' : 'warn'}`;
    validateBtn.disabled = dd.status !== 'complete';
  };
  v.update = () => {};
  v.onStore();
}

// ------------------------------------------------------------------------------------------------ clavier (ordinateur)
function onKey(e) {
  if (!currentView || currentView.name !== 'case') return;
  const t = e.target;
  if (t && (t.tagName === 'TEXTAREA' || (t.tagName === 'INPUT' && t.type === 'text'))) return;
  const c = M.cases[currentView.caseIndex];
  const map = { 1: 'REF', 2: 'X', 3: 'Y', r: 'REF', x: 'X', y: 'Y' };
  const id = map[e.key.toLowerCase ? e.key.toLowerCase() : e.key];
  if (id && engine.has(c.id, id)) { engine.select(c.id, id); e.preventDefault(); }
  else if (e.key === ' ') { engine.toggle(c.id); e.preventDefault(); }
}

boot();
