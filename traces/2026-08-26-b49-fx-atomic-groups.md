# Trace — B49/ADR-124: FX slots morph as one thing

**Trigger** human 2026-08-26: *"The same rule should probably apply to FX
modules, but we're already changing how they work so maybe we should start
moving on that soon."*

**What changed.** `src/hypersaw_clap.cpp`: `morphScaleFirst/Last` (one
hardcoded range) → `morphLead[]`, an explicit index→lead map, identity except
where a group declares otherwise; `morphGroupRange()` for the exempt-as-a-unit
path, which now tolerates non-contiguous groups. Four new groups: each FX
slot's type + amount + tone. ROADMAP B49; DECISIONS ADR-124; tests B49-1/B49-2;
**B33's stale FOUNDATIONS gate sentence corrected** (see below).

**Evidence.**
- Chimera probe (real plugin, corner A = slot1 Drive @ 0.90, corner B = slot1
  Gain @ 0.10, sweep X): **3 of 9 → 0 of 9** positions holding a state no
  corner authored. The bad middle third read `type=Drive, amount=0.10` —
  near-passthrough, so the drive character vanished mid-blend.
- Parity **156/156** within ε=1e-6 (worst 4.262e-09).
- `./verify fast` exit 0.

**An honest gap in my own verification.** The scale-group regression probe
never authored its corners — both pure corners read back the *default* major
scale, so the sweep compared defaults to defaults. It therefore proves
**non-regression** (pre-change and post-change builds give byte-identical
output, A/B'd by stashing the diff and rebuilding) but does **not** prove the
scale group is still atomic. Recorded as inconclusive rather than counted as a
pass. What would settle it: a test hook exposing `morphGroupLead`, or finding
why armed-corner writes to ids 116–128 do not store.

**Stale-truth correction found by mailbox recon.** B33 read *"the design brief
to FOUNDATIONS is the gate before building"*. That gate lifted **2026-08-11**:
the signal-graph brief was filed 08-09, answered the same day (*"ratify what
HYPERSAW needs"*), and ratified locally (`ratify-signal-graph.md`, ADR-088),
with `routing_core.h` + `routing_check` shipped at 7 green invariants; their
OQ-23/OQ-30 rulings have since landed, lapsing R3's deferral of
morph-writes-topology. ROADMAP is the single source of truth and was
contradicting itself (B33 vs B49) — corrected. **I had also been reporting
this brief as unfiled in session roundups for several turns; that was wrong,
and it was wrong in exactly the way LIBRARY L0037 warns about — I trusted my
own carried summary instead of reading the tree.**

**Not done, deliberately.** The ramp itself (scale `mix` by an on-weight, the
ADR-123 shape, using `fx_rack.h:272`'s guaranteed bypass as the zero) and the
type-swap ruling (dip-through-zero vs two crossfading instances) are open in
B49 — the second is a human ruling, and the first is cheap enough to fold into
the FX rework rather than bolt on ahead of it.
