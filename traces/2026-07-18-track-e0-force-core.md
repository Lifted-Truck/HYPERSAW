# 2026-07-18 — Track E0: shared force core, seed-for-seed vs the effect labs

**What.** src/force_core.h — the effects line's shared force system
(SPEC-EFFECTS §1) transcribed verbatim from the labs: one force expression
(the three HTMLs embed it identically), per-engine frozen constants as
Profile (clamps 5.32/5.9/mode-dependent; span floor 0.15/0.15/0.1; inertia
omega0 6+0.5/6+0.5/4+0.4), attractor sets as data (harmonics n*f0,
rhythmic RSET*beat, period multiples). Plus the labs' distribution builder
(even/gauss/cauchy, sorted) and both placement families (ERB/log/harmonic
at 40 or 60 Hz floors; log-time around size).

**Unification ruling (ADR-034).** SwarmCore is phase-domain Kuramoto; the
labs are position-domain springs — distinct systems, both frozen. Shared
for real: mulberry32 only (SwarmCore now delegates to
forcecore::rngNext; parity 51/51 proves bit-neutrality). Drift walks stay
separate (different frozen constants on each side; documented in the
header). The real unification win is forward-looking: E1-E3's four
engines consume ONE module instead of four copies.

**Oracle.** tools/golden/extract_force.mjs slices FilterLab/PhaserLab/
TimeLab live from the HTMLs (own-scope evaluation; no fork);
gen_force_goldens.mjs runs 17 population scenarios (8 s, checkpoints at
ticks 0/1/10/100/1000/22050) into gitignored build-golden/force/;
tools/force_check.cpp mirrors the setups, compares checkpoints
(tolerance 1e-9 log2 units; observed worst 1.3e-14 — the population path
is Float64 end to end, no f32 rounding), and enforces the population
halves of L0-14/15/17/19/20. Wired into ./verify full (verify edit rides
this PR; merge = the human gate).

**Protocol finding (L0002 class, second occurrence).** The reference
numbers reproduce ONLY with drift off: TimeLab defaults driftDepth 30,
but sigma 0.755 / CV 0.1233 / 0.0095 / 0.0553 / equilibrium ratios
0.238 & 0.217 (law-exact) all land at driftDepth 0 — at drift 30 grid
CV reads 0.071 and the echo gravity ratio breaks to 0.481. The
acceptance values encoded a drift-off measurement protocol nobody wrote
down; the probe (scratchpad, reproduced in the goldens) pinned it.
Also: the echo-grav reference captures exactly ONE in-basin tap
(rhythmic attractors are sparse vs the tap spread) — my initial >=2
guard was an invention and was corrected to >=1; the ACCEPTANCE
criterion (equilibrium ratio) was never touched.

**Evidence.** ./verify full green end to end: parity 51/51 (worst
4.3e-9), trajectory GREEN, state GREEN, force selfcheck deterministic,
force_check GREEN (0 failures). E0 gate criteria met; close proposed in
ROADMAP, merge = ratification.
