# ADR-082 proposed — the multi-oscillator namespace, and the layout lab caught up

**Date:** 2026-08-06
**Trigger:** "let's continue" → the interface renovation. The layout lab's own Decision 2
says the multi-oscillator ADR gates all GUI work, so the ADR is the first deliverable.
**Verify:** `./verify fast` GREEN; all 12 labs load clean.

## Why the ADR came before any GUI

CLAP param ids are append-only. Measured: **99 params, ids 1..99, densely packed with no
gaps**, of which **~70 are per-oscillator**. Get the namespace wrong and every saved session
pays forever, so this is designed once — the exact class of irreversible interface decision the
charter gates.

## The decision, and the constraint that fell out of the arithmetic

`id(P, osc k) = id(P, osc 0) + 100k`. Osc 0 keeps every id it has today; blocks of 100 for
osc 1 and 2; slots where a global's id would fall are never allocated, so the gaps document
which ids are global.

The CPU section produced something I did not expect going in. Current cost is 2.5% of one core
at 1x and 6.3% at 2x oversampling (ADR-075, this M3), against a 50% min-spec budget. The voice
loop scales linearly, so at the project's own ×4 min-spec derate **3 oscillators + 2×
oversampling reaches ~75% — over budget**. That is arithmetic on top of an estimated derate,
not a measurement, so the ADR requires a real min-spec run before increment 2 rather than
treating the table as a verdict. But it does mean "3 oscillators" and "2× oversampling" cannot
both be unconditional, which is a design constraint that would have been discovered late and
expensively.

## Near-miss worth recording

The ADR was first written as ADR-080. A guard assertion (`assert '## ADR-080' not in s`) caught
that 080 was already taken — the FX-rack second-axis ADR from 2026-08-03. I had grepped
`^## ADR-07` and read 079 as the ceiling; the real max was 81. Two numbered ADRs sharing an id
would have been quietly corrosive to every future cross-reference. **The assertion did the work
that my grep did not** — cheap guards on "is this name already used" are worth writing even
when you are confident.

## Layout lab caught up to rulings it predated

- **SAW-first**: engine selectors removed from the osc strip; the SPECTRA section dimmed and
  relabelled as legacy-patch-only (`engineOptionGuard`).
- **Compact morph XY on MAIN** (requested 2026-08-05), with the four corner glyphs — the play
  control, explicitly not the editor.
- **Provenance colouring on OSC** (requested 2026-08-05): each parameter wears the hue *and
  glyph* of the corner its live value came from; blended parameters are hatched neutral rather
  than claiming a source; un-morphed parameters stay unstyled as a deliberate third state.
  Verified in-page that the corner variables are byte-identical to the mod lab's `CORNERS`
  (#f2b134 / #ff4d6d / #4cc9f0 / #7ae582) — the standing convention is that corner colour means
  one thing everywhere, and two independently-typed palettes would break it within a week.
- Decision 2 rewritten to carry the ADR's answer, with the superseded text kept dimmed.

## Open for the human (in the ADR)

Slot count (2 or 3); whether the sub-oscillator block (52–55) and `balance` (56) are superseded
by real oscillators or kept; whether oversampling stays global or goes per-osc.
