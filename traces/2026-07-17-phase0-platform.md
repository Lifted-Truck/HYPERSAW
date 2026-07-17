# 2026-07-17 — Phase 0: platform gate work

**What changed.** CLAP-first C++ skeleton: `src/hypersaw_clap.cpp` (impl:
descriptor, note/audio ports, sample-accurate event splitting, deterministic
8-voice sine placeholder — SwarmCore replaces it in Phase 1), entry pair per
the clap-first idiom; root CMakeLists (C++20, Unix Makefiles, macOS 11.0+);
pinned submodules libs/clap @ 1.2.10 and libs/clap-wrapper @ v0.15.1 (VST3
SDK v3.8.0 CPM-pinned inside the wrapper; AUv2 SDK likewise). Formats built:
CLAP + VST3 + AUv2. `tools/renderer_bench.cpp` (ADR-006 spike). CI grew a
macOS + Windows build matrix with pluginval 1.0.4 at strictness 5.
`./verify full` now adds the Release build. ADR-018 (renderer: bank) and
ADR-019 (GUI: choc webview) appended as PROPOSED for gate ratification;
ROADMAP status + proposed E-6 envelope recorded.

**Evidence.**
- Build: all targets green, Unix Makefiles, Release, M3.
- pluginval VST3: SUCCESS at strictness 5 AND at 10 (gate asks ≥5).
- auval: `auval -v aumu Hsaw LfTk` → "AU VALIDATION SUCCEEDED" after install
  + re-sign; `codesign --verify --deep` OK on all three installed bundles
  (global-notes seal gotcha handled by signing the installed copies).
- ADR-006 spike (M3, single thread, -O3, 8 s renders): 2560 osc — bank 66×,
  iFFT(vDSP) 216× realtime; 480 osc — 403× / 947×; 64×5×8 — 75× / 199×.
  Decision driver flipped from throughput (ample both ways) to parity +
  control-rate semantics → bank. Full rationale in ADR-018.
- GUI research: subagent scout report (choc/CLAP-webview-ext/Cmajor
  precedents; risks: Live keyboard focus, wrapper-passthrough unconfirmed,
  multi-instance memory) — folded into ADR-019 with mitigation owners.

**Open before gate close.** Windows CI green (first matrix run in flight);
Live load evidence (next Live launch on this machine; Reaper/Bitwig not
installed locally — CI pluginval is the proxy); human ratification of
ADR-018, ADR-019, E-6 envelope.
