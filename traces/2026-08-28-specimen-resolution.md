# Trace — the specimen returns, at the resolution the machine can afford

**Trigger** human 2026-08-28: *"If we fixed the lagging, maybe I should see
the blob again; mind, it was fairly low resolution on the last pass… The blob
should be the top section on the left column of Main."*

## What changed

Param 178 defaults 1 again. The specimen moves out of the Viz cluster into its
own `#specimenBox` cluster, first child of MAIN's `.vizcol`. The render scale
becomes adaptive in both directions (floor 0.5, ceiling = display pixel grid
capped at 2x) with a hysteresis band (down >75 ms EMA, up <58 ms after 40
sustained comfortable frames). On settling, one frame is drawn at the ceiling
— the plate — and then nothing. Markup defaults (checkbox checked, phaseC
hidden) brought in line with the shell default.

## Evidence

In-browser: scale reaches the 2x ceiling unaided; render 628x560 device pixels
into a 314 px canvas (v2 was ~204 px — about 9x the pixels). Idle draws
EXACTLY 1 frame per 120 and stops. Toggling 178 swaps specimen and MAIN's
phase circle both ways; OSC keeps its own circle; feeds on OSC read
{viz:true, spec:true, scope:false}. verify fast exit 0, lab_load_check GREEN,
installed, auval SUCCEEDED.

## The assumption this rests on, stated

The climb/throttle reads the WALL-CLOCK gap between specimen frames, so it
detects a GPU stall only insofar as that stall shows up in rAF pacing. It does
on a compositor-throttled surface; if the plugin's webview is not paced that
way, the scale will not fall when it should. Test row B74-8 asks the human to
report exactly that symptom, because it is the one thing this design cannot
verify from here.

## Open

Whether 2x at 20 Hz is affordable in the DAW (B74-8). March steps stay at 44,
chosen for the low-res pass; if grazing-edge banding shows at the higher
resolution, stepping them with the scale is the next lever.
