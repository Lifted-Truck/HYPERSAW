# 2026-07-23 — Sixth fold: harmonic detune law (law 4) + `harmReach` (ADR-065)

First of the map's NEW DETUNE LAWS; the supersets are done. The law the human
reverse-engineered from the NI demo: at full spread the voices walk down the
harmonic series, so the spread pattern IS the series.
Off fresh main after #77 merged (gh pr view #77 = MERGED first — L0004 reflex).

## Changes (reference-first, ADR-003)
- swarmsaw.html: `harmReach:1` param; `else if (p.law === 4)` →
  `f = f0·(1 + detune·harmReach·i)`, placed ahead of the ERB branch.
  Ignores dist/anchor by design — the geometry is the integer series.
- swarm_core.h: mirrored (`double harmReach = 1`, same branch, paramSlot).
  NAMED `harmReach`, NOT `reach` — the core's `reach = 5` is already the
  RING-topology neighbourhood radius; a literal transcription of the lab's
  label would have been a duplicate member with a clashing meaning.
- gen_goldens.mjs: harm-series, harm-partial, harm-reach. A coupled harmonic
  scenario is deliberately ABSENT, with a comment saying why (see below).
- trajectory_check.cpp: ADR-065 behavioural anchor for the coupled path.
- DECISIONS.md: ADR-065.

## Evidence
- Geometry verified: detune 1 → 1,2,3,4,5,6,7 × f0 (exact series);
  harmReach 1.833 with n=7 → top voice at H12.
- ./verify full EXIT 0: parity 84/84 → **93/93** within ε, the 9 new at
  **rms 0.000e+00**; all 8 chains + notefuzz GREEN.
- Anchor: bounded/NaN-clean got=0.355; coupling entrains got=**0.462** (gate 0.2).
- git: on branch fold-harmonic-law (off fresh main).

## The finding: a domain limit on bit-parity, not a porting bug
A fourth scenario (law 4, full spread, K 0.6) failed parity at **rms 9.736e-02**.
Diagnosed rather than assumed:
- Bracketed — divergence scales with coupling × spread:
  3.7e-19 → 4.4e-13 → 3.5e-07 → 9.7e-02.
- Falsified directly — perturbing the **JS reference alone by 1 ULP** and
  re-rendering gave **rms 9.383e-02**: JS-vs-itself diverges as far as C++-vs-JS,
  and it GROWS (5.62e-2 over 0–0.5 s → 1.147e-1 over 1–2 s). Stable regimes under
  the same probe: 1.869e-19 and exactly 0.

So the regime has a positive Lyapunov exponent and `Math.sin`/`std::sin` are not
identically rounded — sample-exact parity there is impossible **in principle**.
The golden was REMOVED (not kept red, not given a loosened ε — never weaken a
gate) and replaced by the behavioural anchor.

Anchor tuning is itself evidenced: K 0.6 lifts late-window R by only **0.006** on
an octave-plus stack — indistinguishable from chaotic jitter, an anchor that
could flip sign on any legitimate refactor. K 0.9 is the first value that
actually entrains this geometry (R 0.24 → 0.70), so the check runs where the
claim is true, with margin.

## Open — needs a human call
ACCEPTANCE.md (protected) states the bit-parity contract (L0-1, ε=1e-6 RMS)
without a domain limit. It likely needs an amendment recording that parity is a
valid oracle only in non-chaotic regimes, and that chaotic regimes are covered
behaviorally. ADR-065 records the limit; the protected file is untouched.

## Next
Remaining laws: **stretch** (must be RENUMBERED — the lab's law 3 collides with
the core's tempo-grid law 3), golden distribution, octave spread + root-anchor.
Then the divergences (root-pivot topology, pan default image, saw-shape
retarget). CLAP params batched last. Master HPF deliberately NOT folded —
deferred to a proper filter module.
