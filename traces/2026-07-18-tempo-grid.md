# 2026-07-18 — Phase 2: tempo-grid law (L0-12 green); tap-report resolved

**Tap report (human).** "Notes stick a little longer than expected" —
measured headless: 50 ms tap → ~550 ms tail to −40 dB with the default
160 ms exponential release; voice fully retires (no stuck gate). Verdict:
reference behavior, not a bug; the ADR-021 Release slider is the remedy
(30 ms → −60 dB in 176 ms). Default stays parity-frozen; tighter feels can
ship as presets.

**Tempo-grid law (ADR-022).** Law 3 in SwarmCore, snapping expression
ported verbatim from the DYNAMICS reference beatQ path; bpm host-owned via
CLAP transport (per-block read + transport events, fallback 120);
cycles-per-beat = param 23; GUI: law option + grid slider ("N/b"). Drift
applies after the snap (deliberate loosening; oracle runs at drift 0).
Placement protocol pinned: dist=even, M=16 (the DYNAMICS reference has no
distribution selector).

**Evidence.** L0-12: 8/8 first run — exact multiples at machine precision,
3 rungs/4 Hz @ detune 0.1 and 9 rungs/16 Hz @ 0.6 (ACCEPTANCE numbers
exact), envelope periodicity 22.9× (≥6; reference 10.2×), grid lock
R=0.949 < 2 s (≥0.88). L0-1 parity 30/30 unchanged (laws 0–2 untouched);
trajectories all green; pluginval strictness 10 SUCCESS; auval SUCCEEDED;
seals verified.
