# 2026-07-22 — Fit-scale permanent; roadmap: root-pivot validated + design language

Reflection beat. Human: lab is nearing its limits (a recognizable lag click —
ScriptProcessorNode main-thread contention as the lab grew heavy, a LAB-
architecture limit, not a DSP bug; the C++ core won't have it). Ratified two
things and set a high-level design principle.

## Changes
- **Voice-map fit-scale is now permanent** (docs/design/detune-lab.html): human
  will always want it on. Removed the toggle; the map always auto-fits its Y
  scale to the current voice spread (falls back to 0.5–40× when audio is off).
- **ROADMAP — root-pinned pacemaker topology (validated):** Phase 3 topology
  entry added. Human keeps BOTH mean-field and root as a `sync pivot` toggle
  ("root sync is a great option… a more musical sound in general at a lot of
  settings"). Coupling-law divergence → prototype-first + ADR at port.
- **ROADMAP — design language (high-level guiding principle), Phase 5:** the
  final face should be as visual and intuitive as possible. (1) Naming by feel —
  literal only when conventional; else named for what it feels like in the swarm
  metaphor (K → "cooperation"; "swarm" → maybe "horde"). (2) Every non-obvious
  control pairs with a live visual. Needs a glossary pass: internal id → felt
  name → visual. The engineering names (K/σ/R/dissolve/…) get a player-facing
  translation layer; internal param ids stay stable.

## Evidence
- Script parses; no residual `fitScale` refs; `./verify fast` EXIT 0. Lab HTML +
  ROADMAP; no oracle/core impact, no ADR.
- git: on branch lab-detune-octave-spread (PR #70).

## Reflection — what the detune lab produced (for the eventual reference/core fold)
Validated keepers (human-confirmed by ear): harmonic law (unison→series) + reach;
octave spread + root-anchor; stretch (inharmonic) law; golden distribution;
alternating pan fan + pan curve/invert; per-voice + per-sample freq smoothing
(de-zipper); Karplus-Strong comb (polyphonic); tone tilt (bipolar) + master HPF;
hi-tame + round/round×hi glass; saw-profile + base two-slider morph + Saw Shape
visualizer; drift modes + centre-pin; pan-motion modes; sync-pivot root option;
master comp/limiter; keep-phase; clip meter; voice map. Lab-only limits: SPN
main-thread lag (the port removes it). Next gate: fold the winners into
swarmsaw.html (reference edit, ADR-011/012) → port to swarm_core.h with parity —
a deliberate human-gated milestone, not yet triggered.
