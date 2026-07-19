# 2026-07-19 — Track E1.2: notch swarm (notch_core) bit-exact

src/notch_core.h — PhaserLab ported verbatim on force_core: exciter +
feedback true-notch cascade (SVF y=x-k*v1) + tanh feedback + NaN watchdog;
constant Q. Oracle: gen_notch_goldens.mjs (PhaserLab live from
swarmphaser.html, noise=0) + notch_check (L0-1 parity + L0-16 exactness).
Result: RMS 0.0 on 11/11 first build; L0-16 on-notch 158 dB deeper than
off-notch (ref ~150 dB true-notch nulls — guards the ADR-029b allpass
defect). Wired into verify full (7 oracle chains).

Parity note: PhaserLab's construction-time controlTick is wiped by the
seed-triggered rebuild (generator applies seed last), so notch_core's
omission of it is harmless — pre-render state matches.

E1 status: both frequency engines (filter ADR-043, notch ADR-046) are
parity-proven cores. NEXT: the effect-test shell (external audio in) so
they're auditionable — the human's "testing them out" step.

Evidence: verify full green (parity 51/51 · trajectories · state · force ·
spectra rms 0 · filter rms 0 · notch rms 0).
