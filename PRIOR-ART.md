# SWARM✱ — prior art & positioning

Verified against current products, July 2026. Purpose: precise novelty claims, honest debts, and one risk flag.

## 1. Coupled-oscillator synthesis (the core)

**No shipping product.** Kuramoto/Sakaguchi dynamics are ubiquitous in mathematics, neuroscience, power-grid engineering, and academic demos (MATLAB Central, Wolfram Demonstrations, the KurSL thesis line), and absent from commercial synthesizers. Sethares' TransFormSynth is the nearest academic cousin and inverts our approach (fixed fundamentals, manipulated partials). The K-as-phase-transition instrument, splay/harmonic-multiplication, interference gating, inertia, topology selection, and phase-lag control have no production analog found. **This is the headline novelty; claim it plainly** — but note the honest qualifier: Kuramoto *audio synthesis* exists as a research lineage (Lem/Kuroscillator et al., §6). The precise claim is "no shipping **product**," and defensive publication should cite that lineage, not ignore it.

## 2. Unison detune architecture

- **Xfer Serum 2** is the nearest commercial neighbor and the correction that sharpened this section: it ships selectable **unison tuning modes** — linear (classic default) plus harmonic, ratio, semitone, and step, with an inverted-detune option, alongside the long-standing Stack (octave/fifth layering). These are *pitch-placement modes*: rules for where voices sit.
- **NI Super*Saw** (with AG Cook, July 2026): dual stacked-saw banks, X/Y state morphing, chord/scale quantize with off-grid deviation, per-voice glide/offset. Performance-morphing angle; fixed detune recipe.
- **Roland JP-8000 lineage**: the canonical fixed spacing curve (Szabo), reused implicitly industry-wide.

**Our position:** we factor what Serum conflates. *Distribution* (statistical shape of placement: even/JP/Gaussian/Cauchy/bimodal/pairs/drawn) and *law* (offset→Hz mapping: cents/Hz/ERB/tempo-grid) are orthogonal menus, and neither axis exists elsewhere in statistical (seeded Gaussian/Cauchy→Lorentzian lineshape), perceptual (ERB-flat), or metrical (tempo-grid) form. Serum 2 helps the case: it proves "tuning mode as a menu" is an idiom users already read; we extend the idiom rather than invent it. And no product couples the voices at all.

## 3. Adaptive just intonation (consonance gravity's category)

Mature category — cite it, don't pretend otherwise:

- **Hermode Tuning** — embedded in Logic/Cubase for ~two decades; dynamically retunes fifths/thirds in real time; octave-based.
- **Pivotuner** (VST3/AU MIDI effect) — adaptive pure intonation around a dynamic tuning center, with ET↔JI interpolation (functionally a "gravity amount").
- **Alt-tuner** (TallKite) — explicitly frames itself as the string-quartet/barbershop technique applied to keyboards in real time; 7-limit and beyond.
- **Kontakt Dynamic Pure Tuning**, **Groven system** (historical), Justonic, Mutabor.

**The distinction (ADR-008):** every one of these is a MIDI-domain rule engine — compute target pitches, rewrite notes or emit pitch-bends upstream of the synth. Consonance gravity is a *force inside the audio engine*: a basin, a strength, a settling time, decelerating beats as an audible event, and — the recursion that is the actual novelty — the identical sin(error) coupling law that runs the unison swarm, operating one level up. Novelty claim: not "adaptive JI" but "adaptive JI as emergent behavior of the same dynamical system that generates the timbre."

## 4. Per-partial architecture

