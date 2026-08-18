# 2026-08-18 — mutation-probing our own gates; one can-fire hole found

**Why.** `L0032`'s sharpening (this session) says a must-read-zero check needs a
paired corruption or it cannot fail, and the cheap proof is to neuter the thing
under test and see what stays green. Writing that and not running it would be
the failure the parent scope names: a lesson observed, never learned.

**Method.** Plant a violation in a gate's input, run the gate, restore, and
assert the plant actually applied.

| gate | plant | result |
|---|---|---|
| `presentation_check` | row with an address the shell does not declare | **FAILED** (correct) |
| `test_table_check` | malformed row | **FAILED** (correct) |
| `gen_gui_controls --check` | designed row (`chunk` set), markers intact | **FAILED** (correct) |
| `gen_gui_controls --check` | **`<!--GEN:OSC-->` destroyed, chunk designed** | **GREEN — hole** |

**The hole.** With a `chunk` set on a `global` row on page OSC, the gate
correctly fails ("generated controls are stale"). Destroy that page's `GEN`
marker as well and the same state reports `GREEN (0 generated control(s), gui2
markup current)`. **Deleting the marker silences the failure**: the control is
designed, is placed nowhere, and the gate says the markup is current. The gate
iterates the markers present in the file, so a missing marker is not a missing
control — it is one fewer question asked.

Harmless today (all 181 rows are undesigned, so the count is 0 either way), and
live the moment the first chunk is designed — which is the next queued GUI step.
Same family as the bug already documented in this tool's own comment block at
`tools/gen_gui_controls.py:84-89`: that one computed "already placed" from the
whole file so every param looked handled; this one lets the file drop the
question instead of answering it wrong.

**Proposed fix — NOT applied, `./verify` and its gates are a human gate.**
Assert marker integrity independently of generation: the set of `GEN` markers in
`gui2.html` must equal the set of pages the table names. One comparison; fails
loudly on a deleted or misspelled marker whether or not anything is designed.

**Method failure worth recording.** The first version of this probe replaced
`<!-- GEN` while the real marker is `<!--GEN:` — it matched nothing, the gate
reported GREEN, and that was briefly read as a result. That is `L0032`
instance 3 (a calibration whose plant silently matched nothing) repeated by the
same session that had just written the sharpening. Every probe above now asserts
the plant changed the file before trusting the verdict.

**Verify.** `./verify fast` EXIT=0; all planted files restored clean
(`git status --porcelain` empty for each).
