# Trace — CHROME-001 becomes MAIN's visualizer; internal ids stop reaching the UI

**Trigger** human 2026-08-28: the Chrome 001.dc.html drop ("let's workshop
it"), the phase-circle replacement ruling, and the "#2147483649" screenshot.

## What changed

`src/gui/gui2.html` only (DSP untouched): the specimen canvas + WebGL port
(shader verbatim + uRipple) + sound mapping (poke/film/tension/ripple, all
from the existing viz snapshot); the theme-clear learns to skip the GL canvas
(getContext('2d') locks a canvas's context type — the specimen was dead on
arrival until measured); modDestLabel names synthetic dests and strips
strides/decorations; the ±48 pitch-depth slider keys on dest, not index 0.

## Evidence

In-browser: chromeC GL context null before the clear fix, live after (the
same probe frame: fresh canvas GL ok — that contrast is what named the
culprit); A4 gate edge → poke [-1,0,0] (pitch class 9's azimuth), dent 3.08
mid-decay, no re-poke on held gate, latch clears on release; center pixel
[186,195,238] = lit chrome; labels "Pitch" / "Detune · osc 2" / "Detune".
Screenshot in the PR. verify fast exit 0; lab_load_check green (the GL-less
harness takes the quiet-blank path).

## Open

B74 deferred set (synthetic-dest uniforms, corner rim lobes, multi-poke);
B73 Echo/Room feedback design next in the FX lane; B71 table-side route UX
next in the matrix lane.
