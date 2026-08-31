# 2026-08-31 — MAIN gets its own hands (ADR-150)

**What changed.** The human's 2026-08-31 batch, one theme: MAIN owned nothing —
its XY was the active osc's pad in a different frame, pitch only moved in
integer jumps, the mod wheel had no GUI surface.

- Params 179/180 `mainAsnX/Y`: the MAIN pad's own macro assignment (defaults
  macros 1/2 = detune/K, centered 0.5 → puck starts mid-pad). Two pad
  controllers now, one machine (`wirePad`), different id sources; the OSC pad
  keeps 174-177 behavior.
- Param 181 `oscPitch`: continuous per-osc pitch ±24 st, sums into the tuning
  term beside transpose/octave, NOT in kGlobalIds → twins + morphs smoothly
  (the blend the integer knobs structurally cannot give).
- Morph mini on MAIN (B85 first landing): bilinear MCOLORS ground + live puck
  at (152,153), drag writes both, dirty-gated redraw.
- Wheels cluster on MAIN: bend wheel widget re-mounted there; new mod-wheel
  slider → `setModWheel` host hook → same `srcWheel` a real CC1 lands in.
- Logo: stroke hue offset 90°→120°; morph OFF → medium-slow hue drift
  (~50 s cycle) instead of frozen position colours.
- B88 filed (scale-quantize slider + chord topology, verbatim vision).

**Evidence consulted.** gui2.html pad/tint/wheel closures read in place;
paramscope DEFAULT-LIE sweep (member inits match table); in-pane check —
MAIN renders new clusters, no console errors, mod-wheel readout live,
bend wheel mounted in #wheelsMount.

**Verify.** `./verify full` exit 0, all 15 gates GREEN (parity 156/156 —
oscPitch defaults 0 so the tuning term is bit-clean). lab_load 26/0.
Installed 8c2a446 (CLAP/VST3/AU, Aug 31 00:40).
