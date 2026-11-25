// LABTECHNO — moteur de questionnaire générique, piloté par le manifeste de campagne.
// Types : "single" (choix unique, rendu en boutons segmentés accessibles) et "text" (ligne ou zone de texte).

import { el } from './dom.js';

let uid = 0;

export function renderQuestions(questions, values, { disabled = false, onChange } = {}) {
  const root = el('div', { class: 'questions' });
  for (const q of questions) {
    const name = `q${++uid}_${q.id}`;
    if (q.type === 'single') {
      const fs = el('fieldset', { class: 'q', 'data-q': q.id });
      fs.append(el('legend', {}, q.label, q.required ? null : el('span', { class: 'opt' }, ' (facultatif)')));
      const seg = el('div', { class: 'seg', role: 'radiogroup' });
      for (const o of q.options) {
        const input = el('input', { type: 'radio', name, value: o.value });
        input.checked = values[q.id] === o.value;
        input.disabled = disabled;
        input.addEventListener('change', () => { if (input.checked && onChange) onChange(q.id, o.value); });
        seg.append(el('label', { class: 'seg-opt' }, input, el('span', {}, o.label)));
      }
      fs.append(seg);
      root.append(fs);
    } else if (q.type === 'text') {
      const id = `${name}_t`;
      const wrap = el('div', { class: 'q q-text', 'data-q': q.id });
      wrap.append(el('label', { for: id }, q.label, q.required ? null : el('span', { class: 'opt' }, ' (facultatif)')));
      const input = q.multiline === false
        ? el('input', { type: 'text', id, placeholder: q.placeholder || '', autocomplete: 'off' })
        : el('textarea', { id, rows: '4', placeholder: q.placeholder || '' });
      input.value = values[q.id] || '';
      input.disabled = disabled;
      input.addEventListener('input', () => { if (onChange) onChange(q.id, input.value); });
      wrap.append(input);
      root.append(wrap);
    }
  }
  return root;
}

export function answerLabel(q, v) {
  if (q.type === 'text') return v && v.trim() ? v.trim() : '—';
  const o = (q.options || []).find((x) => x.value === v);
  return o ? o.label : '—';
}
