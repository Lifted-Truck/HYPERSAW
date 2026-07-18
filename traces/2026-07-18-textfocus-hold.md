# 2026-07-18 — Text-entry focus: from grab-once to hold-for-edit

**Human re-test of PR #23's fix:** "focus for a split second and then it
takes it away." Diagnosis refined: the grab WORKS — Live grants key focus,
then re-asserts its own key window a beat later. Two consequences in the
old code: (a) makeFirstResponder without makeKeyWindow only half-claims
keys; (b) commit-on-blur made the host's steal destroy the edit box —
the "flashes back to static" the human saw was our own commit path.

**Fix (hold, don't grab):** macOS hzGrabKeys now claims makeKeyWindow
before makeFirstResponder. The JS edit box drops commit-on-blur entirely;
a 120 ms keep-alive re-grabs whenever activeElement leaves the input, and
blur schedules an immediate re-grab. The edit ends ONLY on Enter (commit),
Escape (cancel), or a click outside the field (commit) via a capture-phase
document listener. closed flag + interval teardown in done().

**Evidence:** build clean; standalone lifecycle exercised headlessly in
the browser pane (create → Enter-commit → reopen → outside-click-commit
all pass, no console errors); verify fast green; bundles installed +
re-signed, seals verified. Host focus tug-of-war itself remains
untestable headless — needs the human's Live re-test.
