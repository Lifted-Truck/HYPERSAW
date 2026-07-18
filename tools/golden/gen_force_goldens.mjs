// Force-core golden generator (Track E0): run the labs' population dynamics
// headlessly and dump trajectory checkpoints for tools/force_check.cpp.
//
// Protocol (probed 2026-07-18, L0002 discipline — see the E0 trace):
//  - driftDepth = 0 for every stat-bearing scenario. The recorded reference
//    numbers (σ 0.755, CV 0.123/0.009/0.055, equilibrium ratios 0.238/0.217)
//    reproduce EXACTLY at drift 0 and do not at TimeLab's default drift 30 —
//    the acceptance values encode a drift-off measurement protocol.
//  - params applied via setParam, 'seed' LAST: its rebuild+reset makes the
//    pre-run state a pure function of the final param values (the C++ side
//    mirrors that state directly instead of replaying the call sequence).
//  - 8 s = 22050 ticks at 44.1k/16; checkpoints at ticks 1/10/100/1000/22050.
//  - population is Float64Array end to end: no f32 rounding anywhere, so the
//    C++ comparison tolerance is transcendental-ulp-level (1e-9), not audio ε.
//
// Usage: node gen_force_goldens.mjs             — write build-golden/force/
//        node gen_force_goldens.mjs --selfcheck — render twice, compare
//        (determinism proof; same semantics as gen_goldens.mjs)

import { FilterLab, PhaserLab, TimeLab } from './extract_force.mjs';
import { writeFileSync, mkdirSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const SR = 44100, TICKS = 22050;  // 8 s
const CHECKPOINTS = new Set([1, 10, 100, 1000, TICKS]);
// Generated artifacts live with the audio goldens (gitignored build-*/):
const outDir = join(dirname(fileURLToPath(import.meta.url)), '..', '..', 'build-golden', 'force');

// Scenario params are COMPLETE for the population path (every knob the
// force system reads is stated, nothing inherited silently).
const BASE_F = { nb: 16, place: 0, dist: 0, center: 9.64, spread: 0.5, K: 0, inertia: 0, driftDepth: 0, driftRate: 0.4, grav: 0, basin: 45, seed: 1234 };
const BASE_P = { nb: 6, place: 0, dist: 0, center: 9.64, spread: 0.5, K: 0, inertia: 0, driftDepth: 0, driftRate: 0.4, grav: 0, basin: 45, seed: 1234 };
const BASE_TE = { mode: 0, nb: 8, dist: 1, size: 0.55, spread: 0.6, K: 0, inertia: 0, driftDepth: 0, driftRate: 0.3, grav: 0, basin: 50, bpm: 120, seed: 1234 };

export const SCENARIOS = [
  // FilterLab — L0-14/15 anchors and criteria
  { name: 'filter-free', Lab: FilterLab, p: { ...BASE_F } },
  { name: 'filter-sync', Lab: FilterLab, p: { ...BASE_F, K: 1 } },
  { name: 'filter-splay', Lab: FilterLab, p: { ...BASE_F, K: -1 } },
  { name: 'filter-grav', Lab: FilterLab, p: { ...BASE_F, grav: 0.8 } },
  { name: 'filter-antigrav', Lab: FilterLab, p: { ...BASE_F, grav: -0.8 } },
  // RNG + drift walks, seed-for-seed (trajectory-only; no stat criteria)
  { name: 'filter-gauss-drift', Lab: FilterLab, p: { ...BASE_F, dist: 1, driftDepth: 40, K: 1 } },
  { name: 'filter-cauchy-777', Lab: FilterLab, p: { ...BASE_F, dist: 2, seed: 777, driftDepth: 40, K: -1 } },
  // Inertia (distinct w0 constants per profile; trajectory-only)
  { name: 'filter-inertia', Lab: FilterLab, p: { ...BASE_F, K: 1, inertia: 0.7 } },
  // PhaserLab — L0-17 shared-core check
  { name: 'phaser-sync', Lab: PhaserLab, p: { ...BASE_P, K: 1 } },
  { name: 'phaser-splay', Lab: PhaserLab, p: { ...BASE_P, K: -1 } },
  // TimeLab echo — L0-19
  { name: 'echo-free', Lab: TimeLab, p: { ...BASE_TE } },
  { name: 'echo-sync', Lab: TimeLab, p: { ...BASE_TE, K: 1 } },
  { name: 'echo-splay', Lab: TimeLab, p: { ...BASE_TE, K: -1 } },
  { name: 'echo-grav', Lab: TimeLab, p: { ...BASE_TE, grav: 0.8 } },
  { name: 'echo-inertia', Lab: TimeLab, p: { ...BASE_TE, K: 1, inertia: 0.7 } },
  { name: 'echo-drift-42', Lab: TimeLab, p: { ...BASE_TE, seed: 42, driftDepth: 60, K: 0.5 } },
  // TimeLab room — L0-20 sympathetic
  { name: 'room-symp', Lab: TimeLab, p: { ...BASE_TE, mode: 1, grav: 0.9, basin: 80 } },
];

function renderGolden(sc) {
  const lab = new sc.Lab(SR);
  for (const [k, v] of Object.entries(sc.p)) if (k !== 'seed') lab.setParam(k, v);
  lab.setParam('seed', sc.p.seed);
  const n = sc.p.nb;
  const lines = [];
  lines.push(`# ${sc.name} ticks=${TICKS} n=${n}`);
  lines.push('# ' + Object.entries(sc.p).map(([k, v]) => `${k}=${v}`).join(' '));
  const dump = t => {
    lines.push(`tick ${t}`);
    lines.push('v ' + Array.from(lab.v.slice(0, n)).map(x => x.toPrecision(17)).join(' '));
    lines.push(`m collapse=${lab.collapse.toPrecision(17)} combReg=${lab.combReg.toPrecision(17)}`);
  };
  lines.push('tick 0');
  lines.push('v ' + Array.from(lab.v.slice(0, n)).map(x => x.toPrecision(17)).join(' '));
  for (let t = 1; t <= TICKS; t++) {
    lab.controlTick();
    if (CHECKPOINTS.has(t)) dump(t);
  }
  return lines.join('\n') + '\n';
}

const selfcheck = process.argv.includes('--selfcheck');
if (!selfcheck) mkdirSync(outDir, { recursive: true });
let fail = 0;
for (const sc of SCENARIOS) {
  const text = renderGolden(sc);
  if (selfcheck) {
    const ok = renderGolden(sc) === text;
    console.log(`${ok ? 'OK  ' : 'FAIL'} ${sc.name}`);
    if (!ok) fail++;
  } else {
    writeFileSync(join(outDir, `${sc.name}.golden`), text);
    console.log(`wrote ${sc.name}.golden`);
  }
}
if (selfcheck) {
  console.log(fail ? `selfcheck: ${fail} NON-DETERMINISTIC` : 'selfcheck: renders are deterministic');
  process.exit(fail ? 1 : 0);
}
