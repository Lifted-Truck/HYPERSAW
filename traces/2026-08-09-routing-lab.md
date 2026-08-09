# B23 — routing topology bench

**Date:** 2026-08-09
**Trigger:** the human's ordering ("First we need the audio context: the
master/mixer page... and from there the routing algorithms. Routing will
probably warrant a lab."), ROADMAP B23.
**Verify:** `./verify fast` GREEN (lab load gate). Behaviour verified offline in
the browser, numbers below.

## What it settles

Three topologies over the same four slots and same two sources:

| | serial? | params 2×4 | params 4×8 | instances 4×8 |
|---|---|---|---|---|
| A per-osc sends → parallel | no | 8 | 32 | 8 |
| B per-osc private chains | per source | 16 | 64 | 32 |
| C matrix (DAG) | arbitrary | 24 | 120 | 8 |

The **DAG is guaranteed by construction**, not by a cycle check: a slot may only
read *earlier* slots, so one forward pass is always correct. An audio thread
cannot afford runtime cycle detection, so making the illegal state
unrepresentable is the only version of this that ships.

Recommendation is **C**, on composition rather than cost: it is the only scheme
that is simultaneously serial-capable, single-instance, morphable and
modulatable. Its id count argues for routing getting its own stride block.
Left unruled — it wants an ADR, because an append-only id-block choice is
permanent.

## Verified offline, not by eye

Identical gesture through all three schemes: rms 0.398 / 0.792 / 0.847, pairwise
max diff 1.31–1.87, all samples finite, none silent. Inside C, rewiring slot 2
from slot 1 instead of from the source changes the output by 1.23 — proof the
edges do work rather than decorate a fixed path. Per-scheme trim verified at
−6 dB → 0.501×; routing UI rebuilds per scheme (8 sliders / 8 buttons / 30
matrix cells).

## Two lab-design corrections

**The schemes are not equally loud, and that would have decided the ruling.**
A 2:1 level difference between A and C means an A/B elects the loud one. Added a
per-scheme trim (remembered across switches) and a live RMS readout so matching
is a number. This is the calibrate-the-detector discipline aimed at the ear
rather than at a probe — the same failure that made `mixer_check` accuse the
mixer earlier today.

**All-bypass was the honest default and a useless one.** The lab first opened
with every slot a wire, where all three topologies sound identical and the lab
demonstrates nothing until configured. Now defaults to drive/delay/lowpass/delay
— a nonlinearity exposes whether summing happened before or after it, and a tail
exposes whether a chain was serial. A default that makes the instrument's own
point invisible is a bug in the lab, not a neutral choice.

## Stale-snapshot near-miss

The preview pane serves a static snapshot; after editing the file, `reload()`
returned the OLD page and the new controls read as absent. Had I trusted it I
would have "fixed" working code. Re-opened the preview with a fresh query string
and the verification ran against the real file. Same class as every other
verify-the-artifact-you-think-you-are-verifying trap this session.

## Evidence consulted

- `ROADMAP.md` § New items (B23 brief), ADR-054 (rack is a grid, not a chain)
- `docs/design/bend-lab.html` — lab conventions (CSS vars, `wire()`, card layout)
- offline render comparisons run in-page; `./verify fast` load gate
