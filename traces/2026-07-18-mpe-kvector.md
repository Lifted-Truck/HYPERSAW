# 2026-07-18 — MPE per-note pitch (ADR-036) + K-vector display smoothing

**MPE (ADR-036).** CLAP NOTE_EXPRESSION/TUNING -> per-swarm noteTune
factor at the ADR-027 seam (f0c = f0cur * tune * noteTune). Wildcard
note matching per CLAP; fresh strikes reset to 1.0 (bit-inert, parity
51/51); legato retargets keep the bend. No new params, no state — it is
a live per-note stream. Needs the human's MPE-controller test in Live
for the Layer-E half (headless can't produce host MPE).

**K-vector smoothing (viz-only).** The pink R·e^ipsi arrow teleported:
psi wraps at +-pi and jitters at low R. Display vector now smoothed in
CARTESIAN space (0.18/frame exponential, ~90 ms) so wrap reads as a
sweep through the center. Numeric readouts stay raw.

**Evidence.** verify full green (51/51, trajectories, state, force);
pluginval 10 SUCCESS; auval SUCCEEDED; seals OK; installed.
