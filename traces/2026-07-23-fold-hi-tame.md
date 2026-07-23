# 2026-07-23 — Second lab→core fold: hi-tame (ADR-061)

Continuing the fold (batch-DSP-then-params cadence). Hi-tame = per-voice
equal-loudness roll-off, gain (f0/f)^hiTame; inert at 0. Off fresh main after
#73 (tone tilt) merged; branch created this turn (gh pr view #73 = MERGED first —
L0004 reflex).

## Changes
- swarmsaw.html (reference, protected — human-authorized): `hiTame:0` param;
  per-voice `hg` gain array (init 1); compute after the tilt coeff loop
  (`hg[i] = pow(f0/max(f0,vf[i]), hiTame)`, guarded on hiTame>0); apply
  `v *= hg[i]` after the tilt in the render voice loop.
- swarm_core.h: mirrored bit-for-bit (param, Swarm.hg, ctor init, controlTick
  compute, render application, paramSlot "hiTame"). Not stateful → no note-on reset.
- gen_goldens.mjs: scenario hi-tame (hiTame 1.0, detune 0.6, n 12).
- DECISIONS.md: ADR-061.

## Evidence
- Inertness proven first: SwarmSynth from origin/main's swarmsaw.html vs edited →
  defaults / tilt-dark / gauss-cloud BIT-IDENTICAL at hiTame=0; hiTame=1 differs.
- ./verify full EXIT 0: parity_check 60/60 → **63/63** within ε; the 3 new hi-tame
  goldens at **rms 0.000e+00**; all 9 chains + notefuzz GREEN.
- git: on branch fold-hi-tame (off fresh main).

## Next
Continue supersets (freq smoothing / keep-phase / drift modes / centre-pin), then
the new detune laws, then the divergences. CLAP params batched at the end.
