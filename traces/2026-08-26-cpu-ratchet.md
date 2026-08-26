# Trace — "some sort of memory leak" measured: tails, not memory (B47)

**Trigger** human 2026-08-26: oversample ruled out (off in their session);
*"I wonder if there's some sort of memory leak; it happens more the longer I
play, and moreso when I morph partway between fairly complex patches."*

**What changed.** `tools/ratchet_probe.cpp` (scratch diagnostic, CMake-wired,
NOT a gate), ROADMAP B47 updated to resolved-in-mechanism. **No DSP or GUI
source touched; nothing to install.**

**Method.** Real plugin, seeded 6-notes/s stream, cost per 5s bucket (wall/real
ratio), occupancy via the FOUNDATIONS test hooks, RSS per bucket. Phases:
silence control → 60s heavy play → 24s silence (recovery discriminator) → the
same at voiceCull −40 → real-morph phases with corners authored through the
shipped arm path (param 159) and verified by readback.

**Findings.**
1. **Not a leak.** RSS flat (~9 MB) across 3+ minutes of play, churn, silence.
2. **Tails, proven by the recovery curve.** After 60s of heavy playing
   (n=16, round on, release 2s), cost holds at the full playing price (~8.8%)
   through **18s of total silence**, then collapses to 0.05% at t≈21s —
   exactly the 9.2·τ = 18.4s the −80dB cull predicts. A leak would not
   recover; tails do, on schedule. Phrase gaps shorter than 18s mean the tail
   set never empties — "more the longer I play", verbatim.
3. **The remedy already shipped**: B38's voiceCull. At −40dB the same run
   recovers by t≈12s, identical playing cost.
4. **No morph-churn premium.** Pinned-at-heavy 9.0%, partway 7.4–9.0% — cost
   tracks ~max(corners), not blend position (pick-mode keeps the heavy
   corner's character intermittently live). Halfway ≠ half, which reads as
   "partway is expensive".
5. Robustness: stepped n flipped 7↔16 every 5.8ms grid tick under load for
   30s — no misbehavior.

**Probe pedigree — three wrong versions, each caught by its own control.**
(a) OFFs with key 0 match nothing (CLAP wildcard is −1): 16 "stuck" voices
were the probe's own unreleased notes. (b) Phase boundaries stranded in-flight
OFFs: 7 more fake-stuck voices. (c) Corners authored before `morphOn=1` are
silently discarded (`morphRouteEdit` no-ops at `hypersaw_clap.cpp:1417`) —
the authoring-readback control caught n=7 where 16 was written. Every phase-2
number from runs 1–2 was meaningless; only run 3 is reported.

**Verify.** `./verify fast` exit 0. Parity untouched by construction (no DSP).
