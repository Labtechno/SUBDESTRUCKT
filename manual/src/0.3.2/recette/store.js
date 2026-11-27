// SUBDESTRUCKT — assistant de recette (ARCH-G3, clé par édition depuis 0.3.0) : stockage de session (localStorage) et document JSON des résultats.
// Clé distincte de la plateforme d'écoute S0.1. Aucune transmission réseau : export, copie ou partage à l'initiative de
// l'utilisateur uniquement.
window.SBDRecetteStore = (() => {
  const SCHEMA = 1;
  // Clé de stockage propre à l'édition (versions.json > recetteStorageKey), injectée par build.py ; valeur historique
  // b5 seulement à défaut. Corrige le partage de clé entre éditions (défaut constaté dans 0.2.0 : clé b5 réutilisée).
  const DEFAULT_KEY = 'labtechno.subdestruckt.manual.arch-g3-recette.v1';
  let KEY = DEFAULT_KEY;
  const STATUSES = ['PENDING', 'PASS', 'FAIL', 'N/A'];
  const now = () => new Date().toISOString();
  const uuid = () => (crypto.randomUUID ? crypto.randomUUID() : 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, (c) => {
    const r = (crypto.getRandomValues(new Uint8Array(1))[0] & 15); return (c === 'x' ? r : (r & 3) | 8).toString(16);
  }));
  let available = true;
  try { localStorage.setItem('__sbd_probe', '1'); localStorage.removeItem('__sbd_probe'); } catch (e) { available = false; }

  function fresh(cfg) {
    const t = now();
    const results = {};
    cfg.tests.forEach((x) => { results[x.id] = { status: 'PENDING', observations: '', naJustification: '', evidence: '', updatedAt: null }; });
    return {
      schemaVersion: SCHEMA,
      campaignId: cfg.campaignId,
      product: 'SUBDESTRUCKT',
      version: cfg.version,
      buildSha: cfg.buildSha,
      documentationSha: cfg.docSha,
      sessionId: uuid(),
      startedAt: t,
      updatedAt: t,
      globalStatus: 'en cours',
      counts: { total: cfg.tests.length, filled: 0, pass: 0, fail: 0, pending: cfg.tests.length, na: 0, naWithoutJustification: 0 },
      configuration: { machine: '', macos: '', daw: '', dawVersion: '', audioInterface: '', sampleRatesTested: [], bufferSizes: '' },
      results,
      generalObservations: '',
      finalValidation: { validated: false, validatedAt: null, statusAtValidation: null },
      client: { userAgent: navigator.userAgent, storageAvailable: available },
    };
  }

  function recompute(doc) {
    const c = { total: 0, filled: 0, pass: 0, fail: 0, pending: 0, na: 0, naWithoutJustification: 0 };
    Object.values(doc.results).forEach((r) => {
      c.total++;
      if (r.status === 'PASS') c.pass++;
      else if (r.status === 'FAIL') c.fail++;
      else if (r.status === 'N/A') { c.na++; if (!r.naJustification.trim()) c.naWithoutJustification++; }
      else c.pending++;
    });
    c.filled = c.total - c.pending;
    doc.counts = c;
    // la progression ne masque jamais un échec
    if (c.fail > 0) doc.globalStatus = c.pending > 0 ? 'en cours — avec FAIL' : 'terminée — avec FAIL';
    else if (c.pending > 0 || c.naWithoutJustification > 0) doc.globalStatus = 'en cours';
    else doc.globalStatus = 'terminée — aucun FAIL';
  }

  function create(cfg) {
    KEY = cfg.storageKey || DEFAULT_KEY;
    let doc = null;
    if (available) {
      try {
        const d = JSON.parse(localStorage.getItem(KEY) || 'null');
        if (d && d.schemaVersion === SCHEMA && d.campaignId === cfg.campaignId && d.buildSha === cfg.buildSha) {
          const base = fresh(cfg);
          cfg.tests.forEach((x) => { d.results[x.id] = Object.assign(base.results[x.id], d.results[x.id] || {}); });
          d.configuration = Object.assign(base.configuration, d.configuration || {});
          doc = d;
        }
      } catch (e) { doc = null; }
    }
    const restored = !!doc;
    if (!doc) doc = fresh(cfg);
    recompute(doc);
    let lastSave = null;
    const listeners = [];
    const persist = () => {
      doc.updatedAt = now();
      recompute(doc);
      if (available) { try { localStorage.setItem(KEY, JSON.stringify(doc)); lastSave = new Date(); } catch (e) { lastSave = null; } }
      listeners.forEach((f) => f());
    };
    return {
      get doc() { return doc; },
      restored, available, STATUSES, KEY,
      lastSave: () => lastSave,
      onChange: (f) => listeners.push(f),
      setResult(id, field, value) {
        if (doc.finalValidation.validated) return false;
        if (field === 'status' && !STATUSES.includes(value)) return false;
        doc.results[id][field] = value;
        doc.results[id].updatedAt = now();
        persist();
        return true;
      },
      setConfig(field, value) { if (doc.finalValidation.validated) return false; doc.configuration[field] = value; persist(); return true; },
      setGeneral(value) { if (doc.finalValidation.validated) return false; doc.generalObservations = value; persist(); return true; },
      canValidate() { return doc.counts.pending === 0 && doc.counts.naWithoutJustification === 0; },
      validate() {
        if (!this.canValidate()) return false;
        doc.finalValidation = { validated: true, validatedAt: now(), statusAtValidation: doc.globalStatus };
        persist();
        return true;
      },
      reopen() { doc.finalValidation = { validated: false, validatedAt: null, statusAtValidation: null }; persist(); },
      reset() { if (available) { try { localStorage.removeItem(KEY); } catch (e) { /* ignoré */ } } doc = fresh(cfg); persist(); },
      json() { return JSON.stringify(doc, null, 2); },
    };
  }
  return { create, get KEY() { return KEY; }, SCHEMA };
})();
