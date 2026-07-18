# 2026-07-18 — Viz regression fixed (buffer truncation); spectrum; param UX

**The regression (human report: all viz dead).** Root cause: the dynamics
viz serialization's grid-status line is ~71 chars; its snprintf buffer was
64 — silent truncation, malformed JSON, JSON.parse threw every frame, and
the catch-all swallowed it: every visual element dead at once. Fixed
(buf 160 with a comment carrying the lesson); GUI now also ships a
standalone mock bridge (installed only when the real one is absent) so
layout/interaction testing in a plain browser exercises the full draw path
— verified live in the pane: phase circle animating, meters, gravity
readout ("3/2 +0.2c"), grid lock warning, spectrum polyline.

**Spectrum (human request, prototype homage).** Audio thread writes a mono
ring (4096, write-only, cheap); the GUI thread computes a 2048-point Hann +
radix-2 FFT ON DEMAND per hzGetSpec poll (~20 fps) → 96 log-spaced bins
30 Hz–16 kHz, drawn prototype-style. Zero audio-thread analysis cost; torn
ring reads are cosmetic-only.

**Param UX (human requests).** Double-click a slider → reset to default
(defaults table mirrors kParams). Shift+drag → 20× fine mode without
jump-to-click. Double-click a value display → inline text entry (Enter
commits, Esc cancels, clamped, log-domain aware) — the Ableton
keyboard-focus test is the human's (the known ADR-019 watch item now has
a real text field to probe it with).

**Also.** Layer-E 3 signed off; Tonality brief ON HOLD per the human
(they'll prime Tonality; scope discussion open — recommendation recorded
in session: the static-ratio-table half is data HYPERSAW can own without
violating the never-reimplement rule; only context-weighting is genuinely
Tonality's domain).

**Evidence.** verify full green (51/51 · trajectories · state 8/8);
pluginval 10 SUCCESS; auval SUCCEEDED; browser-verified GUI; installed.
