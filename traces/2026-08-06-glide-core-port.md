# Glide travel laws ported to C++ with a trajectory oracle (A1 fold, increment 1)

**Date:** 2026-08-06
**Trigger:** A1 fully ruled — laws 1–4, constant-rate default, quantise as a modifier,
per-destination linked. The spec was complete, so B19 became buildable.
**Verify:** `./verify full` GREEN (exit 0, git 519cfad); 12 oracle chains; parity_check
still 147/147 worst 4.262e-09 — unchanged by this work.

## What landed

- `src/glide_core.h` — the four ratified travel laws + the quantise modifier, transcribed
  from bend-lab's `Inertia`. **Law 5 is absent, not dead-coded**: the fold ruling cut it, and
  a commented-out `case 5` would be an invitation to resurrect a control the measurement
  already rejected.
- `tools/golden/extract_glide.mjs` — slices `Inertia` live out of the HTML. The class end is
  found by **brace depth**, not a line number, so an edit inside the class cannot silently
  truncate the slice. Also parses the authored `const P` literal, because evaluating the lab
  without a DOM zeroes every parameter (the trap that made the first roundup print five rows
  of zeroes).
- `tools/golden/gen_glide_goldens.mjs` — 11 scenarios over one fixed gesture (step, hold,
  release, reverse through zero, settle off-grid) so every law is exercised in both directions
  and through the return-multiplier asymmetry.
- `tools/glide_check.cpp` — parity at the L0-1 bar plus behavioural anchors.
- `./verify full` gained the chain. **Protected-path edit, flagged for ratification** — it is
  the same pattern every prior core port used (force, spectra, filter, notch, swarmalator,
  time), and it is additive.

## Result

Parity green on 11/11, worst RMS **3.51e-08** against a 1e-6 bar.

**The anchors are the more interesting evidence.** They were written from the JS measurements
taken days earlier, and the C++ port reproduces them independently:

| anchor | JS measurement | C++ port |
|---|---|---|
| spring overshoot | 18.8¢ | **+18.8¢** |
| constant rate overshoot | none | **+0.0¢** |
| hysteresis at a boundary, ζ 0.5 | 15 → 3 flips | **15 → 3** |

Parity to a golden only proves the port matches a recording. These anchors pin the *character*
each law was chosen for, so a future refactor that keeps parity to a stale golden still trips.

## Deliberately not done

**The core is not in the audio path.** No param ids, no state keys, no GUI. Core + oracle
first, shell integration as its own increment — the order the swarmalator port used, and the
reason this change cannot regress anything: nothing calls it yet, which the untouched
147/147 parity demonstrates rather than asserts.

## Next

Shell integration needs decisions the fold ruling did not cover: how the four destinations
(note pitch / bend wheel / mod wheel / MPE) map onto the existing glide params (11, 33, 34,
70, 75, 89, 90), and whether those params are superseded or re-pointed. Append-only ids mean
that wants the same care as ADR-082.
