# Research swarm — feedback in synthesis: the field, and what it says about OQ #23

**Date:** 2026-08-15 · **Organ:** lead, synthesising four parallel research agents
· **Requested by:** the human — *"the broader field of possibilities with their
pros and cons, and also possibly reveal some occluded solutions."*

Four angles ran in parallel: analog/modular practice · digital synth
architectures · DSP loop theory · modulation-graph feedback and sideways sources
(analog computers, cybernetics, chaotic oscillators). Each was briefed to flag
SENTIMENT vs TECHNICAL and to say "no evidence found" rather than infer. They
did; the gaps below are theirs, honestly reported.

---

## 1. The finding that applies to the QUESTION, not the options

**Every stable feedback architecture surveyed uses one of exactly two bounding
strategies — and the robust ones use both:**

| strategy | what it guarantees | examples |
|---|---|---|
| **Structurally enforced minimum delay** | the loop is **computable** | Max/MSP (one signal vector, 64 samples) · Bitwig Grid (one block) · Reaktor Core / VCV / Pd (one sample) · Csound (one k-cycle) |
| **Bounded loop gain + smoothing in the loop** | the loop is **stable** | DX7 (power-of-two gain steps + a two-sample average inside the loop) · Karplus-Strong (loop filter gain < 1 at *every* frequency) · FDN (unitary feedback matrix) · analog practice (attenuator/VCA in the loop, limiter as backstop) |

**These are orthogonal, and OQ #23 as posed only settles the first.** All three of
its options — unit delay, fixed evaluation order, iterative settlement — are
answers to *"how do we compute a cycle?"* None of them answers *"what stops it
running away?"* And the survey is unambiguous that the second is where real
systems actually fail.

**The natural experiment is Reaktor.** It gives sample-accurate feedback and
pushes the gain bound onto the user; the documented consequence is overflow to
`+INF/-INF/NaN`, with the official mitigations being *"make sure your filter is
always stable"* and *"add clipping in the feedback loop."* The DX7 took the
opposite path — gain quantised to eight power-of-two steps, so it **cannot**
blow up — and is remembered for the sound, not the failures.

**Recommendation to carry to FOUNDATIONS: #23 needs a second half.** Whatever
delay semantics are ruled, a companion ruling should require every feedback edge
to declare a **bounded gain** and carry **smoothing inside the loop**. A delay
rule alone licenses a computable loop that still destroys the instrument.

## 2. Option (b) is not a peer of (a) and (c)

Graph theory, verified rather than relayed: **a directed graph containing a cycle
has no topological ordering.** So "fixed evaluation order" cannot mean what it
sounds like for a graph with a genuine loop — whatever order is chosen, at least
one edge must read the previous iteration's value, which *is* a unit delay, placed
implicitly by the sort rather than declared.

So (b) resolves to either **forbid cycles** or **become (a) with the delay inserted
wherever the sort happens to break the loop.** It is not a distinct mechanism.

This sharpens FOUNDATIONS' own stated preference — *"the only option whose cost is
visible in the graph itself"* — from a preference into a structural claim: (b)
does not hide its cost, it **pays the identical cost at a location nobody chose.**

## 3. Block-rate is the genuinely disputed part, and the dispute is empirical

FOUNDATIONS recommends unit delay at **block** rate. The field splits by *what
travels on the edge*:

- **Audio-rate feedback at block granularity draws complaints.** Bitwig's Grid
  pins feedback to the buffer (~5.33 ms) and its community calls it a real
  limitation — *"not a substitute for serious building."* Max/MSP enforces one
  signal vector (64 samples) and Pd users go out of their way to defeat it with
  `[block~ 1]` to get true single-sample feedback.
- **Control-rate feedback at cycle granularity draws none.** Csound's
  init-pass/performance-pass convention gives an explicit one-k-cycle lag and is
  documented as normal practice, not a limitation.

**Our modulation is control-rate on a 16-sample tick (~2756 Hz), which puts us on
the uncomplained-about side of that line.** Stated as an inference, not a finding:
we have not measured it, and the honest form of the question is not *"which
option"* but **"what travels on our feedback edges — control or audio?"** If
audio ever does, the block-rate answer needs revisiting.

