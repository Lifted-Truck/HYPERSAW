# 2026-07-17 — Phase 1: SwarmCore port + full L0 suite green

**What changed.** `src/swarm_core.h` — statement-by-statement C++ port of the
SwarmSynth reference core (header-only, pure, allocation-free after
construction, no wall-clock). Parity-load-bearing details documented in the
header: doubles everywhere, Float32Array store-rounding reproduced, mulberry32
under JS int semantics via bit-identical uint32 ops, reference's own π/τ
literals kept. `tools/parity_check.cpp` (L0-1) + `tools/trajectory_check.cpp`
(L0-2..5, L0-13); gen_goldens now also emits `manifest.tsv` (params in
application order — the C++ side must replay setParam order to reproduce
rebuild RNG draws). `./verify full` runs the whole chain, regenerating
goldens each run so the HTML reference stays the live source of truth.
ROADMAP: dev state-button request recorded in Phase 2 (debug JSON dump =
preset format, one schema).

**Results.**
- L0-1: **30/30 scenarios green; 24 bit-identical (RMS = 0.0)**; worst
  2.535e-11 (inertia-hunt — chaotic amplification of transcendental ULPs, 5
  orders inside eps=1e-6).
- L0-2..5, L0-13: 21/21 green (output in session).

**Measurement-protocol findings (thresholds untouched).** Probing the JS
reference showed three ACCEPTANCE rows encode a measurement protocol, not
just values: (1) the K=+0.7 shimmer pair (0.24, 0.78) reproduces only under
DENSE R sampling (reference: 0.23/0.80) — 1/s sampling aliases it flat;
(2) K=+0.6's "0.97→0.16 over 3 s" is the initial transient; R afterwards
fluctuates 0.04–0.92 without sustaining lock ("subcritical" formalized as
transient + no-lock); (3) **proposed erratum:** the free-row snapshot values
(R₁ 0.39 / R_N 0.16) don't reproduce even on the reference — free-running
R for n=7 fluctuates around 1/√7 ≈ 0.38 — so free is checked as regime
classification (time-averaged R₁, R_N ∈ (0.1, 0.6)), which ACCEPTANCE's own
tolerance line makes blocking. ACCEPTANCE is human-gated: the erratum is
proposed here and in the PR, not applied.

**Verify.** `./verify fast` green; `./verify full` green end-to-end
(build + 30/30 selfcheck + 30/30 parity + 21/21 trajectories).

**Open.** Human ratification of: the three protocol formalizations, the
free-row erratum, and the Phase 1 gate close ("no UI exists yet and that is
correct" per ROADMAP).
