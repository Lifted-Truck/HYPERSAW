# Proposal — set modules + a routing matrix: what it fixes, what it forces

**Status: groundwork, awaiting the human.** Written 2026-08-26 at the human's
direction: *"We are going to have to come up with an elegant rework of the FX
system since we're replacing the variable slots with set slots and a routing
matrix that allows for feedback and bypass; fortunately I haven't made a single
preset that uses the same module twice, so we could write an algorithm that
ports over to all existing patches nondestructively."*

The direction is right, and it is more right than it looks: it **dissolves**
two open problems instead of patching them. It also forces one architecture
decision that must be made before any code is written. Both are below.

---

## 1. What the rework dissolves for free

### B49's chimera stops being possible

Yesterday's defect: slot `type` is a stepped param in the morph field, drawn
independently of `amount`, so a sweep between a Drive corner and a Gain corner
spent its middle third at `type=Drive, amount=0.10` — a state neither corner
authored (measured: 3 of 9 positions). ADR-124 patched it with an atomic group.

**With set modules there is no `type` param at all.** A module's identity is
structural, not a value, so there is nothing stepped to draw and no chimera to
assemble. ADR-124's group becomes vestigial for FX the day this lands.

### Bypass becomes continuous, so the ramp is free

B49's other half wanted slot presence to *ramp* rather than snap. In a
crosspoint matrix, presence **is** a coefficient, and ADR-088's founding
rationale is exactly this property:

> a coefficient of 0 *is* "not connected" — so connecting and disconnecting are
> one continuous motion.

So "bypass" needs no special case: it is a gain going to zero, and it morphs
continuously by construction. The same is true of ADR-123's osc on/off ramp —
this is the third instance of one principle, and the matrix is its general form.

### It subsumes the FX slot contract proposal

`docs/proposals/fx-slot-contract.md` (2026-08-15, still awaiting the human)
documents the real mess: `amount` has **three different identity points**
across six modules (0, 0.5, and *none*), and **two modules cannot be bypassed at
any setting** — Comp's 0.98 brickwall is always on, Notch measured −5.4 dB and
mono at `amount = 0`.

Under the matrix, bypass stops being the module's job. A module with no live
input edge contributes nothing regardless of its own parameters, so
"is there a no-op value?" stops being a question anyone has to answer per
module. **The rework retires that proposal rather than resolving it** — worth
recording, because otherwise it sits open forever.

---

## 2. The decision the rework forces: feedback is not free

**`routing_core.h` as built cannot do feedback, by design, in one line:**

```cpp
static constexpr bool edgeLive(int from, int to)
{
  return from < NSRC ? true : (from - NSRC) < to;   // slots read EARLIER slots only
}
```

Strict acyclicity is what makes a single forward pass always correct and lets
the audio thread skip cycle detection entirely. Feedback breaks that, and the
two ways out are not equivalent:

| | mechanism | latency in the loop | verdict |
|---|---|---|---|
| **A. Block-rate delay** | one buffer of delay on any backwards edge | **2.9 ms** at 128 @ 44.1k, and it **changes with the host's buffer size** | A 2.9 ms loop is a flanger, not a routing primitive — and a patch that sounds different at 64 vs 512 samples violates our own determinism rule |
| **B. Per-sample matrix** | one-sample delay on backwards edges; matrix runs inside the sample loop | 22.7 µs, buffer-independent | Musically correct; costs a per-sample crosspoint sum instead of a per-block one |

FOUNDATIONS' OQ-23 ruling (*"a cycle is legal, every feedback edge carries a
unit delay at block rate"*) was made for the **modulation** graph, where
block-rate is fine. Inheriting it for audio would be a category error.

**Recommendation: B, and scope it.** Per-sample crosspoint summing over N
modules is N² multiply-adds per sample in the worst case; at N=6 and all
crosspoints live that is 36 MACs/sample ≈ 1.6 M/s per channel — negligible.
It stops being negligible if the roster grows (below), so the honest form is
**per-sample summing over LIVE edges only**, with the live set recomputed per
block.

---

## 3. The sizing question nobody has asked yet

