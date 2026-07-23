# 2026-07-22 — Saw-profile morph + visualizer; voice-map jump fix; K-pivot explained

Human: proceed with the saw-variation prototype (+ sweet-spot test). Also two lab
observations — a voice "stuck" vs K, and voice-map dots jumping on every note-on.

## Bug triage (diagnosed against the real DSP)
- **Voice-map jump on note-on = REAL BUG, fixed.** A freshly-allocated swarm has
  `eff = [0,0,0,0,0]` until its first control-tick; `drawVoiceMap` reads it that
  frame, so the dots flash to the bottom then snap up. Fix: `noteOn` now calls
  `controlTick(s)` once to prime eff/vfTarget/vgain. Verified eff correct on frame
  1; a no-op on the audio at default K/onset/drift (fingerprint Δ=0).
- **"Stuck voice" vs K = NOT a bug (physics).** The stuck voice is `centerIdx` —
  the zero-detune center voice sitting exactly at the swarm mean. Coupling
  collapses the *other* voices toward that mean as |K| rises, so the pivot has
  nowhere to go. Measured: voice 3 (centerIdx) swing 0.011 Hz across K=−0.9…+0.9
  vs 1–6 Hz for the others. Explained to the human, no change.

## Saw-profile prototype (audition lab — docs/design/detune-lab.html)
- Replaced the 3-way `shapeTgt` select with a continuous **`saw profile`** slider
  (0…1) sweeping a 5-anchor bank the raw saw morphs TOWARD:
  glass (parabola) → soft → hollow (triangle) → pure (sine) → reedy (+3rd). New
  `sawShape(k,ph)` method; render crossfades the two adjacent anchors. `round`
  (depth) and `round × hi` (pitch-weight) unchanged.
- **Saw Shape visualizer** (`sawCanvas`): live one-cycle waveform of the current
  shape (raw saw morphed toward the profile by roundness), drawn each frame,
  mirroring the DSP bank — the roadmap's Saw Shape visualizer, prototyped.
- **Sweet-spot test** (measured, glass profile, 7-voice harmonic stack, HF share
  of energy): raw saw 13% · round 0.3 uniform 11.7% · **round 0.3 + round×hi 0.7
  = 10.3%** (keeps ~80% of the raw bite while glassing the top voices) · round 0.6
  = 7.7% (over-rounded/dull). Sweet spot noted in the UI hint.

## Evidence
- **defaults Δ = 0** (round=0 → shaping skipped; prime a no-op at default K/onset/
  drift). Profile sweep, sweet-spot brightness, and the visualizer all verified
  headless + in-browser (no console errors; sawCanvas draws; profile names
  glass/hollow/reedy). `node --check` clean; `./verify fast` EXIT 0. No oracle/core
  impact, no ADR.
- git: on branch lab-detune-octave-spread (PR #70).

## Next
Human auditions the sweet spot by ear. When the saw profiles settle, the real
profiles should be *measured from actual synth saws* (the roadmap direction) —
the current bank is a plausible placeholder set. Folds into swarmsaw.html →
swarm_core.h with parity, retargeting ADR-058's square morph.
