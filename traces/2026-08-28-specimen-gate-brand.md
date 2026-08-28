# Trace — the specimen becomes an off-by-default toggle; the wordmark thins

**Trigger** human 2026-08-28: "untenably slowly (jumpy, jaggy) in the VST…
remove this module for now to preserve performance and roadmap that build,
unless you can think of ways to make this work" + "stroke on the logo a
little thinner".

## What changed

Param 178 (specimen, default 0, GUI-only) with SET → Visualizers toggle; MAIN
gets phaseC back when off (osc-off stamp loop restored); ON = reduced render:
fixed 204×182 internal res upscaled (no dpr — retina doubling was most of
v1's bill), 44 steps, 20 Hz, idle gate (zero draws when silent+settled).
Throttle fixes: EMA threshold 40→75 (40 would throttle every healthy frame at
20 Hz), gaps >250 ms excluded (a pause is not a slow render — measured scale
collapse 0.65→0.455 from pauses alone). Brand EDGE_R 1.4→1.0. B75 filed
(native backend investigation; ADR-019 seam named).

## Evidence

In-browser (real viewport after a background-tab layout trap — a background
pane tab lays out at ~24px wide; measurements from it are garbage): defaults
right, toggle swaps both ways instantly, render 204×182 @scale 0.65, center
pixel lit pearl, EMA gap-immune, wordmark edge matches the 1.5px hairlines in
the zoom. verify fast exit 0; parity 156/156; installed; auval SUCCEEDED.

## Open

The human judges v2-on tenability in the VST (test row B74-4) — that ruling
sets B75's urgency. B73 Echo/Room feedback design still next in the FX lane;
B71 table-side route UX in the matrix lane.
