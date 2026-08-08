# The chord-retrigger "bug" — reproduced, localized, and it is the reference's physics

**Date:** 2026-08-08
**Trigger:** human report — "when I hold most of a chord and try to retrigger a note that I
let go, it only retriggers a fraction of the time."

## The hunt, honestly

1. **Probe gen 1** reported 19/21 failures — a harness artifact (every queued chord event was a
   pointer to one `static` struct: a "6-note chord" was the last key pressed six times).
2. **Probe gen 2** (fixed events, rms oracle) could not resolve one voice among detuned swarms.
3. **Probe gen 3** — Goertzel at the repressed note's own f0, real note_ids, 20 press/release
   cycles, and a settings matrix. Defaults: **0/20 failed**. `retrig = 0`: **7/20 early,
   5/20 still dead at 380 ms.** The failure needs the Retrigger toggle OFF.
4. **Discrimination**: with ~10-cent detune, inter-voice beat periods approach one second, so
   seven random-phase voices can sit in a fundamental null for longer than any attack window.
   Late-window measurement confirmed the nulls persist ≥380 ms — genuinely inaudible starts,
   not transients.
5. **The reference (SwarmSynth, extracted headless from swarmsaw.html) shows the identical
   5/20** under the same experiment. Parity is exact. This is the spec's free-phase behavior.

## What it is

With retrig off, each voice restarts at a seeded-random phase. Occasionally the draw
near-cancels at the fundamental, and slow detune beating holds the null for hundreds of
milliseconds: an audibly "missed" retrigger, a fraction of the time — precisely the report.

## Decision for the human (A13)

- **(a) Leave it** — spec'd free-run character; document it in the retrig control's tooltip.
- **(b) Anti-null redraw** — with retrig off, reject phase draws whose fundamental sum falls
  below a floor and redraw (seeded, deterministic). Kills the artifact, preserves the free-run
  feel — but it CHANGES THE REFERENCE PATH, so it needs a reference-side edit + ADR (protected
  prototypes) and new goldens, per the ADR-011/012 ingest pattern.
- **(c) Per-voice phase spread** instead of pure random (e.g. random rotation of an even
  spread) — no nulls possible, slightly less "free".

Note: retrig defaults to ON. The report implies the human's patch has it off — worth
confirming which control set it (the toggle is labelled "retrigger" in the voice cluster).
