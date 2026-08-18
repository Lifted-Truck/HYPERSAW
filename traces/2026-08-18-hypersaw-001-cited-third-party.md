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
