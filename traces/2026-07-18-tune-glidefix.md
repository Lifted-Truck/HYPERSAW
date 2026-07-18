# 2026-07-18 — Live transposition suite; glide gated on held keys (ADR-027)

**Human clarifications resolved.** (a) Transposition: octave/semi/fine/
pitch-knob (params 35-38) fold into ONE live core tune factor at law
evaluation — the pitch knob bends sounding notes; the ADR-026 note-on-time
octave transpose is removed in favor of it (one mechanism). tune=1.0 is
bit-inert; gravity ratios tune-invariant. Verified: live +1 st bend check.
(b) The intended "octave toggle" is a UNISON octave layer — parked as
PARKED #15 with the physics questions (coupling-graph membership under a
2:1 split; sigma-normalization) for the design session, reference-first.
(c) Mono glide/legato engage only when another key is HELD; tail-only
ringing gets a fresh strike on a new slot (natural overlap) — the previous
test conflated tail-ringing with held.

**Evidence.** verify full green (51/51 parity — tune factor inert at
default; trajectories incl. new tune check; state 8/8 with params 36-38
round-tripping). pluginval 10 SUCCESS · auval SUCCEEDED · seals OK ·
installed.
