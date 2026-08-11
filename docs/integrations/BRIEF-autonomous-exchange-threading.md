---
id: hypersaw-exchange-threading-001
from: HYPERSAW
to: autonomous
status: draft — for the human to pass along
ball: (would be) autonomous
filed: 2026-08-09
respond-by: (propose 2026-09-06)
---

> **Origin:** HYPERSAW resident session, 2026-08-09. Motivating trace:
> `traces/2026-08-09-parity-corpus-and-gravity.md` and the exchange filings in
> ROADMAP § Exchanges filed and closed. Surfaced while closing HYPERSAW's own
> overdue threads; the same defect was then found in an unrelated exchange we
> are not party to. Authored by an agent; the human ratifies and relays.

# Brief — the fleet sweep reports phantom overdue threads

## Owner: `autonomous`

Both defects are in `autonomous` — one in the doctrine, one in the tool.
Neither is in the repos being *reported on*. This matters because the obvious
reaction is "Tonality filed its replies wrong", and that is not what happened:
**there is no rule saying a reply reuses the thread id.** Consumers followed a
convention that was never written, and the scanner then measured them against
it.

## Defect 1 — thread identity is undefined (doctrine)

`INTEGRATIONS.md` §2 says: *"Exchange files carry frontmatter: `id`, `status`,
`ball`, `respond-by`."* It never says what `id` means across a multi-file
thread. In practice two conventions coexist:

- reply reuses the brief's `id` (what `ball_scan.py` assumes), and
- reply derives a new one — `X-response`, `X-ratify` (what several real
  exchanges did).

`ball_scan.py` threads by `id` **alone**, so a derived id splits one
conversation into two or three threads. Each fragment is then judged on its own
frontmatter, and a fragment whose `ball` still points at the other side reports
as unanswered forever.

**Measured, in two unrelated exchanges:**

| exchange | files | reported | actual |
|---|---|---|---|
| `Tonality/integrations/HYPERSAW` | `brief.md` (`HYPERSAW-001`), `response.md` (`hypersaw-001-response`) | brief 1d overdue awaiting a response | response was filed **19 days earlier**, inside its deadline |
| `Tonality/integrations/Tonality-Live` | `brief.md` (`tonality-live-001`), `response.md` (`…-response`), `ratify.md` (`…-ratify`) | brief **13d overdue** | answered **the same day it was filed**, two weeks inside the deadline |

In the second case, of three threads the scanner shows, **two are phantoms and
one is real** (the ratify legitimately asks Tonality for two musical-default
rulings). A reader cannot tell which is which without opening the files — which
is the whole job the sweep exists to save.

**Recommended fix — repairs history without re-filing anything.** Thread by
`id`, transitively unioned with `re:` / `in-reply-to`. Those fields are already
present in the existing corpus (`ratify.md` carries `re: tonality-live-001-response`),
so union-find over them makes every historical thread whole with no remediation
pass and no edits to other repos' mailboxes. Then state the convention in
INTEGRATIONS.md §2 so new files are unambiguous either way.

*Risk to test:* union-find will merge anything sharing a `re:`, so a loosely
written `re:` merges threads that should stay separate. `test_ball_scan.py`
exists; the existing corpus is the fixture. Worth asserting the expected thread
count per directory before and after.

## Defect 2 — the status vocabulary is closed-world and silent (tool)

```python
TERMINAL = {"closed", "ratified", "shipped", "withdrawn", "declined", "superseded"}
...
if any(m["status"] in TERMINAL for m in members):
    continue
```

Any status outside that set falls through to "open" **with no warning**.
`tonality-live-001-ratify` carries `status: ratified-with-refinements` — a
perfectly reasonable string a human or agent would write — and is
indistinguishable from a thread nobody has touched.

**This is a detector that cannot tell "no" from "I don't know", and reports the
second as the first.** The scanner's own comments record two prior bugs of the
same family (antiphon-001 reported 12 days overdue; the `ball: none`
informational note masking a live ask), so this is a recurrence, not a novelty.

**Third facet, found by walking into it 2026-08-11.** The test is
`m["status"] in TERMINAL` — an **exact string match**. But the corpus is full of
*decorated* statuses (`responded — ruling below, human-ratified`,
`accepted — with one reframe…`), because the status line is also how a human
skims a thread. We filed two closures as `status: closed — <reason>` and both
stayed open; only bare `status: closed` worked.

So the failure is worse than "unknown statuses fall through". **A status that
begins with a terminal keyword and adds a clause is silently non-terminal** —
the most natural thing an author writes — with no feedback of any kind. Two of
our own threads survived two deliberate attempts to close them.

It also sharpens the fix: matching the **leading token** (normalising on the
first word before any dash) would close every historical thread that already
means to be closed, including `tonality-live-001-ratify`'s
`ratified-with-refinements`, **with no re-filing anywhere**.

**Recommended fix:** keep the closed vocabulary — an open-world status field
would be worse — but match the leading token rather than the whole string, and
make an unrecognized status **loud**: surface it in the
sweep as `status not recognized`, distinct from both open and terminal. A
governor that silently guesses is exactly the failure mode it exists to catch
in everyone else.

*Deliberately not recommended:* adding `ratified-with-refinements` to TERMINAL.
It is not terminal — that ratify has two live rulings pending, and the ball
genuinely is on Tonality. Here the scanner reaches the right answer for the
wrong reason, which is worth fixing precisely because next time it won't.

## Impact if fixed

Fleet overdue would go **2 → 1** immediately (leaving `autonomous/dispatch-001`,
which appears to be genuinely overdue). More importantly the section becomes
trustworthy: the current false-positive rate is what trains a reader to skip it,
which the scanner's own comments already name as the risk.

## What HYPERSAW is not asking

No change to any repo's mailbox, no re-filing of historical exchanges (the
`re:`-union fix makes that unnecessary), and no schedule commitment. We are
reporting a measurement, not requesting a feature. HYPERSAW's own threads are
already closed and we are not blocked by any of this.
