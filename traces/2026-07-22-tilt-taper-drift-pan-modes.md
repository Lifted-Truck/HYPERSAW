# 2026-07-22 — Lab: tilt taper fix, drift modes, pan-motion modes

Human lab feedback: tone tilt jumped straight to trimming highs the instant it
left 0; drift wanted per-voice character + multiple motion modes (wiggle both
ways on the voice map); pan motion could use modes too (human has more ideas).
Plus a roadmap note reconsidering the saw shape control.

## Changes (audition lab only — docs/design/detune-lab.html)
- **Tilt taper fix:** the LP mapping jumped from bypass to a ~40-harmonic
  lowpass at the first nudge above 0. Now sweeps exponentially from near-bypass:
  LP `2·200^(1−|t|)` (~400→2 harmonics), HP `0.1·24^|t|` (~0.1→2.4× fundamental).
  Verified cutoff: off=bypass · t=0.02 → 21.6 kHz (near-bypass) · 0.5 → 6.2 kHz ·
  1.0 → 440 Hz. No jump.
- **Drift modes** (drift is already per-voice; added character): `drift mode`
  select — walk (1/f mean-reverting, the original) · sine (smooth per-voice LFO,
  decorrelated rates, sways both ways) · sample & hold (stepped random, hold
  shortens with rate). Per-voice `driftPh`/`driftHoldT` state, reset on note-on.
  Verified motion nonzero for all three (S&H steps 3→11 as rate rises); the
  voice-map ○/● wiggle in pitch.
- **Pan-motion modes:** `pan mode` select — drift (independent per-voice LFOs,
  the original) · sweep (one shared LFO auto-pans the whole image together).
  Shared `panSweepPh`. Verified: sweep shift-stdev across voices = 0
  (synchronized) vs drift 0.047 (independent).

## Evidence
- **STEREO defaults Δ = 0** vs HEAD: tilt default 0, driftDepth 0, panMotion 0,
  driftMode/panMode 0 → all inert (walk branch bit-identical to the original).
- Headless: tilt onset smooth; walk/sine/S&H all move; pan sweep synchronized.
- Browser (live): no console errors; driftMode/panMode present + wired.
- `node --check` clean; `./verify fast` EXIT 0. Design-lab HTML — no oracle/core
  impact, no ADR.
- git: on branch lab-detune-octave-spread (PR #70).

## Roadmap
- **Saw-shape direction** (Phase 2 detune-workshop bullet): real instrument
  should NOT ship the saw↔square morph (ADR-058) — saw is a design constraint;
  round-toward-glass OK for now. Shape control should morph through subtle
  sawtooth *variations* (analysed-from-synths + experimental far end), discrete
  sweep acceptable / interpolation ideal, plus a Saw Shape visualizer.
- QM file-hygiene note updated to reflect the relocation done in PR #71.

## Invited (not built — human has ideas)
Pan-motion modes beyond drift/sweep — human said "I have some ideas"; shipped a
minimal two-mode selector as scaffolding, awaiting their specifics.
