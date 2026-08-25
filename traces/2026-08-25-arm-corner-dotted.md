# Trace — armed-corner marking, fixed in the right place this time

**Trigger** human 2026-08-25: *"When you tried fixing the dotted line issue, you
got it backwards: now clicking a corner square doesn't even put dotted colored
lines next to parameters, and all it does is make all bounding boxes dotted
lines."*

**The mistake.** There were TWO independent dotted-line mechanisms and the
previous fix changed the wrong one.

| | mechanism | what it did | previous fix |
|---|---|---|---|
| reported bug | `body.arming .cluster { border-style:dashed }` | dashed the border of **all 36 clusters** — verbatim "dotted lines around all the bounding boxes" | **left untouched** |
| wanted signal | `.row.ghost { border:3px dotted var(--armc) }` | coloured dotted marking per parameter | **deleted** |

So the bug shipped and the feature did not. The previous commit's own comment
asserted the row borders "lined up into what read as boxes around everything" —
a diagnosis reached without looking at the cluster rule sitting six lines below
it.

**What changed** (`src/gui/gui2.html`, CSS only — no JS, no wiring):
- The cluster rule is gone. The arming reminder is now a fixed 4px rail along
  the top edge, which is unmissable and bounds nothing.
- `.row.ghost` regains a coloured broken stripe on its left edge, at the same
  9px offset as `.owned`'s solid stripe, so text stays aligned and solid-vs-broken
  is the only difference. `.row.kcell.ghost` gets the same on its top edge.
- Both are drawn as two stacked background layers, not a dotted border: a
  continuous `--line` layer carries contrast, colour dashes ride on it.

**Measurements that changed the design.**
- A dotted stripe floating on the panel measured **1.34:1 (corner A), 1.31 (D),
  1.77 (C)** — three of four corners invisible in light mode. This is the sixth
  site of the invisible-yellow family, and the first where the fix was measured
  before shipping rather than after a report. Over the ink line the dashes
  measure **5.12–13.10:1**, and the line itself **17.19:1** against the panel.
- The rail had the identical flaw as a plain `--armc` bar and got the identical
  two-layer treatment.

**A regression I introduced and caught.** `.row.ghost > * { opacity:.55 }` also
matched `.row.kcell.ghost > output`, which lives at opacity 0 and rises to 1 on
hover. Pinning it at .55 left every ghost knob cell drawing its value on top of
its own name — visible in a screenshot. Fixed by scoping the child dim to
`:not(.kcell)` and dimming knob cells through label *colour* plus knob opacity,
neither of which touches the choreography.

**Probe discipline — two blind controls before a working one.** The
cluster-dashed control first read 0 with the old rule re-injected: the app's own
poll calls `paintArm(0)` between tool calls and had stripped `.arming`. Then the
knob-cell control read identical values either way: `> output` carries
`transition:opacity .08s`, so a synchronous read after injecting a rule returns
the PRE-transition value. Only after disabling transitions did the control fire
(`0 / 1` shipped vs `0.55 / 0.55` broken). Neither reading would have been wrong
on its face — both would have been reported as passes.

**Verify.** `./verify fast` exit 0; `gen_gui_controls` GREEN (markup current);
plugin rebuilds with the embedded GUI. No DSP touched, so parity is untouched by
construction.
