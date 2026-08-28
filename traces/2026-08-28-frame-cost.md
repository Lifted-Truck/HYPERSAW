# Trace — the blob was never the cost; the frame loop was

**Trigger** human 2026-08-28: *"the interface is still lagging as badly as it
was when the blob was visible. Are we sure it isn't still silently computing
it in the background?"*

## The direct answer, measured

Instrumented `gl.drawArrays` and counted over 120 simulated frames:
- specimen OFF (the shipped default): **0 GL draws**
- specimen ON, nothing sounding: **0 GL draws** (ADR-140's idle gate holds)
- specimen ON, sound playing: 40 draws / 120 frames = its 20 Hz cadence

So no, it is not computing in the background — and the lag was never it.

## What the lag actually was

`vizFrame` + `specFrame` every frame and `drawScope` every second frame, none
gated on whether anything they feed is visible: **150 bridge round-trips per
second on every page**. On FX/MOD/SET/MORPH nothing consumes them, and the JS
half cost MORE there (0.473 ms per vizFrame vs 0.200 on MAIN) because the work
still runs and paints into hidden canvases.

## Fix and result (measured, same harness)

Feeds gated on visible consumers, derived from the DOM; viz/spec halved to
30 Hz on the wordmark's own precedent; halo sweep scoped to the visible page.

| page | before | after |
|---|---|---|
| MAIN | 150 | 92 |
| OSC | 150 | 62 |
| MIX | 150 | 32 |
| FX/MOD/SET/MORPH | 150 | 2 |

## The bug I introduced and caught

`recomputeFeeds()` first went in BEFORE the page reveal, where no page carries
`.on` — every feed read false and OSC's visualizers went dark. The handler's
own comment three lines up says "REVEAL FIRST, THEN PAINT" and records the
same bug from an earlier session. Measured before shipping, moved after the
reveal.

## Open

MAIN still pays 92/s; B76 (one batched snapshot bind) is the structural fix
and B75 (native backend) the one under it. The human's DAW judgement picks.
