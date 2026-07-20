# 2026-07-20 — SPECTRA transposition (ADR-057) + viz-box height fix

Human sweep request: extend more SAW routings to SPECTRA (pitch controls, etc.),
plus a minor GUI fix (phase-circle box resizing when helper text arrives).

## Sweep audit (recorded in ADR-057)
- Already shared (route + show in both): K, onset, dissolve, seed, vol, retrig, width, cascade.
- **Gap fixed now:** transposition (octave/semi/fine/pitch, 35-38) — updateTune hit
  `core` only; spectra_core had no `tune`.
- Deferred (need core work): voice mono/glide (32-34), MPE per-note bend, drift/rtone/
  scatter/panScatter. Structurally SAW-only: dynamics, poles, n, dist, law, digital, inertia.

## Changes
- `src/spectra_core.h`: `SParams.tune = 1.0` + paramSlot "tune"; partial freq and sub-osc
  multiply `s.f0 * p.tune`.
- `src/hypersaw_clap.cpp`: `updateTune()` sets `tune` on core AND spectra.
- `src/gui/gui.html`: ids 35-38 removed from SAW_ONLY (show in both); gravline/beatinfo
  fixed heights (18/34px, overflow hidden) so helper text no longer reflows the box.

## Evidence
- `tune = 1.0` is bit-inert (f0*1==f0) → `./verify full` EXIT 0, parity 54/54,
  **spectra_check GREEN (rms 0)**, all 9 chains green, build clean.
- state_check green (35-38 round-trip; updateTune re-derives tune for both cores on load).
- git: b7763bc on branch feat-spectra-pitch
