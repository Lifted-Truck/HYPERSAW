# 2026-07-22 — Audit fixes: onePoleCoef helper, notefuzz gate, QM relocation

Acting on the read-only audit (2026-07-22), human approved all three:
E1 (DRY the coefficient formula), C1 (wire the ungated voice-hang oracle), and
the Quantum Morph file relocation.

## Changes
- **E1 — `forcecore::onePoleCoef(tau, sr)`** (`force_core.h`): one named helper
  `1.0 - std::exp(-1.0 / (tau * sr))`, replacing the same expression inlined at
  **14 sites** (env atk/rel/dec) across swarm/spectra/filter/notch/time/
  swarmalator cores. Bit-identical by construction (same operands, same order,
  `inline`) — the ADR-009 invariant now lives in ONE greppable place. Left the
  four *different* idioms alone (glide dt-form swarm:615, cutoff-form swarm:837,
  time damp cutoff-form time:129/254 — not the same formula).
- **C1 — notefuzz gate** (`verify` full()): added `notefuzz_check` after
  state_check. It was built (CMakeLists:131) but invoked by neither `verify full`
  nor CI, so the 2026-07-18 "note won't stop when released" hang class gated
  nothing (ADR-038). Statically linked like state_check; no goldens.
- **QM relocation:** `QM-0/QM-2-*-spec.md` → `docs/proposals/`,
  `quantum-morph-lab.html` → `docs/design/` (matching detune-lab.html); root
  decluttered. Added a `.gitignore` block for both paths — they name a private
  sibling by real name, so they stay local-only until aliased (ADR-014). ROADMAP
  (on PR #70) tracks the plan; its QM entry's "sit at root" wording to be updated
  to "relocated" on that branch.

## Evidence
- `./verify full` **EXIT 0**: parity_check **54/54** within eps (swarm scenarios
  rms 0; worst 4.26e-9 @ dyn-ring is the pre-existing dynamics tolerance,
  unchanged by E1); trajectory/state GREEN; **notefuzz_check GREEN (0 hangs)** —
  now gating; force/spectra/filter/notch/swarmalator/time all GREEN.
- E1 bit-identity is by construction (identical arithmetic, inlined) and
  corroborated by parity 54/54.
- git: on branch fix-audit-onepole-notefuzz (off origin/main).

## Not done / deferred (from audit, left as recommendations)
- S2 (LOW): three prototype-lab homes — the QM move fixes part; a full "labs
  under docs/design only" convention pass is deferred.
- E2 (LOW): softMix helper across the FX cores — only worth folding if more DRY
  work happens; not done.
- E3 (LOW): dead `saw2` assignment swarm_core.h:372 — trivial, left for a lab-
  side pass (touches the parity-frozen waveshape region; not worth a core commit
  alone).
