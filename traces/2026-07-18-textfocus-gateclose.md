# 2026-07-18 — Text-entry focus fix (host-refuses-keys); Phase 3 gate close proposed

**Text entry (human report: edit box flashes away).** Diagnosis (the
human's read, confirmed by mechanism): the INVERSE of the classic webview
problem — the host wasn't granting keyboard focus to the webview, so the
new input blurred instantly and the blur-commit destroyed it. Fixes:
(1) choc acceptsFirstMouseClick on; (2) per-platform hzGrabKeys binding
(makeFirstResponder on macOS / SetFocus on Windows) invoked when editing
starts and on click-into-field; (3) the blur handler only commits if the
field GENUINELY held focus — a never-focused box now survives for a
retry click instead of self-destructing. Needs the human's Live re-test
(host focus behavior is untestable headless).

**Phase 3 gate close (ratify-by-merge).** ADR-015 anchors formalized as
ACCEPTANCE L0-22 (q-cluster R_q >= 0.95 across seeds; bistability;
split-as-timbre: 2f0 projection 0.080 +/- 0.015 seed-invariant, f0
residual seed-varying) and ENFORCED in trajectory_check (new projection
checks green: 0.080 exactly; residual spread 0.058). ROADMAP gate line
updated with the full evidence inventory.

**Evidence.** verify full green (51/51 · trajectories incl. L0-22 ·
state 8/8); pluginval 10 SUCCESS; auval SUCCEEDED; seals OK; installed.
