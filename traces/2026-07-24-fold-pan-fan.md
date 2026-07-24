# 2026-07-24 — Eleventh fold: alternating pitch-ranked pan fan as default (ADR-070)

Second approved divergence (#17) — the only DEFAULT-CHANGING one: the stereo
image of every width-bearing patch. Fan = pitch-ranked (index when harmonic),
root dead-centre, alternating sides, curve/invert reshaping. Legacy
x-proportional image kept as `panLayout` 1. Off fresh main after #86/#87.

## Changes (reference-first, ADR-003)
- swarmsaw.html: `panLayout:0, panCurve:0.5, panInvert:0`; pan loop branched
  (fan default / legacy 1); rebuild triggers grew (law/pan* — idempotent).
- swarm_core.h: mirrored inside the ADR-035 scatter structure (fan fills
  pos[], scatter blend + panBase capture unchanged); `<algorithm>` include;
  **std::stable_sort** (JS sort is stable; gaussian/cauchy clamp at ±1 so
  ties are reachable — std::sort would break parity on some seed eventually);
  rebuild triggers mirrored; paramSlot entries.
- gen_goldens.mjs: pan-fan-gauss / -curve / -invert / -harm / pan-legacy;
  DYN_BASE pins `panLayout: 1` (coreToDyn drops it).
- DECISIONS.md: ADR-070.

## The red that earned its keep
First full run FAILED: all 8 dyn scenario groups at rms 2.5e-2..5.6e-2. The
dyn scenarios replay core-side on SwarmCore (default image moved) against
DynSynth (separate frozen program, legacy image). Fix: pin panLayout 1 in
DYN_BASE. The failure is the evidence the pin is honest — the oracle caught a
cross-reference interaction inspection missed.

## Divergence scope, measured
- Manifest diff vs origin/main: all **34 SAW groups changed** (predicted:
  width 0.8 default), all **8 dyn groups sha256-identical**, spectra untouched.
- **pan-legacy** (= old tilt-thin params + panLayout 1) hash-MATCHES the
  pre-fold tilt-thin on all 3 seeds — the old image survives bit-for-bit
  behind the toggle.
- Mono content shifts slightly under the new default (constant-power panning)
  — the lab's documented exception, accepted with the divergence.

## Evidence
- ./verify full EXIT 0 (after the dyn pin): parity 126/126 → **141/141**, the
  15 new at **rms 0.000e+00**; pan-fan-gauss per-seed hashes differ (ranking
  does real work on seeded layouts); trajectory anchors unaffected (R-based);
  all chains + notefuzz GREEN. Post-docs `./verify fast` EXIT 0.
- git: on branch fold-pan-fan (off fresh main).

## Next
#17 remaining: saw-shape retarget only (blocked on measured synth-saw
profiles). Then: KS comb + comp/limiter as E3 rack slots; rootWeight fold;
batched CLAP pass (#18 — now + panLayout/panCurve/panInvert/pivotMode).
