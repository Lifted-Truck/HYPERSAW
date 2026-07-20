# 2026-07-20 — GUI: XY-first + column-packed controls

Human layout feedback (screenshot): (1) the XY pad was the last control cluster,
buried at the bottom — move it first in the control column; (2) the controls used
a row-flow grid (`grid-template-columns: repeat(auto-fit, minmax(215px,1fr))`)
which aligned rows and left ragged vertical gaps below shorter clusters, wasting
space once the area was ≥2 columns wide.

## Changes (src/gui/gui.html only)
- Moved the XY cluster to be the FIRST child of `.controls` (was last).
- `.controls`: row-flow grid → **CSS multi-column** (`columns: 220px; column-gap:
  10px`), so clusters fill each column top-to-bottom and the columns balance by
  height (1 column when narrow, 2+ when wide). Removed `display:contents` so
  `.controls` is the multicol container; `.grid` → plain block wrapper.
- `.controls .cluster`: `break-inside: avoid` (+ `-webkit-` prefix) and
  `margin-bottom: 10px` (columns don't use grid gap).

## Evidence
- Rendered in the browser pane (dev-mock path) at 800px and 1280px: XY is first,
  controls pack into balanced columns with no ragged gaps, at both widths.
- Build clean (gui header regenerates); `./verify fast` green. GUI-only — no core,
  param, or DSP change, so no oracle impact and no ADR.
- git: 550b65e on branch fix-gui-layout-columns
