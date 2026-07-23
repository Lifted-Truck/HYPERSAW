# 2026-07-22 — Glassy saw-shape bench (round + pitch-track) + pan motion

Human brainstorm: give the saws a glassier, harmonic-rich supersaw quality;
from the NI demo the waves get ROUNDER as the voices climb. Wants to A/B a few
saw-generation approaches. Also a pan-motion slider for stereo dynamics. Stereo
decorrelation acknowledged (human's own read: double the rotor + relative detune,
halve max voices — affirmed, deferred to the OSC2/3 multi-bank track).

## Changes (audition lab only — docs/design/detune-lab.html)
- **saw-shape bench (glass):** per-voice morph of the BLEP saw toward a rounder
  waveform. `round` (0…1) = amount; `round → ` target select {triangle (rich) /
  parabola (hollow) / sine (pure)}; `round × hi` (0…1) scales the roundness by
  pitch so higher voices round MORE — the "rounder as they climb" quality. Per-
  voice amount `rnd[i] = round·(1 + roundHi·(2·up−1))` clamped, computed in
  controlTick alongside vgain (up = pitch position, 0 root … 1 top).
- **pan motion** (0…1): slow, decorrelated per-voice pan LFOs (0.08 + i·0.021 Hz)
  sweep the base pan for stereo movement; updated once per block. Modulated pan is
  drawn on the voice map (dots move on X).

## Evidence
- **STEREO defaults Δ = 0** vs HEAD: round=0 → no shaping; panMotion=0 → static
  pan (PL/PR = base). Fully inert.
- Headless: HF-energy ratio drops with roundness — pure saw 0.137 → triangle 0.034
  → sine 0.030 (rounder = fewer highs). round×hi per-voice `[0,.17,.33,.5,.67,.83,1]`
  (root sharp, top fully rounded). Pan motion changes voice pans over time. All-on
  (round+roundHi+panMotion+comb+hpf, 4-note chord) peak 0.667, 0 non-finite.
- Browser (live): no console errors; shapeTgt/round/roundHi/panMotion present +
  wired; in-pane per-voice roundness ramps [0.12,0.33,0.52] with parabola target.
- `node --check` clean; `./verify fast` EXIT 0. Design-lab HTML — no oracle/core
  impact, no ADR.
- git: on branch lab-detune-octave-spread (PR #70)

## Notes / next
- The shaped targets (triangle/parabola/sine) are naive (not BLEP'd) but round OFF
  the high harmonics, so they alias LESS than the saw; oversample cleans residue.
  If a "formula" wins, the port version should band-limit the target properly.
- Deferred (human): stereo decorrelation via a doubled swarm (L/R) with relative
  detune + halved max voices — belongs with OSC2/OSC3 parallel banks (ROADMAP
  architecture-expansion section). Prototype-first when taken up.
- Open question for the ear: is "glassy" best from round-toward-triangle (keeps
  richness) or from cleaner anti-aliasing? The oversample A/B still pending.
