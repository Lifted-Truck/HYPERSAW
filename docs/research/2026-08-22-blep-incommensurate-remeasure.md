# BLEP aliasing re-measured at incommensurate f0 (2026-08-22)

**Queue item:** ROADMAP B12, "BLEP aliasing re-measure at incommensurate f0 —
earlier measurement used a commensurate f0."

## Background — the flaw being fixed

ROADMAP's "Clean-mode aliasing measured (2026-08-01)" entry measured aliasing
with the inter-harmonic-midpoint protocol (L0016/L0017: sample the spectrum at
`(k+0.5)*f0`, where any real harmonic's windowed leakage is structurally
absent, instead of summing every non-harmonic bin) at f0 = E3 (164.8 Hz),
660 Hz, and 1763 Hz, and reported BLEP at −180 to −186 dB re h1. That entry
flagged its own limit before drawing a conclusion:

> these renders used BIN-COMMENSURATE f0 (right for the droop test, wrong
> here) — folded aliases of a commensurate saw land ON the harmonic grid, so
> midpoints are structurally blind to them. The BLEP "−180" rows are the
> protocol seeing nothing, not the saw being that clean.

This is provable, not just suspected: for an FFT of length `N` and an f0 that
is an exact multiple of the bin width (`f0 = K·sr/N` for integer `K`), every
harmonic `k·f0` sits at bin `k·K`, and any aliased fold of a harmonic is a
modular reduction of that bin index by `N` — still an integer, so still an
exact bin. Sampling at midpoints `(k+0.5)·f0`, chosen specifically to avoid
the harmonic bins, therefore also avoids every aliased copy of a harmonic,
independent of how much aliasing is actually present. B12 asks for the
re-run at an f0 with no such relationship to the sample rate.

## Method

Reused the existing protocol (inter-harmonic-midpoint FFT sampling relative
to h1, per L0016/L0017) unchanged except for f0 choice. New scratch tool:
`tools/blep_alias_incommensurate_probe.cpp` (standalone, not wired into
CMakeLists.txt or `./verify`, per brief scope).

