# 2026-07-23 — Seventh fold: stretch (inharmonic) detune law, law 5 (ADR-066)

Second of the new detune laws. Cents placement with the OFFSET stretched by
(1 + B·x²) — piano/bell inharmonicity, the metallic complement to ADR-065's
harmonic law (which lands ON the series; this one deliberately misses it).
Off fresh main after #80 merged (gh pr view = MERGED first — L0004 reflex).

## Changes (reference-first, ADR-003)
- swarmsaw.html: `stretchB:0` param; `else if (p.law === 5)` →
  `rat = 2^(x·detune·100/1200) − 1; f = f0·(1 + rat·(1 + stretchB·x²))`,
  placed ahead of the ERB else.
- swarm_core.h: mirrored (`double stretchB = 0`, same branch, paramSlot).
- gen_goldens.mjs: stretch-flat / stretch-mild / stretch-bell. Also corrected a
  stale comment on the ADR-065 block ("and the law under coupling" — that
  scenario was removed as impossible; see ADR-065).
- DECISIONS.md: ADR-066.

## The numbering decision
The lab menus stretch as **law 3**. Core law 3 is the TEMPO-GRID law
(ADR-005/022) and law 4 is harmonic (ADR-065). A literal transcription would
have overwritten the tempo grid — and `dyn-grid` (which selects law 3 via
`coreToDyn`) would have gone on rendering an inharmonic stack while still
called a grid, quite possibly staying green. Stretch takes **law 5**; the lab
keeps its own numbering (it is an audition instrument, not a numbering
authority). Recorded in both law tables so the next transcription doesn't
re-derive it wrong.

## Evidence
- `./verify full` EXIT 0 — parity 93/93 → **102/102** within ε, the 9 new at
  **rms 0.000e+00**; all 8 chains + notefuzz GREEN.
- **B = 0 ≡ law 0 measured, not asserted.** `1 + (v − 1) == v` is not a general
  FP identity, so a temporary `ident-law0` scenario (law 0, same detune/n) was
  generated next to `stretch-flat` and the renders byte-compared:
  **IDENTICAL on all three seeds** (sha256 `ac88657551bd…` both). Temp scenario
  removed; `stretch-flat` now pins that identity as a regression.
- git: on branch fold-stretch-law (off fresh main).

## Next
Remaining laws: golden distribution, octave spread + root-anchor (the latter
brings `spread`/`anchor`, which the lab's law expressions already assume — note
the lab computes `dep = detune·spread` and `x − anchor·xmin`, so folding those
touches EVERY law, not just one). Then the divergences (root-pivot topology,
pan default image, saw-shape retarget). CLAP params batched last — and that
batch must widen `law` 0..3 → 0..5 with labels, since harmonic and stretch are
both currently unreachable from the host.
