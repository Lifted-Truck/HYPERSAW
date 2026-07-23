# 2026-07-23 — Fourth fold: opt-in frequency glide / de-zipper (ADR-063)

Human approved OPT-IN ("for now") rather than the lab's always-on smoothing,
keeping it a clean inert superset instead of a reference behaviour change.

## Changes (reference-first, ADR-003)
- swarmsaw.html: `freqGlide` param (SECONDS, default 0); per-voice `vfSm`/`fRun`
  + `vfInit`; control-rate smoothing of the frequency target; note-on snap;
  per-sample slew leg in render (quarter time constant); guarded so 0 = the
  original path untouched.
- swarm_core.h: mirrored bit-for-bit (param, Swarm state, controlTick coefficients
  + smoothing + snap, render per-sample leg, paramSlot "freqGlide").
- gen_goldens.mjs: `freq-glide` scenario (0.006 s against S&H drift at full rate —
  the steppiest source, so the smoothing is genuinely under load).
- DECISIONS.md: ADR-063.

## Evidence
- Inert first vs origin/main at freqGlide=0: defaults / drift-walk / drift-sine /
  keep-phase+tilt all BIT-IDENTICAL; glide on changes output.
- Efficacy: S&H per-tick voice-frequency jump **9.26 Hz → 0.54 Hz (94% smaller)**.
- ./verify full EXIT 0: parity 72/72 → **75/75**; the 3 new at **rms 0.000e+00**;
  all 9 chains + notefuzz GREEN.
- git: on branch fold-freq-glide (off fresh main after #75).

## Next
Remaining supersets: centre-pin + pan-motion (coupled — group them), master HPF.
Then the new detune laws (harmonic+reach, stretch, golden). Then the divergences
(root-pivot topology, pan default, saw-shape retarget). CLAP params batched last.
