# 2026-07-18 — Packet UPDATE-001 ingested (Track E); viz cards reflow

**Packet triage (L0001 drill).** Design-session packet UPDATE-001: three
new reference implementations (swarmfilter/swarmphaser/swarmtime —
FilterLab/PhaserLab/TimeLab; banner markers verified, extraction-ready;
sweep clean) + doc deltas. Placement: HTMLs at root (protected class,
verify structure gate + CLAUDE.md updated); SPEC-EFFECTS.md at root with a
provenance/renumber note; DECISIONS gained ADR-029..032 (packet 013-016
renumbered per standing policy; packet 010-012 matched local 015-017
verbatim and were skipped — exactly as the packet's own idempotency note
anticipated); ACCEPTANCE gained L0-14..21 (Track E criteria; the planned
Daido formalization moves to L0-22+, ROADMAP updated); ROADMAP gained
Track E with the local sequencing ruling (E0 = UNIFY with SwarmCore's
existing force implementation, per the packet's own note). UPDATE-BRIEF
archived to docs/change-notes/; files.zip deleted per instruction. The
original prototypes' UI-only focus patch was NOT included in the packet
(mentioned as optional refresh; not parity-relevant — our copies stand).

**ADR-032's "laws for the VST" applied to our GUI now:** range inputs blur
on pointerup, selects blur on change, Escape = blur (+ hzPanic when the
shell binding lands — blur behavior live already).

**Viz cards (human request).** Phase circle+meters, carpet, spectrum, and
XY each in their own card in an auto-fit grid (align-start): wide windows
put them in a row, narrow stacks them. Browser-verified at 1500×620
(4-across) and 760×560 (stacked reflow).

**Evidence.** verify full green (51/51 · trajectories · state 8/8);
pluginval 10 SUCCESS; auval SUCCEEDED; seals OK; installed; sweep clean.
