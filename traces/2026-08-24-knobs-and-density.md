# Knobs + density, lime-on-teal screens, one roundness

**Date** 2026-08-24 · **Branch** `knobs-density` · **PR** #425
**Commits** `b8911ee` (palette + radius), `a970902` (knobs + density)
**Roadmap** B37 Increment 5 · **ADR** 120

## What changed

Three human asks from the same message, in one branch.

1. **Screens go lime on teal.** `--scr-tube` `#180A26`→`#06211F`, `--scr-meter`
   `#59F6E8`→`#B8F227`, `--scr-grid` `#9D6CFF`→`#4FD8BE`; the light-screen
   audition retuned to match. Roles unchanged — only their hexes moved, which is
   the return on naming colours by job.
2. **One roundness.** `--r-card:10px`, `--r-well` kept as an *alias* rather than
   deleted so the spec's vocabulary still reads. Every non-capsule radius points
   at `var(--r)`, canvases included. Documented exceptions: the 6px slider track
   and 13px handle cannot carry a 10px radius; capsules stay 999 because that is
   a shape, not a corner.
3. **Knobs.** `gen_gui_controls` stopped downgrading the table's 121 OSC
   `widget=knob` rows to sliders, and §4's knob behaviour landed.

## The part a future session needs

**A rendering that works can still deliver nothing.** The first knob cut was
correct on every §4 criterion and bought **30px of 1483 (2%)** — four of five
pages byte-identical. I nearly reported it as done.

The cause was geometric, not visual: a knob cell was **78–87px** against a 28px
slider row, so a cluster only broke even at 4-across, and a 5-knob cluster
wrapped to two grid rows and came out **taller** than the sliders it replaced.
The knob was never the problem — 42px of label-and-readout chrome stacked under
a 36px control was.

The fix was to stop giving the readout its own line (it takes the label's slot
on hover/focus/drag) and to drop the unit and scope tag from the visible name,
both kept in `title`.

| | before | after |
|---|---|---|
| knob cell height | 78–87px | **61px, uniform** |
| OSC control column | 1483px | **1303px** |
| OSC page | 1576px | **1395px (−11.5%)** |
| labels clipped | 28 / 45 | **0 / 39** |
| knobs booting blank | 37 / 45 | **0** |

**Generalisable:** when a change is supposed to buy a resource (space, time,
memory), measure the resource against a real baseline before believing it. I
extracted the pre-change file from `main`, served it, and measured every page —
which is what turned "the knobs look great" into "the knobs bought 2%".

## Evidence

- `./verify fast` EXIT=0. `gen_gui_controls: GREEN (119 generated controls,
  gui2 markup current)`, `gui_reach: GREEN`, `depends_check: GREEN`,
  `presentation_check: GREEN (240 rows)`.
- §4 behaviour measured in the browser, not eyeballed: bipolar rest
  `a0 = a1 = 140.00deg`; drag 50px on a ±12 range → **+6.00**; shift → **+1.50**;
  dbl-click → default; disabled knob refuses the drag and dims to `0.55`
  (with an enabled control reading `1` in the same call, so the check can fire).
- Skin non-interference: all 74 knob cells still match `.row[data-addr]`,
  `.row input[type=range]`, `.row output`, `.row label`, `.cluster .row`,
  `.page .row`. Gating sound in both directions — every hidden `.knobs` grid has
  all children hidden, every visible one has at least one visible child.
- Baseline A/B against `git show main:src/gui/gui2.html`, served and measured.

## Bug found on the way

`setPointerCapture` ran **before** `classList.add('drag')` and the
`bridge.gesture(id, true)` notification. A capture throw therefore skipped the
gesture BEGIN while `id` was already set — leaving `pointermove` free to write
parameter values and `pointerup` free to send a gesture END with no matching
begin. An unbalanced gesture is how a host records automation that cannot be
undone. Capture is now last and wrapped in `try/catch`.

Proven with a **must-fire control**, not a self-confirming check: pointerId 999
was shown to genuinely throw `NotFoundError`, and `.drag` was measured `false`
under the old ordering. Post-fix the same sequence logs `[BEGIN 9, END 9]`.

## A measurement artifact I nearly filed as a defect

A drag initially read `output: 0, label: 0` — apparently invisible while
turning. It was the readout's own `80ms` opacity transition being sampled at
t=0. Re-measured after settling: `output: 1, label: 0`, correct. Worth recording
because the failure mode (synchronous `getComputedStyle` during a CSS
transition) will recur in any future UI probe here.

## Why four pages did not move

MAIN declares 18 knobs but **17 are gated** behind bend laws unselected at
default; MIX/FX/MORPH declare 1/4/6. The density win lives on OSC because that
is where the continuous per-oscillator parameters are. Stated explicitly so a
later session does not read the flat numbers as a missed conversion.

## Open

Units are trimmed from knob faces because they live in the label *string* for
many parameters rather than in the generator's `.u` span. If units should show
on the face, the fix belongs in `src/param_presentation.tsv`, not the CSS.
