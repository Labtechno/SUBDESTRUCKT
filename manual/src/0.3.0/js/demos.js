// SUBDESTRUCKT — démonstrations pédagogiques interactives (SIMULATIONS dessinées par le navigateur, sans son).
// Elles illustrent des notions générales ; elles ne sont pas des rendus du moteur SUBDESTRUCKT.
(() => {
  const css = (v) => getComputedStyle(document.documentElement).getPropertyValue(v).trim();
  const setup = (canvas) => {
    const dpr = window.devicePixelRatio || 1;
    const w = canvas.clientWidth, h = canvas.clientHeight;
    canvas.width = Math.round(w * dpr); canvas.height = Math.round(h * dpr);
    const g = canvas.getContext('2d'); g.setTransform(dpr, 0, 0, dpr, 0, 0);
    g.clearRect(0, 0, w, h);
    g.strokeStyle = css('--line'); g.lineWidth = 1;
    g.beginPath(); g.moveTo(0, h / 2); g.lineTo(w, h / 2); g.stroke();
    return { g, w, h };
  };
  const plot = (g, w, h, f, color, lw = 2, amp = 0.42) => {
    g.strokeStyle = color; g.lineWidth = lw; g.beginPath();
    for (let x = 0; x <= w; x++) { const y = h / 2 - f(x / w) * h * amp; x ? g.lineTo(x, y) : g.moveTo(x, y); }
    g.stroke();
  };
  const bind = (root, draw) => {
    const inputs = Array.from(root.querySelectorAll('input, select'));
    const run = () => {
      inputs.forEach((i) => {
        const o = root.querySelector(`[data-out="${i.name}"]`);
        if (!o) return;
        const dec = (i.step && i.step.includes('.')) ? i.step.split('.')[1].length : 0;
        o.textContent = (i.type === 'range' ? parseFloat(i.value).toFixed(dec) : i.value) + (i.dataset.unit || '');
      });
      draw(Object.fromEntries(inputs.map((i) => [i.name, i.type === 'range' ? parseFloat(i.value) : i.value])));
    };
    inputs.forEach((i) => i.addEventListener('input', run));
    window.addEventListener('resize', run);
    run();
  };
  const TAU = Math.PI * 2;

  // 1. fréquence et amplitude
  document.querySelectorAll('.demo[data-demo="sine"]').forEach((root) => {
    const c = root.querySelector('canvas');
    bind(root, (p) => { const { g, w, h } = setup(c); plot(g, w, h, (t) => p.amp * Math.sin(TAU * p.cycles * t), css('--acc')); });
  });
  // 2. phase et somme de deux signaux
  document.querySelectorAll('.demo[data-demo="phase"]').forEach((root) => {
    const c = root.querySelector('canvas');
    bind(root, (p) => {
      const { g, w, h } = setup(c); const ph = (p.phase * Math.PI) / 180;
      plot(g, w, h, (t) => 0.5 * Math.sin(TAU * 2 * t), css('--wt'), 1.5, 0.42);
      plot(g, w, h, (t) => 0.5 * Math.sin(TAU * 2 * t + ph), css('--fm'), 1.5, 0.42);
      plot(g, w, h, (t) => 0.5 * Math.sin(TAU * 2 * t) + 0.5 * Math.sin(TAU * 2 * t + ph), css('--acc'), 2.5, 0.42);
    });
  });
  // 3. harmoniques : construction d'une dent de scie par addition de sinusoïdes (1/k)
  document.querySelectorAll('.demo[data-demo="harmonics"]').forEach((root) => {
    const c = root.querySelector('canvas'); const bars = root.querySelector('canvas.bars');
    bind(root, (p) => {
      const n = Math.round(p.n);
      const { g, w, h } = setup(c);
      plot(g, w, h, (t) => { let s = 0; for (let k = 1; k <= n; k++) s += ((k % 2 ? 1 : -1) * Math.sin(TAU * 2 * k * t)) / k; return s / 1.9; }, css('--acc'));
      const b = setup(bars); b.g.fillStyle = css('--wt');
      const bw = b.w / 34;
      for (let k = 1; k <= 32; k++) { const a = k <= n ? 1 / k : 0; b.g.fillRect(k * bw, b.h - a * (b.h - 10), bw * 0.7, a * (b.h - 10)); }
    });
  });
  // 4. enveloppe ADSR linéaire (même convention que SUBDESTRUCKT : temps = parcours de 0 à 1)
  document.querySelectorAll('.demo[data-demo="adsr"]').forEach((root) => {
    const c = root.querySelector('canvas');
    bind(root, (p) => {
      const { g, w, h } = setup(c);
      const gate = p.gate, total = gate + p.r + 0.2;
      const level = (t) => {
        let lv = 0;
        const dt = total / 600; let st = 'A';
        for (let x = 0; x <= t; x += dt) {
          if (x < gate) {
            if (st === 'A') { lv += dt / p.a; if (lv >= 1) { lv = 1; st = 'D'; } }
            else if (st === 'D') { lv -= dt / p.d; if (lv <= p.s) { lv = p.s; st = 'S'; } }
          } else { lv = Math.max(0, lv - dt / p.r); }
        }
        return lv;
      };
      const pts = [];
      for (let i = 0; i <= 300; i++) pts.push(level((i / 300) * total));
      g.strokeStyle = css('--acc'); g.lineWidth = 2.5; g.beginPath();
      pts.forEach((v, i) => { const x = (i / 300) * w, y = h - 10 - v * (h - 20); i ? g.lineTo(x, y) : g.moveTo(x, y); });
      g.stroke();
      g.setLineDash([4, 4]); g.strokeStyle = css('--dim'); g.beginPath(); const xg = (gate / total) * w; g.moveTo(xg, 0); g.lineTo(xg, h); g.stroke(); g.setLineDash([]);
      g.fillStyle = css('--dim'); g.font = '12px sans-serif'; g.fillText('note relâchée', Math.min(xg + 4, w - 90), 14);
    });
  });
  // 5. modulation de phase (PM) : forme d'onde et raies (fonctions de Bessel), simulation de la formule
  const besselJ = (n, x) => { let s = 0, t = 1; for (let k = 1; k <= n; k++) t *= x / 2 / k; for (let m = 0; m < 40; m++) { s += t; t *= -(x / 2) * (x / 2) / ((m + 1) * (m + 1 + n)); } return s; };
  document.querySelectorAll('.demo[data-demo="pm"]').forEach((root) => {
    const c = root.querySelector('canvas'); const bars = root.querySelector('canvas.bars');
    bind(root, (p) => {
      const { g, w, h } = setup(c);
      plot(g, w, h, (t) => Math.sin(TAU * 2 * t * p.rc + p.index * Math.sin(TAU * 2 * t * p.rm)), css('--fm'), 1.6, 0.42);
      const b = setup(bars); const fmax = 12; // en multiples de la note
      b.g.fillStyle = css('--fm');
      for (let k = -20; k <= 20; k++) {
        const f = Math.abs(p.rc + k * p.rm); if (f > fmax) continue;
        const a = Math.abs(besselJ(Math.abs(k), p.index)); const x = (f / fmax) * (b.w - 20) + 10;
        b.g.fillRect(x - 2, b.h - a * (b.h - 12), 4, a * (b.h - 12));
      }
      b.g.fillStyle = css('--dim'); b.g.font = '11px sans-serif';
      for (let f = 0; f <= fmax; f += 2) b.g.fillText(`${f}×`, (f / fmax) * (b.w - 20) + 4, b.h - 2);
    });
  });
})();