1. Render one `SwarmCore` voice (`n=1`, no detune/width/drift, `K=0`) at a
   given f0, with `digital=0` (naive, no BLEP), `digital=1` (BLEP), and
   `digital=1` + `oversample=1` (ADR-075's 2x OS + BLEP).
2. Skip 20000 samples (envelope/gravity settle, matches `waveshape_check`'s
   skip), take `N = 2^17 = 131072` steady-state samples (bin width ≈
   0.3364 Hz), Hann-window, FFT (self-contained radix-2, no dependency
   added).
3. `h1` = peak magnitude within ±5 bins of f0 (an incommensurate f0 does not
   sit on an exact bin, so a fixed-bin read would undershoot the true peak).
4. For every midpoint `(k+0.5)·f0` up to just under Nyquist, take the local
   peak within ±2 bins and report its level relative to `h1` in dB. Report
   worst and mean across all midpoints sampled.

Three fundamentals: one bin-exact **commensurate control** (`673 Hz` at
`K=2000` of this run's own FFT grid — reproduces the earlier protocol's
blind spot under a controlled setup, rather than trusting the old report's
absolute numbers, which used a different N/window and are not directly
comparable in dB terms), and two **incommensurate** fundamentals — 441.3 Hz
(the brief's example) and 1760.3 Hz (near the earlier 1763 Hz reading,
shifted off any small-integer relationship to 44100).

Calibration check (L0016 — never trust a new detector without one): the
naive incommensurate readings below (−74 to −82 dB) are the "known-bad"
case and confirm the detector fires; raw FFT magnitudes were also inspected
directly (h1 ≈ 4772 at 441.3 Hz naive, midpoint magnitude ≈ 0.34 → ratio
≈ 7.2×10⁻⁵ ≈ −82.8 dB, matching the tool's own reported number) to rule out
a degenerate near-zero-magnitude artifact producing spuriously large
negative dB numbers.

Build: fresh `build-b12/` (`cmake -S . -B build-b12 -G "Unix Makefiles"
-DCMAKE_BUILD_TYPE=Release`, submodules initialized in this worktree first
since they were absent — `git submodule update --init --recursive`). The
probe itself needs no linking (`swarm_core.h` is header-only) and was built
standalone:

```
clang++ -std=c++20 -O2 tools/blep_alias_incommensurate_probe.cpp \
  -o build-b12/blep_alias_incommensurate_probe
build-b12/blep_alias_incommensurate_probe
```

## Results

Aliasing residue, dB relative to h1 (worst / mean across all sampled
midpoints):

| f0 | mode | worst | mean | n midpoints |
|---|---|---|---|---|
| commensurate 673.4 Hz (bin-exact control) | naive | −190.6 | −191.3 | 32 |
| commensurate 673.4 Hz | BLEP | −216.7 | −227.3 | 32 |
| commensurate 673.4 Hz | BLEP + 2x OS | −218.9 | −241.6 | 32 |
| **incommensurate 441.3 Hz** | **naive** | **−81.1** | **−82.8** | 49 |
| **incommensurate 441.3 Hz** | **BLEP** | **−178.2** | **−187.0** | 49 |
| incommensurate 441.3 Hz | BLEP + 2x OS | −177.7 | −185.8 | 49 |
| **incommensurate 1760.3 Hz** | **naive** | **−74.0** | **−75.1** | 12 |
| **incommensurate 1760.3 Hz** | **BLEP** | **−190.5** | **−194.6** | 12 |
| incommensurate 1760.3 Hz | BLEP + 2x OS | −187.4 | −191.2 | 12 |

## Reading

The commensurate control reproduces the flaw exactly as predicted: naive
(unBLEP'd) reads a deceptive **−190.6 dB**, indistinguishable from BLEP —
because at a bin-exact f0 every aliased fold of every harmonic lands
precisely on the harmonic grid the midpoint protocol is built to avoid, so
the metric measures nothing there regardless of how aliased the signal
actually is. Move to an incommensurate f0 and naive immediately shows its
true colors: **−74 to −81 dB worst-case**, a dense, audible aliasing floor —
confirming the detector was blind before, not that naive mode is secretly
clean.

BLEP at the same incommensurate fundamentals reads **−178 to −190 dB worst,
−187 to −195 dB mean** — over 100 dB below the naive floor and far below
any audible threshold (conventionally ~−60 dB is audible territory, low
double digits is not). This is not the same absolute number as the earlier
(differently-windowed, differently-N) commensurate report's "−180 dB," but
it lands in the same regime, and — crucially — it is now measured at
fundamentals where the protocol is structurally capable of seeing aliasing
(as the naive rows prove), rather than at fundamentals where it structurally
cannot. 2x oversampling on top of BLEP does not measurably change the
picture (already at the floor); that is expected — oversampling targets
*droop*, not the aliasing BLEP already suppresses.

## Verdict

**The earlier conclusion survives, and is now actually supported by
evidence rather than by a blind measurement.** ROADMAP's "Clean-mode
aliasing measured" entry left "BLEP aliasing at incommensurate f0" as an
open decision input, expecting (per an even earlier shape-lab reading) "very
clean, but measure, don't assume." That expectation holds: BLEP's true
aliasing suppression at incommensurate fundamentals is ~178–190 dB below the
fundamental — deeply inaudible — while the same protocol correctly exposes
naive mode's real aliasing (−74 to −82 dB, audible) at the exact same
fundamentals, which is the calibration proof that this run is not repeating
the earlier blind spot. The downstream decision this unblocks (ROADMAP's
"fix menu" for HF droop — oversampled BLEP "flat AND clean" vs. a
higher-order kernel vs. a wavetable path) can now treat "BLEP verifies
clean" as measured fact, not assumption: oversampled BLEP is flat (ADR-075,
shipped) **and** now independently confirmed clean at a fundamental the
protocol can actually see aliasing at.

## Open questions

- The commensurate-control numbers here are not numerically comparable to
  the earlier 2026-08-01 report's (different FFT length/window/skip), only
  qualitatively — both show the same blind-spot shape. A literal
  reproduction of the old N/window was not attempted; it was not needed to
  answer B12's actual question.
- Only `n=1`, no detune/width — a multi-voice detuned swarm's aliasing
  budget (intermodulation between voices, not just per-voice harmonic
  aliasing) is a different, larger question and out of scope here.
