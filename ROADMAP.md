# SWARM✱ — roadmap (phase-gated)

Gates are blocking. "Green" = `./verify fast` passes + phase acceptance subset + trace written. Passing ≠ done; done = green + acceptance criteria + DECISIONS/trace updated.

**Status (2026-07-18):** Phase 1 GATE CLOSED (PR #2 merged; protocol findings + free-row erratum ratified with the merge, erratum applied to ACCEPTANCE). Phase 2 in progress: SwarmCore is live in the plugin — placeholder sine replaced, 18-param CLAP surface at prototype ranges (dissolve in seconds, driftDepth in cents), versioned key=value state; pluginval strictness 10 SUCCESS, auval SUCCEEDED post-integration. Phase 2 remaining: tempo-grid law port (needs host tempo; L0-12), bimodal/clustered-pairs distributions (OPEN QUESTION — SPEC lists them, the reference doesn't implement them: extending the reference is a spec change needing a human ruling), GUI v1 + dev state button, webview smoke test, Layer-E 1/2/5 sign-off.

*(Historical status, 2026-07-17 evening:)* Phase 0 largely complete — skeleton builds (CLAP + VST3 + AUv2 via clap-wrapper, pinned submodules), pluginval SUCCESS at strictness 10 (gate asks ≥5), auval SUCCEEDED, all three formats installed locally with intact codesign seals; ADR-006 spike run (bank 66× / iFFT 216× realtime at 2560 osc on M3) with close proposed as ADR-018 (bank); GUI stack proposed as ADR-019 (choc webview). CI matrix (macOS + Windows build + pluginval) GREEN on both platforms (run for 3283ae9; Windows needed static-MSVC-runtime + M_PI portability fixes). **PHASE 0 GATE CLOSED 2026-07-17:** ADR-018 (bank), ADR-019 (webview, with the swappability amendment), and the E-6 envelope ratified by the human; Live load test passed (VST3 loads, plays sine on MIDI input — no GUI yet, as designed). **Recorded residual (human-accepted):** Reaper/Bitwig load evidence deferred — neither host is installed on this machine; CI pluginval on both platforms is the standing proxy; do a real load check when either host is available, no later than the Phase 2 gate. **Windows runtime work deferred (human, 2026-07-18):** the WebView2 backend stays CI-compile-verified only until desktop-coordination begins; Windows runtime validation moves out of the Phase 2 gate to that milestone. Phase 1 (SwarmCore port + parity oracle) is now in progress. Proposed E-6 envelope: min-spec = Apple M1 base / 4-core 2018-class Intel ultrabook, Windows x64 AVX2; 44.1 kHz @ 128-sample buffer; E-6 patch must hold < 50% of one core on min-spec. Deferred ecosystem briefs: Tonality intake brief due at Phase 3 before consonance gravity ships; terrain-sibling intake brief due at Phase 4 with the kernel abstraction (ADR-010(d) — placeholders in the meantime).

## Phase 0 — Platform gate & renderer decision

- CLAP-native skeleton; VST3 via clap-wrapper. Empty plugin builds on macOS + Windows, loads in target hosts (Live, Reaper, Bitwig), passes pluginval at strictness ≥ 5.
- CI: build matrix + pluginval + `./verify fast` wiring (initially trivial-green).
- **ADR-006 spike:** oscillator-bank vs iFFT additive renderer. Benchmark: 128 partials × 5 voices × 4 notes on target min-spec CPU; measure headroom both ways; decide and record. (Architecture note: coupling already runs at control rate, so iFFT frames are a natural fit if the bank loses.)
- **GUI stack decision (ADR-013):** pick the plugin GUI framework in Phase 0 so Phase 2 can ship a real GUI that reproduces the prototype design language (canvas-style phase circle, meters). Record as an ADR.
- Define target hardware envelope for E-6.
- **Gate:** hosts load it, CI is real, ADR-006 closed, GUI stack chosen.

## Phase 1 — SwarmCore port + parity oracle

- Port `SwarmSynth` (SAW core) to C++: mulberry32, seeding scheme, 16-sample control tick, σ-normalized bipolar K with slews, splay (3× authority, center anchor), inertia, R→tone, envelopes, voice stealing, tanh guard.
- Build the parity harness: JS reference renders (Node, checked into repo as golden generators, not binaries) vs C++ output; L0-1 green across the matrix.
- Port the headless trajectory tests: L0-2 through L0-5, L0-13.
- **Gate:** L0-1..5, L0-13 green. No UI exists yet and that is correct.

## Phase 2 — SAW mode feature-complete

- Distribution menu (even / JP / Gaussian / Cauchy / bimodal / clustered-pairs), detune laws (cents / Hz / ERB / tempo-grid with host-tempo sync), onset-lock/dissolve, retrigger, density comp, width + mono audition, digital↔clean, XY pad as macro pair.
- **GUI v1 (ADR-013, pulled forward from Phase 5):** phase circle with dual R₁/Rₙ meters, seat rings, formation polygon, XY pad, live R/σ/pull readouts — the SPEC §5.6 contract, styled to match the prototype design language as closely as possible (extract palette/treatments from the prototype CSS, don't reinvent).
- **Dev state button (human request, 2026-07-17):** a GUI-v1 affordance that copies the current full parameter state as JSON to the clipboard (for debugging / pasting into a session) plus a manual "save preset" action. Design position: the debug dump IS the preset format — one Layer-schema JSON with provenance metadata (SPEC §5.7), no second serialization mechanism. [SHIPPED]
- **Tempo-grid audibility experiments (human request, 2026-07-18):** hard to find settings where the grid lock is clearly audible. Explore: default-detune interactions, u ranges that put beat rates in the 0.5–4 Hz sweet spot, a "grid-forward" preset. Note the Phase 3 grid-status readout (ADR-016/017) directly attacks the legibility half of this — the populated-but-inaudible state becomes visible. Revisit alongside it.
- L0-12 green (grid law); Layer-E 1, 2, 5 sign-off.
- **Gate:** SAW mode is a shippable instrument on its own — playable through its own GUI. **GATE CLOSE PROPOSED (2026-07-18):** Layer-E 1/2/5 signed off by the human — E-1 passed with a recorded caveat (the cloud→order transition retains a steep region on both sides of K=0 despite the σ-normalized squared taper; parked as a possible future macro-mapping refinement, NOT a DSP change — the taper is ADR-004-validated and parity-frozen), E-2 passed, E-5 passed. Retrigger fix confirmed in Live. Remaining distribution scope moved per the reference-first principle: **bimodal** relocates to Phase 3 (the dynamics reference implements its placement tied to two-cluster topology — port them together, with parity); **clustered-pairs** has no reference implementation anywhere — awaiting a prototype update from the design session (human is asking the original agent), then ports with parity. Ratify to close.

## Phase 3 — Dynamics integration

- Topology (mean-field / ring+reach / two-cluster+μ), Sakaguchi α, absolute-K mode, consonance gravity + basin + ratio readout.
- Daido poles q (1–4) with R_q meter (ADR-015); tempo-grid status readout + cause-AND-state lock warning (ADR-016/017).
- Formalize L0 criteria for q-cluster formation / demographics / bistability from the ADR-015 anchors (R_q = 0.97 at q∈{2,3} across seeds; 2f0 projection ~0.080 seed-invariant) and add them to ACCEPTANCE.md at this gate as L0-22+ (L0-14..21 are taken by Track E, ingested 2026-07-18).
- L0-8..11 green; **Layer-E 3 SIGNED OFF (human, 2026-07-18: "I hear it. Sounds great")**.
- **Tonality brief ON HOLD (human, 2026-07-18):** the human will prime Tonality directly; integration scope under discussion (see traces — possible outcome: HYPERSAW owns a richer static ratio table itself and only context-weighting ever involves Tonality, or the integration is skipped). Gravity ships on the default set either way.
- **Gate:** the dynamics lab's verified states are reproducible in-plugin from preset recall. **GATE CLOSE PROPOSED (2026-07-18):** engine parity 51/51 both references; L0-8..12 green; ADR-015 anchors formalized as L0-22 and enforced in trajectory_check (q-cluster R_q, bistability, split-as-timbre projections); surface complete (params 24-31, meters, gravity + grid readouts per ADR-016/017); Layer-E 3 signed off; preset-recall reproducibility guaranteed by state_check's bit-identical-restored-audio requirement. Merging the gate PR = ratification.

## Phase 4 — SPECTRA mode & kernel abstraction

- Per-partial engine at the ADR-006 renderer: amp tilt, stretch, width tilt, width law, cascade, splay-as-interference-gate with per-partial stereo narrowing.
- Kernel abstraction landed: saw / sine share one voice path; wavetable kernel stubbed (terrain-sibling crossover parked until here).
- L0-6, L0-7 green; Layer-E 4 sign-off.
- **STATUS (2026-07-18):** SpectraCore ported (verbatim, own goldens): parity RMS 0.0 on 9/9 scenarios vs the live-sliced JS reference; L0-6 (monotone front, 7.21 s / 1.81 s) and L0-7 (−15.06 dB, narrowing engaged) GREEN, enforced in ./verify full (spectra_check). **OPEN QUESTION (ADR-037, human ruling needed):** the P=1 gate as written assumes one shared voice path, but the two references are distinct frozen programs — rule (a) measured-equivalence interpretation vs (b) SPEC amendment + re-measurement. Shell integration (engine select, GUI, viz) follows the ruling.
- **Gate:** SAW provably = SPECTRA at P=1 (parity between modes on equivalent settings).

## Phase 5 — Performance layer & face

- GUI completion (v1 shipped in Phase 2 per ADR-013): phase carpet, partial strips, gravity readouts, mod-matrix UI — the full §5.6 thesis, same prototype design language.
- MPE: pressure→K, slide→detune, per-note routing. Mod matrix with R and σ as sources. K envelopes/macros.
- Presets with full provenance metadata; deterministic recall test added to L0-13.
- Layer-E full pass; naming decision; demo patches (including the validated recipes: shimmer-K, zipper, erasure, gravity-settle, broken-symmetry pad).
- **Gate:** release candidate.

## Prior art & positioning

Maintained in PRIOR-ART.md; revisit at Phase 3 (before gravity ships) for the freedom-to-operate check flagged there, and at Phase 5 for marketing claims accuracy.

## Track E — effects line (parallel track; ingested 2026-07-18, packet UPDATE-001)

Track E depends only on Phase 0 platform infrastructure and the control-tick scaffolding from Phase 1; it does not depend on the third oscillator engine and can proceed alongside it.

**E0 · Force-core module.** Port the shared force system (home/sync/splay/gravity/drift/inertia on log2 coordinates, per-tick) as a standalone, engine-agnostic module consumed by all four effect engines. Gate: force-core unit tests reproduce the JS labs' population trajectories (collapse σ ratios, gap CVs, equilibrium-law residuals) within L0-14/15/19/20 tolerances, seed-for-seed. **GATE CLOSE PROPOSED (2026-07-18):** src/force_core.h (labs' force system verbatim; Profile per engine; three attractor kinds) + force_check in ./verify full — 17 scenarios seed-for-seed vs Node-extracted lab cores (worst |Δv| 1.3e-14, tolerance 1e-9), population halves of L0-14/15/17/19/20 all green, drift-off measurement protocol discovered and pinned (ADR-034). SwarmCore↔effects unification scoped honestly per ADR-034 (phase-domain vs position-domain: only the RNG is genuinely shared, now delegated, parity 51/51). Merging the PR = ratification; E1 (frequency engines) unblocks.

**E1 · Frequency engines.** Resonator bank + notch swarm on the force core, external audio input. Gate: L0-14 through L0-18 green; notch-exactness regression guard (L0-16) in CI.

**E2 · Time engines.** Tap-swarm delay (host tempo sync replaces the bpm field) + FDN room swarm. Gate: L0-19 through L0-21 green, including the LF-stability and DC-boundedness long-run checks; matrix-sign regression guard (L0-20) in CI.

**E3 · Integration.** Effects as sections of the instrument (post-oscillator) *and* as standalone effect plugin(s) — same cores, two shells. Collapse/comb-regularity/in-basin-error exposed as mod sources. Visualization per SPEC-EFFECTS §7, warnings per ADR-017 (cause AND state).

**Sequencing note:** E0 is small and high-leverage — the force core is the same mathematics the dynamics engine already needs, so if the third original engine is the dynamics engine, build E0 first and have both consume it. If the third engine is already underway with its own force implementation, unify at E0 rather than maintaining two.

**Local sequencing ruling (per the packet's own note):** the dynamics engine is already built inside SwarmCore with its own force implementation — so E0 is a UNIFICATION: extract/share the force mathematics rather than build a second copy.
