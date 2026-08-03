# 2026-08-03 — ADR-077: ensemble onset timing (increment 1)

## Changes
- swarm_core.h: onsetScatter/onsetAlpha/attackScatter params; persistent tOff[]
  timing memory + seeded Box-Muller (gaussT); per-swarm onsD/onsE/onsC; initVoice
  steps the correction system and lays out entries (shifted so the EARLIEST voice
  starts at 0 — the note must not feel late, only internally spread); render holds
  un-entered voices (no phase advance) and fades each in on its own attack.
- hypersaw_clap.cpp: ids 91/92/93.
- gui.html: three rows in Drift, gated on onsetScatter > 0.
- waveshape_check.cpp: lag-1 structure gate.

## Evidence
alpha 0 / 0.25 / 1.0 / 1.5 -> lag-1 +0.985 / +0.679 / -0.072 / -0.550,
onset SD 202 / 35.8 / 26.4 / 31.6 ms. verify full green, parity 147/147.

## Not folded (increment 2)
Per-voice ADSR. The core's envelope is per-swarm; the lab's is per-voice. This
increment folds ENTRY only. Per-voice release/decay needs the envelope rework and
is deliberately deferred rather than half-done.
