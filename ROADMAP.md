# SWARM✱ — roadmap (phase-gated)

Gates are blocking. "Green" = `./verify fast` passes + phase acceptance subset + trace written. Passing ≠ done; done = green + acceptance criteria + DECISIONS/trace updated.

**Status (2026-07-18):** Phase 1 GATE CLOSED (PR #2 merged; protocol findings + free-row erratum ratified with the merge, erratum applied to ACCEPTANCE). Phase 2 in progress: SwarmCore is live in the plugin — placeholder sine replaced, 18-param CLAP surface at prototype ranges (dissolve in seconds, driftDepth in cents), versioned key=value state; pluginval strictness 10 SUCCESS, auval SUCCEEDED post-integration. Phase 2 remaining: tempo-grid law port (needs host tempo; L0-12), bimodal/clustered-pairs distributions (OPEN QUESTION — SPEC lists them, the reference doesn't implement them: extending the reference is a spec change needing a human ruling), GUI v1 + dev state button, webview smoke test, Layer-E 1/2/5 sign-off.

*(Historical status, 2026-07-17 evening:)* Phase 0 largely complete — skeleton builds (CLAP + VST3 + AUv2 via clap-wrapper, pinned submodules), pluginval SUCCESS at strictness 10 (gate asks ≥5), auval SUCCEEDED, all three formats installed locally with intact codesign seals; ADR-006 spike run (bank 66× / iFFT 216× realtime at 2560 osc on M3) with close proposed as ADR-018 (bank); GUI stack proposed as ADR-019 (choc webview). CI matrix (macOS + Windows build + pluginval) GREEN on both platforms (run for 3283ae9; Windows needed static-MSVC-runtime + M_PI portability fixes). **PHASE 0 GATE CLOSED 2026-07-17:** ADR-018 (bank), ADR-019 (webview, with the swappability amendment), and the E-6 envelope ratified by the human; Live load test passed (VST3 loads, plays sine on MIDI input — no GUI yet, as designed). **Recorded residual (human-accepted):** Reaper/Bitwig load evidence deferred — neither host is installed on this machine; CI pluginval on both platforms is the standing proxy; do a real load check when either host is available, no later than the Phase 2 gate. Phase 1 (SwarmCore port + parity oracle) is now in progress. Proposed E-6 envelope: min-spec = Apple M1 base / 4-core 2018-class Intel ultrabook, Windows x64 AVX2; 44.1 kHz @ 128-sample buffer; E-6 patch must hold < 50% of one core on min-spec. Deferred ecosystem briefs: Tonality intake brief due at Phase 3 before consonance gravity ships; terrain-sibling intake brief due at Phase 4 with the kernel abstraction (ADR-010(d) — placeholders in the meantime).

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
- **Dev state button (human request, 2026-07-17):** a GUI-v1 affordance that copies the current full parameter state as JSON to the clipboard (for debugging / pasting into a session) plus a manual "save preset" action. Design position: the debug dump IS the preset format — one Layer-schema JSON with provenance metadata (SPEC §5.7), no second serialization mechanism.
- L0-12 green (grid law); Layer-E 1, 2, 5 sign-off.
- **Gate:** SAW mode is a shippable instrument on its own — playable through its own GUI.

## Phase 3 — Dynamics integration

- Topology (mean-field / ring+reach / two-cluster+μ), Sakaguchi α, absolute-K mode, consonance gravity + basin + ratio readout.
- Daido poles q (1–4) with R_q meter (ADR-015); tempo-grid status readout + cause-AND-state lock warning (ADR-016/017).
- Formalize L0 criteria for q-cluster formation / demographics / bistability from the ADR-015 anchors (R_q = 0.97 at q∈{2,3} across seeds; 2f0 projection ~0.080 seed-invariant) and add them to ACCEPTANCE.md at this gate.
- L0-8..11 green; Layer-E 3 sign-off.
- **Gate:** the dynamics lab's verified states are reproducible in-plugin from preset recall.

## Phase 4 — SPECTRA mode & kernel abstraction

- Per-partial engine at the ADR-006 renderer: amp tilt, stretch, width tilt, width law, cascade, splay-as-interference-gate with per-partial stereo narrowing.
- Kernel abstraction landed: saw / sine share one voice path; wavetable kernel stubbed (terrain-sibling crossover parked until here).
- L0-6, L0-7 green; Layer-E 4 sign-off.
- **Gate:** SAW provably = SPECTRA at P=1 (parity between modes on equivalent settings).

## Phase 5 — Performance layer & face

- GUI completion (v1 shipped in Phase 2 per ADR-013): phase carpet, partial strips, gravity readouts, mod-matrix UI — the full §5.6 thesis, same prototype design language.
- MPE: pressure→K, slide→detune, per-note routing. Mod matrix with R and σ as sources. K envelopes/macros.
- Presets with full provenance metadata; deterministic recall test added to L0-13.
- Layer-E full pass; naming decision; demo patches (including the validated recipes: shimmer-K, zipper, erasure, gravity-settle, broken-symmetry pad).
- **Gate:** release candidate.

## Prior art & positioning

Maintained in PRIOR-ART.md; revisit at Phase 3 (before gravity ships) for the freedom-to-operate check flagged there, and at Phase 5 for marketing claims accuracy.
