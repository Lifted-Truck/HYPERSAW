# integrations/ — HYPERSAW's inbound mailbox

**This directory is where briefs TO HYPERSAW land.** One subdirectory per
correspondent (`integrations/<sender>/`), which is that sender's designated
intake slot and the only path in this tree a visiting agent may write to
(INTEGRATIONS §3, rule zero: writes stay home; the mailbox is the one exception).

Created 2026-08-18 by `/retrofit` at kit 2.1.0. It is empty on creation, and
that is itself the finding it exists to fix: HYPERSAW has been a consumer of
autonomous's doctrine and harness kit, and a named GUI/viz extraction donor for
FOUNDATIONS, **with no inbound slot at all** — every brief naming us has had to
reach us through a correspondent's tree or a side channel. The rule we argued
for in `hypersaw-001` and autonomous accepted is that **a slot should exist when
a consumption relationship exists, not when a problem appears.** This is us
applying it to ourselves.

## What lives here vs. elsewhere

| artifact | lives in |
|---|---|
| a brief filed TO us | `integrations/<sender>/` **here** |
| our response to it | here, beside the brief |
| a brief WE file to someone | **their** tree, in `integrations/hypersaw/` |
| their response to our brief | **their** tree — pull and read; it never arrives here |

That last row is the one that bites. A consumer checking only its own mailbox
cannot distinguish an answered brief from an ignored one — see `CLAUDE.md`
§Mailbox and LIBRARY `L0037`.

## Frontmatter is protocol state

Exchange files carry `id`, `from`, `to`, `status`, `ball`, `filed`, and
`answered_by` once answered. **The resident owns the frontmatter; the body is
the visitor's words and is never edited.** A thread whose answer exists while
its header still reads `ball: provider` is a lie the tree can disprove, and it
cost a full session on 2026-08-18.

## Working copies

Drafts of briefs we file outward live in `docs/proposals/`; the filed copy in
the correspondent's tree is authoritative. Every filing is traced in `traces/`.
