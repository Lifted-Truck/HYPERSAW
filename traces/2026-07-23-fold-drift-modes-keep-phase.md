# 2026-07-23 — Third fold (grouped): drift modes + keep-phase (ADR-062)

Human chose to group low-risk supersets. Two inert-default features, one PR,
each proven individually. Frequency smoothing DEFERRED (it is always-on in the
lab → a behavioural change that shifts drift/sweep transients, not a clean inert
superset; needs its own increment + an always-on-vs-opt-in decision).
Off fresh main after #74 merged (gh pr view #74 = MERGED first — L0004 reflex).

## Changes (reference-first, ADR-003)
- swarmsaw.html: `driftMode`/`keepPhase` params; per-voice `driftPh`/`driftHoldT`;
  synth `lastPhase`; drift-mode branches (sine / S&H alongside the walk);
  note-on `keepPhase ? lastPhase : (retrig ? 0 : rand)` + driftPh/HoldT reset;
  render-tail `lastPhase.set(focus.phase)`.
- swarm_core.h: mirrored bit-for-bit — keep-phase placed ABOVE the core-only
  `scatter` branch (ADR-033) to match swarmsaw precedence and keep rngNext in
  lockstep; paramSlot driftMode/keepPhase; render-tail memcpy snapshot.
- gen_goldens.mjs: drift-sine, drift-sh, keep-phase (with drift) scenarios.
- DECISIONS.md: ADR-062.

## Evidence
- Inert first: extracted vs origin/main → defaults / drift-walk / gauss+tilt
  BIT-IDENTICAL at driftMode=0/keepPhase=0; sine / S&H / keep-phase each differ.
- ./verify full EXIT 0: parity 63/63 → **72/72**; drift rms 0.000e+00, keep-phase
  ≤ 6e-18; all 9 chains + notefuzz GREEN.
- git: on branch fold-motion-phase (off fresh main).

## Next
Frequency smoothing (its own PR + decision), then centre-pin + pan-motion, master
HPF; then the new detune laws; then the divergences. CLAP params batched at end.
