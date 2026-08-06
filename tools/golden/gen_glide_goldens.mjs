// Glide travel-law golden generator (A1 fold): render bend-lab's Inertia model
// to raw f32 trajectories for tools/glide_check.cpp parity. The lab HTML stays
// the single source of truth — the model is sliced live, never forked.
//
// A trajectory, not audio: the travel laws operate at CONTROL rate on a pitch
// offset in semitones. One fixed gesture per scenario (step up, hold, release,
// then a reversal) so the laws are exercised in both directions and through
// the return-multiplier asymmetry.
//
// Usage: node gen_glide_goldens.mjs [--selfcheck]
import { extractInertia, defaults } from './extract_glide.mjs';
import { writeFileSync, mkdirSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = join(dirname(fileURLToPath(import.meta.url)), '..', '..');
const { Inertia } = extractInertia();
const P = defaults();
const CR = 44100 / 16;            // the 16-sample control tick
const TICKS = Math.round(2.4 * CR);

// the gesture every scenario runs, in semitones
function targetAt(i) {
  const t = i / CR;
  if (t < 0.05) return 0;
  if (t < 0.65) return 2;         // step up, hold
  if (t < 1.10) return 0;         // release (exercises retMul on the bend lane)
  if (t < 1.70) return -1.5;      // reverse through zero
  return 0.5;                     // settle somewhere off-grid (quantise bites)
}

export const SCENARIOS = [
  { name: 'glide-const-time',   p: { model: 1 } },
  { name: 'glide-const-rate',   p: { model: 2 } },
  { name: 'glide-lag',          p: { model: 3 } },
  { name: 'glide-spring',       p: { model: 4 } },
  { name: 'glide-spring-ring',  p: { model: 4, damp: 0.2 } },
  { name: 'glide-spring-dist2', p: { model: 4, distOver: 2 } },
  { name: 'glide-retmul',       p: { model: 1, retMul: 0.35 } },
  { name: 'glide-quant-chrom',  p: { model: 2, quant: 1 } },
  { name: 'glide-quant-scale',  p: { model: 3, quant: 2 } },
  { name: 'glide-quant-hyst',   p: { model: 4, quant: 1, qhyst: 25 } },
  { name: 'glide-note-lane',    p: { model: 4, lane: 'note' } },
];

function render(sc) {
  const p = { ...P, ...sc.p };
  const inr = new Inertia(CR, sc.p.lane === 'note' ? 'note' : 'bend');
  inr.reset(0);
  const out = new Float32Array(TICKS);
  for (let i = 0; i < TICKS; i++) out[i] = inr.step(targetAt(i), p);
  return out;
}

const outDir = join(root, 'build-golden', 'glide');
const selfcheck = process.argv.includes('--selfcheck');
if (!selfcheck) mkdirSync(outDir, { recursive: true });
let fail = 0;
const manifest = [];
for (const sc of SCENARIOS) {
  const a = render(sc);
  if (selfcheck) {
    const b = render(sc);
    let same = a.length === b.length;
    if (same) for (let i = 0; i < a.length; i++) if (a[i] !== b[i]) { same = false; break; }
    console.log(`${same ? 'OK  ' : 'FAIL'} ${sc.name}`);
    if (!same) fail++;
  } else {
    writeFileSync(join(outDir, `${sc.name}.f32`), Buffer.from(a.buffer));
    manifest.push([sc.name, Object.entries(sc.p).map(([k, v]) => `${k}=${v}`).join(' ')].join('\t'));
    console.log(`wrote ${sc.name}.f32`);
  }
}
if (selfcheck) {
  console.log(fail ? `selfcheck: ${fail} NON-DETERMINISTIC` : 'selfcheck: renders are deterministic');
  process.exit(fail ? 1 : 0);
}
writeFileSync(join(outDir, 'glide-manifest.tsv'), manifest.join('\n') + '\n');
