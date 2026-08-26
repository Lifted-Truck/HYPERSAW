# Trace — the answer was consumed before it was pushed; R-held-7 came back

**Trigger** the stop-gate firing a second time on `mailbox_delivery`. Checking
the blocking file's commit stack surfaced a commit I had not made —
`cd703fd`, authored in FOUNDATIONS the same day, touching OUR mailbox.

**Read before reporting** (L0043, written one turn earlier): opened
`notice-held-7-pinned.md` in full rather than inferring from the commit
message.

## What came back

**Ruling accepted: Aug-11 governs. Ball: nobody, thread closed both sides.**

And the part that matters more than the ruling: they took the measurement I
volunteered as an aside — *"in case it is useful to your note.h header"* — and
found it **corrected their own rationale**. Their header justified drop-oldest
with our Aug-11 sentence, *drop-newest means a pressed key makes no sound*.
Measured, that overstates it: the sounding note lives outside the stack, so
overflow-by-ONE self-corrects; the lasting defect needs TWO. In their words,
*"the ruling was right and the reason we recorded for it was not."*

It also exposed a gap in their suite — case 6 overflowed only once — so
**R-held-7** now pins the double-overflow case, checked non-redundant against a
mutant that handles the first overflow and refuses every later one (passes
R-held-1..6, fails only R-held-7).

## What that hands us, and why it is not free

They offer R-held-4/5/7 as adapter-driven and *"yours to run today"* — pass
them and ADR-126's open test row closes with nothing built. Vendored headers
refreshed to `cd703fd`; R-held-1..7 now present locally.

**But `conformance_check` runs 8 cases and zero R-held-\*.** Our held stack is
inline state in `hypersaw_clap.cpp` (`heldStack[16]` + `heldCount`), not a type
an adapter can drive. Two routes, one legitimate: extract the shipped stack into
a small header so the adapter drives the REAL code, or reimplement its semantics
in the adapter — **the second is forbidden by L0031** (a test that rebuilds the
mechanism it checks spans the wrong layer, and would stay green while the
shipped stack diverged). ADR-126 also does not yet *report* the dropped key,
which R-held-4 requires. Filed as **B53**, deliberately not started at the tail
of a long session in oracle-sensitive mono code.

**Still ours and explicitly not covered:** the steal-priority oracle. They say
so plainly rather than letting it look covered.

## The red gate, and why "revert" is no longer an option

`mailbox_delivery` remains RED: the filing is committed in FOUNDATIONS' tree
and absent from their `origin/main`. The stop-gate offers *fix or revert*.
**Revert is now foreclosed** — they have already consumed the filing, rewritten
`note.h` from it, and built R-held-7 on it. Reverting would orphan landed work.
Pushes to sibling repos are the human's, so the single resolution is
`git -C ../FOUNDATIONS push origin main`. B52 updated to record that the
revert branch is closed, so the red is not mistaken for indecision.
