# B24 increment 2 — mute/solo as params, per-oscillator meters

**Date:** 2026-08-09
**Trigger:** continuing the mixer track the human ordered first ("First we need
the audio context: the master/mixer page"). B24 remainder, as flagged.
**Verify:** `./verify full` GREEN, 15 gates, L0-1 parity **147/147 worst
4.262e-09 (unchanged)**. `mixer_check` GREEN (built, not yet gated).

## What shipped

`oscMute` (104) and `oscSolo` (105), per-oscillator — so oscillator 2 is
1104/1105. **Params, not GUI state**, because the human asked for automation to
reach them. Shell-owned: they gate the mix stage and never enter SwarmCore, so
the parity goldens cannot see them.

- Mute beats solo. Any solo anywhere silences every non-soloed oscillator.
- `anySolo` is **computed from the params every block, never cached** — a cached
  flag is one more thing to forget to update, and the params stay the single
  source of truth.
- Gain uses the same ~8 ms one-pole as the master fader (a hard 1→0 on a ringing
  oscillator is a click), sharing one `gainSmoothCoef()` so the two faders in
  the mixer cannot drift apart in feel. The 1.0-exact snap plus the skip keeps
  an untouched patch bit-identical rather than "identical up to a converging
  one-pole".
- Oscillator 0 renders straight into the output buffer, so its gain and meter
  are applied in place afterwards; oscillators 1..N are gated inside the
  existing chunk loop. One helper serves both.

Meters ride the existing viz push as `oscPeak[]` — an **array**, so a third
oscillator needs no serializer change. Read **pre-master and pre-FX**, because a
mixer strip answers "is this strip contributing?"; a post-master reading is just
`outPeak` scaled and would go dark when the master fader was down.

In the GUI, a strip silenced by ANOTHER strip's solo is dimmed, deliberately
distinct from its own M being lit — otherwise the mixer cannot tell you which
control silenced you, which is the one question a mixer strip must answer.

## Verified, not merely unbroken

`./verify full` going green proves nothing was broken; it says nothing about
whether a new control does anything. A mute param that is stored, reported back
by `readParam`, and never consulted by the render would pass all fifteen gates
and be a dead switch. So `tools/mixer_check.cpp` asserts the behaviour: baseline
separable, mute silences only its own oscillator, solo silences non-soloed,
soloing both restores both, mute beats its own solo. All five green.

GUI verified in-page: 4 buttons carrying ids 104/105/1104/1105, toggles sending
the right values, solo dimming only non-soloed strips (never the master), meters
moving frame to frame.

## The detector accused the code, and was wrong

`mixer_check`'s first run reported mute as broken: muting oscillator 1 dropped
the 880 Hz bin to **67%**, not to oscillator 2's level.

That was the detector. The probe told the oscillators apart by transposing one
an **octave** — and these are sawtooth oscillators, so oscillator 1's second
harmonic lands exactly on oscillator 2's fundamental. The arithmetic confirms
it: 880 Hz baseline 0.2401 = oscillator 2's 0.1603 + oscillator 1's second
harmonic 0.0803, and 0.1607 / 2 = 0.0803 to three figures. Muting oscillator 1
correctly removed its fundamental AND its harmonic from the other bin.

Fixed by choosing a **tritone** — 2^(1/2) is irrational, so no harmonic of
either fundamental lands on the other. Baseline became 0.1599/0.1599 and mute
leaves the other oscillator at 100.4%.

L0016/L0017 again: calibrate the detector for the signal class before letting it
accuse the code. Worth noting the failure was *plausible* — "mute leaks" is a
believable bug, and the 67% number looked like a real partial-gating defect.

## Open — per-oscillator pan needs a ruling

Not built, deliberately. Two laws produce materially different instruments:
**balance at the mix stage** (shell-only, zero parity risk, but hard-panning
deletes the far-side voices of a seated swarm) versus **image shift in the
core** (musically right here, composes with the existing seat laws, but touches
the parity-locked core and needs an ADR). Recommended (2) in ROADMAP and left it
to the human — a protected-path change is not a default.

## Evidence consulted

- `src/hypersaw_clap.cpp` — param table, `applyOscGainAndMeter`, `oscGainTarget`
- `src/gui/hypersaw_gui.h` / `hypersaw_gui_common.h` — `oscPeak[]` + serializer
- `src/gui/gui2.html` — M/S buttons, `paintSolo`, `drawMeters`
- `./verify full` — 15 gates green; `mixer_check` 5/5
