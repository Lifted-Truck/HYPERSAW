# ADR-082 increment 1 — the multi-oscillator namespace, landed inert

**Date:** 2026-08-06
**Trigger:** ADR-082 ratified (2 oscillator slots).
**Verify:** `./verify full` GREEN (exit 0, git 76e5de2); parity 147/147, worst 4.262e-09 —
identical to before the refactor.

## What landed

The id/state mechanism, with `kNumOsc` still **1**:
- `id(P, osc k) = id(P, osc 0) + 100k`, `kOscStride = 100`, `kMaxOsc = 2` (ratified).
- `kGlobalIds[]` — the ~29 global params; everything else is per-oscillator.
- `findParam` resolves ids in higher blocks back to their osc-0 definition and rejects
  global ids in those blocks (globals have no per-osc mirror).
- `params_count`/`params_get_info` enumerate osc 0 at **exactly the old indices and ids**, then
  append higher blocks; higher oscillators get "Osc2 …" names and an "Osc 2" module.
- State: osc 0's keys unchanged; higher oscillators prefix `o<k>.`; header bumps to
  `hypersaw-state 2` only when `kNumOsc > 1`; loader accepts 1 and 2, and skips `o<k>.` keys
  for oscillators a build does not have.

At `kNumOsc = 1` every one of those is a no-op: same count, same ids, same state bytes.

## The oracles proved inertness — and could not have proved correctness

`./verify full` green with parity unchanged proves the refactor changed nothing. It does **not**
prove the new mechanism works, because at `kNumOsc = 1` none of the new code paths execute.
That is L0011 restated: an oracle that cannot reach the mechanism cannot validate it.

So the build was temporarily flipped to `kNumOsc = 2` and re-run. Result:

| check | at kNumOsc = 2 |
|---|---|
| state save succeeds | OK |
| every param value round-trips exactly | **OK** (twice the params, `o1.` keys included) |
| restored instance renders bit-identical audio | **OK** |
| partial/forward blob loads | OK |
| missing keys keep defaults; unknown keys ignored | OK |
| all 16 new params act (`paramfunc_smoke`) | OK |
| **state blob is versioned** | **FAIL** |

So the mechanism is sound and the only failure is `tools/state_check.cpp:222`, which pins the
header to exactly `hypersaw-state 1`. That assertion is doing its job — the format version
genuinely changed.

## Deliberately NOT fixed here

Widening that assertion to accept version 2 is a **gate change**, and the charter says gates
are not weakened without an explicit human decision recorded in ROADMAP.md. Increment 1 ships
at `kNumOsc = 1`, which emits version 1 and passes untouched. The dependency is recorded now,
while it is cheap, rather than discovered as a red gate mid-increment-2.

Worth noting the bump is a courtesy, not a compatibility requirement: `state_check` already
proves unknown keys are ignored, so a version-1 reader tolerates `o1.` keys. The alternative —
never bumping — was rejected as dishonest: the format did gain a key namespace.

## Ratification correction recorded in the ADR

The ADR's open question "is the sub (52-55) / `balance` (56) superseded by real oscillators?"
was **withdrawn as ill-posed**. `balance` is the two-cluster A/B *coupling* balance inside the
SAW swarm (`kB = 1 - 2*balance`, ADR-051), not an oscillator mixer; the sub exists only in
`spectra_core.h` and SAW has none. Both are ordinary per-osc params. The question came from
reading the layout lab's shorthand ("ONE engine select + A/B balance + sub") as an architecture
description when it was a sketch — a reminder that prose in a design doc is not evidence about
the code.

## Next

Increment 2: `kNumOsc = 2`, a second `SwarmCore`, mixed behind `balance`… and first the
`state_check` version gate, plus the min-spec CPU measurement ADR-082 requires.
