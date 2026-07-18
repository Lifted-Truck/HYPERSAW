# 2026-07-18 — Phase 3 increment 1: dynamics core, parity-proven both ways

**What changed.** SwarmCore gained the full dynamics layer (ADR-023):
topologies (mean-field / nonlocal ring / two-cluster Abrams-Strogatz with
bimodal placement), Sakaguchi α, Daido poles q, consonance gravity
(per-render-block, f0cur, 13-ratio octave-folded snap, basin-limited,
readout feed), lpOut config param (the single structural difference between
references). Laws now run on f0cur (≡ f0 when gravity off). Golden
generator extracts BOTH reference cores; manifest gained engine + notes
columns; parity_check replays either protocol (incl. two-note gravity).
Trajectory suite gained L0-8..11 + ADR-015 anchors.

**Results.** Parity 51/51 (SAW 30/30 unchanged incl. identical worst case;
DYN 21/21, 9 bit-exact, worst 4.3e-9 chaotic ring). Trajectories all green.
Findings en route (L0002 discipline): dist=even had to be pinned for DYN
scenarios (the failure pattern — two-cluster bit-exact, everything else
off — localized it to voice placement in one step); L0-11 snapshot values
are one moment of a wandering pattern (windowed observation, thresholds
unchanged, reference-probed); L0-8's original outside-basin probe note
(350 Hz) actually folds INSIDE the basin — replaced with 240 Hz.

**Not yet exposed.** The plugin surface (CLAP params, GUI dynamics cluster,
R_q/RA/RB meters, gravity + grid-status readouts per ADR-016/017,
absolute-K mode) is increment 2 — the engine is headless-complete only.

**Verify.** verify full green end-to-end (51/51 parity + trajectories +
state_check 8/8 + build).
