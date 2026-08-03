# 2026-08-03 — comb declick + PANIC layout + lab-visuals convention

## Changes
- `src/fx_rack.h`: comb retune declick (per-line output gain `g`, deferred retune,
  smoothed `1/activeLines`), plus `setCombDeclick(rampSec, normSec)`.
- `src/gui/gui.html`: PANIC out of `position:fixed`, into the header flow.
- `tools/waveshape_check.cpp`: T6 paired near/far click gate + planted-bad calibration.
- `ROADMAP.md`: standing convention — lab visuals ship with the feature.

## The click (human: "a tiny amount of clicking when the comb is added")
Two step discontinuities, both real:

1. **`noteOn` wiped a ringing line.** `memset` + `w=0` + `lp=0` while the line was
   audibly sounding — an instant drop to zero.
2. **`norm = 1.0/act` stepped.** The instant a note claimed a line, every *other*
   ringing line's output was divided by a larger integer — up to a 6 dB step,
   mid-ring.

**Fix.** Each line gets an output gain `g`. A retune on a sounding line is now
*deferred*: ramp `g` to 0 (6 ms), apply the new delay at the bottom, ramp back.
Fading DOWN first is the entire point — a ramp-up alone cannot hide a discontinuity
that has already happened. `1/activeLines` is smoothed over 30 ms. Both constants are
in SECONDS, converted in `setSampleRate` (ADR-009).

**Second iteration, from measurement.** Gating the output alone left residue: the
read-pointer jump is a step in `dl`, which is written back into the buffer *ungated*,
so it recirculates every `dly` samples and re-emerges once the ramp is back up — a
click that is merely LATE. The buffer clear was therefore restored, but moved to the
bottom of the ramp: clearing while muted lets the line rebuild from the bus, which is
continuous, so no step exists to recirculate.

## Evidence
| run | near a note-on | far from any note-on | ratio |
|---|---|---|---|
| declicked | 0.02629 | 0.02607 | **1.01** |
| declick disabled (pre-fix) | 0.38124 | 0.02248 | **16.96** |

The note-on neighbourhood is now statistically indistinguishable from anywhere else —
the click is gone, not merely reduced.

`./verify full` exit 0. parity 147/147 (worst 4.262e-09 @ dyn-ring.seed42); all ten
oracle chains GREEN. Installed 44d598a.

## Two detector errors on the way — both mine, both caught before shipping
1. **Wrong signal class for the floor (L0017 again).** The first version compared a
   comb-with-notes against a *dry sine*, whose 0.0078/sample slope the comb's own
   resonances legitimately exceed. The bound meant nothing, and the fix "failed" a
   test that was measuring the wrong thing.
2. **Confounded configurations.** The second version compared two different note
   patterns — different line counts and entry times — so it measured their
   configurations rather than the click. It passed at 1.56× against a 1.6× bound,
   which is a marginal pass and therefore a weak gate.

The fix for both was to make the comparison **paired within one run**: worst jump near
a note-on vs worst jump far from one, same signal, same lines, only position differs.
That is what produced the unambiguous 1.01 / 16.96 split. **A ratio that lands right at
the bound is a signal the detector is wrong, not that the fix is marginal.**

## PANIC layout
`position:fixed; top:8px; right:96px` pinned it over COPY/PASTE STATE, which share that
corner. It is already in the `<header>` markup, so dropping the fixed positioning puts
it in flow next to the wordmark. Trade-off recorded: it no longer floats over a
scrolled view — acceptable while the header is always visible at the panel's height.

## Convention recorded (human direction)
Lab visuals ship with the feature they explain; "no visual" must be argued, not
defaulted. Global visualizer stays for whole-instrument state; feature displays go on
their own tab. Backlog listed in ROADMAP with lab sources. Rationale: HYPERSAW's
controls (`dist→overshoot`, `onset α`, `super-width mode`) cannot be inferred from
their names, and a knob whose meaning is only discoverable by careful listening is one
most users will never touch.
