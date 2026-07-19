# 2026-07-18 — SPECTRA per-partial strip visualizer + velocity on roadmap

**Strip visualizer.** Ported swarmspectra.html drawStrips() into the
plugin GUI: one column per partial, cloud-phase dots up each strip
(brightness = partial amplitude), magenta per-partial R bar along the
bottom. Under cascade the lock bars climb the series low->high — the
zipper made visible. Plumbing: VizSnapshot gains a SPECTRA section
(spectra flag, partials/cloud, partR/partAmp/partPhase); SpectraCore
adds partialAmp(k); publishViz fills it in SPECTRA mode; vizToValue
serializes it (gated by v.spectra, so SAW frames carry zero extra);
gui.html swaps the carpet scope for a strip scope by v.spectra. The
scope-swap runs BEFORE the draws so a draw exception can't strand the
panels (found via a mock-data drawPhase throw — the real snapshot
always carries RN, but robustness first). Additive/viz-only — parity
51/51, spectra rms 0, all oracles green.

**Velocity routing** added to ROADMAP Phase 5 as a first-class mod
source (amp/K/onset destinations, per-dest depth, default-off so the
current velocity-flat behavior and every golden stand). Noted the
ADR-039 interaction (vel<=0 is note-off).

Evidence: verify full green; pluginval 10 SUCCESS; strips verified in
the browser pane (cyan cloud dots + magenta R bars in SPECTRA, carpet
restored in SAW); installed.
