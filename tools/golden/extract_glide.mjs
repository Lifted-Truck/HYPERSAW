// Slice bend-lab.html's travel-law model (Inertia + its two closed-form
// damping helpers) live out of the HTML, so the lab stays the single source of
// truth and there is no forked copy to drift. Same discipline as
// extract_force.mjs.
//
// Slice: from `const TAU` (the constants Inertia closes over) to the end of
// `class Inertia` — located by the line that closes it, found by brace depth
// rather than a magic line number, so an edit inside the class cannot silently
// truncate the slice.
import { readFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = join(dirname(fileURLToPath(import.meta.url)), '..', '..');

export function extractInertia() {
  const html = readFileSync(join(root, 'docs/design/bend-lab.html'), 'utf8');
  const lines = html.split('\n');
  const start = lines.findIndex(l => l.startsWith('const TAU'));
  const clsAt = lines.findIndex(l => l.startsWith('class Inertia'));
  if (start < 0 || clsAt < 0) throw new Error('bend-lab shape changed: TAU / class Inertia not found');
  let depth = 0, end = -1;
  for (let i = clsAt; i < lines.length; i++) {
    for (const ch of lines[i]) { if (ch === '{') depth++; else if (ch === '}') depth--; }
    if (depth === 0 && i > clsAt) { end = i; break; }
  }
  if (end < 0) throw new Error('class Inertia never closes');
  // keep only what Inertia needs: constants, the damping helpers, the class
  const keep = [];
  for (let i = start; i <= end; i++) {
    const l = lines[i];
    if (i >= clsAt || l.startsWith('const TAU') || l.startsWith('const osFromZeta') ||
        l.startsWith('const zetaFromOs') || (i > 156 && i < clsAt && /^\s/.test(l)))
      keep.push(l);
  }
  const src = keep.join('\n');
  const api = new Function(src + '\n;return {Inertia, TAU, TICK};')();
  if (typeof api.Inertia !== 'function') throw new Error('Inertia did not evaluate');
  return api;
}

// The authored defaults, read from the source literal rather than a live object
// (the lab's setup zeroes P when evaluated without a DOM — see glide_roundup).
export function defaults() {
  const html = readFileSync(join(root, 'docs/design/bend-lab.html'), 'utf8');
  const m = html.match(/const P = \{([\s\S]*?)\};/);
  if (!m) throw new Error('P literal not found');
  const P = new Function('return ({' + m[1].replace(/\/\/[^\n]*/g, '') + '})')();
  for (const k of ['gtime', 'rate', 'tau', 'springF', 'damp'])
    if (!(P[k] > 0)) throw new Error(`P.${k} did not survive parsing (${P[k]})`);
  return P;
}
