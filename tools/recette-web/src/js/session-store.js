// LABTECHNO — sauvegarde des sessions de recette (localStorage), document JSON de résultats.
// Chaque modification met à jour `updatedAt`, recalcule `status` et réécrit immédiatement le stockage local.
// Si le stockage est indisponible (navigation privée stricte, quota…), le document vit en mémoire et
// `available === false` : l'interface invite alors à exporter ou copier les résultats.

export const RESULTS_SCHEMA_VERSION = 1;

function nowIso() { return new Date().toISOString(); }

function uuid() {
  if (crypto.randomUUID) return crypto.randomUUID();
  const b = crypto.getRandomValues(new Uint8Array(16));
  b[6] = (b[6] & 0x0f) | 0x40; b[8] = (b[8] & 0x3f) | 0x80;
  const h = Array.from(b, (x) => x.toString(16).padStart(2, '0')).join('');
  return `${h.slice(0, 8)}-${h.slice(8, 12)}-${h.slice(12, 16)}-${h.slice(16, 20)}-${h.slice(20)}`;
}

function probeStorage() {
  try {
    const k = '__labtechno_probe__';
    window.localStorage.setItem(k, '1');
    window.localStorage.removeItem(k);
    return true;
  } catch (e) { return false; }
}

export function isAnswered(q, v) {
  if (!q.required) return true;
  return q.type === 'text' ? typeof v === 'string' && v.trim() !== '' : v !== null && v !== undefined && v !== '';
}

export class SessionStore extends EventTarget {
  constructor(manifest) {
    super();
    this.m = manifest;
    this.key = `labtechno.recette.${manifest.campaignId}.v${RESULTS_SCHEMA_VERSION}`;
    this.available = probeStorage();
    this.lastSaveOk = null;
    this.lastSavedAt = null;
    this.restored = false;
    const loaded = this._load();
    this.doc = loaded || this._fresh();
    this.restored = !!loaded;
    this._recompute();
    this._persist();
  }

  _blankCase() {
    const c = {};
    for (const q of this.m.questions) c[q.id] = q.type === 'text' ? '' : null;
    c.updatedAt = null;
    return c;
  }

  _fresh() {
    const t = nowIso();
    const cases = {};
    for (const c of this.m.cases) cases[c.id] = this._blankCase();
    const ctx = {};
    for (const q of this.m.contextQuestions || []) ctx[q.id] = q.type === 'text' ? '' : null;
    return {
      schemaVersion: RESULTS_SCHEMA_VERSION,
      campaignId: this.m.campaignId,
      campaignTitle: this.m.title,
      audioCommit: this.m.audioSource.commit,
      sessionId: uuid(),
      startedAt: t,
      updatedAt: t,
      status: 'incomplete',
      answeredCases: 0,
      totalCases: this.m.cases.length,
      validated: false,
      validatedAt: null,
      listeningContext: ctx,
      cases,
      audioIntegrity: null,
      client: { userAgent: navigator.userAgent, audioSampleRate: null, storageAvailable: this.available },
    };
  }

  _load() {
    if (!this.available) return null;
    let raw = null;
    try { raw = window.localStorage.getItem(this.key); } catch (e) { return null; }
    if (!raw) return null;
    let d;
    try { d = JSON.parse(raw); } catch (e) { return null; }
    if (!d || d.schemaVersion !== RESULTS_SCHEMA_VERSION || d.campaignId !== this.m.campaignId) return null;
    if (d.audioCommit !== this.m.audioSource.commit) {
      // audio différent : on archive l'ancienne session et on repart de zéro
      try { window.localStorage.setItem(`${this.key}.archived.${Date.now()}`, raw); } catch (e) { /* ignoré */ }
      return null;
    }
    // complète les champs manquants (manifeste enrichi depuis)
    const fresh = this._fresh();
    for (const k of Object.keys(fresh)) if (!(k in d)) d[k] = fresh[k];
    for (const c of this.m.cases) d.cases[c.id] = Object.assign(this._blankCase(), d.cases[c.id] || {});
    d.listeningContext = Object.assign(fresh.listeningContext, d.listeningContext || {});
    d.client = Object.assign(fresh.client, d.client || {}, { userAgent: navigator.userAgent, storageAvailable: this.available });
    return d;
  }

  _recompute() {
    let n = 0;
    for (const c of this.m.cases) if (this.caseComplete(c.id)) n += 1;
    this.doc.answeredCases = n;
    this.doc.totalCases = this.m.cases.length;
    this.doc.status = n === this.m.cases.length ? 'complete' : 'incomplete';
  }

  _persist() {
    if (!this.available) { this.lastSaveOk = false; return false; }
    try {
      window.localStorage.setItem(this.key, JSON.stringify(this.doc));
      this.lastSaveOk = true;
      this.lastSavedAt = new Date();
    } catch (e) {
      this.lastSaveOk = false;
    }
    return this.lastSaveOk;
  }

  _changed(touch = true) {
    if (touch) this.doc.updatedAt = nowIso();
    this._recompute();
    this._persist();
    this.dispatchEvent(new Event('change'));
  }

  caseComplete(caseId) {
    const a = this.doc.cases[caseId];
    return !!a && this.m.questions.every((q) => isAnswered(q, a[q.id]));
  }

  setAnswer(caseId, qid, value) {
    if (this.doc.validated) return false;
    const a = this.doc.cases[caseId];
    if (!a || a[qid] === value) return false;
    a[qid] = value;
    a.updatedAt = nowIso();
    this._changed();
    return true;
  }

  setContext(qid, value) {
    if (this.doc.validated || this.doc.listeningContext[qid] === value) return false;
    this.doc.listeningContext[qid] = value;
    this._changed();
    return true;
  }

  setIntegrity(info) { this.doc.audioIntegrity = info; this._changed(false); }
  setClient(patch) { Object.assign(this.doc.client, patch); this._changed(false); }

  validate() {
    if (this.doc.validated || this.doc.status !== 'complete') return false;
    this.doc.validated = true;
    this.doc.validatedAt = nowIso();
    this._changed();
    return true;
  }

  reset() {
    try { if (this.available) window.localStorage.removeItem(this.key); } catch (e) { /* ignoré */ }
    this.doc = this._fresh();
    this._recompute();
    this._persist();
    this.dispatchEvent(new Event('change'));
  }

  json() { return JSON.stringify(this.doc, null, 2); }
}
