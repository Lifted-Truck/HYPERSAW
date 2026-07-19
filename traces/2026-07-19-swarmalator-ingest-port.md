# 2026-07-19 — Swarmalator engine: ingest + bit-exact port

Human drop: SPEC-SWARMALATOR.md + swarmalator.html. The swarmalator
couples audio phase θ and spatial angle ξ to each other (ring model,
OHS 2017) — pan is a state variable of the tone-making system.

Ingested (ADR-048, spec-in-code per ADR-003): both files added to
protected paths (7th prototype); SPEC ADR cross-refs CORRECTED on ingest
per L0009 — Daido ADR-010→ADR-015, ingest ADR "046"→048 (046 is the
notch swarm here), posture ADR-018→ADR-044.

Ported src/swarmalator_core.h verbatim — STEREO out (pan from ξ), shares
mulberry32 via forcecore (ADR-034), K-norm ADR-004. Oracle
(gen_swarmalator_goldens.mjs + swarmalator_check): stereo L0-1 parity
RMS 0.0 on 9/9 FIRST build; SPEC §5 anchors all pass and match the
measured table (sync R=0.966 vs 0.97; splay 0.056; rainbow max(R±)=0.515
R-incoherent; sync+rainbow both raised; stability peak 0.927 NaN-clean).
verify full now runs EIGHT oracle chains.

STATUS: EXPERIMENTAL, core+oracle only. Shell integration (auditioning)
deferred until the human's first listen of the effects + this — may not
survive, may be joined by other new engines. Under ADR-045 it's a (Γ,W)
point; delivers the parked grain-swarm's spatial dynamics as a special case.

Evidence: verify full green (8 chains). Branch created BEFORE editing (L0004).
