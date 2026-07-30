# 2026-07-29 — ADR-072: batched CLAP param pass (task #18, public-interface gate)

Human-ratified roster (2026-07-29): everything except `lpOut` (core-only oracle
switch), `toneTilt` rename approved, remap ruling delegated. Off fresh main
after #125 merged.

## Changes
- hypersaw_clap.cpp: `law` 0..3 → 0..5 (+harmonic/stretch labels), `dist`
  0..3 → 0..4 (+golden); ids **71..86** — toneTilt hiTame driftMode keepPhase
  freqGlide panMotion panMode motionCenter harmReach stretchB spread anchor
  pivotMode panLayout panCurve panInvert. Ranges are the AUDITIONED lab ranges.
- swarm_core.h: `toneTilt` alias → `p.tilt` (the only core edit; parity-neutral).
- tools/paramleak_probe.cpp: + reverse direction (new SAW ids vs SPECTRA engine).
- tools/paramfunc_smoke.cpp: NEW — every new id driven at an extreme must
  CHANGE the SAW output (stereo-aware; keepPhase via retrigger).

## The ghost id (why the block starts at 71)
Id 70 is intercepted BY NUMBER in applyParam/readParam (ADR-059 dev
inertia-taper exponent) without a row in kParams — so "table max + 1" is not
the next free id. Found because toneTilt landed on 70 first and its writes
were silently swallowed by the taper hook; the functional smoke caught it
(toneTilt DEAD, 15/16 ACT). Comment now lives at the table.

## State compatibility (the widening ruling)
state_save writes RAW values by key (`law=3`), so saved sessions are immune to
the range change by construction — no remap needed. Residual: VST3 normalized
AUTOMATION LANES on law/dist recorded pre-widening will re-scale (3/3 → 5);
accepted and recorded in the ADR — enum automation lanes are rare, sessions
are the thing to protect.

## Evidence
- paramfunc_smoke: **ALL 16 ACT** (+ law 4 / dist 4 alone act), vs a baseline
  with width 1.5 / detune 0.6; keepPhase exercised through a real retrigger.
- paramleak_probe both directions, controls firing: detune moves SAW
  (0.086957→0.090925), partials moves SPECTRA (0.074723→0.058160); id 45 at
  both extremes inert on SAW; all 16 new ids inert on SPECTRA.
- `./verify full` green — all nine chains, parity untouched (worst rms
  5.6e-12, time_check), state_check round-trips the new keys.
