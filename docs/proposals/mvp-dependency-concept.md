# Concept — what we need from FOUNDATIONS before the MVP + test-table push

**Status: FILED 2026-08-15** as `integrations/hypersaw/brief-mvp-dependencies.md`
(`32e869b`, verified on their `origin/main`). This is the working copy;
the filed version drops the draft framing and the closing question. Written 2026-08-15 by the lead organ
at the human's request: *"get the bones of every lab and the full existing
feature list up in a minimum viable way as soon as possible — but before that,
every page and feature gets its table of tests (agentic and human-assisted) — and
I don't want to jump in line ahead of FOUNDATIONS."*

---

## The one-sentence version

**We are not blocked on their code. We are blocked on five shape commitments** —
and the reason to ask before building is that their own standing GUI criterion
already decided the thing our MVP would otherwise do wrong.

## Why asking is cheaper than building

Three facts, each verified rather than assumed:

1. **gui2 reaches 30 of 105 params** (`tools/gui_reach.py`, today). The MVP means
   hand-adding roughly **75 controls**.
2. Their **standing GUI criterion** (their ROADMAP, DECISIONS #64, human-set):
   *"GUI structure derives from registry declarations (auricle's
   triple-maintenance scar is the evidence); styling is data, separable from
   structure … GUI components bind to SCOPES so they reuse wherever the scope
   instantiates."*
3. Their F2 says **facilities arrive by extraction, and HYPERSAW's GUI/viz is a
   named donor.**

Put together: if we hand-write 75 controls now, we build **the exact artefact
they intend to extract, in the shape they have already ruled is the scar.** That
is not a small rework risk — it is building the donor wrong. The MVP is still
the right goal; the question is only whether we build it once or twice.

**The test tables sharpen this, not soften it.** A test table keyed to a control
we hand-placed is keyed to a structure that re-point may generate differently.
Writing ~100 tables against a vocabulary that then changes is the expensive
version of this mistake, because tables are the thing we would least want to
rewrite.

## D0 — A fact about our GUI we have never told them (correction of record)

Checked before asserting it: **`gui2` appears nowhere in FOUNDATIONS' tree** — not
in their ROADMAP, not in their code, not in a single file of our own mailbox.
They have never been told it exists.

So their standing GUI criterion, and F2's naming of "HYPERSAW's GUI/viz" as an
extraction donor, both rest on a phrase — *HYPERSAW's GUI* — that **does not
currently denote one thing**:

| | reaches | pages | build |
|---|---|---|---|
| `src/gui/gui.html` | **102 / 105** params | (single page) | the **default** |
| `src/gui/gui2.html` | **30 / 105** params | MAIN · MIX · OSC · FX | `-DHYPERSAW_GUI2=ON`, **defaults OFF** |

And the trap inside the trap: **`HYPERSAW_GUI2` defaults OFF, yet the human's
installed bundle is gui2** (ROADMAP 2026-08-12, verified by `pg-MIX` in the
binary). So the GUI that gets *played* is the 30-param one, and the GUI that a
default build *ships* is the 102-param one.

**This is the same shape as a finding they already made, one layer up.** They
caught that we have two CLAP shells — `hypersaw_clap.cpp` (105 params,
`coreKey`, string dispatch) and `swarmfx_clap.cpp` (17 params, no `coreKey`,
positional dispatch) — and concluded that *"the two copies have already
diverged, so extracting against one would re-fork on re-point."* **Our two GUIs
have already diverged in exactly the same way**, and nobody has told them.

We are not claiming they leaned on gui2 — they cannot have, they do not know
it exists. The risk is the reverse and quieter: **a criterion written about "the
GUI" while "the GUI" is ambiguous, and while neither candidate is complete.**
That ambiguity is cheap to fix now and expensive to discover at re-point.

## What we would ask, in priority order

Each ask states **what we do if the answer is "not yet"** — none of these blocks
us, per their own rule 2 (consume-when-connected, degrade visibly).

### D1 — Registry-derived GUI: is it real, and what must a declaration carry?
The load-bearing one. If GUI structure derives from registry declarations, we
need the **declaration field set** before writing 75 controls: label, range,
unit, default, group/page, widget hint, scope — whatever the generator will read.
We do not need the generator. We need the fields, so what we write is a
**superset that survives**, not a rewrite.
*If not yet:* give us the **minimum committed subset** — even three fields we can
rely on beats none, and we will carry our extras locally and flag them as ours.

### D2 — Scope vocabulary
Their criterion binds GUI components to **scopes**. We already have three de
facto: **globals (29)**, **per-oscillator (76 copies at `kNumOsc == 2`)**, and
**patch-scope (31, raw-id dispatch, must be data-fixed)** — the last one derived
by `gui_reach.py` from the shell requiring both a shared-object reference and an
early `return`. We need their scope **names and semantics** so we declare ours
compatibly. This is the field our test tables will be keyed on, so it is the one
whose churn costs the most.
*If not yet:* we publish our three as a proposal and mark them provisional.

