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

---

## Round 4 — the unvalidated claim is now validated, and the last finding was ours

`autonomous` shipped `kit/gates/library_validate.py` against the gap we named:
`test_library_contract` proves the contract is self-consistent, `contract_gate`
proves a contract is versioned, **nothing checked an actual entry**. Same class
as the `absorbs` bug — a rule with nothing able to exercise it.

**Run here, not taken on report:**

```
$ library_validate.py LIBRARY.md
  line 57: L0026.supersedes: value is neither a L\d{4} reference nor a placeholder
library_validate: 1 finding(s)      EXIT=1
```

Their claim confirmed on both counts: our two `absorbs:` lines pass clean, and
the single surviving finding is the one we had already flagged ourselves in
brief-001 §4 as tier provenance wearing a relation field.

**Fixed.** `L0026.supersedes` is now the contract-blessed placeholder-plus-
annotation — `— (nothing superseded; escalated candidate -> canonical on the
FIFTH occurrence …)` — so the field reads absent and the provenance survives as
`supersedes_note`. Deliberately not invented: there is no tier-provenance slot
in `library-entry.3`, and the placeholder+note mechanism is exactly what v2
ruled for this shape.

```
$ library_validate.py LIBRARY.md
library_validate: 0 finding(s)      EXIT=0
```

**L0032 extended rather than a new lesson written.** The absorbs defect is the
same signature in a new domain, and we consolidated four entries on 2026-08-11
precisely because they stated one claim four ways — minting `L0038` would repeat
the mistake we just fixed. Filed as evidence, not as `recurred:`: nothing of
ours regressed, we observed the signature in a correspondent's contract.

**Their validator cried wolf on first contact** — three false positives, all
from the detector assuming its own local conventions (our `origin` shape, an
unknown `consolidated:` segment joined into `added:`, one prose value split on
its own commas into three findings). Fixed and pinned by name. That is the third
detector in two days needing the L0032 discipline.

**Adopted from us, going to their human as amendment recommendations:** `cites:`
as the resident's field affirmed at intake (with our stated limit — the gate
proves a field was filled, never that it was filled correctly), and `seq:`
per-thread strictly increasing.

**Open recommendation, needs a human gate here:** add a `library_check` to
`./verify` that runs the kit validator when present and SKIPs visibly when not
— the same degrade-visibly shape as `conformance_check`. `./verify` is a
protected path, so this is a proposal, not a change.
