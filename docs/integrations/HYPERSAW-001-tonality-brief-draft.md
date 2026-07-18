---
id: HYPERSAW-001
status: filed
ball: provider
filed: 2026-07-18
respond-by: 2026-08-01
consumer: HYPERSAW (github.com/Lifted-Truck/HYPERSAW)
---

# Intake brief: ratio priors & context-weighted basins for consonance gravity

## Who is asking
HYPERSAW — CLAP/VST3 synthesizer; held notes are pulled toward simple
frequency ratios by an audio-engine force (its ADR-008: pairwise pulls on
fundamentals, octave-folded ratio snap, basin-limited). Shipping now with a
fixed 13-ratio set + uniform basin knob — the visibly-degraded placeholder
this brief aims to replace (HYPERSAW SPEC Layer 3: "Tonality-supplied
priors, context-weighted basins").

## The need
1. A richer ratio prior: per-ratio weights (attraction) and basin scaling
   (capture radius), rather than 13 equals.
2. Later: context-weighting — basins adapting to harmonic context (held
   identity-key / induced key). The JI half may sit outside a 12-TET core's
   scope; defer-with-rationale on that part is a fine response.

## Proposed interface delta (hot-path rule respected)
HYPERSAW's audio thread never calls Tonality. Proposed: a versioned export
artifact generated offline (CLI or MCP) and loaded at plugin start/preset:
  { "schema": "tonality-gravity-priors.1",
    "provenance": {...},
    "ratios": [ { "num":3, "den":2, "name":"3/2", "weight":1.0, "basin_scale":1.0 } ],
    "context": null }
"context" reserved for the phase-2 variant (keyed by identity-key bitmask
if that is the natural representation).

## Contract tests offered (consumer-authored, resident-landed)
- Schema validation; determinism (identical inputs → byte-identical export);
  fold-safety (all ratios in [1,2), no duplicates).

## Not blocked
HYPERSAW proceeds on its default set; swap-in is one table load.
