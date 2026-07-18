# 2026-07-18 — Phase 3 increment 2: dynamics surface; state oracle catches a real bug

**Shipped.** Params 24-31 (Topology/Reach/μ/α/Poles/Gravity/Basin/Absolute-K)
with enum labels + degree/cents displays; GUI Dynamics cluster; meter region
gains R_q (under poles) and A/B cluster orders (under two-cluster); gravity
readout (ratio name + octave fold + live cents per ADR-008); ADR-016/017
grid-status line verbatim (unit, rungs, cause-AND-state lock warning,
fixed height). Absolute-K semantics: σ→1.0 Hz in coupling targets, taper
kept (minimal bypass; ADR-004); verified identical-oscillator lock R=1.0.

**The bug state_check caught (the oracle earning its keep).** RED on
restored-audio-bit-identity: the shell's hand-maintained readParam key chain
lacked the new dynamics params → get_value returned 0 → state saved lies —
and the value-round-trip check PASSED because both instances lied
identically. Diagnosis: determinism triage (A1==A2 yes, A==B no) →
re-apply-at-own-value bisect → id 24 read back as 0. Fix by construction:
core.getParam through the SAME paramSlot map as setParam; parallel chain
deleted. L0005 written.

**Tonality brief (ADR-010 obligation).** HYPERSAW-001 drafted (ratio priors
/ context-weighted basins; offline export artifact; contract tests offered)
— the push to the Tonality repo was classifier-blocked (cross-repo write),
so filing awaits the human's go-ahead; draft staged in session scratchpad
and reproduced in this trace's commit. Gravity ships meanwhile on the
default 13-ratio set per the not-blocked rule (visibly-degraded
placeholder).

**Evidence.** verify full green (51/51 parity, trajectories incl. absK,
state 8/8 post-fix); pluginval 10 SUCCESS; auval SUCCEEDED; installed.
