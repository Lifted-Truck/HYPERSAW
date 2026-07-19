# Design note — the two coupling axes as first-class architecture

*Archived 2026-07-19 from RECOMMENDATION-coupling-topology-axes.md (root file
deleted after triage). Committed decision is ADR-045 (PROPOSED). Two ADR
cross-references in the original were corrected during triage: FDN mixing
matrix = ADR-030 (orig. said ADR-014); Daido validation = ADR-015 (orig.
said ADR-010). This note preserves the forward-design reasoning; the ADR
carries the commitment.*

## The observation

Every coupling behavior shipped or parked is a choice inside one equation:

    θ̇_i = ω_i + (coupling) · Σ_j  W_ij · Γ(θ_j − θ_i)

Two slots are pluggable: **Γ**, the phase-interaction function (what
"agreement" means), and **W**, the topology (who negotiates with whom):

- Γ = sin → classic sync (SAW).
- Γ = sin(Δθ − α) → Sakaguchi α (dynamics path).
- Γ = Σ K_n sin(nΔθ) → Daido poles (ADR-015; the `poles q` selector is a special case).
- W = all-ones → mean-field. W = banded → ring ± reach. W = block → two-cluster/bimodal.

These are not four features and three topologies — they are scattered
coordinates in a 2-D design space (Γ × W), most of it unvisited, the visited
parts hardcoded as enumerated modes. The effects line shows the same objects
under other names: the FDN room's mixing matrix (ADR-030) is a W; the parked
drawn-distribution idea is a *distribution*-object precedent for the "pure
pluggable object" pattern (note: a distribution is placement, distinct from Γ
— a drawn-*coupling* curve would be the Γ analog, and is the proposed payoff
below, not a current PARKED entry).

## The proposal

Name the two axes as architecture — two small pure objects the force-core
consumes, peer to the existing distribution/law objects:

- **`CouplingFunction` (Γ):** a Fourier coefficient vector {(K_n, φ_n)},
  n=1..H (H≈4 covers everything shipped). Mean-field evaluation via
  generalized order parameters Z_n = ⟨e^{inθ}⟩ (the core already computes
  Z_1 = R and, on the dynamics path, Z_q = R_q). Cost: H complex
  accumulators per control tick. Same math the Daido work validated
  (ADR-015), lifted from a selector to a vector.
- **`Topology` (W):** selects neighbor/weight structure. Present set stays
  mean_field | ring(reach) | two_cluster(μ); the parked zoo (small-world,
  scale-free, modular, spatial, adaptive) becomes enum additions consuming
  one interface. Unifies with the FDN mixing-matrix constructor family
  (ADR-030's "which eigenchannel does the input excite" is a `Topology`
  property).

The refactor: lift Γ and W out of the inline branches into these objects,
prove bit-parity against the current dynamics reference (a representation
change, not a behavior change), then new points on either axis are
single-object additions with their own L0 rows.

## Earned-next payoffs (NOT in scope here)

- **drawn-Γ**: a user-drawn curve whose Fourier transform IS the
  CouplingFunction vector, subsuming Sakaguchi + Daido + friction as canvas
  positions. The phase carpet already visualizes the result.
- **spatial/distance-dependent W**: topology as audible stereo space — the
  natural home for "chimera you can point to."

Both prototype-first per ADR-003 when proposed. Neither is committed by
ADR-045, which commits only to the two seams + the parity-preserving lift.