## 4. Occluded solutions, ranked by how much they are worth to us

### 4.1 A frequency shift inside the loop buys gain margin — and it is cross-confirmed
Berdahl et al. (JASA 2012) measured that a small frequency shift in an acoustic
feedback loop **improves gain margin** — ~3 Hz for ≥3 dB. The mechanism: no single
frequency can re-enter the loop at exactly its own phase, so the runaway
resonance never closes on itself.

**Two independent passes hit this from opposite directions.** The acoustic-control
literature says it works; **Massive X ships a frequency shifter inside its
feedback loop** as a routable element. Almost nobody frames it as a *stabiliser*
— it is sold as a tone colour. That is the definition of occluded: a known,
measured, shipping technique whose actual function is not what it is marketed as.

**Why it matters here specifically: a Kuramoto swarm is already a population of
frequency-shifted copies of itself.** If we ever run audio feedback, the
detuning that defines the instrument may already be doing the job the literature
recommends adding.

### 4.2 Chaos inside an atomic node instead of cycles in the graph — and we already are one
Eurorack's answer to "living modulation" is increasingly **not** graph cycles but
a chaotic ODE integrator as a *source type*: Lorenz, Rössler, Chua (Joranalogue
Orbit 3, Ornaments+Crimes, Pura Belia Fragua). It trades a hard architecture
problem — cycles in a general graph — for a contained one: a deterministic,
seedable integrator with no external wiring and no evaluation-order problem.

**And HYPERSAW is already a coupled nonlinear dynamical system.** The order
parameter, individual phases, and phase-velocity spread are living, deterministic,
seedable, never-quite-repeating signals **already being computed in the audio
path.** Exposing them as modulation sources delivers what people build feedback
loops to get — modulation that feels alive — **with no cycle at all.**

This does not retire #23. It may shrink it: if most of the musical demand is met
without cycles, the ruling can be conservative at little cost.

### 4.3 Unitary feedback matrices — stability by construction
FDN reverbs guarantee stability with an **energy-preserving (unitary) feedback
matrix**: the system cannot blow up regardless of the individual delay lengths.
That is the strongest class of guarantee available — structural, not a runtime
check. Whether a *modulation* matrix could be constrained to a unitary mixing
matrix so cycles are energy-preserving by construction is, as far as the survey
found, **unexplored**. Filed as a speculative transfer, not a recommendation.

### 4.4 Smoothing in the loop is a stabiliser, not a tone control
The DX7 averages the last two feedback values before reapplying them, and the
die-analysis is explicit that this is anti-oscillation, not timbre. Karplus-Strong
generalises it: the loop filter must be **sub-unity at every frequency**. Analog
practice reaches the same place with a lowpass or EQ in the loop.

**This is a direct, uncomfortable caution about our own inertia decision.** A
spring with ζ < 1 does not smooth — it **adds gain at its resonant frequency**.
Route inertia with low damping on a feedback edge is therefore not a stabiliser
but a destabiliser, and the filed brief's caution should be sharpened from *"its
stability is not obvious"* to **"an underdamped spring on a feedback edge adds
loop gain and should be prohibited or clamped to ζ ≥ 1."**

## 5. The design-target tension nobody resolves

The two literatures want different things, and a design that hears only one will
be wrong in a way its own tests cannot see:

- **DSP theory optimises for guaranteed stability.**
- **Performance practice optimises for a wide, controllable region NEAR
  instability.** Gain staged up from zero *"until you can just hear it"*; the
  resonance knob prized for approaching self-oscillation without committing; the
  TB-303's diode ladder valued because it **never quite** oscillates; the
  no-input desk described as *"the instrument plays you just as much as you play
  it."*

So the target is not "prevent instability" but **"make the approach to
instability wide and playable."** Two concrete corollaries the survey supports:
put a **controllable attenuator inside the loop** so loop gain is a modulation
target rather than a fixed property (analog practice notes filter resonance
already *is* exactly this), and keep the **limiter as a safety backstop distinct
from the musical control** — never let the thing that saves you be the thing you
play.

## 6. Honest gaps, as reported by the agents

