# 2026-07-18 — Absolute-K range raised; phase-scatter percentage (ADR-033)

**Absolute-K explained to the human (recorded for posterity).** Normal mode:
K is σ-normalized — coupling = 4·K·|K|·σ_measured, where σ is the live Hz
spread of the voices, so the phase transition sits mid-knob at any detune/
register/law (ADR-004). Absolute mode bypasses that: fixed Hz units,
because σ-normalization COLLAPSES at detune≈0 (σ floors at 0.08 Hz — no
authority at any knob position). Use it when: voices are identical/near-
identical (strict-chimera experiments, L0-10); or you want K's pull
constant while sweeping detune. Unit was my 1 Hz placeholder (max pull
4 Hz — feeble); now 2.5 Hz (max 10 Hz) per human request — no reference
constrains this mode (our superset).

**Phase scatter (param 39).** Generalizes retrigger: phase_i = rng·scatter
when scatter > 0 (1.0 reproduces the retrig-off values); at 0 the legacy
path runs bit-exactly with rng untouched — all goldens stand, proven by
parity 51/51 unchanged. Retrigger checkbox dims when overridden.

**Evidence.** verify full green; pluginval 10 SUCCESS; auval SUCCEEDED;
seals OK; installed.
