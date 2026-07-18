# 2026-07-18 — SPECTRA shell integration (engine select, ids 43-51)

Engine select (43, SAW/SPECTRA) dispatches notes + render to the active
core; switch is click-safe (allOff both, tags/held cleared). SPECTRA
surface 44-51 (partials/tilt/stretch/cloud/cwidth/wtilt/wlaw/cascade);
shared-name knobs (K/onset/dissolve/seed/vol/retrig) mirror into both
cores via applyParam (unknown keys no-op). SPECTRA mode is plain poly
v1 (mono/glide are SAW-side; recorded in ADR-037's follow-up list).
emitNoteEnds reads the ACTIVE core's liveness (thresholds match each
render's skip test). activate() recreates both cores at host rate.
Viz v1: partial-0 cloud on the phase circle. GUI: Spectra cluster +
engine gating (SAW-only controls hide in SPECTRA mode and vice versa;
shared coupling stays) — verified in the browser pane. state_check
auto-covers 43-51 (round-trip + bit-identical restored audio).

Evidence: verify full green (51/51 · trajectories · state · force ·
spectra incl. P=1 gate dR=0); pluginval 10 SUCCESS; auval SUCCEEDED;
seals OK; installed.
