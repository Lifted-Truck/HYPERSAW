# 2026-07-21 — Pan pitch-ordering, bipolar tilt + master HPF, clip meter

Human batch: (1) golden/cauchy/gaussian pan wasn't ordered correctly; (2) tone
tilt bipolar (also remove low end) + a separate highpass; (3) a level/clip bar;
(4) reduce beating. Plus roadmap: scale quantization.

## Changes (audition lab only — docs/design/detune-lab.html)
- **pan pitch-ordering (bug fix):** the alternating fan was ordered by voice
  INDEX, but seeded/quasi-random distributions (gaussian/cauchy/golden) put
  indices in scrambled pitch order → the fan didn't follow low→high. rebuildPan
  now ranks voices by pitch (x for spatial laws, index for harmonic) and assigns
  the fan by rank. dist/seed/law changes now also re-rank the pan. Verified:
  gaussian |pan| is monotonic in ascending pitch (0 … 0.8).
- **bipolar tone tilt:** range −1…1. >0 = darken (LP, unchanged); <0 = thin /
  remove low end (per-voice HP = v − LP, corner 0.3→2.4× the fundamental). Cutoff
  still rises as √(f/f0). Measured: thin(−0.8) cuts low energy.
- **master highpass** (new `hpf`, 0…1 → 20…300 Hz, DC-blocker one-pole): a global
  low-cut for rumble/mud, independent of the per-voice tilt. Measured to cut lows.
- **output/clip meter:** tracks the pre-tanh peak; bar goes green→amber→red, shows
  peak dBFS, and flags CLIP when the signal drives the final tanh past 1.0
  (audible soft-clip). Read-only (doesn't touch audio).
- **drift ceiling 25→60 ¢:** wider pitch wander for smearing beat rates.

## Evidence
- **STEREO defaults Δ = 0** vs HEAD: pan re-rank is identity on the monotonic
  default distribution; tilt/hpf default off; meter is read-only. Fully inert.
- Headless: gaussian pan monotonic by pitch rank; bipolar-tilt-thin and master HPF
  both cut low energy; clip meter reads pre-tanh peak >1 under hard drive.
- Browser (live): no console errors; hpf/level bar/clipflag present; tilt −1…1;
  readouts thin/dark, hpf 133 Hz, drift 60 ¢; level bar live (−0.7 dB / 76%).
- `node --check` clean; `./verify fast` EXIT 0. Design-lab HTML — no oracle/core
  impact, no ADR.
- git: on branch lab-detune-octave-spread (PR #70)

## Beating — partial (build + advice, deeper fix deferred)
Raised drift ceiling; the two other built-in beating tools are the **normalize**
stage (envelope-follower makeup FILLS the beat nulls — the direct de-beater) and
**retrigger-off** (phase decorrelation → steadier sum). The genuinely effective
missing piece is **stereo decorrelation** (per-voice L/R detune so the throb
decorrelates across the field — how commercial supersaws do it); it needs a
dual-phase render (two accumulators per voice), so it's offered as a focused next
pass rather than bolted on here.

## Roadmap added
Phase 2 forward note: **scale/pitch quantization** — snap voices into a chosen
song key + scale. Design Qs recorded (quantize target vs smoothed freq; harmonic-
law interaction; per-voice vs swarm; voice-map snap-lines). Prototype-first in the
detune lab.
