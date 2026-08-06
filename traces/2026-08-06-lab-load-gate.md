# Lab load gate — closing L0026 with tooling, five occurrences late

**Date:** 2026-08-06
**Trigger:** human approval of the recommendation to close the L0026 gap
("I'll follow your advice"), after the fifth occurrence — written by the agent
that had just documented the trap.
**Verify:** `./verify fast` GREEN; proven to go RED on an injected break and
back to GREEN on restore.

## What was built

`tools/labharness/lab_load_check.mjs`, wired into `./verify fast`. It executes
every `<script>` block of every lab HTML in a `node:vm` context whose DOM and
audio globals are universal proxies — every property access returns another
callable proxy, so no lab can fail for lack of a canvas or an AudioContext.
What survives that is genuine JS: temporal-dead-zone errors, typos, bad
references. If it throws at load, the page is broken in a browser too.

**Why not a static linter pass.** The obvious implementation — scan for
identifiers referenced above their `const`/`let` declaration — over-flags
massively, because a function *declared* early that references a later const is
fine as long as it is *called* later, which is the common case. That detector
would cry wolf on nearly every lab. Two detectors in the preceding two days had
to be recalibrated for exactly that failure (the sweep's 53 false "dead"
routings; the polarity marker libelling `ENV → Kboost`). Executing the file
reproduces the real failure mode with a false-positive rate of zero by
construction.

## Calibration — the step that matters

A gate is worthless until it is shown to fail on the bug it exists for. Both
historical instances were re-injected into a copy of the current mod lab:

| case | source | result |
|---|---|---|
| `mtx` declared below the `wire()` calls that reach it | the real 2026-08-05 bug that hid the whole matrix for weeks | **caught** — `ReferenceError: Cannot access 'mtx' before initialization` |
| `new ModLab(SR)` with no `SR` in scope | written by me, 2026-08-06, immediately after documenting the trap | **caught** — `ReferenceError: SR is not defined` |
| untouched copy (control) | — | passes |
| all 12 real labs | — | pass |

The first injection attempt was itself wrong — it re-inserted the declaration
*above* the wiring, so the "bug" reproduced nothing and the checker passed. That
near-miss is the point: a calibration that is not itself checked will happily
report success. Fixed by asserting the injected declaration's character offset
is greater than the offending `wire()` call's before running the check.

## One false positive found and fixed, in the checker

The first full run reported `spectra-lab.html` broken with
`ReferenceError: Event is not defined`. That was the *checker's* missing global,
not the lab's bug — the same cry-wolf class this gate exists to prevent, and it
would have been shipped as a lab defect had I not read the message. Fixed by
giving the sandbox real `Event`/`CustomEvent`/`KeyboardEvent`/`MouseEvent`
constructors.

## Knowledge-loop update

L0026's falsifier read: *"the fix is tooling, not care, since knowing about the
trap demonstrably did not prevent the repeat."* That is now discharged, so the
entry is promoted **candidate → canonical** and the falsifier restated: the
lesson now falsifies if a load-killing lab bug reaches review while the gate
passes — i.e. if executing under stubs stops resembling a browser load.

Five occurrences, the last written by an agent that had just finished writing
the warning, is about as strong as evidence gets that documentation does not
scale as a control.

## Evidence consulted

- `docs/design/*.html` — 12 labs, all green
- `verify` (`fast()`) — gate added after the knowledge-loop integrity check
- `LIBRARY.md` L0026, `INDEX.md` L0026 pointer
- red/green proof: injected `notDefinedAnywhere()` into `width-lab.html` →
  `./verify fast` exit 1; restored → exit 0
