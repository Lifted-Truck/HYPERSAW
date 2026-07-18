# 2026-07-18 — Voice mode (mono/glide/legato), super-width, octave; PR #13 unblocked

**Merge fix.** PR #12 confirmed merged on retry; #13's DECISIONS conflict
resolved (ADR-023 ordered before ADR-024) — MERGEABLE again.

**Super-width (ADR-025).** Width extends to 1.5: mid/side side-boost (≤2×)
pre-softclip, only reachable >1; pan-law contribution clamped at 1 (anti-
phase guard). Side energy ~4× at 1.5 vs 1.0; bounded.

**Voice mode (ADR-026).** Core: retargetNote + exponential glide (τ=glide,
per-tick coef per ADR-009, f0cur moved multiplicatively for gravity
compat), armed only by the shell — poly path untouched. Shell: mono
last-note-priority with 16-deep held stack and return-on-release; legato
toggles phase/env continuity; octave = shell transpose. GUI Voice cluster.
Param 15 renamed "Mono Fold". Checks: glide settles <1c at 12τ (first
check draft was impatient — 5τ leaves ~7c of an octave, expectation bug
not glide bug), legato env continuity, non-legato glide-from-current,
width side-energy. Octave reading chosen: TRANSPOSE (the other reading —
sub-octave layer for bigness — offered to the human as an alternative).

**Evidence.** verify full green: parity 51/51 (defaults inert), all
trajectories incl. new ADR-025/026 block, state_check 8/8 (new params
round-trip). pluginval 10 SUCCESS · auval SUCCEEDED · seals OK · installed.
