# 2026-08-19 — ADR-096: the note lane gets all five travel laws

**What changed.** `GlideCore` replaced the hard-wired one-pole at
`swarm_core.h:1238`, so played notes travel by the same five laws as the wheel.
Ids 137-145 (`noteLawLink`, `noteLaw`, `noteTime`, `noteRate`, `noteSpringF`,
`noteDamp`, `noteDistOver`, `noteQuant`, `noteHyst`) mirror the bend block minus
`retMul`. `bendTau` widened 1-400 -> 1-2000 ms. The Bend group moved MAIN -> OSC,
which is what the human asked for ("the bend law needs to replace the glide logic
in the voices section of the osc page").

**Why id 33 survived the merge.** The human proposed widening `bendTau` and
merging the pair. The obvious survivor is `bendTau` — and it is the wrong one.
`docs/presets/serum-parity-reference.json` stores `"glide":0.89` in SECONDS;
`bendTau` is MILLIseconds. Retiring id 33 into it reads 0.89 as 0.89 ms, which is
not slow portamento but none. So id 33 keeps id, key, unit and range, the core
converts at the use site, and the widening serves the linked case instead.
Evidence consulted: `docs/presets/serum-parity-reference.json` (the stored value),
`git log -S bendTau` (bbdfd0c, hours old, so widening it disturbs nothing).

**Why the gate being green is not the claim.** `trajectory_check` passed the
domain change untouched, which is the comfortable result. Its criterion is
"reaches target within 1c in **12 tau**" — tau-relative, so a one-pole in Hz and
a one-pole in semitones satisfy it identically. A plant (`lp.tau = p.glide *
10000.0`) was built and run: `FAIL ADR-026 glide reaches target within 1c (12
tau) got=-342.475`, `trajectory_check: RED (1 failure)`. Plant removed, GREEN
restored. So the gate demonstrably covers the lane's TIMING, and demonstrably
covers nothing else. Curve shape is an ear ruling, filed as NTR-3, open.

**A gate that passed while the C++ did not compile.** `./verify fast` returned
EXIT=0 with `swarm_core.h:272` failing to build — fast runs the structural gates
only and never invokes the compiler. Worth remembering before reading a fast
green as "it builds".

**Verify.** `./verify full` EXIT=0. `parity_check: 156/156 within eps=1e-06
(worst 4.262e-09 @ dyn-ring.seed42)` — unmoved, as it must be: the JS reference
has no glide, so parity is structurally blind here rather than reassuring.
`trajectory_check: GREEN (0 failures)`, `state_check: GREEN`, `notefuzz_check:
GREEN (0 hangs)`.

**Also landed.** `shown_when` gained AND across comma-separated clauses
(`noteLawLink=0,noteLaw=3`). With one key, gating on the link showed seven law
controls at once and gating on the law showed them while the lane was following —
dead controls either way. Same missing capability the chord layer needs for
OR-across-keys.
