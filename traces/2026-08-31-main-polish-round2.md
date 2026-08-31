# 2026-08-31 — MAIN polish round 2 (ADR-151)

**What changed.** Review fixes on the ADR-150 landing: mini morph now uses
the ONE extracted `paintMorphField()` (aesthetic parity with the big pad by
construction), wakes on `_padInvalidate` (scheme-switch blank fixed), and
shows only while morph is on via `data-selfgate` (applyGates was clobbering
cluster visibility it didn't own). Presets un-nested; Macros moved to the
controls column. All XY assignment selects gain None (params 174-177/179/180
max 8; C++ alias slots emit 0.0 unassigned instead of &7-wrapping). Pads
carry live route notes (assigned macro + matrix routes riding it). Specimen:
uK eased (~280 ms lead-up / ~500 ms relax); held-wave temporal rate capped at
26 rad/s — it scaled with spatial wavelength and under-sampled into the
"jerky quivering" on high notes.

**Evidence.** verify full exit 0; lab_load 26/0; in-pane gating +
field-pixel + route-note checks all pass. Installed 6ee53f5.
