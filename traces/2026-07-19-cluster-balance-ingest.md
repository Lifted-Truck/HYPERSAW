# 2026-07-19 — Two-cluster A/B balance knob (ADR-051): proposal triaged + ported

Human dropped PROPOSAL-cluster-balance.md + swarmdynamics_AB.html clone.

VERIFIED ADDITIVE (L0001 discipline) before ingest: extracted both DynSynth
cores headless; balance=0 reproduces the current reference BIT-IDENTICALLY
(worst |Δ|=0); change confined to the two-cluster branch (kB = 1-2*balance,
cluster B *= kB). Reproduced the measured sweep (R_A pinned 1.00; R_B falls
1.00→0.41→0.21 across balance; R_B floor rises with mu 0.03→0.33) — matches
the proposal qualitatively; exact floors settle-window-sensitive (L0002).

INGEST (ADR-011/012): clone replaced swarmdynamics.html (old in git history);
both drop files deleted. PORT: swarm_core.h two-cluster kB (default 0 inert);
shell param 56; GUI row gated two-cluster-only. ORACLE: golden
dyn-cluster-balance (balance 0.5) → parity 54/54 (proves the NEW code path,
not just inertness); L0-23 anchors in trajectory_check (balance 0 both sync;
0.5 split 0.60; 1.0 split 0.79; mu floor monotone) + ACCEPTANCE L0-23.
Per-cluster-norm refinement DECLINED (reference uses global norm; parity
wins). Addresses PARKED #16.

Evidence: verify full green (parity 54/54, trajectory incl L0-23, state);
pluginval 10 SUCCESS; installed. Branch created BEFORE editing.
