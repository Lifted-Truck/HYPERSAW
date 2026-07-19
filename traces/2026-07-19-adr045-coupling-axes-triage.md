# 2026-07-19 — Triage: coupling-topology-axes recommendation → ADR-045 (PROPOSED)

Triaged RECOMMENDATION-coupling-topology-axes.md (untracked drop). It
proposes lifting the coupling function Γ (Sakaguchi/Daido) and topology W
(mean-field/ring/two-cluster) out of the dynamics engine's inline branches
into two pure force-core objects — a Γ × W factorization mirroring the
existing distribution × detune-law split, with a bit-parity guard so the
representation change can't alter shipped behavior.

ASSESSMENT: technically sound, idiomatic, recommended for ratification.
Captured as ADR-045 PROPOSED (implementation gated on human ratification +
critic review — it touches the force-core seam). DISCERNMENT (per the
recurring external-packet error class): TWO wrong ADR cross-refs corrected
— FDN matrix ADR-014→ADR-030; Daido ADR-010→ADR-015. Also corrected a
conceptual slip: drawn-distribution (PARKED #1) is a distribution-object,
not Γ; the Γ analog (drawn-coupling) is a proposed future payoff, not a
current PARKED entry. Design reasoning archived to docs/design/; root file
deleted.

Sequencing: source pitched it "as Phase 3 finishes" — Phase 3 is CLOSED,
so the lift is a clean standalone refactor whenever prioritized; gates
neither E1 nor Phase F.

Separately: 2 background agents dispatched to verify the PRIOR-ART §6
Kuramoto-lineage citations aren't hallucinated (Lem/Kuroscillator program;
Collins/Lambert antecedents; the no-product market claim).

Evidence: verify fast GREEN. Docs-only.
