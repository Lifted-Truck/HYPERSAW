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

## ADR-012 · Reference timeline consolidated; change notes archived under docs/ — ACCEPTED
The definitive timeline of the SAW reference, for any agent reconstructing it: (1) 2026-07-17 morning — `swarmsaw.html` v1 arrives with the spec kit; (2) mid-spin-up — `swarmsaw_v2.html` + the splay-legibility change note arrive (v2 = v1 + R_N readout + splay visualization; diff-verified additive), amendments folded into SPEC §5.6 / ACCEPTANCE L0-3 (ADR-011); (3) post-ratification cleanup — v1 deleted (human-approved), **v2 renamed to `swarmsaw.html`**, so the ADR-003 reference names hold again and both v1 and v2 remain recoverable in git history (v1 and the pre-rename v2 live in the initial commit). Policy from here: reference-implementation updates land as in-repo edits under version control with an ADR; externally-authored change notes are archived at `docs/change-notes/YYYY-MM-DD-<topic>.md`, folded into SPEC/ACCEPTANCE in the same change, never left as loose root files.

## ADR-013 · GUI is an early priority, not a Phase 5 deliverable — ACCEPTED
Human direction at ratification (2026-07-17): a GUI matching the prototypes' visual design as closely as possible is an early priority. ROADMAP amended: Phase 2 now includes GUI v1 (phase circle with dual R₁/R_N meters, seat rings, formation polygon, XY pad, live readouts — the SPEC §5.6 contract, styled from the prototype design language); Phase 5 keeps the full face (phase carpet, partial strips, mod-matrix UI, MPE surface). The prototypes' CSS/canvas code is the design reference — extract tokens (palette, meter treatments, layout) rather than reinventing. Consequence for Phase 0: the CLAP skeleton must pick the GUI stack early enough that Phase 2 can ship a real GUI; renderer spike (ADR-006) and GUI-stack choice are both Phase 0 exit criteria now.

## ADR-014 · Public repo: private siblings are aliased in tracked files — ACCEPTED
Pre-push sweep (2026-07-17) found one referenced sibling project whose repo is private; its name appeared in the kit docs. Human ruled scrub-before-push: all tracked-file references use a neutral alias ("terrain sibling"), the unpushed history was rewritten so the name never reaches the public remote, and the alias→name map lives in PRIVATE-NOTES.md (untracked, gitignored). Standing rule: never write a private sibling's real name into a tracked file; public siblings (e.g. Tonality) are named directly. The verify leak gate covers paths/identity; this alias rule covers project names — both are push-blocking.

## ADR-015 · Daido multi-pole coupling verified; ADR-007 partially amended — ACCEPTED
*(Arrived 2026-07-17 as an external ADR drop numbered 010; renumbered here — this log was already at ADR-014. Claims independently re-verified headless on the v2 dynamics core before merging: (a),(d),(e) reproduced exactly, (c) reproduced to three decimals, (b) reproduced qualitatively.)*
Measured in the dynamics lab (mean-field path, `poles q` select, q-harmonic mean field sin(ψ_q − qθ − α), tick-held):
(a) **q-cluster formation**: q=2 → R₂ = 0.97 across seeds with R₁ low; q=3 → R₃ = 0.97; clusters lock hard.
(b) **Spontaneous symmetry breaking**: cluster population splits are seed-dependent (measured 8/16, 12/12, 8/16, 14/10, 11/13 at identical parameters) — demographics are preset character.
(c) **Split is a timbre dimension**: 2f0 energy constant (~0.080 projection) across seeds while residual f0 tracks cluster imbalance (12/12 → 0.006; max imbalance → up to 0.064), i.e. the split ratio is a fundamental-vs-octave mix knob the seed rolls.
(d) **Bistability**: identical parameters, aligned start → full sync persists (R₁ = 0.99); scattered start → clusters. Two coexisting attractors, confirming hysteresis potential.
(e) **ADR-007 amendment**: the claim that q-harmonic mean-field coupling cannot be tick-held was WRONG — ψ_q and qθ both advance at ~q·f, so their difference drifts at q·detune (slow); tick-holding locks cleanly at q ≤ 4. ADR-007's *other* rationale stands: q-coupling admits full sync as a coexisting solution (that is now (d), a feature), so assigned-slot splay remains the correct mechanism where a deterministic multiplication outcome is required. Daido coupling ships alongside it as the emergent sibling.
Meter note: under q-coupling, sync shows R₁ AND R_q high; clusters show R₁ low + R_q high (unlike SWARMSAW's R_N, where N=7 magnification keeps sync-side R_N low).
Spec placement: `poles q` joins Layer 2 (coupling); generalizes the drawn-coupling-function direction (Daido harmonics ⊂ arbitrary Γ(Δθ)).

## ADR-016 · Coupling × beat-grid interaction is physics; the UI must disclose it — ACCEPTED
*(Arrived in the same drop as external ADR-011; renumbered. Suppression direction re-verified headless — locking a populated grid suppresses grid-rate envelope power; the ~80× magnitude is the design session's measurement and is harness-dependent (a coarser independent method gave ~5× at detune 0.6): treat the direction as blocking, the magnitude as reference.)*
Measured: with the tempo-grid law active and populated (3–7 rungs, exact multiples verified), raising K to lock (R = 0.97) suppresses envelope power at the grid rate ~80× (6.0e-4 → 7.7e-6); same under Daido q=2. Locked voices share one frequency → no gaps → no beats. This is the intended "K loosens/overrides the grid" negotiation, not a defect — but it is invisible without instrumentation and was reported as a regression in testing. Requirement: any surface exposing the tempo-grid law MUST show live grid status (unit, rungs occupied, and a lock warning when coherence is high) so the populated-but-overridden state is legible. Reference implementation: `beatinfo` readout in swarmdynamics.html. Hierarchy note for presets: grid-forward patches want K subcritical; document K ≲ 1×σ as the compatible range. Warning condition as originally stated here is superseded by ADR-017.

## ADR-017 · Amendment to ADR-016: lock warnings require cause AND state — ACCEPTED
*(Arrived in the same drop as external ADR-012; renumbered, internal reference updated from "ADR-011" to ADR-016. Falsifier scenario re-verified: retrigger zeroes phases, so R = 1.00 at note-on with Ksm = 0.000 by construction; the corrected gate is implemented verbatim in swarmdynamics.html drawBeatInfo.)*
ADR-016 specified the grid-suppression warning condition as coherence alone (R or R_q > ~0.8). Falsified in testing: with K = 0 and retriggered phases, R reads 1.00 with Ksm = 0.000 — the swarm is coherent from initial conditions, not from coupling, and no suppression is occurring (nothing holds the voices; the grid reasserts as they disperse). Corrected condition, verified both directions: warning fires iff coupling is engaged (Ksm > ~0.05 Hz) AND coherence is high (R, R_q, or both cluster orders > 0.8). General principle for all such displays, here and in the VST: order parameters measure state, not cause — any UI claim about what the coupling is *doing* must gate on the coupling being nonzero. Reference implementation updated in swarmdynamics.html; UI additionally reserves fixed height for the warning line so its appearance cannot reflow the layout.
