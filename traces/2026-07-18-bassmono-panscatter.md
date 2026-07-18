# 2026-07-18 — Bass mono (M/S elliptic) + pan scatter (ADR-035)

**Bass mono (ids 40/41, shell).** One Butterworth TPT-SVF high-pass on
the side channel: L = M + HP(S), R = M - HP(S). Phase-coherent by
construction (no L/R crossover mismatch), mono-compatible, one filter
total. Runs before the spectrum ring feed so the visualizer shows the
actual output. Isolated numeric check: side gain -24.1 dB @ 30 Hz,
-3.0 dB at the 120 Hz corner, 0.0 dB @ 1 kHz. States reset on engage.

**Pan scatter (id 42, core).** Confirmed the human's series-panning
diagnosis at swarm_core.h finishRebuild: pan = clamp(x[i]) * width —
monotone in detune offset, so frequency order IS spatial order. Fix:
blend pan positions toward a seeded Fisher-Yates permutation drawn from
an INDEPENDENT stream (seed-derived, never advances the phase/drift
stream). 0 = legacy arithmetic verbatim; parity 51/51 green is the
bit-inertness proof. Slider (not a toggle) per the request.

**Also fixed in passing:** gui.html DEFAULTS stopped at id 38 — scatter
(39) had no double-click reset. Extended through 42.

**Evidence.** verify full green (51/51 parity, trajectories, state 8/8,
force GREEN); pluginval 10 SUCCESS; auval SUCCEEDED; seals OK;
installed. GUI checked in the browser pane: rows render with units
(120Hz / %), bass-xover soft-gate dims until bass mono engages.