`RoutingMatrix` carries `static_assert(NSRC + NSLOT <= 32)`. Today's rack is 6
real modules. **B45's filter roster adds five more** (SVF, ladder, comb,
formant pair, tilt) — that is 11 modules, 121 crosspoints, and with four morph
corners **484 morphable values for routing alone**.

So the roster and the matrix are one decision, not two:

- every module always present → idle cost scales with the roster, where today
  `FxType::Off` costs *literally nothing* (`case FxType::Off: break;`);
- therefore the renderer **must** skip modules with no live path — the exact
  analogue of `fx_rack.h:272`'s `mix <= 0` guaranteed bypass, promoted from a
  per-slot check to a graph property;
- and B47's finding applies here too: a feedback comb can ring indefinitely, so
  the FX graph needs its own analogue of B38's voice cull, or CPU ratchets the
  same way release tails do.

---

## 4. The migration algorithm — and the hazard in it

The human's premise (*"I haven't made a single preset that uses the same module
twice"*) makes the port injective and therefore exact. Two corrections:

**(a) The unit is the CORNER, not the patch.** Each of the four morph corners
stores its own four slot types. The no-duplicate property must hold **within
each corner**, which is four times as many chances to violate it as the patch
view suggests. The algorithm must *detect* the collision, never assume it away.

**(b) Cross-corner duplicates are fine and strictly better.** Corner A holding
Drive@0.9 in slot 1 and corner B holding Drive@0.3 in slot 2 is *not* a
collision: both become the one Drive module, whose `amount` then morphs
continuously between 0.9 and 0.3 — which is an improvement on today, where they
were two unrelated slots.

```
for each corner c in A..D:
    seen = {}
    for slot s in 0..3:
        t = type[c][s]
        if t == Off: continue
        if t in seen: REFUSE — report (patch, corner, slots) and stop
        seen[t] = s
        module[t].amount[c] = amount[c][s]
        module[t].tone[c]   = tone[c][s]
    # order becomes topology: chain the non-Off modules in slot order
    prev = SOURCE
    for s in slot order where type[c][s] != Off:
        edge[c][prev -> module[type[c][s]]] = 1.0
        prev = module[type[c][s]]
    edge[c][prev -> OUT] = 1.0
    # mix: today's per-slot dry/wet becomes the module's own send depth
    for s: module[type[c][s]].send[c] = mix[c][s]
```

**Nondestructive** means: write the new keys **alongside** the old ones and
bump the chunk version, so a patch saved by the new build still loads in the
old one until the old one is retired. Never rewrite the v-old keys in place.

### The order hazard, which is a ruling not a bug

Corner A may be `Drive → Comb` while corner B is `Comb → Drive`. The matrix
expresses both. But morphing between them passes through a state where **both
edges are partially open** — which is a *parallel blend* of the two modules,
not either serial order, and it does not sound like either endpoint.

That is the same "what does a flip between structures *mean*" question
FOUNDATIONS has open (their OQ #2), now in its most concrete form. Two answers:

1. **Accept it.** Parallel-in-the-middle is a legitimate and arguably musical
   interpretation of morphing between two orderings.
2. **Argmax the topology.** Route edges follow QM-0's `QUANTUM` law — pick a
   corner's whole topology rather than blending coefficients — so order flips
   discretely while module *parameters* still blend.

Our own ROADMAP already names both laws (`BLEND` vs `ARGMAX`) and records they
are *not* two settings of one dial. **This ruling should be made before the
migration is written**, because it decides whether ported patches need their
route edges marked as one atomic group per corner.

---

## 5. Suggested order

1. **Rule** the feedback mechanism (§2) and the topology-morph law (§4). Both
   are human calls and both gate design, not just implementation.
2. **Fix the seam debt first if it is cheap** — unrelated, but it is the only
   open cross-repo obligation and it touches note handling the FX rework does
   not.
3. Extend `routing_core.h` with the ruled feedback mechanism, keeping
   `routing_check`'s 7 invariants green and adding cycle cases.
4. Port `fx_rack.h`'s six modules to a module interface with no bypass duty.
5. Write the migration with the collision detector as a **gate**, not a
   warning, and a golden corpus of real presets as its oracle.
6. Retire `docs/proposals/fx-slot-contract.md` as subsumed (§1).
