// LABTECHNO — moteur d'écoute générique (Web Audio, sans dépendance).
//
// - Charge des fichiers audio, vérifie leur SHA-256 octet pour octet, puis les décode.
// - Lecture exclusive : une seule source audible à la fois. Une bascule démarre la nouvelle source à la même
//   position temporelle que la précédente.
// - Les rendus d'un même groupe (un cas) partagent une position, conservée quand on change de groupe.
// - Aucun traitement du signal : gain unitaire, connexion directe à la sortie. Seul un fondu linéaire de
//   `switchFadeMs` est appliqué au démarrage, à la pause et à la bascule pour éviter un clic (0 = coupure sèche).
// - Le contexte audio est demandé à 48 kHz : les WAV 48 kHz sont alors décodés sans rééchantillonnage.

const AC = window.AudioContext || window.webkitAudioContext;

export async function sha256Hex(arrayBuffer) {
  const d = await crypto.subtle.digest('SHA-256', arrayBuffer);
  return Array.from(new Uint8Array(d), (b) => b.toString(16).padStart(2, '0')).join('');
}

export class ListeningEngine extends EventTarget {
  constructor({ switchFadeMs = 5, loop = false, sampleRate = 48000 } = {}) {
    super();
    this.fade = Math.max(0, switchFadeMs) / 1000;
    this.wantedRate = sampleRate;
    this.loop = !!loop;
    this.ctx = null;
    this.buffers = new Map();   // "group/id" -> AudioBuffer
    this.integrity = new Map(); // "group/id" -> {ok, expected, actual, bytes, error}
    this.positions = new Map(); // group -> secondes
    this.selected = new Map();  // group -> id
    this.group = null;
    this.active = null;         // {src, gain, id, group}
    this.playing = false;
    this.liveSources = 0;       // sources démarrées et pas encore terminées (contrôle d'exclusivité)
    this.anchorCtx = 0;
    this.anchorPos = 0;
    document.addEventListener('visibilitychange', () => { if (document.hidden) this.pause(); });
  }

  context() {
    if (this.ctx) return this.ctx;
    try { this.ctx = new AC({ sampleRate: this.wantedRate, latencyHint: 'interactive' }); }
    catch (e) { this.ctx = new AC(); }
    // iOS 17+ : lecture même avec l'interrupteur silencieux (catégorie « playback »).
    try { if (navigator.audioSession) navigator.audioSession.type = 'playback'; } catch (e) { /* non supporté */ }
    return this.ctx;
  }

  get sampleRate() { return this.ctx ? this.ctx.sampleRate : null; }
  key(group, id) { return `${group}/${id}`; }
  has(group, id) { return this.buffers.has(this.key(group, id)); }

  decode(ab) {
    const ctx = this.context();
    return new Promise((resolve, reject) => {
      const p = ctx.decodeAudioData(ab, resolve, reject);
      if (p && typeof p.then === 'function') p.then(resolve, reject);
    });
  }

  static supported() { return typeof AC === 'function'; }

  // items : [{group, id, url, sha256, bytes}] ; onProgress(done, total, item, result)
  // result.ok = fichier intègre (taille + SHA-256) ; result.playable = décodé et prêt à lire.
  async load(items, onProgress) {
    let done = 0;
    const one = async (it) => {
      const k = this.key(it.group, it.id);
      const res = { ok: false, playable: false, expected: it.sha256, actual: null, bytes: 0, error: null };
      try {
        const r = await fetch(it.url, { cache: 'no-cache' });
        if (!r.ok) throw new Error(`HTTP ${r.status}`);
        const ab = await r.arrayBuffer();
        res.bytes = ab.byteLength;
        res.actual = await sha256Hex(ab);
        if (it.bytes && ab.byteLength !== it.bytes) throw new Error(`taille ${ab.byteLength} ≠ ${it.bytes}`);
        if (res.actual !== it.sha256) throw new Error('empreinte SHA-256 différente');
        res.ok = true;
        if (!ListeningEngine.supported()) throw new Error('Web Audio indisponible dans ce navigateur');
        this.buffers.set(k, await this.decode(ab));
        res.playable = true;
      } catch (e) {
        res.error = String(e && e.message ? e.message : e);
      }
      this.integrity.set(k, res);
      done += 1;
      if (onProgress) onProgress(done, items.length, it, res);
      return res;
    };
    return Promise.all(items.map(one));
  }

