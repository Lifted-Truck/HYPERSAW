# INDEX — retrieval map (HYPERSAW)

Read this in full at ORIENT; pull only matching LIBRARY entries. Tags:
swarm-dynamics · parity-oracle · plugin-platform · realtime-perf

- [L0001] parity-oracle (canonical) — reference prototypes update by file-drop and external ADR logs can collide with the local log; diff-verify, renumber/merge, headless-verify claims, fold into SPEC/ACCEPTANCE before goldens.
- [L0002] parity-oracle/swarm-dynamics — failing checks on a bit-parity port mean protocol mismatch, not port bugs; probe the reference, reproduce the measurement, or surface an erratum. Never adjust thresholds.
- [L0003] plugin-platform — every new C++ tool target repeats the MSVC traps (M_PI, -O3, runtime mismatch); grep before pushing, guards in CMake.
- [L0004] plugin-platform — a filed PR branch is frozen; follow-up pushes race the async merge (bit twice). New work = new branch; verify ancestry after merges.
- [L0005] plugin-platform/parity-oracle — duplicated key chains drift and symmetric lies pass value checks; single-source the maps, trust the audio-identity oracle.
- [L0006] plugin-platform — a wrapper's defaults are the host-facing surface: trace advertisement + inward event type + gating flags in the wrapper source (and look for its plugin-side extension seam) before claiming "host X delivers Y".
- [L0007] plugin-platform — "note doesn't stop" reports: exonerate core→shell→wrapper headlessly; AU wrapper forwards 0x90-vel-0 as NOTE_ON (shell must remap); fuzz timestamps drawn sorted or acausal orders fake hangs.
