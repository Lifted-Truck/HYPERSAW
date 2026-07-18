# 2026-07-18 — Phase 4: SpectraCore port, bit-exact 9/9; P=1 gate question surfaced

**Port.** src/spectra_core.h transcribes SpectraSynth verbatim (P<=24
partials x M<=7 cloud voices; per-partial Kuramoto clouds; cascade lock
gate; wmix interference narrowing; f32 SINE table with double interp;
f32 RMW output accumulation; shared mulberry32 via forcecore).

**Oracle.** gen_spectra_goldens.mjs slices SpectraSynth live from
swarmspectra.html, renders 9 scenarios (1 s, 512-block, held A3) to raw
f32; spectra_check compares full-stream RMS (<= 1e-6) and runs L0-6/7
behaviorally. Result: RMS 0.0 on ALL NINE scenarios first shot —
bit-exact including seeded free-phase, stretch, cascade, both width
laws, P=1. L0-6: front monotone, completion 7.2098 s (cascade 1, ref
8 +-30%), 1.81 s (cascade 0.5, <= 3). L0-7: -15.0589 dB (ref -14.8),
wmix 0.857. Wired into ./verify full (verify edit rides the PR).

**Gate honesty (ADR-037).** SPEC §2's "one engine, SAW = P=1" cannot be
satisfied by a literally shared voice path while both references stay
parity-frozen (sigma floors, slews, envelopes, phase updates, kernels
all differ). Options (a) measured-equivalence reinterpretation vs (b)
SPEC amendment surfaced for the human; shell integration deferred
behind the ruling per the charter (no work on ambiguous criteria).
