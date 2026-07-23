# 2026-07-23 — Fifth fold (grouped): pan motion + centre pin (ADR-064)

Grouped because coupled: the centre pin scales pan motion as well as drift.
Off fresh main after #76 merged (gh pr view #76 = MERGED first — L0004 reflex).

## Changes (reference-first, ADR-003)
- swarmsaw.html: `panMotion`/`panMode`/`motionCenter`; signed base pan stored in
  rebuild (bit-inert); per-voice `cdist`; drift weighted by
  `mw = 1 − motionCenter·(1 − cdist)` (exactly 1.0 when off); per-block pan-motion
  block (drift / sweep modes) producing modulated L/R gains; voice loop reads them.
- swarm_core.h: mirrored. Member named `panBase` (rebuild has a local `pan`);
  base pan captured POST-scatter (ADR-033) so motion rides on top of scatter.
  paramSlot for all three.
- gen_goldens.mjs: pan-drift, pan-sweep, centre-pin (drives BOTH legs).
- DECISIONS.md: ADR-064.

## Evidence
- Inert first vs origin/main: defaults / drift-walk / wide+tilt / glide+S&H all
  BIT-IDENTICAL at the defaults; each feature differs when engaged.
- Centre pin measured: centre voice drift swing 8.06 → **0.00 Hz** (pinned);
  edge voice 9.98 → 9.98 Hz (untouched).
- ./verify full EXIT 0: parity 75/75 → **84/84**, all 9 new at **rms 0.000e+00**;
  all 9 chains + notefuzz GREEN.
- git: on branch fold-pan-motion (off fresh main).

## Next
Master HPF (last simple superset), then the NEW DETUNE LAWS (harmonic+reach,
stretch, golden) — substantive ADRs — then the divergences (root-pivot topology,
pan default image, saw-shape retarget). CLAP params batched last.
