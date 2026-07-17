# SWARM✱ — decisions (append-only)

## ADR-001 · One kernel-agnostic engine — ACCEPTED
SAW mode is the per-partial swarm engine at P=1 with a saw kernel; SPECTRA is the same engine at P>1 with a sine kernel; wavetable kernel reserved (terrain-sibling crossover). Rationale: the three prototypes converged on identical state (phases, vf, couple, R/ψ/σ, slewed K) differing only in kernel and partial count. Consequence: Phase 4 must prove mode parity at P=1.

## ADR-002 · CLAP-native, VST3 via clap-wrapper — ACCEPTED
Carried from prior platform exploration (JUCE alternatives). Revisit only if a Phase 0 host-compat blocker appears.

## ADR-003 · JS prototypes are the reference implementation — ACCEPTED
swarmsaw/swarmspectra/swarmdynamics HTML cores are the spec-in-code. C++ correctness is defined as parity (L0-1) plus trajectory regressions, not as independent reimplementation. The prototypes' headless Node harnesses are the templates for `./verify`. Divergence requires an ADR here.

## ADR-004 · σ-normalized K with squared taper; absolute-K escape hatch — ACCEPTED
K_Hz = 4·K·|K|·σ_measured places the phase transition mid-knob at any detune/law/register (validated). σ floored at 0.05–0.08 Hz. Normalization collapses for identical oscillators, so an absolute-K advanced mode is required for strict-chimera experiments (L0-10 note).

## ADR-005 · Tempo-grid quantization is a detune law — ACCEPTED
Not a subsystem: voices are placed by the distribution, then Hz offsets snap to the nearest grid multiple. First implementation (forced ladder) ignored the detune knob and produced ±23 Hz spreads; quantize-to-grid fixed spread, exactness, and coherence simultaneously (L0-12). Unit is cycles-per-beat.

## ADR-006 · Additive renderer: oscillator bank vs iFFT — OPEN
Phase 0 spike. Bank is simpler and parity-friendly; iFFT scales past ~200 sines and fits control-rate coupling naturally. Decide on measured min-spec headroom at 128 partials × 5 voices × 4 notes.

## ADR-007 · Splay is assigned-slot, not emergent N-th-harmonic coupling — ACCEPTED
N-th-harmonic mean-field coupling rotates at N·f and cannot be held across a 16-sample control tick; it also admits full-sync as a spurious solution. Assigned slots anchored on the center voice (argmin |x|) are stable, tick-holdable, and avoid pitch sag. Splay authority is 3× sync (detune fights slot precision; at 1× the multiplication was only 2.4× baseline vs 9× at 3×). Detune under splay is reframed as "looseness."

## ADR-008 · Consonance gravity is an audio-engine force — ACCEPTED
Runs on per-note f0cur inside the engine (per-render-block pairwise pulls, basin-limited, octave-folded ratio snap), not as MIDI preprocessing. This is the substantive distinction from the Hermode/Pivotuner/Alt-tuner category (see PRIOR-ART) and what makes settling an audible physical event. Note-on resets to ET so the settle is per-chord.

## ADR-009 · Coupling slew coefficients are per-tick at 2756 ticks/s — ACCEPTED (recorded as a trap)
16-sample ticks at 44.1 kHz. τ ≈ 1/(coef × 2756). A 10× mental error here silently destroyed the cascade zipper once. All time-constant math in the C++ port must be expressed in seconds and converted, never hand-tuned per-tick.

## ADR-010 · Spin-up ratifications (2026-07-17) — ACCEPTED
Survey conducted via /spinup; answers in project.manifest.json (provisional pending final ratification). (a) Project/repo name is HYPERSAW; SWARM✱ branding stays open until the Phase 5 naming decision. (b) Architecture rung 2 — single thread + fresh-context verifier/subagents; earned by the parity oracle, fleet not earned (one engine per ADR-001). (c) Model pins per doctrine: lead/critic Opus, implementer Sonnet, verifier Haiku; judgment-bearing DSP/oracle work stays with the lead. (d) Tonality and terrain-sibling intake briefs deferred to Phase 3 / Phase 4 respectively; the default ratio set and stubbed kernel are the visibly-degraded placeholders. (e) Kit docs (SPEC/ACCEPTANCE/ROADMAP/PRIOR-ART/PARKED + this log) adopted as canonical; ACCEPTANCE.md numbers are the oracle contract.

## ADR-011 · swarmsaw_v2.html supersedes swarmsaw.html as the SAW reference — ACCEPTED
Per CHANGE-NOTE-splay-legibility.md (2026-07-17): v2 = v1 + R_N (Daido) order parameter in controlTick + splay-legibility visualization (dual meters, seat rings, formation polygon). Diff-verified additive; no audio-path changes. SPEC §5.6 and ACCEPTANCE L0-3 amended accordingly. R_N is parity-relevant for the C++ port (displayed AND asserted). swarmsaw.html retained for history until the human approves deletion; ADR-003's reference list should be read with this substitution.
