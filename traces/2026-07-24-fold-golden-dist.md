# 2026-07-24 — Eighth fold: golden (irrational) distribution, dist 4 (ADR-067)

Last cheap detune-geometry fold. `x = 2·(((i+1)·φ⁻¹) mod 1) − 1` —
low-discrepancy irrational placement, even-but-inharmonic; structured like
even/JP, non-repeating like the seeded dists.
Off fresh main after the branch-cleanup round (repo now main-only — the
L0004 pre-push reflex is moot on a fresh branch, but checked anyway).

## Changes (reference-first, ADR-003)
- swarmsaw.html: cauchy (the old trailing `else`) made explicit
  `else if (dist === 3)`; golden is the new `else`. No menu entry, per the
  standing convention (engine code is the spec; tempo-grid precedent).
- swarm_core.h: mirrored; `std::fmod` == JS `%` for positive operands.
- gen_goldens.mjs: golden-dist (wide, 11 voices), golden-sync (K 0.8).
- DECISIONS.md: ADR-067.

## Evidence
- Inertness proven by manifest diff, not assertion: goldens generated from the
  edited reference AND origin/main's reference — all 102 pre-existing renders
  sha256-identical; only the 6 new files added.
- Golden draws NO rngG values — safe because grng re-seeds at every rebuild
  top, so a zero-draw dist can't desync the seeded dists.
- ./verify full EXIT 0: parity 102/102 → **108/108**, the 6 new at
  **rms 0.000e+00**; all chains + notefuzz GREEN.
- Coupled scenario (K 0.8) is inside the parity domain (ADR-065 limit): same
  regime as the long-standing sync goldens, renders exactly.
- git: on branch fold-golden-dist (off fresh main).

## Next
Task #15: octave spread + root-anchor — `dep = detune·spread` and
`x − anchor·xmin` thread through EVERY law; rewrites the placement block, so
the inertness proof (spread=1, anchor=0 bit-exact vs origin/main) is the
load-bearing step. Then the comp/limiter decision (#16), divergence ADRs
(#17), batched CLAP pass (#18: law 0..3→0..5, dist 0..3→0..4, new params).
