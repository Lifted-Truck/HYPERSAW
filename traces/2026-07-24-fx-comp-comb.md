# 2026-07-24 — FX rack increment 2: Comp + Comb slots (ADR-071)

Implements the day's rulings: comp/limiter and the polyphonic KS comb as rack
slot types 4/5 (E3), not core folds. First real cores past the ADR-054
placeholders. Off fresh main after #88.

## Changes
- src/fx_rack.h: FxType::Comp/Comb; ctor + setSampleRate (alloc main-thread,
  buffers sized to ~20 Hz at sr; empty-buffer div-by-zero guarded by ctor
  default alloc); noteOn/noteOff note feed; both process cases. Comp coeffs
  converted from the lab's per-sample 44.1 kHz values to SECONDS (ADR-009):
  atk 63.58 µs, rel 15.11 ms.
- src/hypersaw_clap.cpp: rack.noteOn at the common note-on (both engines);
  rack.setSampleRate in activate; labels + fx type ranges 0..3 → 0..5.
- DECISIONS.md: ADR-071.

## Architectural divergence, recorded honestly
Bus-side comb ≠ lab comb: lab feeds each swarm's comb that swarm's own audio;
the rack feeds every tuned line the whole bus (sympathetic strings). Per-note
tuning and polyphony preserved; per-voice isolation not. If A/B says isolation
matters, the comb moves core-side as its own decision.

## Evidence
- All-Off passthrough: parity 141/141 untouched; all chains + notefuzz GREEN
  (notefuzz drives the shell factory → new note feed fuzzed).
- Headless engaged-path probe (real CLAP plugin, 4-note chord):
  Comp peak 0.840 → **0.523**, rms 0.225 → 0.209 (mild, as designed);
  Comb rms 0.105 → 0.401 (single note, mix 0.9), tail → 0.000000 after
  note-off; NaN-clean, bounded.
  Probe first ran single-note and caught its own blind spot: A3 peaks at
  0.34 < the 0.4 knee, so Comp measured identical to Off — chord drive is
  what makes the evidence real.
- pluginval strictness 10 SUCCESS (incl. param fuzz over the new type range).
- ./verify fast EXIT 0 post-docs.
- git: on branch fx-ks-comb (off fresh main).

## Next
#17: only saw-shape retarget left (blocked on measured saw profiles).
#18 batched CLAP pass: law 0..5, dist 0..4, spread/anchor/pivotMode/
panLayout/panCurve/panInvert + tilt/hiTame/driftMode/keepPhase/freqGlide/
panMotion/panMode/motionCenter/harmReach/stretchB. rootWeight fold pending.
Rack later increments: per-slot param pages (comb res knob), real
filter/notch/time cores as slots, saturation engine, routing grid.
