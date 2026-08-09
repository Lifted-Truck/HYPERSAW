# FOUNDATIONS notice review — and a bug found while answering it

**Date:** 2026-08-09
**Trigger:** human — *"go ahead and review its notices"*.
**Verify:** `./verify fast` GREEN. Gravity finding measured with an isolated
probe (below); no source changed.

## The two notices

**`notice-intake`** — registers the channel, states F2's shape, explicitly owes
nothing: *"Ball: consumer — passive. Nothing owed. No acknowledgement
required."* No reply needed. Its three "what we will read at F2" items are
directions for `INTEGRATION-STANDBY.md`, and are now reflected there.

**`notice-f2-parity-corpus`** — one narrow ask, and a good one: which patches
must be in the corpus, what makes bit-identity legitimately fragile, and where
the parity boundary sits. Reply drafted at
`docs/integrations/DRAFT-foundations-response-parity-corpus.md`. **Not filed** —
FOUNDATIONS has a session live and the standby regime holds; the notice itself
offers the standby doc as an acceptable location, which is where the summary
went.

## The finding

Answering "what makes bit-identity fragile" turned up a real one.
`SwarmCore::render()` calls `gravityStep(frames / sr)` — gravity integrates once
per render call at dt = block length, explicit Euler on a nonlinear ODE. So the
output depends on how a block is SUBDIVIDED, not just on its total length.

| gravity | whole call vs 256-chunk | vs 333-frame |
|---|---|---|
| 0.00 | 0 | 0 |
| 0.50 | 1.028 | 1.029 |

Gravity off, bit-identical under any subdivision. Gravity on, a different sound.
Consequences and the proposed fixed-grid fix are in ROADMAP; the fix is
parity-affecting and touches the protected JS reference, so it is gated.

## Four probes, three of them wrong — and why that matters

The core-level measurement above was clean on the first try. The **plugin-level**
attempt to show oscillator 0 and oscillator 1 diverging was wrong three times,
and each failure was the same species:

1. Soloed osc 1, captured, then soloed osc 2 and captured — **70 blocks later**.
   Comparing two moments of an evolving swarm, not two oscillators. Caught by
   the `grav=0` control "failing" by 0.84 with nothing to fail about.
2. Rendered each from a fresh `plug_reset` — residual 0.0395 at `grav=0`.
   Suspected the solo gain's one-pole state surviving reset.
3. Silenced by volume instead of solo to rule that out — **byte-identical
   numbers**, so it was not the gain smoothing.
4. Actual cause: `plug_reset` clears gates but **not core phase state**, and both
   cores render every block regardless of volume. The silent oscillator had
   already advanced through the first render before being measured.

The `grav=0` control is what made all of this visible. Without a case that
*must* read zero, the 0.169 at `grav=0.6` would have looked like a clean
confirmation of exactly the bug I was expecting to find. That is the shape of a
false accusation that survives review: a plausible number, in the predicted
direction, from a confounded rig.

So the reply and the ROADMAP entry state consequence 2 as a **prediction from a
proven mechanism**, not a measurement. The mechanism is measured; the end-to-end
consequence is not, and saying so is the whole point.

## Evidence consulted

- `src/swarm_core.h:475-521` (`gravityStep`), `:523-537` (`render` entry)
- `src/hypersaw_clap.cpp` mix stage — osc 0 unchunked, osc 1..N `kMixChunk`
- `CMakeLists.txt` — `-O2` only; no fast-math, no `-march=native`
- absence of FTZ/DAZ anywhere in `src/`
- `./verify full` corpus shape: 15 gates, `parity_check` 147/147 worst 4.262e-09
