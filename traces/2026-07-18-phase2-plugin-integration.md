# 2026-07-18 — Phase 2: SwarmCore live in the plugin

**What changed.** `src/hypersaw_clap.cpp` rewritten around the parity-proven
core: placeholder sine deleted; note/param events feed SwarmCore directly
(sample-accurate event splitting preserved); 18-param CLAP surface with IDs
frozen (append-only), ranges mirroring the prototype UI — dissolve exposed in
seconds (prototype knob is log10 s), driftDepth in cents, stepped enums for
dist/law/mono/retrig with text labels; params + state extensions (versioned
key=value blob; core.p is the single source of truth, no shadow copies).
ACCEPTANCE free-row erratum applied per Phase 1 gate ratification (protocol
notes recorded alongside the regime table). ROADMAP status updated; Phase 2
open question recorded: bimodal/clustered-pairs distributions are in SPEC but
not in the reference — extending the reference is a spec change, needs a
human ruling before implementation.

**Evidence.** Build green (all formats); pluginval strictness 10 SUCCESS on
the VST3 with the full param surface (params get exercised by its automation
tests); installed bundles re-signed, seals verified; auval SUCCEEDED.
`./verify full` green end-to-end post-integration (build + 30/30 golden
self-check + 30/30 L0-1 parity + 21/21 trajectories) — the adapter did not
disturb the core.

**Open.** Live re-test with params (human; next launch). Tempo-grid law port
+ L0-12. GUI v1 (webview smoke test first, ADR-019 risk list). Distribution
ruling above.
