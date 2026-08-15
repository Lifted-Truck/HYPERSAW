# Dispatch brief — FOUNDATIONS note-conformance adapter

**Provenance.** Authored by HYPERSAW's lead organ, 2026-08-15, for a scoped
subagent with zero conversation history. Motivating decision: FOUNDATIONS'
`notice-note-conformance` (their R5 / DECISIONS #80) asks us to run their
14-case suite against our own note bookkeeping; the human ruled 2026-08-15 that
their headers are vendored **untracked** because this repo is public and theirs
is private. Recorded in ROADMAP under "FOUNDATIONS — fleet protocol acked;
conformance run BLOCKED on repo visibility".

**This brief is committed deliberately.** The lead filed a commitment to
FOUNDATIONS (`ack-fleet-protocol`, counter 1) that every round-2 dispatch brief
would be readable in our tree, because our contamination path is top-down — the
lead reads their tree and its framing can enter a brief. This file is that
audit trail. Nothing in it is derived from any FOUNDATIONS path outside
`integrations/hypersaw/` and the two vendored headers.

---

## Files in scope

- **CREATE** `tools/conformance_check.cpp` — the adapter + a `main()`.
- **EDIT** `src/hypersaw_clap.cpp` — test hooks only, in the existing
  `hypersaw_test_*` block near the bottom (see `hypersaw_test_dump_forensics`
  for the established pattern). Declare them in `src/hypersaw_clap_entry.h`.
- **EDIT** `CMakeLists.txt` — add the `conformance_check` target, guarded so it
  is only defined when `libs/vendor/foundations/note_conformance.h` exists.

**Explicitly OUT of scope.** `ROADMAP.md` (lead is the only writer). `verify`
(the lead wires the gate). `libs/vendor/foundations/**` (their code — read it,
never edit it). Any change to DSP, to `swarm_core.h`'s steal policy, or to the
shell's note behaviour. **If a conformance case fails, DO NOT change our
behaviour to make it pass** — a red is a finding to report, and changing the
instrument to satisfy an external suite is a decision neither of us owns.

## What you are building

Their suite (`libs/vendor/foundations/note_conformance.h`, read its header
comment — it is the contract) drives an adapter *you* write over *our* shipped
bookkeeping. Duck-typed:

```
static constexpr std::size_t kMaxNotes;    // our kPoly = 16
using Handle = ...;
struct OnResult { Handle handle; bool stole; foundations::NoteRef stolen; };
OnResult noteOn(const foundations::NoteRef&);
std::size_t release(const foundations::NoteRef&);   // wildcard query, returns count
foundations::NoteRef end(Handle);                   // returns the identity retired
bool live(Handle) const;
```

Then `main()` calls `foundations::runNoteConformance(adapter)` and returns
non-zero on any failure. The mono half (`runHeldStackConformance` over our
`heldStack`/`heldCount`) is **optional** — do it only if the poly half is
finished and clean.

## What our bookkeeping actually looks like (read these before designing)

- `src/hypersaw_clap.cpp:845` — `struct NoteTag { int32_t noteId; int16_t port,
  channel, key; bool active; }`, `tags[kPoly]`. **Identity only.**
- Same file, `retireTag(int slot)` — queues an overwritten identity into
  `pendingEnds[]`; `emitNoteEnds` flushes it each block. This is the closest
  thing we ship to the suite's `end()`.
- `src/swarm_core.h:1137` — `alloc()`, the **three-tier steal policy** (ADR-083).
  Tier 1 free-oldest, tier 2 releasing-**quietest**-first, tier 3 oldest gated.

**The structural fact that will shape your design:** identity lives in the
shell, steal policy lives in the engine (it reads envelope state the shell
cannot see). So `OnResult::{stole, stolen}` spans **both** — you cannot
implement the adapter over `tags[]` alone. Do not "solve" this by
reimplementing the steal rule in the adapter; that would be an oracle testing
its own copy of the thing under test, which this project has been burned by
(see `LIBRARY.md` on detectors sharing the assumption they are checking).

## Acceptance criteria

1. `conformance_check` builds and runs; report on stdout; exit non-zero iff any
   case failed.
2. It drives the **real plugin** through the **real** note path — the same
   discipline `trace_check` follows (its header explains why a test that
   reimplements the mechanism it checks proves nothing).
3. **Calibration is required, and it is the part that makes this real.** Plant a
   defect, show the suite catches it, remove it. At minimum: make `retireTag`
   drop the identity instead of queueing it, and show `R-end-1` goes red. Record
   the before/after in your trace. A suite that has never rejected anything on
   our tables is not evidence.
4. `./verify full` still GREEN, **parity unchanged at 147/147, worst
   4.262e-09** — if that number moves, you changed behaviour; stop and report.
5. A trace in `traces/` per the provenance skill.

## What to report back

The colour of every case, verbatim. **A red is a valuable result, not a
failure of yours** — the lead has already predicted, in writing to FOUNDATIONS,
that `R-retrig-*` is our most likely red (same-key retrigger is our admitted
non-pin). If it goes red, report it and stop; do not fix it.

Also report, explicitly, **anything the adapter forced you to assume** about our
note semantics that the code does not state. Round 1's lesson here was that a
brief bounds a subagent's attention and the lead owns the questions — so if you
notice something outside this brief that looks wrong, say so in the trace rather
than treating silence as "no open questions".

## Verify target

`./verify fast` while iterating; `./verify full` before reporting done.
