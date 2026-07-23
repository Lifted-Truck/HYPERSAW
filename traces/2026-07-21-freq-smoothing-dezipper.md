# 2026-07-21 — Detune lab: per-voice frequency smoothing (de-zipper)

Human, in harmonic law: "the journey up the slider is a grainy one with lots of
clicks." Diagnosed as control-rate zipper: the harmonic law moves high voices
across octaves (voice 7 travels f0 → 7·f0), and voice frequencies are recomputed
only at the control rate (2756 Hz). A sparse slider event throws a large one-tick
frequency step at the top voices — that stepping is the grain, big steps are the
clicks. Classic detune laws never exposed it (they move voices ±100 ¢).

## Change (audition lab only — docs/design/detune-lab.html)
- Per-voice target-frequency slew: `vfSm[i] += fSmooth·(target − vfSm[i])`,
  `fSmooth = 1 − exp(−(16/sr)/0.006)` (~6 ms one-pole). Downstream uses vfSm.
- Snaps (no glide) on note-on (`s.vfInit=0`) and on rebuild (layout change), so
  a new note or a voice-count change starts on-pitch instead of swooping.
- At steady state vfSm == target → held-note audio is unchanged.

## Evidence
- **Steady-state defaults bit-identical** to HEAD: fingerprint Δ = 0 (smoother
  converges to target when params are static; only slider *motion* is affected).
- Realistic sparse-drag sim (dep jumps 0.08 every ~12 ticks, held between): top
  voice worst-case per-tick frequency jump 105.6 Hz → 12.0 Hz (**89% smaller**),
  the large step now spread over ~6 ms instead of one control tick.
- Browser: no console errors. `node --check` clean; `./verify fast` EXIT 0.
  Design-lab HTML — not in the build or the 9 oracle chains, no parity/core
  impact, no ADR.
- git: on branch lab-detune-octave-spread (PR #70)

## Notes / next
- 6 ms is a fixed internal de-zipper (reduce-not-invent). If audible glide
  control is wanted, expose it as a "spread glide" knob later.
- Any residual grain at high notes/wide spread is likely genuine aliasing (high
  harmonics near Nyquist) — that's what the `oversample` toggle is for; separate
  from this zipper fix.
- Karplus-Strong comb module confirmed a keeper by the human ("love it in any
  case") — retain through the port.
- When the harmonic law + this fix fold into swarmsaw.html and port to
  swarm_core.h, the freq slew must be expressed in seconds → per-tick coeff
  (ADR-009 discipline: no hand-tuned per-tick constants), same as other slews.
