# 2026-07-21 — Detune lab: harmonic spread law (unison → series)

Human, from the NI demo: with spread maxed, removing voices one at a time
"walked them down the harmonic series." That reveals the spread pattern — at
full spread the voices sit on the natural harmonic series (integer multiples of
the root), and spread is a morph from unison to that series. Not a detune law at
all.

Why this is the key insight:
- The lowest voice stays at the root because voice 1 is always 1·f0.
- The coherent/metallic character is intrinsic: a phase-relatable harmonic stack
  of saws IS a comb. The KS comb I built last pass was chasing an effect the
  geometry can just *be* — the harmonic spread needs no added resonator.
- "At least an octave" = voice 2 lands exactly on 2·f0 (the octave); the rest
  stack above.

## Change (audition lab only — docs/design/detune-lab.html)
- New detune law option 4 "harmonic (unison → series)". Voice `i`:
  `f = f0 · (1 + dep·i)`, dep = detune·spread. dep=0 → unison; dep=1 → f0·(1+i)
  = the series 1,2,3,…,N. Ignores distribution/anchor (voice index IS the
  harmonic). Root voice (i=0) fixed at f0.
- `detune` readout for law 4 shows the top harmonic reached: `H1–H{1+dep·(n−1)}`.

## Evidence
- Headless: at detune=1/spread=1/n=7 the voices land on f/f0 = 1,2,3,4,5,6,7
  exactly; at detune=0.5 → 1,1.5,2,2.5,3,3.5,4 (clean unison→series morph).
- **Defaults bit-identical** to HEAD (law defaults to 0; law-4 path unreached):
  fingerprint Δ = 0.
- Browser: readout `H1–H7.0` (full) / `H1–H4.0` (half); law option present; no
  console errors. `node --check` clean; `./verify fast` EXIT 0. Design-lab HTML
  — not in the build or the 9 oracle chains, no parity/core impact, no ADR.
- git: on branch lab-detune-octave-spread (PR #70)

## Next (hear-first, then possibly a LIBRARY lesson)
Human confirms by ear that harmonic law = the NI sound. If yes:
- It likely SUPERSEDES the stretch/comb approaches for this specific character
  (harmonic stack is coherent by construction). Stretch stays for the inharmonic
  bell/metal; harmonic is the coherent metal.
- Candidate variations to audition: odd-harmonics-only (square-ish, hollow),
  amplitude 1/n rolloff per partial (natural-series timbre), root-weight to lean
  on lower partials. Distribution selector could pick WHICH partials.
- Worth a knowledge-loop entry once confirmed: "the coherent supersaw 'spread'
  is a unison→harmonic-series morph, not a detune law" — held pending the ear
  check (write gate: verify before writing).
Winner folds into swarmsaw.html (reference) → port to swarm_core.h with parity.
