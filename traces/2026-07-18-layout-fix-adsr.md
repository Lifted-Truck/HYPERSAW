# 2026-07-18 — Resize layout fix (browser-verified) + ADSR (ADR-021)

**Layout.** Human screenshot showed value readouts clipping through panel
borders and stray numbers next to selects at large sizes. Root causes:
grid children refusing to shrink below content size (fixed with
minmax(0,1fr) + min-width:0 guards on rows and the outer grid), range
inputs without explicit width, header without flex-wrap, and displayValue
writing numeric text for select/checkbox rows. Verified by serving
gui.html locally and screenshotting at 1600×700 and 720×440 (the resize
extremes): clean at both, two-column reflow at minimum, units rendered.

**ADSR (ADR-021).** attack/decay/sustain/release (params 19–22, appended)
with defaults reproducing the reference AR bit-exactly: at sustain >= 1 the
render loop takes the reference's exact expressions and the time defaults
are the reference's own constants; sustain < 1 enters a new attack→decay
phase machine (deliberate superset divergence, recorded). Envelope times in
seconds converted per-rate (ADR-009 discipline); GUI gains an Envelope
cluster with log-scaled time sliders (ms display under 10 ms); state keys
additive.

**Evidence.** `./verify full` green — **parity 30/30 with the identical
worst case (2.535e-11)**, i.e. the ADSR change is proven invisible at
defaults; trajectories 21/21; pluginval strictness 10 SUCCESS; auval
SUCCEEDED; seals verified; sweep clean.
