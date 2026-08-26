# Trace — B50 FX rework groundwork, and the mono overflow measured

**Trigger** human 2026-08-26, two items: the FX rework direction (set modules +
routing matrix with feedback and bypass, plus a nondestructive port), and
*"I may need your help understanding the drop-newest issue you're talking
about."*

**What changed.** `docs/proposals/fx-matrix-rework.md` (new); ROADMAP B50.
No code.

## The mono held-stack question, measured rather than described

`hypersaw_clap.cpp:3170` — `if (heldCount < 16) heldStack[heldCount++] = …`.
The stack is the mono fallback chain (last-note priority, 16 deep). When it is
full the *new* key is silently not recorded: drop-newest.

**It does not hang or silence anything**, because the SOUNDING note is tracked
separately in `core.voiceAt(monoSlot).midi`, and the release path only
retargets when the released key is the sounding one (`:3084`). Overflowing by
exactly one self-corrects.

**Measured cost, overflow by two** (scratch probe, real plugin, mono+legato):
hold 40..55 (stack full), press 70 → sounds 70, press 71 → sounds 71, release
71 → **sounds 55, not 70** — while 70 is still physically held. The
intermediate overflow key is forgotten entirely. Drop-oldest would have evicted
40 and kept 70 in the chain.

So: real, correct-to-fix, and reachable only above 16 simultaneously held keys
in mono — hands can't, but a sustained MIDI clip, an arp feeding mono, or
stacked chords can.

**Why it is a ruling and not a chore:** our Aug-11 answer promised FOUNDATIONS
*"drop-oldest, please, and we will change ours to match"*; the change was never
made; our Aug-25 answer then said *"keep drop-newest… we would rather keep
parity"* — an argument that only worked because the promise was unkept. They
shipped drop-oldest and quote our Aug-11 reasoning in their header. They are
asking which round governs.

## The FX rework — the three findings worth having before design

1. **It dissolves B49 instead of patching it.** No `type` param means no
   stepped structural param in the field (chimera impossible); presence as a
   coefficient means bypass is continuous (the ramp is free). It also
   **retires** `docs/proposals/fx-slot-contract.md` — bypass stops being a
   module duty.
2. **The existing matrix cannot do feedback.** `routing_core.h:63`'s `edgeLive`
   permits a slot to read only earlier slots — that strictness is what makes a
   single forward pass correct. Block-rate delay (FOUNDATIONS' OQ-23, made for
   the *modulation* graph) puts **2.9 ms** in the loop and varies with host
   buffer size; per-sample with a one-sample delay is 22.7 µs and
   buffer-independent. Human ruling, recommended B.
3. **Migration: the unit is the CORNER, not the patch** — four corners each
   store four slot types, so the no-duplicate premise must hold four times
   over; the algorithm detects collisions as a gate. Cross-corner duplicates
   are fine and better. And the order hazard (A: Drive→Comb, B: Comb→Drive)
   morphs through a parallel blend that sounds like neither — BLEND vs ARGMAX,
   a ruling that must precede the migration.

**Verify.** `./verify fast` exit 0. No code changed; parity untouched.