**NI Razor** proves the additive per-partial substrate commercially (and sits in Cook's own palette — good lineage to cite). Nothing in Razor or elsewhere puts a coupled *cloud* on each partial, cascades lock up the series, or erases partials by splay interference.

## 5. Positioning paragraph (for README/marketing, keep accurate)

The ingredients that resemble parts of SWARM✱ exist as separate mature categories — adaptive-JI tuners (Hermode, Pivotuner, Alt-tuner), additive resynthesis (Razor), and configurable unison (Serum 2's tuning modes, NI Super*Saw's morphing) — but no shipping instrument generates timbre, tuning, and performance gesture from one coupled-oscillator dynamical system. SWARM✱ is not a thicker supersaw; it is the supersaw's physics, exposed and played.

## 6. Coupled-oscillator synthesis research lineage (added 2026-07-19)

Section 1's "no shipping *product*" holds — and is sharpened here. There is a
real **research** lineage for Kuramoto audio synthesis, and honest positioning
cites it generously rather than claiming an unqualified first.

**The lineage (user-sourced).** Lem & Orlarey, *"Kuroscillator: A Max-MSP
Object for Sound Synthesis using Coupled-Oscillator Networks"* (CMMR 2019),
behind it a sustained program — Lem, *"Sound in Multiples"* (SMC 2019); Lem,
*"The Collective Rhythms Toolbox"* (SMC 2022); Lem, CCRMA PhD (2022) — with
antecedents in Collins, *"Errant Sound Synthesis"* (ICMC 2008) and Lambert's
stigmergic oscillator synchronization (ICMC 2012).

**What it establishes as prior art:**
- Kuramoto ensembles for real-time audio synthesis, including the mean-field
  order-parameter **O(N) reduction** (Faust, 2019) — the same optimization this
  project uses.
- Phase-coupled additive sine banks (Kuroscillator-audio, N ≤ 30).
- Coupled **trigger clocks** for rhythm generation (Kuroscillator-rhythm) and
  the *"rhythm-timbre spectrum"* (Lem 2022) — a prior articulation of the
  cloud↔tone continuum at trigger rates. (Prior art for a *possible future*
  grain/rhythm engine — **not in the current instrument**; see PARKED #18.)
- Chimera states and local/global coupling topologies in a musical context.

**Revised novelty posture — honest, and stronger.** "No Kuramoto *products*
exist" stands, stated precisely: no instrument-grade or commercial
implementation; the research implementations are Max externals and toolboxes
aimed at drone/experimental practice. This project's distinct contributions are
the mechanisms and instrument architecture **that ship in the app**: bipolar K
with assigned-slot splay (harmonic multiplication / interference erasure),
σ-normalized coupling, the distribution × detune-law factorization (incl.
ERB-flat and tempo-grid), the gravity family and its **implemented** attractor
sets (harmonics, just ratios, beat divisions, period multiples), Daido poles
and Sakaguchi/topology as performance controls, the effects line on the shared
force core, the equilibrium/stability laws with Layer-0 guards, R→timbre
routing, onset-lock/dissolve, seeded determinism, and the polyphonic instrument
form itself. **Defensive publication should cite this lineage explicitly** —
contributing named, measured mechanisms to an existing research conversation is
a stronger and more honest position than an unsupported first-ness claim.

**Triage note (2026-07-19).** The source delta additionally listed an *Arnold
tongue / Arnold-layer modulation surface*, a *sample-onset gravity attractor*,
and a *built grain engine* among the "distinct contributions." None are in the
current instrument (zero references in SPEC / ROADMAP / DECISIONS), and the
Arnold surface is an undecided exploration from parallel research — all three
are **excluded here** so this document never claims features that do not ship.
Mirollo–Strogatz pulse coupling is parked (#18) as the natural substrate *if* a
grain-trigger engine is ever built.

## 7. Risk flag — freedom to operate

The adaptive-intonation space carries US patents on real-time intonation methods (e.g., 5,501,130 Gannon; 7,807,908 automatic real-time variable performance intonation), plus the Hermode commercial lineage. Our mechanism differs in kind (audio-domain coupling force vs computed-target MIDI rules), and several patents in the area are old enough to have expired — but if consonance gravity is a headline feature of a commercial release, commission a real prior-art/FTO review at Phase 3. This file is an engineering positioning document, not legal advice.
