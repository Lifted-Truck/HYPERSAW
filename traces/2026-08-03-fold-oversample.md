# 2026-08-03 — ADR-075: opt-in 2x oscillator oversampling

## Changes
- swarm_core.h: `oversample` param (+rebuild trigger, since ITD is expressed in
  sub-samples and must double with the clock); voice sum wrapped in an osSub loop
  at half phase step; 63-tap halfband built in the ctor; per-swarm decimator
  history (osZL/osZR/osW). osSub == 1 leaves the original path bit-identical.
- hypersaw_clap.cpp: id 88 `oversample`.
- gui.html: toggle in Output & perception, labelled with the CPU cost.
- waveshape_check.cpp: OS-off determinism gate + 15 kHz recovery gate.

## Evidence
- parity 147/147 unchanged (worst rms 4.262e-09, pre-existing dyn-ring).
- droop: 10 k −2.17 → −1.23 · 15 k −4.50 → −2.13 · 20 k −7.56 → −5.65.
- CPU 2.5% → 6.3% of one core (8 notes x 16 voices), E-6 budget 50%.
- verify full green; GUI parse-checked.

## Open
Recovery is ~1.3 dB short of the JS spike's prediction at 15 kHz. Prime suspect:
the 1x R→tone output pole on the summed signal, which the spike did not model.
Isolation measurement attempted and abandoned (harness string-literal breakage);
worth finishing — if confirmed, oversampling the output pole too would close the
gap without touching the oscillator path.
