# SWARM✱ — parked register (append-only; nothing gets silently dropped)

Format: idea — why parked — revisit trigger.

1. **Drawn-by-hand distribution curve** — pure GUI work; the engine already consumes arbitrary x[i]. Revisit: Phase 5 GUI build (this is a v1 feature, parked only in *sequence*, per explicit design intent).
2. **Distribution morphing while notes sound** — offsets gliding under coupling is a real gesture (bimodal→uniform under sync), needs glide semantics + preset interpolation design. Revisit: Phase 5 mod matrix.
3. **Clustered-pairs distribution** — trivial engine-side (dyad offsets); listed in SPEC Layer 1; implementation slotted Phase 2. Parked here only until then so it can't fall off.
4. **Stuart–Landau oscillators (amplitude dynamics)** — brings amplitude death: a third erasure mechanism distinct from splay cancellation and R→amp. Doubles per-voice state. Revisit: post-v1, or if a "swarm FX" sibling product emerges.
5. **Swarm-of-swarms recursion (clouds of clouds)** — conceptually clean under ADR-001, combinatorially explosive in UI. Revisit: post-v1.
6. **Strict chimeras via absolute-K + identical oscillators** — absolute-K mode ships (ADR-004); the dedicated exploration preset pack is parked. Revisit: Phase 5 demo patches.
7. **Cross-note *phase* coupling (beyond ratio gravity)** — phase-locking distinct notes' fundamentals at simple ratios (true harmonic fusion, chord-as-one-timbre). Needs sample-accurate cross-swarm coupling. Revisit: after Phase 3 gravity ships and its CPU cost is known.
8. **Designed mono fold-down** — optimize pan/phase assignment so mono sum is a deliberate alternate timbre. Revisit: Phase 2 (E-5 findings will motivate or dismiss).
9. **Tempo-synced coupling** (K itself pulsing on the grid, distinct from tempo-grid detune law) — natural once K is a mod destination. Revisit: Phase 5 mod matrix.
10. **Terrain-sibling wavetable as swarm kernel** — the "every preset is somewhere" crossover; kernel abstraction lands Phase 4, integration after. Revisit: Phase 4 exit.
11. **Weather data as coupling modulation** — the terrain sibling's open design question, doubly speculative here. Revisit: only if it ships there first.
12. **Per-voice spectral shading** (edge voices darker/thinner; saw→tri by |offset|) — spec'd as straightforward, no prototype needed; slotted Phase 2 stretch. Parked so it's tracked.
13. **Tonality ratio priors for gravity** (context-weighted basins, higher-limit sets) — after Phase 3 baseline gravity; integration via Tonality MCP/exports per cross-repo policy ("writes stay home").
14. **iFFT-mode partial ceiling past 128** — contingent on ADR-006 outcome.
15. **Unison octave layer** (human request 2026-07-18) — spread part of the swarm ±1 octave for classic huge-stack unison. OPEN DESIGN QUESTIONS for the prototype session (reference-first, like clustered-pairs): which voices shift; do octave-shifted voices join the first-harmonic coupling graph (1:2 phase-locking is not what mean-field sin() coupling does); how does σ-normalization treat a bimodal ~octave spread (measured σ would dwarf the intra-band spread and skew K scaling). Note: the bimodal distribution's ±100¢ ceiling cannot reach an octave — this is a distinct feature, not a bimodal preset. Revisit: when the design session answers, port with parity.
