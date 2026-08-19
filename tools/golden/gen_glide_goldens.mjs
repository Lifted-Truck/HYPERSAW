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
/* TIE-LANDING GESTURE (added 2026-08-19 with the tie-break fix). The standing
   gesture settles at 0.5, which in C major is NOT equidistant from anything — so
   the entire tie path was untested code and both defects lived there unseen:
   ties resolving downward by loop order, and chromatic disagreeing between
   Math.round and std::lround on negatives. `L0031`: a reference oracle certifies
   agreement only over the surface the reference RENDERS, and this surface was
   never rendered.
   Lands on exact ties of both signs: +1 (C-major tie between 0 and 2) and -1.5
   (a chromatic tie, and the sign where the two rounding functions disagreed). */
function tieTargetAt(i) {
  const t = i / CR;
  if (t < 0.10) return 0;
  if (t < 0.60) return 1;        // scale tie: equidistant from 0 and 2 in C major
  if (t < 1.10) return -1.5;     // chromatic tie, negative: lround vs Math.round
  if (t < 1.60) return 2.5;      // chromatic tie, positive
  return 1;                      // settle ON the tie, so the held value is the tie
}

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
  // Ties, both modes and both signs, with hysteresis OFF so the tie-break itself
  // is what is measured rather than the stickiness that usually masks it.
  // model 0 (OFF) IS LOAD-BEARING HERE, not a lazy choice. Under any moving law
  // the output approaches its target asymptotically and never lands exactly on a
  // midpoint, so the tie path is unreachable and a scenario using one is a test
  // that cannot fail — the first version of these two used model 1 and a planted
  // regression sailed straight through them. With the law off `x = target`
  // bit-exactly, which is also the only way a real patch reaches a tie.
  { name: 'glide-tie-scale',    p: { model: 0, quant: 2, qhyst: 0 }, gesture: 'tie' },
  { name: 'glide-tie-chrom',    p: { model: 0, quant: 1, qhyst: 0 }, gesture: 'tie' },
  // The scale MASK is now user-drawable (bend-lab's hzScalePicker), so the
  // quantiser must be proven over more than the one hardcoded C-major set it
  // shipped with. These three cover what the picker can actually produce:
  // a non-zero ROOT (exercises the ((c-root)%12+12)%12 wrap in both
  // references), a WIDE-GAP set where the nearest degree is often >1 semitone
  // away and ties at exactly 0.5 are reachable, and a SPARSE rooted set that
  // combines both. Without these, every scale but C major was untested code.
  { name: 'glide-quant-root3',  p: { model: 3, quant: 2, scaleRoot: 3,
        scaleMask: [1,0,0,1,0,1,0,1,0,0,1,0] } },                 // minor pentatonic on D#
  { name: 'glide-quant-whole',  p: { model: 2, quant: 2,
        scaleMask: [1,0,1,0,1,0,1,0,1,0,1,0] } },                 // whole tone, root C
  { name: 'glide-quant-sparse', p: { model: 4, quant: 2, scaleRoot: 7, qhyst: 20,
        scaleMask: [1,0,1,1,0,0,0,1,1,0,0,0] } },                 // hirajoshi on G
  { name: 'glide-note-lane',    p: { model: 4, lane: 'note' } },
];

function render(sc) {
  const p = { ...P, ...sc.p };
  const inr = new Inertia(CR, sc.p.lane === 'note' ? 'note' : 'bend');
  inr.reset(0);
  const out = new Float32Array(TICKS);
  // A scenario may pick the tie-landing gesture instead of the standard one.
  const gest = sc.gesture === 'tie' ? tieTargetAt : targetAt;
  for (let i = 0; i < TICKS; i++) out[i] = inr.step(gest(i), p);
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
