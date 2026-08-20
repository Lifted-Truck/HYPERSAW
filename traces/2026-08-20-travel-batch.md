# 2026-08-20 — two more travel bugs, and the link ships FOLLOW

**Reported.** Glide works now, but: mono-with-legato-off doesn't glide on the
re-strike; lag mode doesn't work; the link should default to follow; poly glide
is redundant; the quantise visualiser is stuck one selection behind.

**Both bugs were the same shape as the last one — a `p.glide > 0` gate left
behind by ADR-096.**

1. `retargetNote`'s non-legato branch restored the departure pitch only when
   `p.glide > 0`. `initVoice` has already moved f0 TO the target, so for any law
   carrying its own time the glide had zero distance. Measured: const-rate
   re-strike read `441 441 441...` dead flat.
2. The render site overrode `lp.tau = p.glide * 1000` UNCONDITIONALLY, so a lane
   set to "follow bend law" on the LAG law read id 33 (default 0) instead of
   `bendTau` — reading, correctly, as "lag mode doesn't work". Measured: FOLLOW +
   lag 400 ms read `441` flat; now ramps `240 -> 432`.

**Ownership, stated once.** tau is the SHELL's to resolve, because the shell owns
the link. The core no longer converts at the use site. But `setParam("glide")`
still writes `noteLaw.tau` at the CORE level, because that is what ADR-026
defined glide as and trajectory_check drives SwarmCore directly — removing it
turned a 500 ms glide into a 60 ms one (`ADR-026 non-legato ... got=274.806`,
expected ~220). The shell calls setNoteLaw() after setParam, so a following lane
still ends with bendTau. **The gate caught this, not I.**

**Defaults reversed by human ruling (2026-08-20).** `noteLawLink` now ships
FOLLOW — "quite confusing otherwise", and a divergent note law would need its own
visualiser to be legible. This reverses the 2026-08-19 ruling that shipped
own-settings to protect stored `glide`; that compatibility now lives in a state
migration: a patch naming `glide` but not `noteLawLink` predates the lane by
definition and is restored to own+lag. `polyGlide` ships ON (opt-out, not the
switch that enables gliding) and is gated to `voiceMono=0` where it can act.

**The stale visualiser.** Params reach the engine through a queue drained on the
audio thread, so `getBendCurve()` called from the input handler renders the law
as it was BEFORE the edit — one selection behind, forever. It now redraws on the
ECHOED value, which is proof the engine has the new law, guarded on an actual
change because `syncFromEngine` repaints every parameter twice a second.

**Verify.** `./verify full` EXIT=0 · parity 156/156 (worst 4.262e-09) ·
trajectory GREEN · state GREEN · mpe GREEN. Probe: all six travel scenarios ramp.
