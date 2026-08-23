# Which engines make it into horde — the state of each, for the decision

**Date:** 2026-08-23 · **Why:** the human deferred B36 (SPECTRA unreachable in the
shipped GUI) with *"I'm leaving spectra unreachable for now while we work out
which engines make it into the final version."* That makes the engine roster the
blocking question rather than the GUI work, and several other items queue behind
it. This is the evidence, not a recommendation dressed as one — the roster is a
product decision and it is the human's.

## The board

Every row below is measured from this repo at `./verify full` EXIT=0 today, not
recalled.

| engine | core ported | own gate | in the audio path | reachable in gui2 (shipped) | blocker to shipping |
|---|---|---|---|---|---|
| **HYPERSAW** (`swarm_core.h`) | yes | `parity_check` 156/156 | **yes**, ×2 oscillators | yes, fully | none — it *is* the instrument |
| **SPECTRA** (`spectra_core.h`) | yes | `spectra_check` GREEN, worst rms **0** | **yes** (engine id 43) | **no** — all 17 of its params absent | GUI only (B36) |
| **Swarmalator** (`swarmalator_core.h`) | yes | `swarmalator_check` GREEN, worst rms **0** | **no** — 0 references in the shell | no | shell integration + a decision that it earns a slot |
| **CANTO / formant** | no — prototype only | none (candidate: unseeded RNG) | no | no | RNG seeding → goldens → port → GUI; **and** an open question about the sound (ADR-091 A4) |
| **WARP / FX-C** | no — prototype only | none (candidate: unseeded RNG) | no | no | not an engine — a shared post-stage (ADR-092); belongs to the FX roster, not this one |

## What each row actually costs from here

**SPECTRA — cheapest possible "yes".** It is already in the audio path with a
bit-exact port (worst parity RMS **0**, not merely inside epsilon) and its own
gate. The only thing standing between a user and it is one cluster of controls on
gui2's OSC page. If the answer is "SPECTRA stays", the work is B36 and it is
small. If the answer is "SPECTRA goes", that is a much larger deletion — engine
id 43, the core, the gate, the goldens, and a state-compatibility story for any
patch that stored `engine=1`.

**Swarmalator — the quiet one.** Fully ported and gated to a bit-exact standard,
and referenced **zero** times in the shell. It has been carried as proven-but-
unshipped since ADR-048. That is either a free engine waiting for a slot, or dead
weight the repo has been maintaining a gate for; it has never been put to the
question.

**CANTO — the expensive one, and the least certain.** Everything before shipping
is still ahead of it: seed the masking RNG, generate goldens, port to C++, build
a GUI surface. Yesterday's work also left a genuine doubt on the *sound* rather
than the engineering — the human's own words were *"not terribly impressed with
how this sounds as a polyphonic instrument"*, and its polyphony was reverted. It
is the only candidate where the open question is "is this good" rather than "is
this finished".

**WARP — miscategorised if it appears on this list at all.** ADR-092 already
ruled it is not a fourth engine but the prototype for FX-C, a shared post-stage
every source hands off to. It should be decided on the FX roster, with the
granular sibling / OTT / redux / glitch tier, not here. Noting it so the
distinction survives the discussion.

## The question under the question

The roster decision is really about what horde *is*. Two coherent shapes:

- **One deep engine.** HYPERSAW alone (×2 oscillators), with everything else
  expressed as FX and modulation. The swarm taken seriously as physics, one idea
  pursued to its end. Fewest surfaces, strongest identity, and the README's
  elevator pitch already reads this way.
- **A family of dynamical engines.** The §Domain framing — HYPERSAW, SPECTRA,
  and new members joining over time. More surface to design, document, test and
  explain, and every engine added multiplies the GUI and preset story.

These are not equally cheap, and the difference is not in the DSP: it is in the
interface and the parameter surface, which is where the last two weeks of work
have actually gone.

## What is blocked on this

- **B36** (SPECTRA in gui2) — explicitly deferred pending this.
- **ADR-091 A4** — whether CANTO continues at all.
- **Swarmalator** — has no roadmap item to be blocked on, which is itself the
  finding: it is proven, gated, and nobody has decided anything about it.
- The **engine selector's** shape, and whether per-oscillator engine choice
  (ids 43 / 1043) is a feature or an accident of the stride scheme.

## Honest limits of this document

It ranks nothing and recommends nothing, because the inputs that decide it —
what the instrument should feel like, what its identity is worth defending, how
much surface is too much — are not in the repo and cannot be measured from it.
What is here is the part that *can* be: what exists, what it costs, and what is
waiting on the answer.
