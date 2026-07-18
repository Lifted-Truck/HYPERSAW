# INDEX — retrieval map (HYPERSAW)

Read this in full at ORIENT; pull only matching LIBRARY entries. Tags:
swarm-dynamics · parity-oracle · plugin-platform · realtime-perf

- [L0001] parity-oracle (canonical) — reference prototypes update by file-drop and external ADR logs can collide with the local log; diff-verify, renumber/merge, headless-verify claims, fold into SPEC/ACCEPTANCE before goldens.
- [L0002] parity-oracle/swarm-dynamics — failing checks on a bit-parity port mean protocol mismatch, not port bugs; probe the reference, reproduce the measurement, or surface an erratum. Never adjust thresholds.
- [L0003] plugin-platform — every new C++ tool target repeats the MSVC traps (M_PI, -O3, runtime mismatch); grep before pushing, guards in CMake.
