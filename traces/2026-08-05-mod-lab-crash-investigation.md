# 2026-08-05 — mod-lab crash: ROOT-CAUSED and fixed

## Human report
"I did just crash something by making CHODEP dependent on R while chorus was linked
about 50% to main." Then, when I reported it undiagnosed: *"Did you try mapping other
things to R and mapping CHODEP to other things to see if it's either of those?"*

That factorial is exactly what was needed, and it is what found it.

## The answer: it is the DESTINATION, not the source
Run headlessly with a fresh engine instance per trial:

| trial | result |
|---|---|
| R → choDep + link 0.5 (the exact report) | clean |
| **K1 → choDep** | **non-finite (watchdog fired 1×)** |
| **LFOA → choDep** | **non-finite (watchdog fired 2×)** |
| ENV → choDep | clean |
| R → K / Kboost / detune / cutoff / level / phDep / morphX / morphY | all clean |
| link sweep 0 … 1 with R → choDep | all clean |

So `choDep` is the culprit and the source only decides how often you hit it: R moves
slowly and smoothly, so it lands on the bad state rarely — which is why the human saw it
intermittently rather than always.

## Root cause: a circular-buffer wrap that rounds to exactly `len`
Caught by snapshotting chorus state at the first non-finite sample:
`w = 0`, `dly[0] = 2.0000000000000195`, inputs and delay line both clean.

Heavy `choDep` modulation drives the target delay NEGATIVE (`base 0.014 + dep·lfo` with
`dep ≈ 0.0145` and `lfo ≈ −1`), so it pins at the `Math.max(2, …)` floor. Then
`rd = w − dly` is a tiny NEGATIVE number, about −2e−14. The old code did
`while (rd < 0) rd += len`, giving `8192 − 2e−14` — but the ulp of 8192 is ~1.8e−12, so
that expression **rounds to exactly 8192**. `line[8192]` is `undefined`, undefined
arithmetic is NaN, and the NaN is immediately written back into the delay line, so one
sample poisons the lab permanently.

**Fix:** true modulo (`rd -= Math.floor(rd / len) * len`) plus an explicit `i0 >= len`
guard and a branchless `i1` wrap. Verified: every trial above is now non-finite-free and
the watchdog never fires.

## Second finding, NOT yet explained — reported rather than buried
With the NaN gone, `K1 → choDep` and `LFOA → choDep` at depth 1 show **intermittent loud
transients**: per-second peaks read `0.57 22.53 0.60 0.62 0.69 0.57 0.76 0.75 23.02 0.64`.
It is **bounded, not runaway** (no growth over 10 s) and `fb = 0`, so it is not feedback;
at depth 0.4 the peak is 0.47. The NaN was previously masking this, because the watchdog
zeroed the block before the level showed. Cause unknown; a candidate is the read pointer
sweeping through the write pointer while the delay is pinned at its floor. Left open.

## The tooling that made this possible
`tools/labharness/modlab_probe.mjs` — the lab's first `<script>` block is deliberately
DOM-free, so it evaluates in Node with a FRESH `ModLab` per trial. Every earlier failure
came from a browser context that could not be reset: `navigate` reused the live JS
context, so "fresh page" tests carried poisoned state and stale settings. The harness
makes state provable instead of assumed, and it generalises to the other labs.

## Lessons
- **Verify the reset before trusting the experiment.** A probe that cannot prove it
  started from a known state cannot support any conclusion.
- **Factorial isolation beats narrative debugging.** The human's question — vary the
  source, vary the destination — cut through in one pass what a chain of plausible
  hypotheses had not.
