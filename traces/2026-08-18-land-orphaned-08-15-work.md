# 2026-08-18 — landing two commits that never merged (2026-08-15)

**What.** Rebased `route-inertia-decided` onto `main` and landed it: the
four-agent feedback field survey report (194 lines, **present nowhere on main**)
and two ROADMAP entries — *FEEDBACK FIELD SURVEY — and OQ #23 is missing a half*
and *INERTIA DECIDED — a property of the ROUTE*.

**How it was missed.** The branch was cut 2026-08-15, never had a PR opened, and
later branches were cut from `main` rather than from it, so nothing downstream
ever noticed the absence. Found by an explicit sweep of local branches against
`origin/main`, not by any gate. Nothing cross-repo was lost: the inertia decision
itself was filed with FOUNDATIONS on the day and they adopted it — what was
missing was *our own* record, in the file the charter names as the single source
of truth.

**Conflict resolution.** Both commits prepend to a newest-first log, so git's
"put it at the top" is wrong by construction — a replayed 2026-08-15 entry
belongs at its date. Both were placed by date: below the four 08-15 feedback
entries already on main (the lab was built *from* this survey's hypotheses, so
the survey precedes it) and above *INERTIA EVERYWHERE*, which preceded the
decision. Resolver kept in the scratchpad; the anchor is passed in per commit
because the conflict region covers only the top of the file while the correct
anchor usually sits in the untouched tail.

**Honesty fix.** The survey entry's title asserts *"OQ #23 is missing a half"* —
true on 2026-08-15, overtaken on 2026-08-17 when #23 was ruled (partly on our
evidence). Landed as written with a dated forward-pointer rather than rewritten:
the log records what we thought on the day, and the correction belongs beside it,
not in place of it.

**Two process failures of mine, recorded because they nearly cost more.**
1. A first resolver assumed the chronological anchor lived inside the conflict
   region; it does not. The script raised, `grep` in the same `&&` chain returned
   success, and the shell went on to `git add` a file **still containing conflict
   markers** and continue the rebase. Aborted and redone. The lesson is the one
   this session already wrote up: a step that "passed" because a *different*
   command's exit code was consulted is a check that cannot fire.
2. `git add -A` swept the human's untracked `GoopBox.jsx` and `.gitattributes`
   into the commit — files I had explicitly said I would leave alone. Stripped
   with `git rm --cached` and amended; both are untracked again, unmodified.

**Verify.** `./verify fast` EXIT=0.
