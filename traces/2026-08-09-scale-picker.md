# Scale picker — a pitch-class set is a shared control

**Date:** 2026-08-09
**Trigger:** human — *"when it's in scale mode it will need a scale selector. It
might be nice to be able to choose the semitone pattern with a little
approximation of an octave on a keyboard. This could also be useful for effects
or modulations we add down the line."*
**Verify:** `./verify full` GREEN — 13 chains, L0-1 parity **147/147 worst
4.262e-09 (unchanged)**, `glide_check` 0 failures worst rms 3.51308e-08.

## What changed

`hzScalePicker` in `docs/design/bend-lab.html`: root selector, named-scale
dropdown (18 scales), and a one-octave keyboard whose keys toggle degrees.
Shown only in scale mode. Three new oracle scenarios in
`tools/golden/gen_glide_goldens.mjs` + `tools/glide_check.cpp`.

## The gap was L0023, not a missing nicety

`scaleMask[12]` and `scaleRoot` already existed in BOTH references —
`bend-lab.html` P literal and `src/glide_core.h:54` — and had since the A1 fold,
with nothing anywhere able to set them. Scale mode had therefore only ever meant
C major; the dropdown option literally read `scale (major)`. A reachable range
with no control is an invisible feature.

## Ruling: the mask is the truth, the name is UI

Consumers store and transmit `{root, mask}` only, never a scale ID. This is what
keeps `glide_core.h` free of a scale table — a new named scale is a UI-table row
that adds no core change and **no parity surface**. The rejected alternative was
a `scale` enum param, which forces the same table into C++ and makes every new
scale a parity risk. Hand-drawn sets are first-class, not a degraded mode: the
dropdown reverse-matches the mask to a name or reads *custom*.

Keyboard is ABSOLUTE, mask is RELATIVE — changing root transposes the lit keys
(C major → D major moves the accidentals), matching the core's
`((c - root) % 12 + 12) % 12`.

## Empty set made unreachable rather than handled

Root stays lit; the last lit degree cannot be cleared. An empty mask is the one
input the quantiser has no defined answer for: both references fall through to
plain rounding, and `Math.round(-0.5) = -0` vs `std::lround(-0.5) = -1` disagree
on exact .5 ties. Blocking it at the only control that can produce it beats a
downstream guard written twice in two languages.

The tie divergence is latent in chromatic mode too, unreached by the current
gesture. Recorded, not silently "fixed" — changing either reference's rounding
moves goldens, so it is a decision, not a cleanup.

## Oracle widened to match the new reachable space

`glide-quant-scale` had only ever rendered C major, so every other mask became
untested code the instant the picker existed. Added `glide-quant-root3`
(D# minor pentatonic — exercises the non-zero-root wrap), `glide-quant-whole`
(whole tone — wide gaps, .5 ties reachable), `glide-quant-sparse` (G hirajoshi +
hysteresis). All three parity **rms 0**.

**Calibration — the step that matters.** rms 0 is also what a scenario returns
if the mask is being ignored entirely, so the masks were checked to be
non-vacuous: the four scale scenarios emit genuinely different step sets.

| scenario | root | set | emitted steps |
|---|---|---|---|
| glide-quant-scale | C | major | −1 · 0 · 2 |
| glide-quant-root3 | D# | minor pentatonic | −2 · 1 |
| glide-quant-whole | C | whole tone | −2 · 0 · 2 |
| glide-quant-sparse | G | hirajoshi | −2 · 2 |

Pairwise max divergence 1–3 semitones. No scenario can pass by the mask being
inert.

## In-page verification (the component, not just the load)

Driven live in the browser: 12 keys (7 white / 5 black); default C major lights
CDEFGAB; root=D + natural minor yields mask `[1,0,1,1,0,1,0,1,1,0,1,0]` and
lights C D E F G A + A# with B dark (correct D natural minor); clicking a black
key flips the readout to *custom*; clicking every lit key three times leaves
1 degree — the empty-set guard holds and the root stays lit.

Layout: `.sp` needs 96px label gutter + 182px keyboard = 278px inside a ~316px
card content box (`column-width:340px`, 12px padding), so no repeat of the
2026-08-08 column-overflow collision. NB the preview pane renders these labs at
`viewport [0,0]`, where every row measures 0 width — those numbers are not a
layout signal, and were nearly read as one.

## Standby / library

Recorded in `INTEGRATION-STANDBY.md` (component inventory, contract stated) and
`docs/integrations/corelib-insights.md`. NOT extracted to a shared module —
FOUNDATIONS standby forbids speculative extraction, and the second consumer
earns it. Copying once is the honest price of ADR-003 single-file labs.

## Evidence consulted

- `src/glide_core.h:40-56,150-176` — Quant enum, scaleRoot/scaleMask, quantise()
- `docs/design/bend-lab.html` — P literal (159-160), JS quantise (263-)
- `tools/golden/extract_glide.mjs` — slice anchors held (component sits outside
  the `const TAU` → end-of-`class Inertia` slice, and adds no second `const P`)
- `./verify full` log: 13/13 chains GREEN