  duration(group = this.group) {
    const id = this.selected.get(group);
    const b = id && this.buffers.get(this.key(group, id));
    if (b) return b.duration;
    for (const [k, buf] of this.buffers) if (k.startsWith(group + '/')) return buf.duration;
    return 0;
  }

  position(group = this.group) {
    if (this.playing && this.active && group === this.active.group) {
      const d = this.duration(group);
      let t = this.anchorPos + (this.ctx.currentTime - this.anchorCtx);
      if (this.loop && d > 0) t %= d; else t = Math.min(t, d);
      return Math.max(0, t);
    }
    return this.positions.get(group) || 0;
  }

  _emit() { this.dispatchEvent(new Event('state')); }

  _fadeOut(node, now) {
    const g = node.gain.gain;
    try {
      g.cancelScheduledValues(now);
      g.setValueAtTime(g.value, now);
      if (this.fade > 0) g.linearRampToValueAtTime(0, now + this.fade); else g.setValueAtTime(0, now);
    } catch (e) { /* noeud déjà arrêté */ }
    node.src.onended = () => { this.liveSources -= 1; };
    try { node.src.stop(now + this.fade + 0.005); } catch (e) { /* déjà arrêté */ }
  }

  _start(group, id, offset) {
    const ctx = this.context();
    const buf = this.buffers.get(this.key(group, id));
    if (!buf) return false;
    if (!(offset >= 0) || offset >= buf.duration - 0.001) offset = 0;
    const now = ctx.currentTime;
    const src = ctx.createBufferSource();
    src.buffer = buf;
    src.loop = this.loop;
    const gain = ctx.createGain();
    src.connect(gain);
    gain.connect(ctx.destination);
    if (this.fade > 0) {
      gain.gain.setValueAtTime(0, now);
      gain.gain.linearRampToValueAtTime(1, now + this.fade);
    } else {
      gain.gain.setValueAtTime(1, now);
    }
    const node = { src, gain, id, group };
    src.onended = () => {
      this.liveSources -= 1;
      if (this.active === node) { // fin naturelle (lecture sans boucle)
        this.active = null;
        this.playing = false;
        this.positions.set(group, 0);
        this._emit();
      }
    };
    src.start(now, offset);
    this.liveSources += 1;
    if (this.active) this._fadeOut(this.active, now);
    this.active = node;
    this.playing = true;
    this.anchorCtx = now;
    this.anchorPos = offset;
    return true;
  }

  _resume() {
    const ctx = this.context();
    if (ctx.state !== 'running') { try { ctx.resume(); } catch (e) { /* ignoré */ } }
  }

  // Sélection d'un rendu par l'utilisateur : bascule (si lecture en cours) ou démarrage à la position conservée.
  select(group, id) {
    this._resume();
    if (this.group !== group) this.pause();
    this.group = group;
    const pos = this.position(group);
    this.selected.set(group, id);
    this._start(group, id, pos);
    this._emit();
  }

  play(group = this.group) {
    this._resume();
    if (this.group !== group) this.pause();
    this.group = group;
    const id = this.selected.get(group) || 'REF';
    this.selected.set(group, id);
    this._start(group, id, this.positions.get(group) || 0);
    this._emit();
  }

  pause() {
    if (!this.playing || !this.active) return;
    const g = this.active.group;
    this.positions.set(g, this.position(g));
    this._fadeOut(this.active, this.ctx.currentTime);
    this.active = null;
    this.playing = false;
    this._emit();
  }

  toggle(group) {
    if (this.playing && this.active && this.active.group === group) this.pause(); else this.play(group);
  }

  seek(group, t) {
    const d = this.duration(group);
    t = Math.max(0, Math.min(t, Math.max(0, d - 0.001)));
    if (this.playing && this.active && this.active.group === group) this._start(group, this.active.id, t);
    else this.positions.set(group, t);
    this._emit();
  }

  setLoop(on) {
    on = !!on;
    if (this.playing && this.active) {
      const t = this.position(this.active.group);
      this.anchorPos = t;
      this.anchorCtx = this.ctx.currentTime;
      this.active.src.loop = on;
    }
    this.loop = on;
    this._emit();
  }

  state(group) {
    return {
      group,
      selected: this.selected.get(group) || null,
      playing: this.playing && !!this.active && this.active.group === group,
      activeId: this.active && this.active.group === group ? this.active.id : null,
      position: this.position(group),
      duration: this.duration(group),
      loop: this.loop,
    };
  }
}
