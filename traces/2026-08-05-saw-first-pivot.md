# 2026-08-05 — SAW-first pivot: SPECTRA hidden from new patches

## Changes
- `src/gui/gui.html`: SPECTRA removed from the engine selector's options; legacy guard
  restores it when a saved patch reads back as SPECTRA.
- `ROADMAP.md`: strategic pivot section + intelligent-randomness ruling.

## What was NOT done, deliberately
Nothing was deleted. Param id 43 keeps its 0..1 range and its labels; `spectra_core.h`
still compiles and runs; state save/load still round-trips an engine value of 1; host
automation can still select it; `spectra_check` stays in `./verify full`. The change is a
single removed `<option>` plus a guard — reversing it is one line.

**Why the guard matters.** Hiding an option that existing patches may already hold is how
you silently break someone's work: the select would show SAW while the engine played
SPECTRA, or the value would be clamped away on the next write. `engineOptionGuard()` runs
in the poll path and, if the engine ever reads back as SPECTRA, re-adds the option
labelled "SPECTRA (legacy patch)" — the patch keeps working AND explains itself.

## Evidence
`./verify full` exit 0. parity_check 147/147 (worst 4.262e-09 @ dyn-ring.seed42);
state / notefuzz / waveshape / force / **spectra** / filter / notch / swarmalator / time
all GREEN. The SPECTRA chain staying green is the point: the core is untouched.

## Next
Layout lab resumes as the IA audition for the renovation — SAW-only, with
multi-oscillator, morph and mod-matrix pages as the live questions. The lab currently
stages SPECTRA in its oscillator selectors and will need updating to the new direction.
