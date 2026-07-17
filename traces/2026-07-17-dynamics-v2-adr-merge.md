# 2026-07-17 — Dynamics v2 swap + external ADR merge

**What changed.** Second external drop of the day: `swarmdynamics_v2.html` +
`DECISIONS_2.md`. The dropped ADR log's 001–009 matched ours verbatim; its
010–012 were new content colliding with our numbering (we were at ADR-014).
Merged as ADR-015 (Daido multi-pole coupling; ADR-007 partially amended),
ADR-016 (grid × coupling suppression; UI disclosure requirement), ADR-017
(lock warning gated on cause AND state), each with provenance + verification
notes; internal cross-reference renumbered; `DECISIONS_2.md` deleted per
human instruction. Prototype swapped: v2 verified a strict superset of v1
(6 replaced lines, all extended; q=1 default path identical), renamed to
`swarmdynamics.html` per the ADR-012 naming policy. SPEC folds: Layer 2
gains Poles q row; §4 tempo-grid gains the grid×coupling disclosure rule;
§5.6 readouts gain grid status + R_q. ROADMAP Phase 3 gains Daido poles,
grid readout, and L0-formalization items. L0001 promoted candidate→canonical
(second independent file-drop occurrence).

**Verification (headless, Node, core extracted from v2).**
- (a) q-clusters: R₂ = 0.97 at seeds {1234, 777, 42}; R₃ = 0.97 — exact match.
- (c) timbre split: P(2f0) = 0.080 seed-invariant; P(f0) 0.0059–0.0643
  tracking imbalance — matches the drop's 0.080 / 0.006 / 0.064 to ~3 decimals.
- (d) bistability: aligned start R₁ = 0.99 sustained at identical knobs.
- ADR-017 falsifier: retrigger → R = 1.00 at Ksm = 0.000 by construction;
  corrected gate implemented verbatim in drawBeatInfo (Ksm > 0.05 AND
  coherence > 0.8, fixed-height warning line).
- ADR-016 suppression: direction confirmed; ~80× magnitude is
  harness-dependent (~5× via an independent coarser envelope method at
  detune 0.6) — merged with magnitude marked reference, direction blocking.

**Verify.** `./verify fast` green pre-commit (output in session).

**Open questions.** None new; ACCEPTANCE formalization of ADR-015 anchors is
a recorded Phase 3 gate item, not done now (criteria ratify at that gate).
