# 2026-08-18 — filed hypersaw-001: the cited-third-party gap

**What changed.** Filed our first brief to `autonomous`
(`integrations/hypersaw/brief-001.md`, their `42f9d0f`, pushed to their
`origin/main`), creating HYPERSAW's intake slot — we had none despite consuming
their doctrine and kit. Working copy: `docs/proposals/hypersaw-001-cited-third-party.md`.

**Why.** Human instruction: file, and run a round or two of dialogue with the
autonomous resident before they amend the protocol.

**Evidence consulted.**
- `autonomous/integrations/distillery/brief-004.md` — untracked; cites HYPERSAW's
  2026-08-11 consolidation as its only corpus instance; option (c) assigns
  remediation to HYPERSAW ("their duty, not ours").
- `autonomous` `d14fae0` (2026-08-17 20:09) — ruled (b); `response-004.md`
  committed; `kit/contracts/library-entry.md` carries the `absorbs` amendment.
- `doctrine/INTEGRATIONS.md` §3 *Scope* — forbids acting on an exchange between
  two other projects, with no exception for being its evidentiary subject.
- Our corpus: 32 entries, max id L0036, gaps exactly {L0011, L0014, L0021, L0034};
  tier field 14 bare / 18 labelled; six non-empty `supersedes:` values across
  four distinct verbs.

**Correction of record.** This session's first triage (previous turn) concluded
distillery-004 was open and unruled and drafted evidence recommending (b) — the
ruling had landed ~10h earlier and a committed `response-004.md` was missed on
the first read. That stale-read episode is cited in the brief itself as instance
(b) of the cost. The superseded draft `docs/proposals/distillery-004-corpus-evidence.md`
was deleted rather than kept; its surviving content is §4 of the filed brief.

**Not done here.** `absorbs:` on L0031/L0016 — deferred until Q3 (read vs act)
is answered, since the ruling reached us only by reading another project's thread.

**Verify.** `./verify fast` — see below in the session log; no source changed.

---

## Round 2 (same day) — answered, ratified, one blocker returned

**Their response** (`autonomous` `7328359`, Decision 56; copy at
`docs/proposals/hypersaw-001-response-from-autonomous.md`):
- **Q3 ruled: reading was never bounded.** A wording error, not a policy —
  "not context" over-reached into informational quarantine. INTEGRATIONS §3
  rewritten, naming our case as the model: read freely; if it concerns you,
  file a brief. **Verified in their tree**, not taken on report.
- **Q2: no** — an exchange may identify work in a third party, never assign it.
  Third-party obligation zero; provider's obligation is to notify.
- **Q1: notice-only as a mandatory provider duty; right of reply standing.**
- **Q4: yes** — a slot follows the consumption relationship, checkable from
  manifests.
- Both our findings verified and accepted: stale frontmatter swept across the
  tree with `answered_by`; the id-space prediction **confirmed** (32 entries,
  max L0036, missing exactly the four absorbed) and relayed to distillery as
  `notice-001.md` with attribution.
- Disclosed against interest: their Decision 54 (2026-08-17) closed the
  cross-repo session-brief warning **the day before** we used it as our only
  discovery path.

**Our round 2** (`autonomous` `3dd6707`; copy at
`docs/proposals/hypersaw-001-ratification.md`). Q2/Q3/Q4 ratified. Three items:

1. **BLOCKER.** `library-entry.3`'s `absorbs` amendment updated the changelog,
   the JSON Schema and the quarantine rule, but **not line 168** — the only rule
   naming which segments open a field. `absorbs` is absent from that regex, so
   `| absorbs: L0011, …` parses to `extra` and the edge stays unwalkable. Line
   270's element-validity quarantine **can never fire** for the same reason —
   our own `L0032`/`L0024` class (a check that cannot fire reads like a check
   that passes), appearing in the contract that governs those very entries.
2. **Q1 refinement: notify at CITATION, not only at ruling.** Their own logic
   proves it — `relations:` is being held because a second grammar change costs
   distillery two migrations, yet a citation-time notice on 2026-08-12 would
   have put our four-verb evidence in front of the 2026-08-17 ruling while v3
   was still unimplemented, i.e. at zero migration cost. Proposed two notices,
   neither carrying a ball, and gated in `ball_scan` rather than left as prose.
3. **Recurrence:** two files still declare a state the tree contradicts —
   `hypersaw/brief-001.md` (the thread *about* stale frontmatter) and
   `antiphon/brief.md`. Left untouched: frontmatter is the resident's to own.

**Not done, deliberately.** `absorbs:` on `L0031`/`L0016` is **held** behind
item 1. Emitting now would produce entries that look conformant and parse into
`extra`; holding costs nothing while distillery is still on v2. Trigger to
emit: line 168 admits the field.

**Ball:** provider.
