# 2026-07-20 — SPECTRA ADSR (ADR-055): parity-safe superset, own reference AR

The shell's Attack/Decay/Sustain/Release knobs (ids 19-22) routed to the SAW
core but were DEAD in SPECTRA mode — SpectraCore had no ADSR, running a
hardcoded AR (`atk=1-exp(-1/(0.004·sr))`, `rel=1-exp(-1/(0.18·sr))`). Added
ADSR to SpectraCore mirroring ADR-021's superset-with-inert-default.

## What changed
- `src/spectra_core.h`: SParams gains `attackS/decayS/sustainL/releaseS`
  defaulting to SPECTRA's OWN reference constants (0.004/0.18/1.0/0.18 — NOT
  SAW's 0.003/0.16); SSwarm gains `inAttack` (re-armed per strike); render()'s
  hardcoded AR replaced by the ADR-021 sustain machine (sustainL>=1 → the exact
  former expressions; sustainL<1 → attack→decay); paramSlot maps
  sAttack/sDecay/sSustain/sRelease (names distinct from SAW's shared keys so
  shell id-19 never leaks into SPECTRA via the shared-name mirror).
- `src/hypersaw_clap.cpp`: ParamDefs 65-68 (S.Attack/Decay/Sustain/Release,
  SPECTRA reference defaults); set/read routed to the spectra core alongside
  44-55. State round-trips via the existing kParams-by-coreKey loop.
- `src/gui/gui.html`: rows 65-68 added to the Envelope cluster, gated
  SPECTRA_ONLY (19-22 already SAW_ONLY) — one cluster, engine-appropriate ADSR.
- `DECISIONS.md`: ADR-055.

## Why own params, not the shared 19-22 (the load-bearing call)
The shell's `attack` defaults to 0.003 (SAW's). Routing it into SPECTRA would
push 0.003 into the core at plugin load, silently shifting SPECTRA's default AR
off its 4 ms reference — and NO oracle would catch it: spectra_check constructs
SpectraCore DIRECTLY (spectra_check.cpp:116,137,205), bypassing the shell, so
the goldens stay green at the core's own 0.004 default while the shipped plugin
diverges. Per the epistemic-discipline doctrine (a comfortable green that
doesn't test the real path is the dangerous case), both goldens AND plugin must
be reference-exact — own params with own defaults achieve that. Same precedent
as ADR-042 (SPECTRA sub-osc took its own ids 52-55) and ADR-037's framing (two
distinct frozen references with genuinely different envelope constants).

Prototype-first (ADR-003) satisfied: swarmspectra.html has NO envelope to
mirror, and the ADSR shape is byte-identical to the already-ratified ADR-021
superset (not a new divergent envelope) — reference AR is the base, ADSR the
accepted extension. No new prototype required.

## Evidence
- `./verify full` → exit 0, all EIGHT oracle chains GREEN (recorded
  .harness/last-verify.json). spectra_check: 9/9 parity rms=0 (default ADSR
  path byte-identical to the former hardcoded AR), L0-6 zipper (7.21s/1.81s),
  L0-7 gate (-15.06 dB), P=1 gate dR=0. state_check GREEN — restored instance
  renders bit-identical (the 4 new params round-trip).
- Behavioral probe (headless SpectraCore, confirms the knobs are LIVE not
  inert): sSustain=0.3 → sustain-window rms 17% of peak (decay engages);
  sRelease 2.0s → tail rms 0.057 where 0.02s release is silent.
- pluginval strictness 10 on HYPERSAW.vst3 → SUCCESS.
- auval -v aumu Hsaw LfTk → AU VALIDATION SUCCEEDED (installed + resealed).

Branch `spectra-adsr-adr055` created off main tip (487fa06) BEFORE editing
(L0004). git hash at verify: 487fa06 (pre-commit).
