# 2026-07-24 — Tenth fold: root-pinned pacemaker, pivotMode (ADR-069)

First approved divergence (#17): a COUPLING-LAW change, not placement/tone.
pivotMode 1 = every voice entrains to the voice nearest f0; sin(root−self)=0
for the root, so it stays and the swarm folds onto the played pitch. Splay
re-anchors on the root too. Both modes kept (human, 2026-07-22).
Off fresh main after #85; #86 (docs) open in parallel with disjoint files.

## Changes (reference-first, ADR-003)
- swarmsaw.html: `pivotMode:0`; rootIdx scan (only when pivoting); pivot branch
  in the sync term; c0 re-anchored on the root.
- swarm_core.h: mirrored on the topo-0/poles-1 path only (lab has neither
  topology nor poles — other combos gated off by construction). alphaR rides
  inside the pacemaker term (0 → bit-equal, ADR-023 pattern). + paramSlot.
- gen_goldens.mjs: pivot-lock / pivot-splay / pivot-anchor — all COUPLED
  (pivot only acts through coupling), spreads modest to stay in the L0-1
  parity domain.
- trajectory_check.cpp: ADR-069 anchor.
- DECISIONS.md: ADR-069.

## Kept as-is (flagged, not "fixed")
The pacemaker drops the R scaler → onset just off K=0 slightly stronger than
mean-field. Folded verbatim: re-tapering would diverge from what the human
auditioned; a knob-curve (ADR-059 precedent) can address feel later without
touching DSP.

## Evidence
- Inertness by manifest diff: all **117 pre-existing renders sha256-identical**.
- ./verify full EXIT 0: parity 117/117 → **126/126**, the 9 new at
  **rms 0.000e+00** — the COUPLED pacemaker renders exactly (a pinned root
  leaves no drifting mean to amplify ULPs, unlike the ADR-065 chaotic case).
- Anchor: bounded/NaN-clean both K signs; entrains ΔR **0.672** (gate 0.2).
- Post-docs `./verify fast` EXIT 0.
- git: on branch fold-root-pivot (off fresh main).

## Next (#17 remaining)
- **Alternating-pan default image** — the one TRUE divergence (changes the
  DEFAULT stereo of existing patches): needs re-measured goldens for every
  width-bearing scenario + its own ADR; approved 2026-07-24.
- **Saw-shape retarget** — still blocked on measured synth-saw profiles.
Then KS comb + comp/limiter as E3 rack slots; batched CLAP pass (#18, now
+ pivotMode).