- **No evidence found** that any commercial modulation matrix documents refusing
  cycles with an error; where prohibition exists it appears implicit — the UI
  simply does not expose the routing.
- **No evidence found** for the Oberheim Matrix-12 handling cycles either way.
- **Unverified**: how Phase Plant, Vital, Pigments and Serum break zero-delay
  cycles internally. A feedback *amount* parameter exists in each; the mechanism
  was not documented in what the agents could reach.
- **Inference, not evidence**: that feedback patches fail to recall identically.
  Structurally plausible from chaotic sensitivity to initial conditions, but no
  practitioner account saying so was found. Worth measuring on our own seeded
  system rather than assuming.

## Sources

Consolidated from four passes; the ones load-bearing above.

- [A Review of Methods for Resolving Delay-Free Loops — Chowdhury, CCRMA](https://ccrma.stanford.edu/~jatin/papers/DelayFreeLoops.pdf)
- [The Art of VA Filter Design — Zavalishin](https://www.native-instruments.com/fileadmin/ni_media/downloads/pdf/VAFilterDesign_1.1.1.pdf)
- [Yamaha DX7 chip reverse-engineering, part 4 — Ken Shirriff](http://www.righto.com/2021/12/yamaha-dx7-chip-reverse-engineering.html)
- [Feedback control of acoustic musical instruments — Berdahl, Smith, Niemeyer, JASA 2012](https://pubs.aip.org/asa/jasa/article/131/1/963/823554/Feedback-control-of-acoustic-musical-instruments)
- [Applications of Feedback Control to Musical Instrument Design — Berdahl](https://www.cct.lsu.edu/~eberdahl/Papers/berdahl-thesis-augmented.pdf)
- [Bitwig Userguide — Grid Devices and Thru Signals](https://www.bitwig.com/userguide/latest/special_connections/)
- [The Grid: How do I do a feedback loop? — KVR](https://www.kvraudio.com/forum/viewtopic.php?t=537173)
- [Max Cookbook — Delay effect with feedback](https://music.arts.uci.edu/dobrian/maxcookbook/delay-effect-feedback)
- [Reaktor 6 Feedback Loop — KVR](https://www.kvraudio.com/forum/viewtopic.php?p=6511381)
- [Massive X Manual — Overview / Routing](https://native-instruments.com/ni-tech-manuals/massive-x-manual/en/overview-of-massive-x)
- [Karplus–Strong string synthesis — Wikipedia](https://en.wikipedia.org/wiki/Karplus%E2%80%93Strong_string_synthesis)
- [Digital Waveguide Interpretation of Karplus-Strong — CCRMA](https://ccrma.stanford.edu/~jos/Mohonk05/Digital_Waveguide_Interpretation_Karplus_Strong.html)
- [FDN Reverberation — DSPRelated / Smith](https://www.dsprelated.com/freebooks/pasp/FDN_Reverberation.html)
- [Allpass Feedback Delay Networks — arXiv](https://arxiv.org/pdf/2007.07337)
- [Delay And Feedback — Csound FLOSS Manual](https://flossmanual.csound.com/sound-modification/delay-and-feedback)
- [Playing with Feedback: Unpredictability, Immediacy, and Entangled Agency in the No-input Mixing Desk — CHI 2023](https://dl.acm.org/doi/10.1145/3544548.3580662)
- [No-input is my instrument — Lee Tusman](https://leetusman.com/nosebook/no-input)
- [Feedback patching: what are your uses? — MOD WIGGLER](https://www.modwiggler.com/forum/viewtopic.php?t=262708)
- [Joranalogue Orbit 3 chaos oscillator](https://www.signalsounds.com/joranalogue-orbit-3-eurorack-chaos-oscillator-module)
- [Circuits of Chaos: Lorenz, Chua, Rössler — Electronic Design](https://www.electronicdesign.com/technologies/analog/article/55131908/circuits-of-chaos-building-lorenz-chua-and-rossler-strange-attractors)
- [Barkhausen stability criterion — Wikipedia](https://en.wikipedia.org/wiki/Barkhausen_stability_criterion)
- [Denormal Numbers in Floating Point Signal Processing — de Soras](https://ldesoras.fr/doc/articles/denormal-en.pdf)
