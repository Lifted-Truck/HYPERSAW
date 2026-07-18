# 2026-07-18 — Phase 2: GUI v1 (choc webview) + ADR-019 smoke test PASSED

**What changed.** GUI v1 behind the ADR-019 seam: `src/gui/hypersaw_gui.h`
(the seam — VizSnapshot plain struct + GuiHost callbacks, zero webview/CLAP
types), `src/gui/hypersaw_gui.mm` (ALL webview/Cocoa specifics; choc pinned
as submodule libs/choc @ a08bfd84), `src/gui/gui.html` (embedded at build
via tools/embed_file.py — hex array, self-contained, webfonts deliberately
dropped for offline operation). Design language lifted from swarmsaw.html:
palette/layout/classes verbatim, drawPhase() ported line-for-line (seat
rings, formation polygon, order vector, dual R₁/Rₙ meters), XY pad
(x=detune, y=bipolar K per SPEC performance layer), all 18 params in
prototype cluster layout (dissolve slider log₁₀ like the prototype knob).
Dev state button (ROADMAP request): COPY STATE dumps
{"plugin","schema",params} JSON to clipboard+textbox; PASTE STATE applies
it — the dump IS the preset format. Shell wiring: SPSC param queue
(GUI→audio, drained in process/flush, emits CLAP param_value + gesture
events so host automation records), double-buffered viz snapshot published
per process block, CLAP gui ext (cocoa; other platforms deliberately absent
→ generic editor, Phase 2 remainder). swarm_core.h gained one read-only
accessor (centerIndex) — parity-neutral, verified below.

**Evidence.**
- **ADR-019 risk #2 retired:** pluginval strictness 10 SUCCESS including
  Editor and Editor Automation tests — the webview NSView passes through
  clap-wrapper into VST3 and survives host-style editor open/close cycles.
- auval SUCCEEDED on the GUI build; seals verified post-install.
- `./verify full` green (30/30 parity + 21/21 trajectories) — the accessor
  and shell changes did not disturb the core.

**Open (human).** Visual test in Live: GUI appears, phase circle animates,
XY pad drives detune/K, COPY/PASTE STATE round-trips. ADR-019 risk #1
(keyboard-focus stealing in Live) needs a human check — type in Live's
transport with the GUI open; key-forwarding work is queued if it bites.
Risk #3 (multi-instance memory) measured later in Phase 2. Windows GUI
(WebView2) is a recorded Phase 2 remainder.