### D3 — Stage-3 doorframes: slot chain + mod bus, and specifically CYCLES
The MVP includes the FX overhaul, the modular routing page, per-corner morph
shape modules, and **routing feedback**. That is Stage 3 territory. They
deliberately did **not** extract our live matrix (*"their code, still hot"*),
which reads as permission to keep building — but `mod_routing.h`'s increment 1
records **cycles deliberately validating, pending their OQ #23**, and #23 is
verbatim:

> **Feedback-edge semantics in the modulation graph.** … the semantics must be
> *chosen and documented, not emergent from implementation accident*. Options:
> mandatory unit delay on feedback edges (block-rate) · fixed evaluation order ·
> iterative settlement. Recommendation on file: **unit delay at block rate** …
> **Must be ruled before any mod-bus doorframe hardens.**

**Their recommendation is the same one we reached independently** — we proposed a
feedback send with an explicit one-block delay and a hard-capped loop gain before
reading #23. That convergence is worth reporting to them as evidence (their own
two-consumer rule counts independent arrival), and it is also the strongest
argument for *not* inventing a second cycle model in the meantime.

Also live and adjacent: **OQ #16, signal-graph topology**, where our six-scheme
routing lab is already one of three convergent consumers, and whose status is
*"three reports that a chain is insufficient are not a design."*

Ask directly: build toward the five-tuple and `port.h` typed ports, or keep our
own shape and re-point later?
*If not yet:* feedback stays **lab-only** (`routing-lab-modular.html`), which is
where we wanted to prototype it anyway, and does not enter `routing_core.h`.
Given #23's own words — *must be ruled before any mod-bus doorframe hardens* —
this is the one item where we would be **actively unhelpful** by shipping first:
a second cycle model in a donor codebase is exactly the "emergent from
implementation accident" that #23 exists to prevent.

### D4 — Test-table interop: ours alone, or a shape they can consume?
This is the new ask and it may be the most valuable. They already ship
conformance suites keyed to rulings, and this week that machinery twice found
that **a suite's encoding was not the rule it claimed** (R8, and the fill
precondition). If per-feature test tables ever become a library facility, ~100
tables in a private format is the rework.
*If not yet:* we design our tables so the **assertion is separable from the
harness** — which we should do regardless, because that separation is exactly
what R8 was about.

### D5 — Re-point timing (the scheduling fact, not a design ask)
Their Stage 4 re-point is recorded as **"clear from their side"**. Whether it
lands before or after our MVP push changes everything: before, we build once on
the new bones; after, we build carefully or twice. We are not asking them to
hurry — we are asking for the **order**, so we can sequence rather than collide.

## What we are explicitly NOT asking for

Their implementation, their generality, or to be unblocked. We proceed degraded
on every item above. We are also **not** asking them to reprioritise: their
mediator is single-threaded *by design* (their words: *"the mediator is the slow
part by design"*), and the human's instruction was not to jump the queue.

## The reciprocal half

This is not a one-way ask, and the brief should say so plainly. **Our MVP is
their donor.** F2 names HYPERSAW's GUI/viz as extraction material; a
feature-complete gui2 with a test table per feature is the richest donor they
could get — *if* it is built in the shape they will extract. Built blind, it is
75 controls of scar tissue they then have to un-pick. Answering D1 and D2 is
cheaper for them than extracting from a GUI that ignored their own criterion.

## Recommended sequencing, if they answer nothing

Stated so the plan is not hostage to the reply:

1. **Test tables first, as the human asked** — they are the least
   re-point-sensitive artefact IF the assertion is kept separable from the
   harness (D4's fallback). Start with pages whose params are already reachable.
2. **MVP the labs whose features are NOT Stage-3 territory** — oscillator, voice,
   envelope, output/perception. These touch the registry (D1/D2) but not the mod
   bus.
3. **Hold feedback routing at the lab boundary** until OQ #23 resolves.
4. **Do not hand-place 75 controls** until D1 has an answer, even a partial one.
   This is the only item where waiting is cheaper than building.

## Filed, and what the human decided

Filed in full rather than the D1+D5 subset — the human's call: *"better to work
this all out now."* Their added observation is what became **D0**, and checking
it changed its shape: they suspected FOUNDATIONS had been leaning on gui2 without
realising how incomplete it is. FOUNDATIONS cannot have been — **`gui2` appears
nowhere in their tree.** The real risk is quieter and worth more: a criterion
written about "the GUI" while "the GUI" denotes two diverged things, neither
complete. That is now told.

## Superseded question (kept for the record)

Whether to file this as a brief at all, or to ask only **D1 + D5** now and keep
the rest until re-point is scheduled. The full version is more useful to them but
is also a larger ball to hand a deliberately-slow correspondent, and the
instruction was explicitly not to jump the queue.
