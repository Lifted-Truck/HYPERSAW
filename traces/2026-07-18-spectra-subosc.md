# 2026-07-18 — SPECTRA per-voice sub-oscillator (ADR-042; first beyond-prototype)

Per-voice, uncoupled, -1 oct (f0/2), own phase accumulator outside the
Kuramoto graph, follows the note envelope, mono/centered. Params 52-54
(SPECTRA-only): subOn / subVol / subWave (sine→tri→smooth-square morph).
Guarded so subOn=0||subVol=0 → byte-identical reference path → spectra
parity rms 0. subGain = subVol*0.6.

Positive test (scratchpad/sub_test): A3 held, 110 Hz Goertzel — off 1.2,
sine 6066, square 7183; PASS. state_check GREEN (round-trip + restored
audio identical). GUI: Sub controls in the Spectra cluster, gated
SPECTRA-only; fixed the SPECTRA_ONLY gate to hide label-wrapped toggles
(the sub toggle was showing in SAW). verify full green; pluginval 10.

First deliberately-original feature — ships under the additions-are-safe
half of ADR-041; reference-path MODIFICATIONS still wait for Phase F.
