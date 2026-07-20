# 2026-07-20 — SAW waveshape morph (ADR-058) + roadmap notes

Human request: a knob that reshapes the saw waves in real time ("other waves but
not sine"), short of a full wavetable, to hear non-saw waves in the swarm.

## Change
- `src/swarm_core.h`: `shape` param (0=saw, 1=square). Osc morph
  `v = saw(ph) − shape·saw(ph+½)` — both polyBLEP-corrected → band-limited.
  Guarded `if (shape>0)` so shape 0 is bit-exact.
- `src/hypersaw_clap.cpp`: param id 69 "Saw Shape" (SAW-core key, fallback-routed).
- `src/gui/gui.html`: slider in "The swarm", SAW_ONLY + DEFAULTS.
- `tools/trajectory_check.cpp`: ADR-058 bounded/NaN-clean guard (shape 0.5 & 1.0).
- ROADMAP: swarmalator spatial-blend-slider idea (0=current/1=full spatial, SAW+SPECTRA,
  spread-interaction to resolve, hear swarmalator first); SPECTRA new-params forward note.

## Evidence
- shape 0 bit-inert → `./verify full` EXIT 0, parity 54/54, all 9 chains GREEN, build clean.
- ADR-058 anchor GREEN (bounded/NaN-clean at shape 0.5 & 1.0).
- square-ness is provable (saw−saw(ph+½)) + confirmed by ear; no numeric shape assertion.
- git: 3d5d6b0 on branch feat-saw-waveshape

## Follow-ups (ADR-058)
- pulse-WIDTH knob (offset ≠ ½ → PWM family); triangle (integrate square). Cheap
  extensions of the same two-saw machinery.
