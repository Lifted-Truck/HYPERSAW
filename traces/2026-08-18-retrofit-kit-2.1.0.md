# 2026-08-18 — /retrofit: pre-2.0.0 → 2.1.0

**What changed.** `CLAUDE.md` gained a marker-delimited `## Mailbox` section;
`integrations/` created with a README; `.gitattributes` tracked; manifest
declares `"kit_version": "2.1.0"`; ROADMAP entry.

**Before / after** — the deterministic delta, which is the whole plan:

```
kit: 2.1.0   declared: pre-2.0.0   BEHIND by 2 entries
  → 2.0.0 (2026-08-17) baseline                    [x] × 11
  → 2.1.0 (2026-08-17) mailbox scope rule in every charter

kit: 2.1.0   declared: 2.1.0   CURRENT
  nothing to do — re-running the retrofit is a no-op
```

**Finding the checker structurally could not make.** Every 2.0.0 item scored
`[x]`, including `.gitattributes (LF)` — which was **untracked**. `currency.py`
reads the filesystem, so the file was present for it and absent for every clone
and for CI. Decision 34 calls that LF pin load-bearing for hash ledgers, golden
renders and byte-identical replay, so the guarantee existed on one laptop and
nowhere else. Not a defect in the checker — presence on disk is what it claims
to measure — but the third time today that a check answered a neighbouring
question reassuringly.

**Finding 2.1.0 forced.** HYPERSAW had **no `integrations/` at all** while being
a consumer of autonomous's doctrine and kit and a named extraction donor for
FOUNDATIONS. Exactly the `hypersaw-001` Q4 finding, applied to ourselves.

**Deliberate divergence, human-ruled.** The 2.1.0 retrofit action instructs the
charter to say *"exchanges between other repos are ignored."* INTEGRATIONS §3 was
corrected 2026-08-18 on this repo's own brief: reading is never bounded, only
acting or escalating is. Writing the action verbatim would have installed a rule
superseded that morning. Charter states the corrected form and marks the
divergence in place.

**Filed to autonomous** (`integrations/hypersaw/note-001.md`): the superseded
wording survives in two propagating places — the CHANGELOG's 2.1.0 retrofit
action, and autonomous's own `CLAUDE.md`, which still reads *"not a to-do, not a
warning, not context"*, the phrase §3 now says over-reached into informational
quarantine. Every repo retrofitted to 2.1.0 after us installs the old rule until
that text changes.

**Evidence.** `currency.py` before/after as quoted; `./verify fast` EXIT=0;
`library_validate` 0 findings (unchanged, LIBRARY untouched).

**Append-only discipline.** Charter section is marker-delimited
(`kit:2.1.0 begin/end`); no existing content rewritten; protected paths,
`./verify` and its gates untouched; the human's untracked `GoopBox.jsx` left
alone.
