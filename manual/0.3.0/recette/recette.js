// SUBDESTRUCKT — assistant de recette ARCH-G3 : interface (formulaires, progression, export, copie, partage).
(() => {
  const cfg = window.SBD_RECETTE_CONFIG;  // {campaignId, version, buildSha, docSha, tests, groups, exportFileName}
  const store = window.SBDRecetteStore.create(cfg);
  const $ = (s, r = document) => r.querySelector(s);
  const el = (tag, attrs = {}, ...kids) => {
    const n = document.createElement(tag);
    Object.entries(attrs).forEach(([k, v]) => { if (v === false || v == null) return; if (k === 'class') n.className = v; else if (k.startsWith('on')) n.addEventListener(k.slice(2), v); else n.setAttribute(k, v === true ? '' : v); });
    kids.flat().forEach((k) => { if (k != null && k !== false) n.append(k instanceof Node ? k : document.createTextNode(String(k))); });
    return n;
  };
  const toast = (msg) => { const t = $('#toast'); t.textContent = msg; t.hidden = false; clearTimeout(toast._t); toast._t = setTimeout(() => { t.hidden = true; }, 3000); };

  // ---- configuration matérielle et logicielle
  const conf = $('#config');
  const fields = [
    ['machine', 'Mac (modèle et puce)', 'ex. Mac Studio M4 Max'], ['macos', 'Version de macOS', 'ex. macOS 15.x'],
    ['daw', 'DAW', 'ex. Reason'], ['dawVersion', 'Version de la DAW', 'ex. 13.x'],
    ['audioInterface', 'Interface audio', 'ex. modèle de la carte son'], ['bufferSizes', 'Tailles de buffer utilisées', 'ex. 128, 256'],
  ];
  fields.forEach(([k, label, ph]) => {
    const id = `cfg-${k}`;
    const input = el('input', { id, type: 'text', placeholder: ph, autocomplete: 'off' });
    input.value = store.doc.configuration[k] || '';
    input.addEventListener('input', () => store.setConfig(k, input.value));
    conf.append(el('div', { class: 'field' }, el('label', { for: id }, label), input));
  });
  const srBox = el('fieldset', { class: 'field' }, el('legend', {}, "Fréquences d'échantillonnage testées"));
  ['44100', '48000', '88200', '96000'].forEach((sr) => {
    const id = `sr-${sr}`;
    const cb = el('input', { type: 'checkbox', id, value: sr });
    cb.checked = store.doc.configuration.sampleRatesTested.includes(sr);
    cb.addEventListener('change', () => {
      const set = new Set(store.doc.configuration.sampleRatesTested);
      cb.checked ? set.add(sr) : set.delete(sr);
      store.setConfig('sampleRatesTested', Array.from(set).sort());
    });
    srBox.append(el('label', { class: 'check', for: id }, cb, ` ${Number(sr) / 1000} kHz`));
  });
  conf.append(srBox);

  // ---- tests
  const list = $('#tests');
  const byId = Object.fromEntries(cfg.tests.map((t) => [t.id, t]));
  const cards = {};
  cfg.groups.forEach((g) => {
    list.append(el('h2', { id: `g-${g.id}` }, g.title));
    g.tests.forEach((tid) => {
      const t = byId[tid];
      const r = store.doc.results[tid];
      const name = `st-${tid}`;
      const radios = store.STATUSES.map((s) => {
        const id = `${name}-${s.replace('/', '')}`;
        const inp = el('input', { type: 'radio', name, id, value: s });
        inp.checked = r.status === s;
        inp.addEventListener('change', () => { if (inp.checked) { store.setResult(tid, 'status', s); refreshCard(tid); } });
        return el('label', { class: `st st-${s.replace('/', '')}`, for: id }, inp, el('span', {}, s));
      });
      const obs = el('textarea', { id: `obs-${tid}`, rows: '3' }); obs.value = r.observations;
      obs.addEventListener('input', () => store.setResult(tid, 'observations', obs.value));
      const na = el('textarea', { id: `na-${tid}`, rows: '2' }); na.value = r.naJustification;
      na.addEventListener('input', () => { store.setResult(tid, 'naJustification', na.value); refreshCard(tid); });
      const ev = el('input', { id: `ev-${tid}`, type: 'text', placeholder: t.evidenceHint }); ev.value = r.evidence;
      ev.addEventListener('input', () => store.setResult(tid, 'evidence', ev.value));
      const naWrap = el('div', { class: 'field na-field' }, el('label', { for: `na-${tid}` }, 'Justification N/A (obligatoire si N/A)'), na);
      const card = el('article', { class: 'tcard', id: tid, 'aria-labelledby': `h-${tid}` },
        el('header', {}, el('h3', { id: `h-${tid}` }, el('span', { class: 'tid' }, tid), ` ${t.title}`), el('span', { class: 'badge', id: `b-${tid}` })),
        el('p', { class: 'why' }, t.why),
        el('details', {}, el('summary', {}, 'Procédure détaillée'),
          el('p', {}, el('strong', {}, 'Prérequis : '), t.prerequisites.join(' ')),
          el('p', {}, el('strong', {}, 'Compétences ou outils : '), t.skills),
          el('ol', {}, t.steps.map((s) => el('li', {}, s))),
          el('p', {}, el('strong', {}, 'Résultat attendu : '), t.expected),
          el('p', {}, el('strong', {}, 'Critère officiel : '), t.official.expected),
          el('p', {}, el('strong', {}, 'PASS si : '), t.pass), el('p', {}, el('strong', {}, 'FAIL si : '), t.fail),
          el('p', {}, el('strong', {}, 'En cas d\'échec : '), t.onFail)),
        el('fieldset', { class: 'status' }, el('legend', {}, `Statut de ${tid}`), radios),
        naWrap,
        el('div', { class: 'field' }, el('label', { for: `obs-${tid}` }, 'Observations'), obs),
        el('div', { class: 'field' }, el('label', { for: `ev-${tid}` }, 'Référence de preuve (nom de capture, fichier…)'), ev));
      cards[tid] = { card, naWrap, inputs: [obs, na, ev, ...card.querySelectorAll('input[type=radio]')] };
      list.append(card);
    });
  });

  function refreshCard(tid) {
    const r = store.doc.results[tid];
    const c = cards[tid];
    const b = $(`#b-${tid}`);
    b.textContent = r.status === 'N/A' && !r.naJustification.trim() ? 'N/A — justification manquante' : r.status;
    b.className = `badge b-${r.status.replace('/', '')}`;
    c.card.dataset.status = r.status;
    c.naWrap.hidden = r.status !== 'N/A';
    const locked = store.doc.finalValidation.validated;
    c.inputs.forEach((i) => { i.disabled = locked; });
  }

  // ---- synthèse, progression distincte de la validation
  const json = $('#json');
  function refreshSummary() {
    const d = store.doc, c = d.counts;
    $('#progressBar').value = c.filled; $('#progressBar').max = c.total;
    $('#progressText').textContent = `${c.filled} / ${c.total} tests renseignés (progression de la recette — ce n'est pas une validation)`;
    $('#counts').textContent = `PASS ${c.pass} · FAIL ${c.fail} · N/A ${c.na}${c.naWithoutJustification ? ` (dont ${c.naWithoutJustification} sans justification)` : ''} · PENDING ${c.pending}`;
    $('#globalStatus').textContent = `Statut global : ${d.globalStatus}`;
    $('#globalStatus').className = c.fail ? 'bad' : (c.pending ? '' : 'good');
    const fails = Object.entries(d.results).filter(([, r]) => r.status === 'FAIL').map(([k]) => k);
    const fl = $('#failList');
    fl.hidden = fails.length === 0;
    fl.textContent = fails.length ? `Tests en échec : ${fails.join(', ')}` : '';
    const v = d.finalValidation;
    $('#validation').textContent = v.validated ? `Recette validée le ${new Date(v.validatedAt).toLocaleString('fr-FR')} (statut : ${v.statusAtValidation}). Réponses verrouillées.` : 'Recette non validée.';
    $('#validateBtn').disabled = v.validated || !store.canValidate();
    $('#reopenBtn').hidden = !v.validated;
    $('#general').disabled = v.validated;
    fields.forEach(([k]) => { $(`#cfg-${k}`).disabled = v.validated; });
    srBox.querySelectorAll('input').forEach((i) => { i.disabled = v.validated; });
    json.value = store.json();
    const ls = store.lastSave();
    $('#saveState').textContent = !store.available ? 'Stockage local indisponible : exportez ou copiez vos résultats avant de quitter.' : (ls ? `Enregistré automatiquement à ${ls.toLocaleTimeString('fr-FR')}` : 'Enregistrement automatique actif');
  }
  store.onChange(refreshSummary);
  cfg.tests.forEach((t) => refreshCard(t.id));
  const general = $('#general'); general.value = store.doc.generalObservations;
  general.addEventListener('input', () => store.setGeneral(general.value));
  refreshSummary();
  if (store.restored) toast('Session de recette restaurée.');

  // ---- actions
  $('#validateBtn').addEventListener('click', () => {
    if (!store.canValidate()) return;
    const c = store.doc.counts;
    if (window.confirm(`Valider définitivement la recette ?\nPASS ${c.pass} · FAIL ${c.fail} · N/A ${c.na}\nLes réponses seront verrouillées (vous pourrez rouvrir pour corriger).`)) {
      store.validate(); cfg.tests.forEach((t) => refreshCard(t.id)); toast('Recette validée. Exportez ou copiez les résultats.');
    }
  });
  $('#reopenBtn').addEventListener('click', () => {
    if (window.confirm('Rouvrir la recette pour corriger des réponses ? La validation finale sera annulée.')) {
      store.reopen(); cfg.tests.forEach((t) => refreshCard(t.id));
    }
  });
  $('#exportBtn').addEventListener('click', () => {
    const blob = new Blob([store.json()], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = el('a', { href: url, download: cfg.exportFileName });
    document.body.append(a); a.click(); a.remove();
    setTimeout(() => URL.revokeObjectURL(url), 30000);
    toast(`${cfg.exportFileName} exporté.`);
  });
  $('#copyBtn').addEventListener('click', async () => {
    const text = store.json();
    let ok = false;
    try { if (navigator.clipboard && window.isSecureContext) { await navigator.clipboard.writeText(text); ok = true; } } catch (e) { ok = false; }
    if (!ok) { json.focus(); json.select(); try { ok = document.execCommand('copy'); } catch (e) { ok = false; } }
    toast(ok ? 'Résultats copiés : collez-les dans votre message à la direction technique.' : 'Copie automatique impossible : le texte JSON est sélectionné ci-dessous.');
  });
  if (navigator.share) {
    $('#shareBtn').hidden = false;
    $('#shareBtn').addEventListener('click', async () => {
      const file = new File([store.json()], cfg.exportFileName, { type: 'application/json' });
      try {
        if (navigator.canShare && navigator.canShare({ files: [file] })) await navigator.share({ files: [file], title: cfg.exportFileName });
        else await navigator.share({ title: cfg.exportFileName, text: store.json() });
      } catch (e) { if (e.name !== 'AbortError') toast('Partage impossible : utilisez Exporter ou Copier.'); }
    });
  }
  $('#resetBtn').addEventListener('click', () => {
    if (window.confirm('Effacer toutes les réponses de recette enregistrées sur cet appareil ? Cette action est irréversible.')) {
      store.reset(); location.reload();
    }
  });
  window.__sbdRecette = store;  // inspection par les tests automatisés
})();
