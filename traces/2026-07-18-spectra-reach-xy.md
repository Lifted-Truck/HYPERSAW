# 2026-07-18 — SPECTRA reach: 32 partials + XY→cascade; post-E1 transition recorded

**32 partials (ADR-040).** kPMax 24→32 (range extension; reference math
untouched, goldens ≤12 hold at rms 0). VizSnapshot per-partial arrays +
strip viz grew to 32; param 44 max 32; slider max 32.

**XY→cascade in SPECTRA.** The XY x-axis was hardwired to detune (a
SAW-only param — inert in SPECTRA). Now mode-aware via xAxisId():
cascade (id 51) in SPECTRA, detune (id 4) in SAW; y=K in both. Label
updates ("x=cascade"/"x=detune"), crosshair reloads from the live value
on engine switch (poll-time, never mid-drag), gestures target the right
id. Verified headlessly: SPECTRA XY drives ids 51+6, label correct.

**ADR-041 / Phase F.** Recorded the human's scheduled transition: at the
Track E1 gate the parity-to-prototype contract graduates to forward
performance standards (new golden references from the liberated impl,
L0 suites repointed one engine at a time, prototypes → provenance).
Nothing changes today's gates. Octave-locked mono sub parked (#17).

Evidence: verify full green (51/51 · trajectories · state · force ·
spectra rms 0); pluginval 10 SUCCESS; installed.
