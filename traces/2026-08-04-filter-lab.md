# 2026-08-04 — swarm-filters lab: the verdict turned into three numbers

## Changes
- `docs/design/filter-lab.html` — new bench (lab campaign 3 item 2).
- `ROADMAP.md` — B6 status + findings section.
- `LIBRARY.md` / `INDEX.md` — L0026.

## Why this lab first
Campaign 3 lists three labs. Swarm filters was chosen because it is the only one with a
**concrete stated defect** to chase ("not quite there yet"), it has parity-proven cores to
bench against, and it unblocks E1 and the FX rack. Quantum morph is partly blocked by its
own brief (item g: capture the human's mod-matrix collision ideas *before* designing) and
SPECTRA expansion has no success criterion yet.

## Measured the shipped core BEFORE building anything
A C++ probe swept `filter_core.h::processExternal` sine-by-sine (steady-state RMS out ÷
RMS in, 48 kHz). Three defects, all structural rather than accidental:

1. **Resonance is a backwards volume knob** — peak +0.98 / −3.21 / −9.32 dB at qbase
   0.1 / 0.5 / 0.9. N summed *unity-gain* bandpasses capture less power as they narrow.
2. **No low end, worse with Q** — 40 Hz is 24.2 dB below peak at default, 28.4 dB at high
   Q. There is no DC path in the bank at all.
3. **A band-pass hump, not a filter** — rolls off both sides in every configuration; 27.1
   dB nulls between bands, worse with fewer bands.

Plus a gap: **no key tracking on the effect path** (`setNoteFreq` moves only the gravity
centre, only for harmonic placement).

## Fixes auditioned, measured in the lab
- **Q compensation** (÷√Q, since summed power ∝ 1/Q): level swing **9.0 → 1.0 dB**.
- **LF preserve** (one-pole at the lowest band, added back): deficit **22.6 → 4.7 dB**.
- Conventional multimode and bank→conventional series included for the brief's
  "how would this sit in the rack" question.

## Fidelity — cross-checked, and its limit stated
The lab's band POSITIONS come from its own seeded draw, not `forcecore::buildOffsets`, so
its absolute curve is not the core's curve sample-for-sample. What was cross-checked is
what the bench is FOR: LF deficit **24.2 dB (C++) vs 22.6 (lab)**, Q swing **10.3 vs 9.0**.
Both follow from summing unity-gain bandpasses and survive any draw. The lab says this in
its own text so nobody quotes its absolute dB as a core measurement.

One readout needed a second caveat: **Q swing means the opposite thing on the conventional
filter** — a resonant filter is supposed to grow a peak at its corner (+14.4 dB, a 33 dB
"swing"), and that is the feature, not a defect. Same statistic, opposite meaning.

## Bug found — the same trap twice in one day
The page loaded looking almost right: markup, CSS and sliders all fine, canvases blank and
meters showing em-dashes. `wire()` invokes its callback once during setup; that callback
called `schedule()`, which touched a `let pending` declared further down — temporal dead
zone, so the first `wire()` threw and silently took the rest of the top-level script with
it (no keyboard, no drone, no draw loop).

This is the **second** occurrence today; `bend-lab.html` failed identically hours earlier,
and I had written a comment about it there. Knowing the trap did not prevent the repeat,
which is why L0026's falsifier says the fix is tooling (no-use-before-define), not care.
Located by re-evaluating the script body in a try/catch to recover the line — checking the
DOM rather than the screenshot is what surfaced it both times.

## Evidence
`./verify fast` exit 0 at 87620d2 (docs-only). Lab verified in-browser: script completes,
both canvases painted, diagnostics populated — shipped 1.56 dB @ 624 Hz / 22.6 dB LF
deficit / 27.1 dB null / 9.0 dB Q swing; Q-comp + LF-preserve 4.7 dB deficit / 1.0 dB swing.

## Open (human)
Whether the bank becomes a proper rack filter (fixes 1+2, or in series with a conventional
multimode), whether key tracking is added and at what default, or whether the bank stays a
*resonator/formant* effect with a conventional filter built beside it. The measurements say
it is currently neither one thing nor the other — a plausible reading of the verdict.
