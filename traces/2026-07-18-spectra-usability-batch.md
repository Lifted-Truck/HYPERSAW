# 2026-07-18 — SPECTRA usability batch (on PR #30)

Four fixes from live testing, atop the SPECTRA shell integration:
1. Polyphony: SAW kPoly 8->16, SPECTRA kSPoly 4->8 (user hit ~6-7).
   kSPoly<=kPoly invariant held (shell VoiceTag array is kPoly-sized,
   indexed by active-core slot). Parity 51/51 unchanged — voice count
   is a per-instance array bound, not per-voice math. Worst-case CPU
   (SPECTRA 8x24x7=1344 sine osc) inside the ADR-006 spike headroom.
2. SPECTRA stereo width was UNREACHABLE (core key "swidth" vs the
   shell's "width"/id14). One slider now drives both: applyParam id14
   also calls spectra.setParam("swidth"). Removed 14 from SAW_ONLY so
   it shows in both engines; state restore covers it (mirrored value).
3. XY pad moved from the visualizer column into the controls column
   (user request) as its own cluster.
4. Empty-cluster auto-hide bug: it hid the XY cluster (a pure-canvas
   panel reads as "no visible rows"). Now skips clusters with zero
   gateable rows.
Plus the slider-crawl fix (e.shiftKey) carried here for branch
self-consistency (also standalone in PR #34).

Evidence: verify full green (51/51 · trajectories · state · force ·
spectra); pluginval 10 SUCCESS; GUI moves verified in the browser pane
(XY in controls both modes, width in both, SPECTRA cluster intact);
installed.
