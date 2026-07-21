# 2026-07-21 — Detune lab: voice tone / anti-alias / power / metallic comb

Human, auditioning the wide/anchored spread: it gets noisy up top (brightness +
aliasing), there's destructive self-interference (power lost to beating), and the
NI spread has a coherent "almost Karplus-Strong" metallic quality — layered
spreading comb filters as detune climbs to an octave. Build the audition levers.

Framing (recorded for the port decision): detuned saws are *phase-incoherent* —
each voice's harmonic series beats against the others → smeared noise. A
KS/comb sound is the opposite: one excitation, phase-locked regularly-spaced
partials decaying through a feedback delay. So the metallic emulation is NOT
"clean up the detune" — it's to re-impose coherent resonance with a tuned
feedback comb on the summed swarm.

## Change (audition lab only — docs/design/detune-lab.html, not a reference prototype)
Seven default-inert knobs (defaults reproduce the prior lab bit-for-bit):
- `tone tilt` (0..1): per-voice pitch-tracked one-pole LP; constant harmonic
  count so octave-up voices aren't disproportionately bright. Trims highs AND
  the energy that would alias. tilt=0 → coeff 1 = passthrough.
- `oversample` (1/2/4×): osc bank runs at N× the rate, BLEP'd there, decimated
  through an RBJ biquad LP at 0.45·SR. The diagnostic: how much "noise" is real
  aliasing vs. inherent density. 1× = the current single-rate path.
- `drive` (0..1): post-sum tanh, level-normalized by tanh(pre) so it adds
  non-cancelling harmonics without a loudness jump.
- `normalize` (0..1): post-sum envelope-follower makeup toward a 0.5 target —
  fills the beat nulls (the "more power without bumping" lever), vs. the static
  `density comp` exponent.
- `root weight` (0..1): in anchored/unipolar spread, fades the ensemble toward
  the lowest voice as it spreads up (energy skews to the root). Gated by `anchor`
  so it's inert in bipolar mode.
- `comb mix` + `resonance` (Karplus-Strong): feedback delay tuned to the played
  note (focus f0), one-pole damping in the loop, fb 0.6..0.98. Re-imposes
  phase-coherent regularly-spaced resonance → the coherent metallic character.

## Evidence
- **Defaults bit-identical to HEAD** (spread/anchor commit): headless render
  fingerprint Δ = 0 over a 2-note chord × 60 blocks. The A/B is honest.
- Headless smoke across all features: 0 non-finite samples, peak < 1 (softclip
  holds), levels track intent — rootWeight lowers level (energy concentrates),
  tilt trims peak, OS is level-neutral, drive/normalize/comb add power.
- Browser pane: no console errors; all 7 controls present, wired, readouts
  correct (rootWeight 0.60, tilt "trim 0.50", os 4, drive 0.40, normalize 0.70,
  comb 0.80, combRes 0.90).
- `node --check` on the script; `./verify fast` EXIT 0. Design-lab HTML — not in
  the build or the 9 oracle chains, no parity/core impact, no ADR.
- git: on branch lab-detune-octave-spread (PR #70)

## Next (audition → port)
Human A/Bs by ear. Open questions the audition answers: (1) does 2×/4× audibly
reduce the noise, i.e. is the AA investment worth a core change (would be an
oscillator-spec ADR, prototype-first); (2) is the KS comb the right metallic
primitive or does it want layered/spreading taps (2nd detuned delay keyed to
spread); (3) normalize vs. drive vs. K-coupling as the preferred power lever.
Winners fold into swarmsaw.html (reference) → port to swarm_core.h with parity.
