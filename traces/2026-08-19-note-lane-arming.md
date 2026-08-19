# 2026-08-19 — the note lane never armed; the Voice panel was empty by design

**The report.** "The bend still doesn't seem to be hooking into the notes, and
we've lost the polyphonic bend toggle." Both true, one cause.

**The bug.** ADR-096 gave the note lane five laws but left the ARMING predicate
at `p.glide > 0` (`swarm_core.h`, both retarget sites). That was the right
question while lag was the only law — lag with tau 0 is instant. It is the wrong
question with five, because constant-time, constant-rate and spring never read
tau at all. Worse, `glide` is gated `noteLaw=3` in the presentation table, so
selecting any other law HID the only control that could arm the lane. Poly Glide
(id 89) sat behind the same `&& p.glide > 0`, so the polyphonic bend toggle was
present on screen and inert — "lost" is exactly right.

Fixed with `noteTravels()`: off never travels, lag keeps its historical
`glide > 0` gate (so every patch storing glide=0 keeps snapping, bit-identical),
and the three laws that carry their own time arm on selection alone.

**The regression test, and why arrival could not be it.** New trajectory
criterion: constant-rate at 24 st/s, `glide` left at 0, retarget 220 -> 440,
sampled MID-FLIGHT at 0.25 s. Travelling reads ~311 Hz; measured 313.473. A lane
that never arms SNAPS — and a snapped lane arrives at 440 perfectly, so the
arrival check passes either way. Plant confirmed it: restoring `p.glide > 0` gave
`FAIL ... got=440.000` on the mid-flight check while `ADR-096 const-rate arrives`
stayed OK. **The discriminating half of a test is rarely the half that states the
goal.**

**Also fixed.** `pushNoteLaw()` now runs in `plug_activate`; without it the cores
ran GlideCore's own defaults — including an empty scale mask, which the quantiser
reads as "no degree admitted" — until the first note/bend/scale edit.

**The empty Voice panel.** Not a layout bug. `gen_gui_controls.py:125` skips any
id a human already wrote ("generation never fights a human"), and MIX's channel
strips hand-place 35/36/37 — so octave/semi/fine could never be generated onto
the OSC page whatever the table said. Hand-placed a Pitch cluster instead, NOT
`data-fixed`, so `effId()` remaps it to the selected oscillator; the MIX strips
stay data-fixed mirrors and `setControl` repaints them from the same push. Mono,
Legato and Pitch moved in with the travel controls and the Voice panel is gone.

**Found in passing.** Every per-oscillator row carried a static `osc1` tag, so
selecting OSC 2 left the page insisting you were editing OSC 1 while `effId()`
quietly sent everything to OSC 2. The tag now follows the selector (verified:
`osc1` -> `osc2` on click).

**Known, not fixed.** Display order within a group is param-id ascending
(`gen_gui_controls.py:185` sorts by id), so the presentation table cannot express
order and Note Lag (33) sits between Mono (32) and Legato (34). Left for the GUI
pass rather than reshuffling every group right before a test session.

**Process note.** A `git checkout <branch> -- <paths>` onto a DIRTY working tree
discarded this session's uncommitted work; it was redone from the session record
and re-verified identical (mid-flight 313.473 both times). Stash or commit before
checking out paths from another branch.

**Verify.** `./verify full` EXIT=0 · `parity_check: 156/156 within eps=1e-06
(worst 4.262e-09)` · `trajectory_check: GREEN` · `state_check: GREEN` ·
`test_table_check: GREEN (61 tests)`.
