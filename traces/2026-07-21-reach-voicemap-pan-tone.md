# 2026-07-21 — Detune lab: reach, voice map, alternating pan, hi-tame, per-sample glide

Human batch after the harmonic-law session: (1) add the "reach" knob (decouple
top harmonic from voice count); (2) a real-time voice visualizer (pan × pitch,
target vs actual); (3) reintroduce pan scatter + a new default alternating pan
fan; (4) smooth the detune slider further; (5) tame harsh high voices
(equal-loudness + tone-tilt weighted to highs); (6) make root-weight work in
harmonic mode. Human also flagged the growing "unusable terrain" — a future
session to set per-mode parameter limits.

## Changes (audition lab only — docs/design/detune-lab.html)
- **reach** (0.25…4, default 1): harmonic law now `f = f0·(1 + dep·reach·i)`.
  reach=1 → natural series 1..n (inert). reach≠1 spreads n voices evenly across
  [1, 1+reach·(n−1)] — taller/denser than the voice count (fractional partials)
  when >1. Verified: n=7, reach=1.833 → 1, 2.83, 4.67, 6.5, 8.33, 10.17, 12.
- **voice map** (new canvas + drawVoiceMap): X = pan, Y = log pitch (×f0), with
  harmonic gridlines. ○ = raw law target (s.vfTarget), ● = actual instantaneous
  freq (s.eff), stub between = freq-smoother lag during a sweep. Root ● magenta.
- **alternating pan fan** (new default): root voice centre, each higher voice a
  step farther out on ALTERNATING sides (0 centre · 1 →R · 2 ←L …). **pan
  scatter** (0…1) blends toward a seeded random pan to break the ordered fan.
  Stored as this.pan[i] for the map. This intentionally changes the default
  stereo image (see parity note).
- **per-sample frequency glide** (fRun, ~1.5 ms): on top of the 6 ms control-rate
  smoother, chases eff every output sample → removes the residual 2756 Hz stair
  the coarse smoother left. Snapped on note-on via firstTick.
- **hi tame** (0…1): equal-loudness per-voice amplitude roll-off `(f0/f)^hiTame`
  — turns higher voices down. **tone tilt** cutoff changed to rise only as
  √(f/f0), so it trims higher voices more than the root (weighted to highs).
- **root weight in harmonic mode**: position now `i/(n−1)` and un-gated by anchor
  when law=4 (harmonic is inherently root-anchored). Verified vgain tapers 1→0.

## Evidence
- **MONO defaults Δ = 0** vs HEAD (non-pan DSP bit-identical: reach=1, hiTame=0,
  tilt=0, fRun converges at steady state). Stereo fingerprint intentionally
  differs — the new pan fan is a deliberate default change, not a silent one.
- Headless: reach hits the user's exact example; root-weight tapers in harmonic;
  pan fan `[0, .13, −.27, .4, −.53, .67, −.8]`; scatter breaks it; hiTame=1 drops
  the top voice to 0.14×; ALL-ON (9 voices, every stage) peak 0.964, 0 non-finite.
- Browser (live, audio up in pane): no console errors; all controls wired
  (reach→H12.0, hiTame, panScatter); voice map renders dots on the harmonic grid
  (cyan actual / magenta root / amber target rings), pan on X.
- `node --check` clean; `./verify fast` EXIT 0. Design-lab HTML — not in the
  build or the 9 oracle chains, no parity/core impact, no ADR.
- git: on branch lab-detune-octave-spread (PR #70)

## Next
- ROADMAP: a session to set per-mode usable parameter ranges — the lab now
  reaches a lot of unusable terrain (extreme reach/spread/detune combos). Likely
  a per-law clamp/curve table folded in at port time.
- When the detune workshop settles, fold winners (harmonic law + reach + pan fan
  + freq glide + tone/loudness) into swarmsaw.html → port to swarm_core.h with
  parity. Freq glide + tilt slews must be seconds→per-tick coeffs (ADR-009).
