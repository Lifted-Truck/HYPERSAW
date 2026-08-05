# 2026-08-04 — SPECTRA lab: the brief's premise was only half right

## Changes
- `docs/design/spectra-lab.html` — new bench (lab campaign 3 item 1).
- `ROADMAP.md` — B6 status + findings.

## Measured the core before building
**SPECTRA does not sound like SAW — it sounds DARKER.** Centroid **562 vs 2449 Hz** at
A2, because 12 partials at 110 Hz stop at 1.3 kHz. The brief assumed the problem was
differentiation; the measurement says the problem is reach. That reframes the question to
"is dark-and-evolving the identity, or should it chase brightness?", with partial count as
the lever and CPU as the cost.

**K spends 85 % of its travel doing nothing.** Core, 0.05 steps: R at the free-run floor
(~0.28) from K 0 to 0.45, drifting slightly DOWN (0.282 → 0.251); the whole lock happens
between **0.65 and 0.85**; saturated above. Lab port: usable band 0.50–0.65 = **15 %** of
travel → **40 %** with a piecewise taper (2.7×). Fourth taper failure in this project.

**`seed` cannot move the spectrum, by construction** — cloud offsets are `2m/(M−1) − 1`,
an even ramp, so seed only touches phase. Measured 0.00 dB across seeds. SAW uses seeded
gaussian/cauchy draws. Even/gaussian/cauchy spacing is offered in the lab as a candidate.

**cascade and dissolve are healthy and buried** — and they are the identity, because a
detuned saw bank cannot stagger which partial locks when. Cascade: 5.7–11.2 dB sustained,
R climbing 0.32 → 0.52 over seconds. Dissolve: 0.05 s → immediate, 8 s → R 0.974 at 4.5 s.

## Two of my own errors, both caught by measurement
1. **Three wrong instruments before the right one.** Steady-state FFT (blind to timing
   knobs — reported cascade and dissolve as 0.00 dB "dead"); zero-crossing proxy (blind to
   spectral knobs, dominated by the fundamental — disagreed with the first about onset);
   time-resolved FFT (blind to phase lock, which magnitude spectra average away). The
   right instrument was the **order parameter R**, which the engine already computes.
   Had I stopped at the first, I would have reported two healthy controls as dead.
2. **My first taper measured WORSE than no taper** — `0.45 + 0.45·K^0.65` gave 10 % of
   travel vs raw's 20 %, because a power curve rises fastest exactly where the transition
   needed it slowest. Replaced with a piecewise map. Only measuring the taper caught it.

**Honest limit recorded in the lab:** Kuramoto lock is a genuine phase transition, so the
knee is physics. A taper can move the knee to mid-knob; it cannot make lock gradual.

## Evidence
Lab verified in-browser: script completes; polyphony correct (3 held, release middle
leaves 45 and 57 gated); audible (peak 0.33 → 0.52 as K rises); R 0.365 → 0.971;
raw transition 15 % of travel vs 40 % tapered; centroid readout 425 Hz, top partial
1.3 kHz — reproducing the darkness finding in the lab itself.
`./verify fast` exit 0.

## Open (human)
Brightness direction; whether cloud spacing becomes a real parameter; whether
cascade/dissolve get promoted in the GUI; whether the K taper folds.
