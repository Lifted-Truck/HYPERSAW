# Trace — the corner-copy bug was a workaround that outlived its defect

**Trigger** human 2026-08-27: *"if I edit the on/off of an oscillator in one
corner, it seems to copy that setting to every other corner."* Filed as
"probably unrelated" to the FX work; it was unrelated, and it was real.

## Reproduced before diagnosing

Author corner A osc2 ON, corners B/C/D OFF. Then, **unarmed**, toggle the power
at corner B. Corners C and D — authored OFF, never visited — both read ON.

Worth noting the first probe **failed to reproduce it**: armed corner editing
works correctly, and that is what I tested first because the human said "in one
corner". Only the unarmed path shows it. A probe that had stopped at the armed
case would have reported the bug absent.

## Cause

ADR-100 A3 wrote every enable edit into all four corners, guarding a real
problem: *"the next grid tick reads the corner's stored enable and reverts it,
and a power switch that snaps back reads as broken."*

**ADR-109 removed that problem and nobody removed the guard.**
`morphRouteEdit` runs before the enable block and stores the edit in every
path — armed into the armed corner, unarmed into the corner that *won* the
parameter. The grid tick reads back what was just written. The net had been
catching nothing for some time.

**What it cost:** the feature ADR-100 exists for. Its own header promises *"the
morph grid can hold 'off in this corner, on in that one'"*, and the amendment
made that impossible for any edit not made with a corner armed.

## Verified in both directions

Removing a guard is only safe if the thing it guarded stays fixed, so both were
measured:

- **The bug is gone:** C and D keep their authored OFF; the edit lands only on
  the corner being stood on.
- **The old symptom has not returned:** an unarmed toggle at pad positions 0.0,
  0.5 and 1.0 holds through 3 s (~500 grid ticks) at each. If `morphRouteEdit`
  were not storing, this is where it would snap back.

Parity 156/156; every gate green; installed.

## The shape worth remembering

A workaround outlived the defect it was written for, and the change that removed
the defect was a *different* ADR that never knew the workaround existed. Neither
ADR is wrong on its own terms — only reading them together shows the
redundancy. The lesson is not "write fewer workarounds"; it is that a workaround
carries a **stated premise**, and when the surrounding mechanism changes, that
premise is the thing to re-check.

## Also filed

**B60 — Round × Pitch becomes bipolar** (human, same message). Recorded as a
*range* decision rather than a DSP one, because the maths already does it:
`rnd[i] = clamp(round * (1 + roundHi * (2*up - 1)))` has `2*up - 1` running
−1..+1 across the swarm, so a negative `roundHi` skews roundness to the low
voices with no formula change. The only obstacle is the declared lower bound of
0. Parity-safe as a superset (default 0, `1 + 0*x == 1`, no golden sets it) —
the same shape as ADR-056's bipolar onset lock, named as the precedent.
