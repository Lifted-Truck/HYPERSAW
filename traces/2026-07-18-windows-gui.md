# 2026-07-18 — Windows GUI backend (WebView2 via choc)

**What changed.** The webview backend split per the ADR-019 seam design:
`hypersaw_gui_common.h` (JS bridge bindings + viz serializer, platform-free)
shared by `hypersaw_gui.mm` (Cocoa attach, refactored) and new
`hypersaw_gui_win.cpp` (WS_CHILD SetParent attach; choc's embedded WebView2
COM declarations — no external SDK). Shell gui extension now compiles on
both desktop platforms (HYPERSAW_WINDOW_API selects cocoa/win32; parent
handle per platform). CMake: WIN32 gui sources + ole32/shlwapi/version;
embed step resolves the Python interpreter via find_package (bare `python3`
is absent on Windows runners — L0003 trap family).

**Honesty line.** The Windows backend is CI-COMPILE-VERIFIED ONLY: no
Windows machine in the loop, so host embedding, focus, and resize on
Windows are a recorded Phase 2 residual (same class as Reaper/Bitwig).
macOS behavior unchanged (same shared bridge; refactor only).

**Evidence.** macOS build clean; verify full green (parity 30/30 identical
worst case, trajectories green); Windows compile status = the PR's CI.
