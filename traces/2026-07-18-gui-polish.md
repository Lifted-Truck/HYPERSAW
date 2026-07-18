# 2026-07-18 — GUI polish: resize, drift range/units (ADR-020)

**Human feedback (Live test).** GUI works; no keyboard stealing with
sliders/pad (ADR-019 risk #1 downgraded — remaining watch item is the state
textbox, the only text-enterable field). Window was fixed 780×480 and
scrolled both ways; drift depth range too small; drift rate units opaque.

**What changed.** (1) Resizable editor: can_resize + hints, clamp
720×440–1600×1000, default 920×600; shell owns current size; webview child
autoresizes; gui.html layout made fluid (minmax grid, auto-fit clusters).
(2) Drift depth 0–25c → 0–100c (ADR-020: reference pins the DSP mapping,
prototype UI ranges are ergonomic defaults the plugin may exceed —
recorded; automation-rescale caveat noted for post-1.0). (3) Drift rate
displays the real random-walk speed (0.2 + knob·8) as "N.N /s" in both
value_to_text and the GUI; dissolve and depth gained unit suffixes too.
Branch note: PR #4 had merged into phase2-saw-integration after PR #3
reached main, stranding the GUI commit off-main — this branch is based on
the GUI-carrying branch, so its PR delivers GUI v1 AND the polish to main.

**Evidence.** Build green; pluginval strictness 10 SUCCESS; auval
SUCCEEDED; seals verified; `./verify full` green (30/30 + 21/21) — core
untouched this change.
