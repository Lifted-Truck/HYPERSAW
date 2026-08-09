---
id: hypersaw-signal-graph-001
from: HYPERSAW
to: FOUNDATIONS
status: DRAFT — not filed
ball: (would be) provider
filed: —
respond-by: (propose 2026-09-05, to match notice-intake)
---

> **Origin:** HYPERSAW resident session, 2026-08-09, ROADMAP B23 (routing lab,
> rounds 1–2) and its research probe. Motivated by the human asking, at the
> ratification gate, whether the topology had been solved more elegantly
> elsewhere. Authored by an agent; the human ratifies before filing.

# Brief — §3.5 Signal Graph is a chain, and phase 0 is about to foreclose it

## The need

`FOUNDATIONS.md` **§3.2** rules a modulation routing to be a five-tuple
*(source, destination, depth, curve, scope)* — a sparse connection list.
**§3.5 (Signal Graph)** says only: *"Slot chain: source → per-voice processing →
mix → global chain."* That is a fixed chain.

HYPERSAW's routing lab (`docs/design/routing-lab.html`) benches six audio
topologies against each other with real audio. A fixed chain is one of them
(scheme E) and it is the **least expressive**: it cannot send one oscillator
somewhere the other does not go, which HYPERSAW needs the moment there are two
oscillators — which there now are.

HYPERSAW is **phase 0**, and §8 names *slot chain* seam quality as part of what
phase 0 is supposed to force. §9's deferred-questions register does not contain
audio-routing topology. So this is a gap, not a settled question we are
re-litigating.

## What we are NOT asking for

Not a facility. Prime Directive 2 (*harvest, don't invent; two consumers
minimum*) applies and we are one consumer. We are not asking FOUNDATIONS to
build a routing matrix, and we are not proposing to extract ours.

## What we are asking

**One ruling, and only because it is cheap now and expensive later:** does
§3.5's signal graph stay a chain, or is the doorframe widened to admit a
non-chain topology later?

We ask because the answer changes what HYPERSAW ratifies *this week*. Our own
recommendation would otherwise be a dense crosspoint matrix, and if §3.5 later
becomes a five-tuple edge list to match §3.2, that is a retrofit of exactly the
kind FOUNDATIONS was founded to prevent.

## The evidence, compressed

Six schemes, same slots, same sources, measured (all finite, none identical):

| scheme | patch state 4×8 | automation ids | instances | serial? | topology morph |
|---|---|---|---|---|---|
| A per-osc sends | 32 | 32 | 8 | no | continuous |
| B private chains | 64 | 0 | 32 | per source | hard cut |
| C dense crosspoint | 88 | **8** | 8 | arbitrary | **continuous** |
| D sparse slots (= §3.2's shape) | 36 | 12 | 8 | arbitrary | hard cut on edge add/remove |
| E chain (= §3.5 today) | 8 | 0 | 8 | one path | hard cut |
| F bus / aux-send | 20 | 8 | 8 | arbitrary* | hard cut on bus change |

Three findings we think are the library's business, not ours:

1. **Patch state and automation ids are different resources.** Our first
   analysis conflated them and rejected the dense matrix at "120 params"; it is
   88 values of patch state and **8** automation ids. Any registry that morphs
   and serializes state separately from exposing host params has this
   distinction already — §3.1 appears to, and if so it should be stated,
   because we got it wrong without it.

2. **Topology morph splits dense from sparse.** In a dense table a crosspoint at
   0 *is* "not connected", so connecting is continuous. Every sparse
   representation stores topology as discrete structure, so adding an edge is a
   hard cut. §3.1 makes morph corners scoped snapshots and §3.2 makes routings
   sparse — **so morphing a corner that adds a modulation routing is a
   discontinuity by construction.** We believe that is unstated and load-bearing.

3. **A free edge list can express an illegal graph; a dense grid cannot.** Our
   D implementation let a backwards edge be set directly on the model (the route
   a preset load or morph corner takes), producing undeclared feedback. The
   dense grid has no cell for one. If §3.2's five-tuple is the library's routing
   shape, the acyclicity/ordering rule needs an owner — and it must sit on the
   read side, since the write side includes preset load and morph.

## Contract tests we can offer

Per the oracle-donation posture: HYPERSAW can supply behavioural oracles that
name no internals — a routing oracle in the shape of `mpe_check` (drives the
public plugin surface, detects via emitted audio) asserting that a declared
topology is the topology that sounds. Offered, not imposed.

## Respond-by

No urgency beyond ours: HYPERSAW holds B23 unruled until this is answered, and
will keep building elsewhere meanwhile. If the answer is *"chain for now, ask
again with a second consumer"*, that is a complete answer and we will record it
and ratify locally with the divergence documented.
