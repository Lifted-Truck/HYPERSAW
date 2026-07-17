# 2026-07-17 — Ratification, cleanup, GUI reprioritization

**What changed.** Manifest status → RATIFIED (user go-ahead). Human-approved
deletions: `swarmsaw.html` v1 and `files.zip`. `swarmsaw_v2.html` renamed →
`swarmsaw.html` (reference names in ADR-003 hold again; both pre-rename
versions recoverable at the initial commit). Change note archived to
`docs/change-notes/2026-07-17-splay-legibility.md`; all references updated
(verify structure check, CLAUDE.md §Domain, SPEC §5.6, ACCEPTANCE L0-3,
README). Timeline + archival policy recorded as ADR-012. GUI pulled forward
to Phase 2 per human direction, recorded as ADR-013 (ROADMAP Phases 0/2/5
amended; GUI-stack choice added to Phase 0 exit criteria). README repo-status
refreshed; L0001 evidence updated with the rename. HYPERSAW registered in
autonomous's ecosystem tracks (execution-project registry; autonomous commit
7a7dbc1, its verify green, push left to the human).

**Why.** Post-ratification clean start requested by the human; v2 timeline
made explicit for future agents; GUI priority is a ratified direction change.

**Evidence consulted.** Reference-sweep grep of all swarmsaw_v2/CHANGE-NOTE/
files.zip mentions; pre-push privacy sweep (leak gate green; identity grep
clean; repo-visibility check of all referenced ecosystem projects via gh).

**Verify.** `./verify fast` green pre-commit (output in session; note the
structure check was updated for the rename in the same change — filename
only, no gate weakened).

**Open questions.** Pre-push sweep found one flag: one referenced sibling
project is a private repo and was named in the kit docs going public — human
ruled scrub-before-push; redacted to "terrain sibling" (ADR-014; alias map
in PRIVATE-NOTES.md, untracked).
