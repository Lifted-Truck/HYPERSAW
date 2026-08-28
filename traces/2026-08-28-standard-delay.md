# Trace — the standard delay, and the three bugs its own oracle found

**Trigger** human 2026-08-28: *"Echo and room need feedback, and the feedback
from their lab didn't really work very well."* Built from
`docs/plans/2026-08-28-day-plan.md` §2.

## What changed

New `src/delay_core.h` (framework-free, no lab — its oracle is its spec) and
`tools/delay_check.cpp` (10 impulse-response invariants). Rack: FxType::Delay
= 9, heap-held cores, per-slot params via setDelayParam/getDelayParam,
snapTime on slot selection, tempo pushed per block. Shell: params 232-263
(four blocks of 8), kDelaySyncLabels, fxNtype range 8 -> 9. Presentation: 32
rows; the mutually-exclusive time controls gate on `fxNtype=9,dNsync=0|1`.

## Evidence

delay_check GREEN, exact numbers: spacing 4800/9600/14400; generation ratio
0.5000 and 0.5000 at feedback 0.5; one repeat at feedback 0 (energy 1.0000,
nothing after at 0.00e+00); bounded 1.0000 over 10 s at feedback 1.08;
ping-pong gen2 0.700 on the opposite channel with 0.000 on the origin; sync
6000 samples for 0.25 beat at 120 BPM; damp bypass peak 0.6000 vs 0.0632
damped. parity 156/156 (the slot defaults Off — parity-safe superset),
time/routing/state/preset/paramscope/mod GREEN, rtsafety GREEN,
`./verify fast` exit 0. In-browser: type select offers Delay; controls hidden
at Off, shown at Delay, Time swaps with Beats on the mode, slot 2 unaffected.

## What the oracle caught in its own author's work

1. The soft limiter shaped the DRY path (full-scale input read back 0.976).
2. Damping was always in circuit — the very complaint this module answers.
3. Nothing snapped the read head on load, so repeats landed late and pitched.
Plus two measurement traps: peak is not loop gain (fractional reads split an
impulse), and summed magnitude is not brightness (a one-pole preserves it).

## Open

Increment B (center-tap Echo regen) and C (Room RT60 remap) are designed in
the day plan and not yet built. delay_check wants a human decision to become
a ./verify gate. B68's listening A/B is the human's (test rows B68-3/4).
