# 2026-08-18 — kit 2.4.0: vendored kit-owned gates

**What changed.** `.kit/kit-gates.sh` + `.kit/MANIFEST` vendored by `kit_sync.py`;
`migrate_to_vendored.py --apply` removed the kit-owned `record()` and
`leak_gate()` from `./verify` (422 → 350 lines, 72 removed) and replaced them
with a `. .kit/kit-gates.sh` source plus `kit_integrity` in `fast`.

**The three required checks, all passing.**

1. `./verify fast` EXIT=0; every project gate still runs (mailbox_delivery,
   presentation_check, gen_gui_controls, test_table_check, gui_reach).
2. Reachable, not merely present: `grep -c 'kit/kit-gates.sh' verify` = **3**.
3. It fires: a planted POSIX home path (`/U-s-e-r-s/<name>/…`, spelled apart here
   so this trace does not trip the gate it describes) is named in the output.

**The upgrade was real for us.** Our copied `leak_gate` carried **no Windows
identity pattern** — confirmed against the deleted block: zero matches for the
drive-letter home form. The vendored gate has it, and a planted Windows home
path is now caught. That path was open in this repo until today.

## Regression found and repaired: the migration deleted a gate that was ours

`migrate_to_vendored` reported "72 removed, **all kit-owned**". Not all of it
was. The private-sibling name check (ADR-014, added 2026-08-17 after three
tracked files were found carrying real names) had been written **inside** the
kit's `leak_gate()`, so it went out with the function. After migration,
`grep leakcheck-names verify .kit/kit-gates.sh` returned nothing: the alias rule
was unenforced again, silently, one day after being enforced.

**The script is not wrong to have done it** — it identifies kit code by function
boundary, and by that criterion the block was inside a kit-owned function. The
error was ours, a year of it compressed into a day: **project substance living
inside kit mechanism.** The kit gate's business is identity PATHS; our
correspondents' names are HYPERSAW's business. Restored as `private_name_gate()`
in `verify`, project-owned, called beside `leak_gate` in `fast`, with a comment
saying why it must never move into `.kit/`.

**Both controls exercised, because a gate that cannot fire reads like a gate that
passes (L0032):**

```
plant a real private name (the granular sibling) -> FIRES, names the file
plant benign prose ("in place of the …")   -> correctly silent
```

**Method failures, recorded — twice in one task, same shape as the day's theme.**
1. A first restore script raised `StopIteration` on a bad anchor and wrote
   nothing; the `bash -n verify && echo "syntax OK"` and `./verify fast` that
   followed reported cleanly **on the unmodified file**, and were briefly read as
   confirmation. The gate was still absent.
2. The second attempt asserted the wrong occurrence count (3, actual 2) and
   refused to write. That one failed in the safe direction — and is the reason
   the first failure was caught at all.
   Both are the same lesson this session already wrote up twice: a step that
   "passed" because a *different* command's exit code was consulted.

**Not done, per the instruction.** Nothing pushed; nothing committed into
`autonomous`. `.kit/*` untouched by hand.

**And the gates caught this trace.** Committed before running `verify` on it, so
both fired on the very file describing them: two literal identity paths for the
kit gate, and the granular sibling's real name — written while documenting the
plant — for the restored one. Fixed and amended. Writing *about* a pattern
produces the pattern; that is the third time this repo has learned it (ROADMAP
2026-08-17, the alias-gate entry that tripped itself) and the first time both
gates demonstrated it on the same file.
