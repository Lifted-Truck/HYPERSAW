# 2026-07-20 — Internal FX rack, increment 1 (placeholder skeleton)

First implementation increment of ADR-054 (internal FX rack). Human picked this
as the next build ("routing grid ASAP, placeholder-first").

## What changed
- **`src/fx_rack.h`** (new) — `FxRack`: 4 series slots processed in slot order,
  each running one placeholder effect in place on the stereo bus. Types:
  `Off / Drive (dry-wet tanh) / Filter (one-pole LP) / Gain`. Off is `continue`
  (never touches the buffer) → all-Off is a bit-exact passthrough (the parity
  gate). Header-only, allocation-free, no wall-clock (charter RT rules).
- **`src/hypersaw_clap.cpp`** — 8 append-only params (ids 57–64: 4×{type,amount};
  coreKeys `fx{n}type`/`fx{n}amt` are unique non-core strings used only as
  state-blob keys). apply/readParam intercept 57–64 → `rack`; `rack.processStereo`
  runs post-oscillator, post-bass-mono, pre-viz-feed. Order = slot index;
  "reorder" = reassign types to slots (the simplest thing that makes FX order
  audible — the human's #1 requirement).
- **`src/gui/gui.html`** — "FX rack" cluster (4 slot type-selects + amount
  sliders); DEFAULTS 57–64; amount dims when its slot is Off (SOFT_GATES).
  Shown in both engines (not in SAW_ONLY/SPECTRA_ONLY).

## Evidence
- Build: `.clap` / `.vst3` / `.component` compile + link clean.
- `./verify full` — EXIT 0, all 9 oracle chains GREEN. parity_check 54/54
  (cores untouched); **state_check GREEN — "restored instance renders
  bit-identical audio"** (the 8 rack params round-trip; all-Off restores
  identically).
- pluginval **strictness 10 SUCCESS** on the built VST3 (param fuzz exercised
  all 65 params incl. the 8 new ones).
- GUI `<script>` passes `node --check`; rack markup/defaults/gates present.
- git: 16f8c16 on branch feat-fx-rack-increment-1

## Deferred (later increments)
- Real cores as slot types (filter/notch/time + a saturation engine); the
  saturation drive is the squareness-experiment lever.
- FX params as mod destinations (needs the mod matrix); true parallel/matrix
  grid; drag-reorder UX; per-voice-vs-global placement.

## Not done here (human Layer-E — needs a DAW; the ~/Library install + auval were
blocked by the destructive-pattern hook, correctly)
- Install to `~/Library`, `auval -v aumu Hsaw LfTk`, and load in a host to hear
  the rack + confirm order-significance by ear. All-Off inertness is guaranteed
  by construction + state_check, so this is a listen/see pass, not a correctness
  gate.
