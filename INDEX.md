# INDEX — retrieval map (HYPERSAW)

Read this in full at ORIENT; pull only matching LIBRARY entries. Tags:
swarm-dynamics · parity-oracle · plugin-platform · realtime-perf

- [L0001] parity-oracle (canonical) — reference prototypes update by file-drop and external ADR logs can collide with the local log; diff-verify, renumber/merge, headless-verify claims, fold into SPEC/ACCEPTANCE before goldens.
- [L0002] parity-oracle/swarm-dynamics — failing checks on a bit-parity port mean protocol mismatch, not port bugs; probe the reference, reproduce the measurement, or surface an erratum. Never adjust thresholds.
- [L0003] plugin-platform — every new C++ tool target repeats the MSVC traps (M_PI, -O3, runtime mismatch); grep before pushing, guards in CMake.
- [L0004] plugin-platform (canonical) — a filed PR branch is frozen, and a STACKED PR can silently merge into its already-merged base (GitHub doesn't retarget). New work = new branch off main; prefer un-stacked; verify the child reached origin/main after any merge. PRE-PUSH REFLEX: before pushing to any branch not created this turn, `gh pr view <branch>` must show OPEN (recurred 2026-07-22: 14 commits stranded on #70's merged branch).
- [L0005] plugin-platform/parity-oracle — duplicated key chains drift and symmetric lies pass value checks; single-source the maps, trust the audio-identity oracle.
- [L0006] plugin-platform — a wrapper's defaults are the host-facing surface: trace advertisement + inward event type + gating flags in the wrapper source (and look for its plugin-side extension seam) before claiming "host X delivers Y".
- [L0007] plugin-platform — "note doesn't stop" reports: exonerate core→shell→wrapper headlessly; AU wrapper forwards 0x90-vel-0 as NOTE_ON (shell must remap); fuzz timestamps drawn sorted or acausal orders fake hangs.
- [L0008] plugin-platform (canonical) — "host feature X doesn't work" can be a per-device HOST toggle (Live's MPE mode) or a stale scan-cache, not your code; verify host enablement + delivery mechanism before another code round. The concrete "gate elsewhere" branch of [L0006].
- [L0009] parity-oracle/plugin-platform (canonical) — external design/prior-art packets cite WRONG local ADR numbers, collide on ADR numbers, and claim absent features; grep every ADR cross-ref + scope claim against DECISIONS/SPEC/ROADMAP and web-verify citations before merging a triage. Extends [L0001].
- [L0010] plugin-platform (canonical) — CI costs money: macOS bills 10x, docs-only PRs shouldn't build, dedup push+PR triggers, put macOS post-merge-only; local ./verify full is the stronger gate. Long 'queued' on a public repo = account block, not strictness.
- [L0011] parity-oracle/plugin-platform — an oracle that builds the core directly can't see a shell-layer default divergence; give a second engine's control its OWN param+default (not the shared shell id) so goldens AND the shipped plugin are reference-exact. Coverage-gap cousin of [L0005].
- [L0012] parity-oracle/swarm-dynamics — a parity failure can be the REGIME (chaos), not the port: 1-ULP-perturb the JS reference alone; if JS-vs-itself diverges as far as C++-vs-JS and grows, remove the golden and anchor behaviour where the effect is unmistakable. Never loosen ε. Refines [L0002].
- [L0013] parity-oracle/plugin-platform — lab→reference→core folds cross independent namespaces: grep the destination for every name AND enum index first (reach→harmReach; lab law 3 vs core tempo-grid 3 → law 5); record the mapping in the ADR + both law tables.
