# 2026-07-21 — Voice-map fit, pan curve/invert, polyphonic comb, rebuild split

Human batch (4 concrete builds; two design questions handled in-conversation):
fit-scale the voice map; replace pan scatter with a pan curve + invert toggle;
make the Karplus-Strong comb polyphonic; and (found while there) stop a width
drag from redrawing the seeded distribution.

## Changes (audition lab only — docs/design/detune-lab.html)
- **voice map fit scale** (checkbox): when on, Y range tracks a low multiple of
  the lowest/highest voice ratio (lo/1.3 … max(lo·1.6, hi·1.3)), smoothed 0.1/frame
  so it doesn't jump. Off = fixed 0.5…40×f0. Display-only.
- **pan curve** (0…1, default 0.5) replaces pan scatter: γ = 6^(0.5−curve) reshapes
  the distance-from-centre. 0.5 = linear (the existing fan, inert), <0.5 centre-
  focused (bunch near centre), >0.5 side-focused (pushed to edges). **pan invert**
  (toggle): flips the triangle so the top voice sits centred and the root goes wide.
- **polyphonic Karplus-Strong comb**: comb buffers moved from one global delay to
  per-swarm (per-note). Each swarm's `dly` tunes to its own f0 on note-on and its
  ring clears; the comb now runs inside the per-swarm sample loop (before env), so
  a held chord gets one tuned resonator per note. Drive/normalize stay master-bus.
- **rebuild split** (rebuildDist / rebuildPan): voice frequencies (x) recompute
  only on n/dist/seed; pan only on n/width/curve/invert. Fixes a real bug — a width
  drag used to re-draw the seeded gaussian/cauchy distribution, jittering the
  pitches (and clicking). Verified gaussian x is now stable across a width change.

## Evidence
- **STEREO defaults Δ = 0** vs HEAD: pan curve 0.5 reproduces the fan, comb off,
  rebuild split inert. Fully bit-identical at default.
- Headless: pan curve centre/linear/sides + invert all correct (invert → root at
  −0.8 edge, top voice centred); polyphonic comb delays 110 Hz→401 / 440 Hz→100;
  gaussian x stable across width drag; 3-note comb chord peak 0.966, 0 non-finite.
- Browser (live): no console errors; panScatter gone, panCurve/panInvert/fitScale
  present and wired; in-pane comb delays [401,100] on a 2-note chord.
- `node --check` clean; `./verify fast` EXIT 0. Design-lab HTML — no oracle/core
  impact, no ADR.
- git: on branch lab-detune-octave-spread (PR #70)

## Deferred to conversation (not built)
- **Multi-cluster / octave-cluster Kuramoto** (human "strange idea"): emergent
  A/B/C grouping. Real area — higher-harmonic coupling sin(kΔθ) → k balanced
  clusters; a frequency/octave-proximity coupling KERNEL → clusters by register.
  Belongs with the Phase-3 dynamics topology work (mean-field/ring/two-cluster +
  Sakaguchi α already roadmapped, line 38) and the kernel abstraction (Phase 4).
  Prototype-first per ADR-003 before any core change. Logged for a lab spike.
- **Noise/clicks massage** (human): polyphonic comb + rebuild-split remove two
  click sources (comb retune, width-drag distribution jitter). Remaining noise is
  mostly aliasing (high partials near Nyquist) — gated on the human reporting the
  oversample 2×/4× A/B: if it audibly clears, that decides the AA investment
  (oscillator-spec ADR, prototype-first). Candidate next declick pass: voice-steal
  transient (env not reset on steal), per-block param interp for pan.
