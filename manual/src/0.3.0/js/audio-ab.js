// SUBDESTRUCKT — lecteurs d'exemples audio : lecture exclusive (un seul son à la fois dans tout le manuel), bascule A/B
// au même instant de lecture, aucune lecture automatique. Les fichiers sont des rendus du moteur réel (audio/examples.json).
(() => {
  let current = null;  // {audio, button}
  const stopCurrent = () => {
    if (!current) return;
    current.audio.pause();
    current.button.classList.remove('on');
    current.button.setAttribute('aria-pressed', 'false');
    current.button.querySelector('.state').textContent = '▶';
    current = null;
  };
  document.querySelectorAll('.ab').forEach((box) => {
    const buttons = Array.from(box.querySelectorAll('button.play'));
    buttons.forEach((btn) => {
      const audio = new Audio();
      audio.preload = 'none';  // aucun téléchargement avant le premier clic (données mobiles)
      audio.src = btn.dataset.src;
      btn._audio = audio;
      audio.addEventListener('ended', () => { if (current && current.audio === audio) stopCurrent(); });
      btn.addEventListener('click', () => {
        if (current && current.audio === audio) { stopCurrent(); return; }
        // bascule A/B : reprend au même instant si l'autre son de la même comparaison jouait
        let t = 0;
        if (current && buttons.includes(current.button)) t = current.audio.currentTime;
        stopCurrent();
        // fichier pas encore chargé (preload none) : positionner dès que la durée est connue
        const seek = () => { try { audio.currentTime = t; } catch (e) { /* ignoré */ } };
        if (audio.readyState >= 1) seek(); else if (t > 0) audio.addEventListener('loadedmetadata', seek, { once: true });
        const p = audio.play();
        if (p && p.catch) p.catch(() => {});
        btn.classList.add('on');
        btn.setAttribute('aria-pressed', 'true');
        btn.querySelector('.state').textContent = '❚❚';
        current = { audio, button: btn };
      });
    });
  });
  document.addEventListener('visibilitychange', () => { if (document.hidden) stopCurrent(); });
  window.SBD_AUDIO = { stop: stopCurrent, playing: () => (current ? current.button.dataset.src : null) };
})();
