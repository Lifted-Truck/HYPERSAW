# 2026-08-03 — bend lab: pitch-bend inertia bench (experiment, pre-fold)

## Changes
- docs/design/bend-lab.html: new lab. Four inertia models on the pitch lane —
  off / one-pole lag / rate limit / mass-spring / lag→rate series — plus a
  `return ×` asymmetry (bend lane only) and an applies-to selector (bend wheel /
  note pitch / both). Offline step-response + vibrato characterisation with live
  meters, a live trajectory plot, a draggable wheel, and five fixed gestures.
- ROADMAP.md: experiment section with the fold decision left open.

Nothing in `src/` was touched. This is a bench, not a fold — the human asked to
*test* the idea, and the reference-first rule (ADR-003) says the audition comes
before the core learns anything.

## Why four models, not one
"Inertia" names three different physical claims that do not sound alike, and
picking one silently would have decided the feature by accident:
- **lag** (one-pole) is *proportional* — every move takes the same time
  regardless of size.
- **rate limit** is *constant-velocity* — big moves take proportionally longer,
  small moves are nearly instant. On a −12 st dive this is a completely
  different instrument from the lag.
- **mass-spring** is the only one that is inertia literally: it can overshoot
  and ring, because a mass in motion does not stop when the force does.

The bench runs the inertia at **tick rate** (2756/s @ 44.1k), not sample rate,
because that is where a fold would put it — the ADR-027 live-tune factor is read
once per tick at law evaluation. Measuring a filter the plugin would never
actually have would be measuring the wrong thing. Time constants are in
ms/seconds and converted at use (ADR-009).

## Evidence — meters calibrated against closed form
The characterisation code was **sliced out of the lab and run in node**, not
retyped, so the calibration cannot pass while the shipped lab is wrong
(L0011: an oracle that constructs its own copy tests its own copy).

| case | measured | closed form |
|---|---|---|
| lag τ=60 ms, lag to 50% | 41.36 ms | τ·ln2 = 41.59 |
| lag τ=60 ms, settle ±5 ¢ on 2 st | 221.32 ms | τ·ln40 = 221.33 |
| lag, vibrato depth @ 5 Hz | 46.89 % | 1/√(1+(ωτ)²) = 46.86 |
| lag, vibrato phase lag | 34.30 ms | atan(ωτ)/ω = 34.47 |
| rate 24 st/s, lag to 50% of 2 st | 41.36 ms | 1.0/24 = 41.67 |
| rate 24 st/s, settle | 80.91 ms | 1.95/24 = 81.25 |
| spring f=4 ζ=0.6, overshoot | 18.76 ¢ | 200·e^(−πζ/√(1−ζ²)) = 18.96 |
| spring ζ=1.0 (critical), overshoot | 0.00 ¢ | 0 — must not overshoot |
| model off | 0 lag, 100 % depth | exact |

**Planted-bad (L0016 — calibrate the detector on a known-bad case too):**
spring at ζ=0.06 reads 5 reversals, 166 ¢ overshoot, settle "> 600 ms". A
detector that only ever sees clean input has not been calibrated.

`./verify fast` exit 0 at a6560aa (docs-only change; the C++ chains live in
`full` and nothing they cover was touched).

## Bug found while checking
The page loaded blank and every meter read "—". `wire()` fires `recalc()` during
setup, and `recalc()` touches the canvas contexts, which were `const` further
down the file — temporal dead zone. The **first** `wire()` call threw and
silently took the entire rest of the script with it: no gestures, no keyboard,
no draw loop. Function declarations hoist; `const` does not. Fixed by declaring
the canvas handles above the wiring, with the reason written next to them.

Worth naming because the failure was *silent* — the page rendered, the sliders
moved, and only the em-dashes gave it away. Checking the DOM rather than the
screenshot is what caught it.

## Open — for the human's ear, not the meters
The bench measures how each model behaves; it cannot say which one is *right*.
The specific tension it puts a number on: every model is a low-pass on your
hand, so inertia costs wheel vibrato (lag τ=60 ms already keeps only 47 % of a
5 Hz wobble). If both are wanted, that is an argument for keying inertia to bend
*distance* rather than applying it flat — untested, and deliberately not built.
