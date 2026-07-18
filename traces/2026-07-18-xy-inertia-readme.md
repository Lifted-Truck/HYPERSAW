# 2026-07-18 — XY two-way binding; inertia taper (measured); README catch-up

**XY pad (human report).** Was one-way: the pad drove the knobs but
setControl never updated the crosshair state, and nothing polled the host.
Fixed: crosshair follows knob/slider moves, and the GUI polls hzGetParams
~2×/s (skipped while dragging) so Ableton device-panel moves and automation
reflect back.

**Inertia (human report: "feels binary").** Measured R over K × w before
touching anything (table in session): smooth at K ≥ 0.85; at near-critical
K the first epsilon of w drops the marginal lock a regime, then w 0.02–0.3
is a dead plateau. Split verdict (ADR-024): initial step = deliberate
reference physics (softening = DSP divergence — offered as a
reference-update question, not done); dead plateau = knob-mapping defect,
fixed with a parity-safe sqrt taper at the CLAP layer (one documented
shell-side knob slot for exact state round-trips; both state-load paths
routed through applyParam).

**README** rewritten to the clarity standard: current map (src/tools/docs),
Phase 0–2 closed / Phase 3 in flight status, oracle chain description,
validation + deferral lines, dated. docs/img/ created for the human's
offered screenshots (yes — wanted).

**Verify.** full green on this branch (30/30 SAW parity — pre-#12 base;
PR #12 discovered still OPEN despite believed merged, flagged to human);
trajectories; state_check 8/8 incl. the new taper round-trip.
