# SWARM✱ — prior art & positioning

Verified against current products, July 2026. Purpose: precise novelty claims, honest debts, and one risk flag.

## 1. Coupled-oscillator synthesis (the core)

**No shipping product.** Kuramoto/Sakaguchi dynamics are ubiquitous in mathematics, neuroscience, power-grid engineering, and academic demos (MATLAB Central, Wolfram Demonstrations, the KurSL thesis line), and absent from commercial synthesizers. Sethares' TransFormSynth is the nearest academic cousin and inverts our approach (fixed fundamentals, manipulated partials). The K-as-phase-transition instrument, splay/harmonic-multiplication, interference gating, inertia, topology selection, and phase-lag control have no production analog found. **This is the headline novelty; claim it plainly.**

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

## 6. Risk flag — freedom to operate

The adaptive-intonation space carries US patents on real-time intonation methods (e.g., 5,501,130 Gannon; 7,807,908 automatic real-time variable performance intonation), plus the Hermode commercial lineage. Our mechanism differs in kind (audio-domain coupling force vs computed-target MIDI rules), and several patents in the area are old enough to have expired — but if consonance gravity is a headline feature of a commercial release, commission a real prior-art/FTO review at Phase 3. This file is an engineering positioning document, not legal advice.
