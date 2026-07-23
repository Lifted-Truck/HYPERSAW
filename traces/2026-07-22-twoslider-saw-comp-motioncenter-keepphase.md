# 2026-07-22 — Two-slider saw shape, master comp/limiter, centre-pin motion, keep-phase

Human batch (all lab): (1) two-slider shape — top-level saw shape + roundness
shape independently; (2) mild compression + optional limiter (chords clip);
(3) pin the centre voice, scale motion by distance from centre; (4) a keep-phase
retrigger toggle (continue from the last phases).

## Changes (audition lab only — docs/design/detune-lab.html)
- **Two-slider shape:** added `saw base` (top-level saw bank: classic → curved →
  fat → driven → aggressive; `sawBaseVar()`); the existing shape select became
  `roundness →` (`sawProfile`, the roundness-target bank). base crossfades from
  the BLEP saw (anchor 0), then roundness morphs toward its target. **Base
  contents are PLACEHOLDER** — real profiles must be measured from synth saws
  (ROADMAP). Saw Shape visualizer now renders base + roundness.
- **Master comp + limiter:** `compress` (0…1) mild peak compressor (thr 0.4,
  ratio→5:1, fast atk / slow rel) before the soft clip; `limiter` toggle =
  brickwall at −0.2 dB. Verified: loud 8-note chord output peak 1.0 → 0.909
  (comp) → 0.753 (limiter).
- **Centre pin** (`motionCenter` 0…1): scales drift + pan motion by each voice's
  distance from the fundamental (`cdist = |vf−f0|/max`). At 1 the fundamental is
  pinned (drift swing 0 Hz) and the edge voice keeps full swing. Applied to drift
  (per-voice) and pan motion (via focus cdist). One-tick cdist lag (negligible).
- **keep phase** toggle: `noteOn` continues from `lastPhase` (a render-time
  snapshot of the focus swarm) instead of reset/random — a third phase mode.

## Evidence
- **defaults Δ = 0**: sawBase 0 (BLEP), comp 0, limiter off, motionCenter 0
  (mw=1, bit-identical drift), keepPhase off → all inert.
- Headless: comp/limiter tame the chord peak; centre pin pins the fundamental
  (0 Hz) vs edge (10 Hz); keep-phase resumes the snapshot; base morph changes the
  waveform. Browser: no console errors; all 6 controls present + wired.
- `node --check` clean; `./verify fast` EXIT 0. No oracle/core impact, no ADR.
- git: on branch lab-detune-octave-spread (PR #70).

## Roadmap
Saw-shape note updated: the two-slider design (base + roundness) is prototyped;
the base-saw bank contents are placeholder pending measured synth-saw profiles.
