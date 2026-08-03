# 2026-08-03 — CI fix · comb resonance folded · envelope cluster + display

## Changes
- `tools/waveshape_check.cpp`: `M_PI` → literal (CI fix); T7 comb-tone reachability gate.
- `src/fx_rack.h`: per-slot `tone` axis; comb `fb = 0.6 + 0.38·tone`.
- `src/hypersaw_clap.cpp`: ids 96–99 (`fx1tone..fx4tone`); per-voice envelope viz fill.
- `src/swarm_core.h`: `onsD0` (initial onset delay — viz bookkeeping only).
- `src/gui/hypersaw_gui.h`, `hypersaw_gui_common.h`: `envOnsetMs/envAtkMs/envRelMs`.
- `src/gui/gui.html`: scatter controls moved to Envelope; envelope display; FX tone rows.
- `DECISIONS.md`: ADR-080, ADR-081.

## 1. CI
`M_PI` is not defined by MSVC without `_USE_MATH_DEFINES`. macOS built fine; only CI
caught it. `waveshape_check.cpp` already spells π as a literal in two other places for
exactly this reason, so the fix follows the file's own idiom rather than adding a define.

## 2. Comb resonance — the human was right
`detune-lab.html` has **comb mix** *and* **resonance**; only mix was folded. ADR-071 had
recorded the gap explicitly ("resonance fixed at the lab default until the rack grows
per-slot param pages").

Folded as **one generic per-slot axis** (ids 96–99), not a comb-specific param: the next
slot type wanting a second control now costs no new ids and no new concept. Uses the
lab's own law `fb = 0.6 + 0.38·tone`, and `tone = 0.5` reproduces the hardcoded `0.79`
**exactly** — parity 147/147, and every saved state loads unchanged.

**Gated, because an inert default is how a dead control hides.** Parity proves only that
0.5 changes nothing; it says nothing about whether the knob is *connected*. The FX
dropdowns shipped Comb unreachable for exactly that reason and no oracle saw it. T7
measures ring time to −40 dB: **99 / 190 / 772 ms at tone 0.1 / 0.5 / 0.9**, required
strictly increasing.

## 3. Envelope cluster + display
Moved ids 91/92/93/94/95 from Drift to Envelope. The split is now **Drift = pitch
variation, Envelope = amplitude/time variation**. Onset scatter was the debatable one —
it is timing, not shape — but it moves *when each voice's envelope begins*, which is
precisely what the display shows, and splitting it from attack/release scatter would
scatter one idea across two clusters.

**The display is fed by the engine, not re-derived.** New viz fields publish, per voice,
the times the core actually assigned, recovered from the coefficients in use
(`t = −1/(sr·ln(1−c))`). The scatter draws from the core's seeded stream, so a JS
reconstruction would be a second implementation free to drift — the display would
eventually lie, and lie *plausibly*. Cost: one core array, `onsD0`, never read in the
audio path.

## Evidence
- `./verify full` exit 0. parity **147/147** (worst 4.262e-09 @ dyn-ring.seed42); all ten
  chains GREEN, including the new T7 and the declick gate at ratio 1.01 / planted 16.96.
- GUI verified in-browser against a synthetic viz snapshot: with no scatter the canvas
  has **0** amber pixels (nominal curve only); with 8 scattered voices, **2520** amber
  pixels across **173** distinct columns, and the readout reads "8 voices · entry spread
  44.0 ms · attack 2.2–6.1 ms · release 0.12–0.26 s".
- Cluster membership asserted programmatically: 91/92/93/94/95 all report `Envelope`,
  drift depth (id 9) still reports `Drift`, tone ids 96–99 all report `FX rack`.
- Installed 66df6f2.

## Note
Row labels were wrapping to two lines after the move ("onset scatter", "attack
scatter"); shortened to `onset ±` / `attack ±` / `release ±`, which also reads better
beside the `· correction` child row.
