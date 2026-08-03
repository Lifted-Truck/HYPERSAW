# 2026-08-02 — ADR-074: super-width 3-mode fold (F/A/D)

Ship list ratified by the human off the width-lab characterization; C/E retired as
subsumed. Off fresh main after #162.

## Changes
- swarm_core.h: `superMode` param (+rebuild trigger); mode F = fan-seat steepening
  (exponent folded into gamma) + per-voice far-channel ITD rings (kItdRing 256,
  itdW head advanced once per sample); post-mix stage now mode-gated — A verbatim
  ADR-025 M/S, D allpass-side (apZ state, 700 Hz); everything inert at width <= 1.
- hypersaw_clap.cpp: id 87 `superMode` stepped 0..2 + labels.
- gui.html: super-width select in Output & perception, soft-gated to width > 1,
  added to SAW_ONLY.
- waveshape_check.cpp: three ADR-074 gates (F clean / A,D pinned-cliffing).

## Evidence
- verify full green post-fold; parity 147/147, worst rms 4.262e-09 (pre-existing).
- Gates: F 0 cliffs · A 1,867 · D 14,300 at width 1.5 on the parity-recipe patch.
- paramleak: all SPECTRA rows still inert; GUI blocks parse.

## Rulings embedded
- Default superMode 0 (F) = deliberate default-output change at width > 1.
- ITD max fixed at the 0.6 ms audition value; D's corner fixed 700 Hz — trims later
  if wanted, no id churn.
