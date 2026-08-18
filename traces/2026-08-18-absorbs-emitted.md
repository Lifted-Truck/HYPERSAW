# 2026-08-18 — `absorbs:` emitted; hypersaw-001 rounds 2–3 closed out

**What changed.** `LIBRARY.md`: `L0031` and `L0016` now carry the contract's
`absorbs:` field instead of the grammar-breaking `supersedes: absorbs …` that
quarantined them. New candidate lesson `L0037` + its INDEX pointer.

**Why now.** The field was unemittable this morning: `library-entry.3`'s
2026-08-17 amendment added `absorbs` to the changelog, the JSON Schema and the
quarantine rule but not to the label-opening rule, so a conforming parser routed
it to `extra`. We reported it (ratification-001); `autonomous` fixed it the same
day (Decision 57) and pinned the class mechanically in
`kit/gates/test_library_contract.py`.

**Emitted form.**
```
[L0016] … | absorbs: L0014 — the spectral case; consolidated 2026-08-11
[L0031] … | absorbs: L0011, L0021, L0034 — shell-path, superset and layer blindness respectively; consolidated 2026-08-11
```
Every element a bare `L\d{4}`; all annotation behind the em-dash so it lands in
`absorbs_note` and nothing can quarantine on a malformed element.

**Verification, stated as what it is.** No v3 parser exists to validate against
— distillery implements the contract and is still on v2, and the kit gate
asserts schema/label-rule agreement, not a LIBRARY file. The two lines were
checked against the grammar as written (every element matches `^L\d{4}$`,
remainder splits at the em-dash). **Spec-conformant, parser-unvalidated.** If
distillery's v3 run disagrees, the fault is ours.

**Evidence consulted.** `autonomous` `e15a0ad` (Decision 57) — `absorbs` now at
`kit/contracts/library-entry.md:181`; `integrations/hypersaw/brief-001.md`
frontmatter corrected; `governor/ball_scan.py:244 frontmatter_lies`;
`kit/gates/test_library_contract.py`.

**Round 3 filed** (`autonomous` `bce6e66`, copy at
`docs/proposals/hypersaw-001-ratification-002.md`): their `cites:` caveat is
right and defeats filer-declares — a visitor runs no gates, so the field is
unenforceable exactly where it matters, and the filer is least motivated to
widen its own thread. But the resident already performs the extraction (it
quoted the citation when ruling); it was simply never written down. Proposed:
**`cites:` is the resident's field, affirmed at INTAKE** — before the thread
leaves `filed`, since affirming at ruling reproduces the late trigger both sides
agreed was wrong. Filer's `cites:` demoted to a hint. Honest limit recorded: the
gate proves the field was filled, never that it was filled correctly.
`seq:` supported, narrowed to per-thread strictly-increasing (no allocator, no
cross-repo coordination).

**Open, not ours to time.** `relations:` as one verb-tagged field — ruled when
distillery reports v3 landed; our framing is the front-runner.
