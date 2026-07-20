# 2026-07-20 — Bipolar onset lock (ADR-056)

Human request: make onset lock bipolar (negative = an initial splay burst instead
of sync), asking whether `dissolve` still returns it to neutral. Approved direct
implementation ("throw it in, we can revert").

## Changes
- `src/swarm_core.h`: `Kenv = 8·onset·fabs(onset)` (signed); route by sign —
  `syncT += max(0,Kenv)`, `splayT += max(0,−Kenv)·3`.
- `src/hypersaw_clap.cpp` + `src/gui/gui.html`: onset range 0..1 → −1..1.
- `tools/trajectory_check.cpp`: L0-24 anchor (ADR-056 block).
- `ACCEPTANCE.md` L0-24; `DECISIONS.md` ADR-056.

## Evidence
- Superset is bit-inert for onset ≥ 0 → `./verify full` EXIT 0, parity 54/54,
  all 9 chains GREEN (build clean).
- L0-24 anchor GREEN: splay-onset early R < sync-onset early R (Δ=0.92);
  dissolve neutralizes at K=0.9 (late |ΔR|=0.003); NaN-clean.
- The anchor SURFACED a real nuance (recorded in ADR-056 / L0-24): dissolve
  returns the coupling boost to neutral (Kenv→0), but near-critical K the
  splay-vs-sync start can settle into different basins even after Kenv→0
  (path-dependence, not a bug) — so convergence is checked only at strong K.
- git: 1faa892 on branch feat-bipolar-onset

## Open / revert
- ×3 splay-onset gain is a first guess (perceptual symmetry); tune by ear.
- Revert = restore `8·onset²`, the original sync/splay lines, and the 0..1 range.
