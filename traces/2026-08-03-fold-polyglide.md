# 2026-08-03 - ADR-076: poly glide

## Changes
- swarm_core.h: `polyGlide` param, `lastNoteF` member; noteOn seeds f0/f0cur from
  lastNoteF and arms the existing glide; retargetNote refreshes lastNoteF.
- hypersaw_clap.cpp: id 89.
- gui.html: toggle beside retrigger; glide-time soft gate now mono OR poly glide.

## Evidence
- A2 -> A3 at glide 0.30 s: start 110.0 Hz, 50 ms 125.8, 300 ms 179.8, 1.5 s 219.2.
- Inert both ways: polyGlide 0, and polyGlide 1 with glide 0, both start at 220.0.
- parity 147/147; verify full green.

## Glide source mode (added same PR)
`glideMode` id 90: 0 = held note (legato, default), 1 = last note (memory).
Measured (220 = no bend, 110 = bends in): legato 110.0 overlapping / 220.0 after a
rest; memory 110.0 / 110.0. anotherHeld is computed BEFORE alloc() — alloc can
steal a gated voice, so asking afterwards misreports.

## Deferred
Per-voice chord mapping (each new voice bending from its own nearest predecessor).
lastNoteF is a single frequency, so a chord bends as a block. Recorded in ADR-076
rather than half-implemented.
