/*
 * gen_goldens.mjs — L0-1 golden generator (SAW core, Phase 1).
 *
 * Renders the parity scenario matrix on the JS reference (SwarmSynth,
 * extracted live from swarmsaw.html — see extract_core.mjs) and writes
 * Float32LE stereo-interleaved renders + a manifest to build-golden/.
 * Binaries are build artifacts, never committed (ROADMAP Phase 1); this
 * generator is the checked-in source of truth for what a golden IS.
 *
 * Modes:
 *   node gen_goldens.mjs              — write goldens + manifest
 *   node gen_goldens.mjs --selfcheck  — render every scenario twice and
 *                                       byte-compare (determinism gate for
 *                                       ./verify full; exits nonzero on drift)
 *
 * Scenario matrix: {3 seeds} x {param vectors covering each SAW subsystem}
 * per ACCEPTANCE L0-1. A3 = 220 Hz (midi 57), 4 s renders, 44.1 kHz,
 * note-off at 3 s so release tails are covered.
 */

import { mkdirSync, writeFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import { createHash } from 'node:crypto';
import { extractCore } from './extract_core.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = join(here, '..', '..');
const outDir = join(repo, 'build-golden');

const SR = 44100;
const SECONDS = 4;
const NOTE_OFF_AT = 3;
const BLOCK = 1024;
const MIDI = 57; // A3 = 220 Hz
const SEEDS = [1234, 777, 42];

// Param vectors covering each SAW subsystem (L0-1). Keys are SwarmSynth.p
// members; anything unspecified stays at the reference default.
const SCENARIOS = [
  { name: 'defaults',      p: {} },
  { name: 'sync-lock',     p: { K: 1.0 } },
  { name: 'sync-shimmer',  p: { K: 0.7 } },
  { name: 'splay-jp7',     p: { K: -1.0, dist: 1, n: 7 } },
  { name: 'inertia-hunt',  p: { K: 0.85, inertia: 0.5 } },
  { name: 'rtone-full',    p: { K: 1.0, rtone: 1.0 } },
  { name: 'scatter-drift', p: { retrig: 0, driftDepth: 0.5 } },
  { name: 'onset-dissolve',p: { K: 0.9, onset: 1.0, dissolve: 2.0 } },
  // Seeded distributions — these (with scatter-drift) are the scenarios where
  // the seed axis actually varies the render: JP/even + retrig + no drift are
  // seed-independent by design (SPEC Layer 1: seed governs Gaussian/Cauchy
  // draws, un-retriggered scatter, drift streams).
  { name: 'gauss-cloud',   p: { dist: 2, detune: 0.4, n: 16 } },
  { name: 'cauchy-cloud',  p: { dist: 3, detune: 0.3, n: 16, retrig: 0 } },
];

const mtof = (m) => 440 * Math.pow(2, (m - 69) / 12);

function renderScenario(SwarmSynth, seed, params) {
  const s = new SwarmSynth(SR);
  for (const [k, v] of Object.entries({ seed, ...params })) s.setParam(k, v);
  s.noteOn(MIDI, mtof(MIDI));
  const total = SECONDS * SR;
  const L = new Float32Array(BLOCK), R = new Float32Array(BLOCK);
  const out = new Float32Array(total * 2);
  let noteOffDone = false;
  for (let off = 0; off < total; off += BLOCK) {
    if (!noteOffDone && off >= NOTE_OFF_AT * SR) {
      s.noteOff(MIDI);
      noteOffDone = true;
    }
    s.render(L, R);
    const n = Math.min(BLOCK, total - off);
    for (let i = 0; i < n; i++) {
      out[(off + i) * 2] = L[i];
      out[(off + i) * 2 + 1] = R[i];
    }
  }
  return out;
}

function sha256(f32) {
  return createHash('sha256').update(Buffer.from(f32.buffer, f32.byteOffset, f32.byteLength)).digest('hex');
}

const selfcheck = process.argv.includes('--selfcheck');
const SwarmSynth = extractCore(join(repo, 'swarmsaw.html'), 'SwarmSynth');

const manifest = { generated_by: 'tools/golden/gen_goldens.mjs', reference: 'swarmsaw.html',
                   sr: SR, seconds: SECONDS, note: MIDI, note_off_at: NOTE_OFF_AT, entries: [] };
let failures = 0;

if (!selfcheck) mkdirSync(outDir, { recursive: true });

for (const scenario of SCENARIOS) {
  for (const seed of SEEDS) {
    const name = `${scenario.name}.seed${seed}`;
    const a = renderScenario(SwarmSynth, seed, scenario.p);
    const hashA = sha256(a);
    if (selfcheck) {
      const hashB = sha256(renderScenario(SwarmSynth, seed, scenario.p));
      const ok = hashA === hashB;
      if (!ok) failures++;
      console.log(`${ok ? 'OK ' : 'DRIFT'} ${name} ${hashA.slice(0, 12)}`);
    } else {
      const file = `${name}.f32`;
      writeFileSync(join(outDir, file), Buffer.from(a.buffer));
      let peak = 0, sum = 0;
      for (let i = 0; i < a.length; i++) { const v = Math.abs(a[i]); if (v > peak) peak = v; sum += a[i] * a[i]; }
      manifest.entries.push({ name, file, seed, params: scenario.p, sha256: hashA,
                              peak: +peak.toFixed(6), rms: +Math.sqrt(sum / a.length).toFixed(6) });
      console.log(`wrote ${file} peak=${peak.toFixed(3)}`);
    }
  }
}

if (selfcheck) {
  if (failures) { console.error(`gen_goldens: ${failures} scenario(s) NON-DETERMINISTIC`); process.exit(1); }
  console.log(`gen_goldens: ${SCENARIOS.length * SEEDS.length} scenarios deterministic`);
} else {
  writeFileSync(join(outDir, 'manifest.json'), JSON.stringify(manifest, null, 2));
  console.log(`manifest: ${manifest.entries.length} goldens -> build-golden/`);
}
