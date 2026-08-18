# Proposal — engine SLOTS, and chords as OSCILLATOR COPIES

**Status: PROPOSAL, not a decision.** Written 2026-08-18 by the lead organ at the human's
request, from their two observations in session: *"I expect the chords to use copies of the
OSC itself, not split the voices within the same swarm"* and *"maybe instead of allowing each
oscillator to decide which engine it uses, there could be one slot per engine but each can be
multiplied via chords or a scalar; this would make the morph more straightforward as well.
Probably worth a discussion."*

Nothing here is built or started. It exists so the reasoning is on record **before** an ADR,
because the change it describes is cheap now and expensive later, and that asymmetry is the
main thing the reader needs to weigh.

---

## Part 1 — Chords are COPIES of the oscillator, not a partition of its voices

**Recommended: adopt.** The alternative is not merely worse, it is self-defeating.

The roadmap entry of 2026-08-18 posed SEAT (partition the existing N swarm voices across
chord degrees) against STACK (a bank per degree) and deferred the choice to the ear. On the
numbers the choice is already made: **7 voices across a triad is 2.33 voices per degree.**
That is not three supersaws, it is a detuned triad. The swarm's identity is having enough
voices to lock, drift and beat against one another; partitioning starves every degree of
precisely the quantity the engine is built from. SEAT does not underperform STACK, it deletes
the engine and leaves the chord.

**It also dissolves a danger the same entry raised.** That entry warned that "independent"
glide might be false in the ear, because gliding voices are moving targets for the Kuramoto
pull. Under copies, each chord note is its own swarm coupling **internally**; glide
independence is then true *by construction* rather than a property we hope survives coupling.
The worry was real for SEAT and evaporates under STACK.

### The question copies create is better than the one they answer

If each chord note is its own swarm, **do the swarms couple to each other?**

- **Independent** — three supersaws. Predictable, and what a chord is normally expected to be.
- **Cross-coupled** — chord notes pull on one another, and as K rises a chord can **melt
  toward a cluster**, losing its own harmony as coupling wins. Nobody else can ship this; it
  is the instrument's thesis (a coupled-oscillator synth) applied to harmony rather than to
  detune.

**Recommendation: cross-copy coupling is an explicit parameter, defaulting to zero.** The
normal case is then a chord, and the extreme case is the signature — and neither is reachable
by accident.

**The lab must answer one question before any of this hardens:** does a cross-coupled chord
sound like a musical behaviour, or does it just sound broken? If it is real, the copies
architecture has a second justification beyond tidiness. If it is not, copies are still
right, for the plain reason above.

---

## Part 2 — One slot per ENGINE, multiplied, instead of per-oscillator engine choice

**Recommended in principle; the cost below is real and must be paid deliberately.**

### What today actually costs, measured

Engine choice is a per-oscillator enum — `src/hypersaw_clap.cpp:144`,
`{43, "engine", "Engine", 0, 1, 0, true, kEngineLabels}` — and the presentation table splits
**29 global / 76 osc1 / 76 osc2**. Of each oscillator's 76, the **Spectra group is 24 rows**.

So roughly **a third of every oscillator's parameter surface is inert at any moment**, and
across the pair that is **48 parameters that are reachable, automatable, modulatable,
morphable and meaningless in the current mode**. They are not harmless: every one of them can
be assigned a modulation route or a morph corner value that silently does nothing, which is
the "superset with inert defaults" blindness `L0031` records — parity cannot see it, because
the reference never renders the inert half either.

### The morph argument is precedent, not speculation

This project has already decided this exact question one layer down. **ADR-088** chose a
dense crosspoint matrix over every sparse routing scheme because *"a corner interpolates
values"*, and in a dense table a coefficient of 0 **is** "not connected", so connecting and
disconnecting are one continuous motion — whereas every sparse scheme stores topology as
discrete structure, making a repatch a **hard cut**.

**Engine-choice-per-oscillator is discrete structure of exactly that kind.** If corner A has
osc1 = HYPERSAW and corner B has osc1 = SPECTRA, a morph between them must interpolate
between a Kuramoto swarm and a per-partial engine. That is undefined, so it can only snap.
One slot per engine makes every corner *the same engines in the same slots*, and every morph
a value interpolation — which is the property ADR-088 was willing to restructure the routing
model to obtain.

**Stated honestly: this is a latent problem, not a live one.** MORPH is still a disabled tab
(`src/gui/gui2.html:587`), so nothing is broken today. The claim is that the current shape
guarantees a hard cut *when* morph is built, and that the corner vocabulary (A GLASS · B GRIT
· C HOLLOW · D BLOOM) is already fixed and expects to interpolate.

### What the change costs

**The real loss is two instances of one engine set differently** — a bright wide supersaw
layered under a narrow sub-ish one, which is an ordinary and good patch idiom. A plain scalar
multiplier destroys it, because copies would differ only in pitch.

**The fix keeps both, and is on-brand.** Let the multiplier carry **per-copy offsets**: a
spread applied across copies over a chosen parameter set, not pitch alone. Copy 2 is copy 1
with +12 detune and −20% width. That is the swarm concept applied one level up — **a swarm of
swarms** — so it preserves layering, keeps morph as pure value interpolation, and reuses a
mental model the instrument already teaches its user.

### The constraint that decides the timing

**CLAP parameter ids are append-only by specification** — `libs/clap/include/clap/ext/params.h:212`,
*"Stable parameter identifier, it must never change"*, recorded in ADR-088 §4. Restructuring
the parameter space therefore means **new ids with the old ones abandoned**, not renames,
plus a state-compatibility break for any saved patch.

Right now that costs almost nothing: `kNumOsc == 2`, no presets have shipped, and the
id space was deliberately blocked with room to spare (`kMaxOsc == 2`, *"2000-2999 stays free
for a third"*, `src/hypersaw_clap.cpp:297`). After presets exist, or after a third engine has
been added under the current shape, it costs a migration. **The asymmetry is the argument for
deciding soon, and it is the only part of this proposal that is time-sensitive.**

---

## What is NOT proposed

- No change tonight. This touches the parameter model, state, morph and the shell at once,
  and the current push is to make every feature testable — pulling the parameter surface out
  from under that is the wrong order.
- No claim that SPECTRA-in-a-slot is a drop-in. Engine surfaces differ; what a "copy" means
  for a per-partial engine is a lab question, not an assumption.
- No decision on how many copies, or whether the multiplier is chord-driven, scalar, or both.

## Recommended sequencing

1. **Chord-copy lab** (already roadmapped): SEAT is ruled out on the arithmetic above, so the
   lab's job narrows to cross-copy coupling — is a melting chord musical? — and to
   independent glide, now expected to work by construction.
2. **ADR on engine slots**, written against whatever that lab shows about copies, since a
   copy is the same mechanism the multiplier would use.
3. **Only then** the id restructure, as one deliberate act with the state break named in
   advance.

## Open questions for the human

- **Cross-copy coupling default:** zero (a chord is a chord) as recommended here, or a small
  non-zero so the instrument's character is present without being sought?
- **Per-copy offsets:** which parameter set may be spread across copies? Everything is
  powerful and unshippable as a UI; detune/width/level is the obvious minimum.
- **Does SPECTRA keep a slot at all** in the first version, or does the restructure land with
  HYPERSAW alone and SPECTRA folded in when CANTO arrives?
