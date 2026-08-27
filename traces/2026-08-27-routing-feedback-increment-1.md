# Trace — routing feedback, increment 1; and the blocker ADR-128 did not price

**Trigger** human 2026-08-27: *"I would like to make headway on the FX today,
and ideally even get the mod matrix up and running."* Plus two design questions
answered first, because both gate the matrix rather than delaying it.

## What shipped

`routing_core.h` takes per-sample feedback (ADR-128). Backwards and self edges
are legal and read `zPrev` — the previous **sample's** slot output — so the
graph is still one forward pass and the audio thread still needs no cycle
detection. That property was what the old forward-only strictness bought; the
one-sample delay buys it back without forbidding the topology.

**Inert by construction:** `setSerialChain()` sets no backwards edge, so `zPrev`
is never read. Parity **156/156** unchanged.

`routing_check` 9 → 11 invariants. The old *"an illegal backwards edge changes
nothing"* was **retired as obsolete, not weakened** — it encoded the
forward-only contract that ADR-128 (a human ruling) replaced. Four take its
place, including the two that pin the ruling itself:

- *no backwards edge: the engine is stationary and unchanged* — 64 samples, all
  identical, `zPrev` never read
- *a backwards edge is inert for exactly one sample, then live* — sample 0
  equals the forward-only result, sample 1 diverges. This asserts the delay
  directly rather than inferring it
- *a self edge is delayed, not a regress* — the rule at its tightest
- and the retained control: *a forward edge in the same shape changes it at once*

## The blocker for increment 2 — found while building, not resolved by ADR-128

The scalar `process()` is per-sample and takes feedback naturally. **The shell
does not call it.** `processBlock` computes each slot's *entire block* before
the next slot gathers from it, so a backwards edge there can only ever see the
**previous block's** output — which is precisely the 2.9 ms block-rate feedback
the ruling rejected.

**One-sample feedback is impossible across block-processed slots without
interleaving.** ADR-128 ruled the semantics and I recorded its CPU cost, but not
this: the ruling implies a change to the **slot contract**, not only to the
matrix. Two ways out, both real:

- **(A)** the slot interface becomes per-sample. Simplest matrix; every module
  is rewritten, block vectorisation is lost, and a compressor that detects over
  a block is not naturally per-sample.
- **(B)** only slots inside a cycle interleave; everything else keeps block
  processing. Needs cycle detection at edit time (control thread, cheap) and two
  slot interfaces.

Recommend **B** on cost. It is a ruling, and it gates increment 2.

## Two design questions answered from the code

**Per-note scope** (human: *"can we make all parameters per-note, or are some
necessarily global?"*). The rule: **a parameter can be per-note iff the state it
controls already lives per-note.** Three tiers, recorded in B34 where the matrix
will read them. Tier 1 free (scalars inside the per-voice tick — `struct Voice`
already carries `phase/driftS/couple/vf/eff/mom`). Tier 2 possible but not free
(the `rebuild()` set, because `x[]`/`panL[]`/`panR[]` at `swarm_core.h:1751` are
core-level and shared by every sounding note). Tier 3 necessarily global — the
FX rack, and **for a stronger reason than CPU**: it is post-oscillator, so by
the time audio reaches it the notes have been summed and there is no note
identity left to be per-note *about*.

**B57, the silent spring.** Filed with the human's refinements — modulator page,
follow-toggles visible only when the lane they follow is active, doubles as an
interval-based alternative to keytracking. **Correction:** I told the human last
turn that I had filed B57. I had not. It exists now.

## Verify

`./verify fast` exit 0 · `routing_check` GREEN, 11 invariants · `parity_check`
156/156 within ε=1e-6 (worst 4.262e-09).
