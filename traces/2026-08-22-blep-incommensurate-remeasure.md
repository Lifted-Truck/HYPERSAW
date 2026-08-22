# blep-incommensurate-remeasure — BLEP aliasing re-measured at an incommensurate f0

- **Queue item:** ROADMAP B12, "BLEP aliasing re-measure at incommensurate f0
  — earlier measurement used a commensurate f0"
- **Why:** the 2026-08-01 "Clean-mode aliasing measured" entry flagged its
  own protocol limit — a bin-commensurate f0 makes every aliased fold of a
  harmonic land exactly on the harmonic FFT-bin grid, so the inter-harmonic
  midpoint aliasing metric (L0016/L0017) is structurally blind to it there.
  B12 asked for the re-run at f0 with no such small-integer relationship to
  44100 Hz.
- **Evidence consulted:** ROADMAP.md ("Clean-mode aliasing measured
  (2026-08-01)", "HF-rolloff hypothesis CONFIRMED", B12 queue entry);
  LIBRARY.md L0016/L0017 (detector calibration lessons); `docs/design/
  shape-lab.html` (the midpoint-sampling protocol's original site);
  `src/swarm_core.h` (`digital`/`oversample` params, BLEP implementation
  around line 857); `tools/waveshape_check.cpp` (SwarmCore render pattern
  reused for the probe).
- **Alternatives rejected:** reproducing the earlier report's exact
  N/window/skip to get numerically comparable absolute dB values —
  rejected because B12's actual question (does BLEP verify clean at an f0
  the protocol can see) does not need bit-for-bit comparability, only a
  sound, calibrated re-run; the report notes this explicitly as an open
  question rather than silently claiming equivalence.
- **What changed:** new scratch tool `tools/blep_alias_incommensurate_probe.cpp`
  (standalone, header-only build against `src/swarm_core.h`, not wired into
  CMakeLists.txt or `./verify`); new report
  `docs/research/2026-08-22-blep-incommensurate-remeasure.md` with the
  measured tables, calibration evidence (naive incommensurate reads −74 to
  −82 dB, proving the detector is not blind at these fundamentals; the
  commensurate control reproduces the old blind spot at −190 dB regardless
  of BLEP on/off), and the verdict: BLEP's true aliasing suppression at
  incommensurate f0 is −178 to −190 dB, so the earlier "BLEP verifies clean"
  expectation survives, now on real evidence instead of a blind measurement.
- **Verify:** `./verify fast`, exit 0, git d6d6520 (`.harness/last-verify.json`).
  No src/ or existing-tool edits; new files only, per brief scope.
- **Open questions:** none beyond what the report states — the commensurate
  numbers here are not literally comparable to 2026-08-01's (different
  FFT length/window/skip, only qualitatively equivalent), and multi-voice
  detuned-swarm aliasing (intermodulation, not just per-voice harmonic
  aliasing) is a separate, larger question left open.
