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

## Addendum — quantise step timing was a FOLD GAP, not a feature request

The human asked for "a timing mode toggle with sync and Hz, and a slider for
each". Before designing one: `docs/design/bend-lab.html` has carried **`qTime`**
since 2026-08-07 (human proposal), a gate that lets a quantised step COMMIT only
once per interval — "what turns the quantiser from a zipper into a glissando
RUN". `glide_core.h` never received it. So this was an unported reference
feature, and the work was a fold, not an invention.

- **Core**: `qTime` ported with the reference's semantics — the law's dynamics
  keep moving, only the EMISSION is gated; the timer resets on COMMIT not on
  attempt; `reset()` arms it open so the first step is never delayed.
- **Goldens**: three gated scenarios (chromatic, scale, spring). The generator
  slices the lab live, so these are the reference's own output.
- **Shell**: ids 146-148 are the musical FACE of the one number the core reads.
  `sync` reuses `kGridSteps` (cycles/beat) rather than minting a division table,
  so its snapping and names are the tempo grid's. The core never learns tempo,
  exactly as it never learned scale names.

**A scenario that could not fail, caught by calibration.** `glide-qtime-chrom`
first used qTime 120 ms against a 6 st/s climb — steps arrive every 167 ms, so a
120 ms gate never bit and the scenario read rms=0 under a planted
gate-ignoring defect while the other two went red. Recalibrated to 400 ms / 12
st/s; all three now fail on the plant (worst rms 0.71) and match at rms=0.
**Calibrate a gate against the defect it exists for, not against the happy path.**

**Stale binary, twice more.** A restore-then-rebuild reported "0 files built" and
kept the RED verdict; deleting the object file forced the recompile. Identical
numbers across a rebuild remain the tell.
