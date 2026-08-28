# Trace — macros, the XY pads repurposed, and modulation drawn as an offset

**Trigger** human 2026-08-28: *"refer to the style guide … I don't want the
knob/dial positions themselves to be moved, I want it to work on an offset
system that's visually clear. Let's also build a set of, say, 8 macro knobs
controllable from Main, and also switch the XY grids so they're now macro
controllers with variable assignments."*

## What changed

- `src/hypersaw_clap.cpp` — params 166-173 (macros, mod source slots 2-9) and
  174-177 (XY axis assignment, per osc); `modLiveJson()` (base + applied per
  active dest); refusal range 161-177 at route-add.
- `src/gui/gui2.html` — MAIN gains the Macros cluster; both XY clusters become
  macro pads with assignment selects; SET gains the static data-fixed
  assignment rows (gui_reach's honest copy); the ADR-121 `.kmod`/`.kmnow` seam
  gets its first caller (`modHaloFrame`, ~20 Hz off the rAF loop); sliders get
  the §4 band+tick; the right-click menu gains the in-place macro submenu.
- `src/param_presentation.tsv` + `tests/feature_tests.tsv` — 12 rows, 4 tests.

## Evidence

- UI Spec §4 (human, 2026-08-23): KNOB mod halo + SLIDER mod band — the
  offset-not-movement law was already specified; ADR-121 had built the CSS
  seam with "nothing calls this yet". This increment is the caller.
- macro_probe (scratchpad, linked against the shipped impl lib): applied
  0.7500 = 0.55 + 0.25·0.8·range with the gate CLOSED; dest readback 0.5500
  throughout; macro back to 0 returns applied to base; macro/assign-as-dest
  refused; xyAsn readback exact.
- In-browser (served gui2, stubbed bridge): pad writes [166,0.75],[167,0.75];
  after SET reassignment writes [170,0.5],[167,0.5]; halo --m0 84° --m1 196°
  --mang 14° (hand-checked against the conic math); slider .smod/.snow render
  and clear; submenu lists 8 macros; macro knobs and 161-165 show no mod menu.
- `./verify fast` exit 0 (presentation_check, test_table_check, gui_reach all
  extended and green); parity 156/156 worst 4.262e-09; mod_check green;
  lab_load_check green.

## Open

Route persistence (the known ADR-136 gap, now the loudest one) — next
increment. Per-note fan-out and B70 depth-mod behind it.
