# 2026-07-19 — Track E1 increment 1: resonator bank (filter_core) bit-exact

src/filter_core.h — FilterLab ported verbatim, consuming force_core for
the population (E0 unification pays off) + the filter audio path (polyBLEP
exciter, TPT SVF bank, collapse→Q, wet/dry, tanh). Oracle chain:
gen_filter_goldens.mjs (FilterLab live from swarmfilter.html, noise=0) +
filter_check (L0-1 parity + L0-14 audio). Result: RMS 0.0 on 11/11
scenarios FIRST build; L0-14 collapse→Q Qeff 30.4 under K=+1. Wired into
verify full (now 6 oracle chains). noise=0 protocol pinned (Math.random
dither non-deterministic; runtime noise mix ported + seeded).

Increment 1 of E1. Next: notch swarm (L0-16/17), external-audio input,
effect-plugin shell target. Population halves stay in force_check.

Evidence: verify full green (parity 51/51 · trajectories · state · force ·
spectra rms 0 · FILTER rms 0); pluginval 10 SUCCESS.

MPE: 3 Sonnet research agents — advertisement path clean (v0.15.1
line-for-line), SPECTRA is a bend dead-end (no noteTune), SAW plumbing
clean → suspicion on Live scan-cache / host behavior. Fix tabled.
