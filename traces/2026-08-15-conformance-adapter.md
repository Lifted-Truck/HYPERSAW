# Trace — FOUNDATIONS note-lifecycle conformance, run against our bookkeeping

**Date:** 2026-08-15 · **Organ:** lead · **Branch:** `conformance-adapter`

## What changed

- **NEW** `tools/conformance_check.cpp` — adapter over our shell's note
  bookkeeping + our own END ledger.
- `src/hypersaw_clap.cpp` / `src/hypersaw_clap_entry.h` — three test hooks
  (`hypersaw_test_tag_at`, `hypersaw_test_retire_slot`, `hypersaw_test_slot_gated`)
  plus `hypersaw_test_poly`. Read-only windows + one shipped mutator. **51
  insertions, 0 deletions** — no product behaviour touched.
- `CMakeLists.txt` — `conformance_check` target, guarded on the vendored header
  existing, so it simply does not exist on CI.
- `verify` — additive gate with an explicit SKIP branch.
- `.gitignore` + `libs/vendor/foundations/` (untracked) — their headers, pinned.

## Why the dispatch came back to the lead

First attempt was a Sonnet implementer in the fleet. It **stalled during
reconnaissance** (watchdog, 600 s, no progress) with nothing written — its last
step was reading `hypersaw_clap.cpp`, a 2500-line file, to locate note
bookkeeping spread across four regions. **The brief pointed it at the right file
and made it pay the whole recon cost.** That is a lead error of the same family
as round 1's: the brief bounded attention correctly but budgeted it wrongly. A
brief that hands over pre-digested line ranges is the fix, not a bigger model.

## Result

```
ok     R-steal-2  ok  R-end-1  ok  R-end-2  ok  R-ident-1  ok  R-ident-2  ok  R-retrig-2
FAIL   R-steal-1: released-before-gated
FAIL   R-retrig-1: same-key retrigger
ok     LEDGER (ours): every identity issued was ENDed exactly once — 38 note-ons, 38 ENDs
conformance_check: GREEN — suite 6 passed / 2 failed (2 expected-red, pinned), ledger GREEN
```

`./verify full` GREEN, exit 0. **parity 147/147, worst 4.262e-09 @
dyn-ring.seed42 — unchanged**, so no product behaviour moved.

## Two reds diagnosed, and BOTH first-draft diagnoses were wrong

This is the part worth keeping. The first run was **3 red plus a red ledger with
15 leaked identities**, which reads like a serious lifecycle bug. It was not.

**Adapter artifact 1 — the orphan I manufactured.** My `end()` called the
shipped `retireTag()` bare. The shell *never* does that: it calls `retireTag`
only immediately before overwriting the tag with a new note. Calling it alone
retires an identity while its voice is still GATED — a sounding voice with no
identity, which is the mono-poison condition `retireTag` exists to prevent. **I
built a state the product cannot reach and then measured it.** Fixing `end()` to
gate the voice off first took R-end-1 green and the ledger from 15 leaks to
38/38 exact.

**Suite artifact 2 — a side-effecting call inside a short-circuit.** Their Case 2
is `ok = ok && owed.take(a.end(hs[i]))`. Once `ok` is false, **`a.end()` is never
evaluated** — the suite stops driving the adapter mid-case and leaves state that
poisons what follows. That is what produced the 15 "leaks": 15 `end()` calls that
never happened. Reported to them.

**The remaining two reds share one root, and it is a model divergence, not a
defect.** We retire an identity at **gate-off** (`hypersaw_clap.cpp:905`, the
2026-07-31 END-at-release redesign — emitting at env death made hosts wait on an
invisible ~1.1 s tail). Their cases assume retirement at `end()`/steal. So:
R-steal-1 sees no steal because the released note's END was already delivered a
block earlier; R-retrig-1 finds the retrigger reusing the first instance's slot
with its identity already retired. **The ruling — "an identity may only be
discarded through a path that delivers its END" — holds: 38 issued, 38 ENDed,
exactly once.** We fail the encoding, not the rule.

**Not fully explained, stated as such:** *why* the retrigger lands in the
released instance's slot rather than a free one, given tier 1 checks free slots
first. Mechanism confirmed by trace; cause not established. Not claimed as
correct or incorrect.

## Calibration (required — a suite that never rejected anything is not a gate)

Planted the panic bug's exact shape: `retireTag` drops the identity instead of
queueing it.

| | ledger | suite failures |
|---|---|---|
| shipped | GREEN, 38/38 | 2 (both pinned) |
| planted | **RED, 38 issued / 20 ENDed, 18 leaks** | **4** |

Reverted; `git diff --stat` confirms the file is back to 51 insertions of hooks
only. Both the ledger and the suite discriminate.

## The gate pins the red set rather than demanding green

Blocking on red would halt work over a divergence nobody has ruled wrong;
ignoring it would be silence. So `conformance_check` pins the *current* set: a
new failure is a regression, and an expected failure turning green means the
record is stale. Both exit non-zero. This is a new non-blocking-by-default gate
with its reason stated, not an existing gate weakened.

## Evidence consulted

`libs/vendor/foundations/note_conformance.h` (295 lines, pinned e9058de);
`src/hypersaw_clap.cpp:845` (tags), `:863` (retireTag), `:879`/`:905`
(emitNoteEnds, END-at-release + re-press guard), `:1636` (poly note-on);
`src/swarm_core.h:1137` (ADR-083 three-tier steal).
