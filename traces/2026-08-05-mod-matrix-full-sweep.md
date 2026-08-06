# Full mod-matrix sweep — the crash fix was half a fix

**Date:** 2026-08-05
**Trigger:** human, after the chorus crash was root-caused — *"We should probably run a
full deterministic probe of all mod connections to make sure there aren't other similar
issues out there."*
**Verify:** `./verify full` GREEN (exit 0, git 31049a1); worst time parity rms 5.6e-12
against a 1e-6 bar.

## What was built

`tools/labharness/modlab_sweep.mjs` — exhaustive deterministic sweep of every routing in
`docs/design/mod-lab.html`: 12 sources × 9 destinations × 2 polarities = **216 routings**,
each from a FRESH `ModLab` (the fresh-instance-per-trial discipline that the crash
investigation had to learn the hard way — persistent NaN state produced four consecutive
false findings before it). Three detectors: non-finite output / watchdog fires, level
blow-ups, and dead routings.

## Finding 1 — the loud transients were the same bug, not a second one

The previous fix guarded the read INDEX but derived `frac` from the un-wrapped `rd`:

```js
rd -= Math.floor(rd / len) * len;   // rd rounds UP to exactly len
let i0 = rd | 0;
if (i0 >= len) i0 -= len;           // index fixed -> 0
const frac = rd - i0;               // ...but frac = 8192 - 0 = 8192
```

So the exactly-`len` case stopped producing a NaN and started producing an **8192×
extrapolation**. Captured live at the failing sample (`K3 → choDep @ +1`):

| | value |
|---|---|
| neighbours `l0`, `l1` | `-0.13589`, `-0.13212` |
| `frac` | **8192** |
| interpolated `v` | **30.73** |
| synth stage peak | 0.49 |
| chorus stage peak | **8.99** |

Fixed by wrapping `rd` itself before it is used for either purpose. Level blow-ups across
the matrix: **6 → 0**. Every one of the six had been a `choDep` routing, which is what
localized it.

The lesson: the first fix was validated by "the crash stopped happening," and it did stop
happening — the NaN became a merely-very-loud number. A fix confirmed only by the
disappearance of its symptom is not confirmed.

## Finding 2 — same bug class in shipping C++, worse consequences

`src/time_core.h` has four fractional-delay reads that wrap `i1` but never `i0`. A `rp`
of `-1e-13` becomes `kBuf - 1e-13`, inside the ulp of `kBuf` (2.9e-11 at `1<<17`), and
rounds to exactly `kBuf` — an **out-of-bounds read on the audio thread**, where JS only
gave a NaN. Guarded at all four sites; parity unchanged (worst 5.6e-12), so the guard is
inert in normal operation.

Reachability, stated honestly: rarer here than in the lab. The mod lab hit it constantly
because `dly` *asymptotically smooths toward* a clamped floor of 2 and lands at
`2.0000000000000195`, so it fires whenever `w` passes that value. `time_core` clamps `d`
at read time, so a pinned `d` is exact and safe; only the unpinned path admits it. Rare,
but the consequence is UB and the fix is one line.

`src/fx_rack.h` checked and safe: integer comb delay, `newDly` clamped to `[2, len-1]`,
so the modulo numerator cannot go negative.

## Finding 3 — one genuinely dead routing (design question, not a bug)

`R → Kboost` at positive depth is bit-identical to no routing. Two mechanisms compose:
`kb = 8 * Math.max(0, kbMod)` is half-wave rectified, and the `R` source is bipolar
(`R * 2 - 1`). Measured across the coupling knee:

| rotor K | detune | max R | `+1` depth |
|---|---|---|---|
| 0.35 (default) | 0.3 | 0.334 | **DEAD** |
| 1.0 | 0.3 | 0.996 | alive |
| 0.35 | 0.05 | 0.984 | alive |

It revives exactly where R crosses 0.5 — the phase transition, not a threshold anyone
chose. The code comment already flagged the uni-vs-bipolar question; the measurement
sharpens it from "halves the given depth" to "fully dead below the knee." Raised as
register item **A9** for a human ruling rather than fixed unilaterally.

## Calibration note — L0016, again

The sweep's first run reported **53 dead routings and every one was my bench**: K5–K8 are
hard-zeroed above the rotor's oscillator count (the lab's own UI hides those rows, so they
are correct-by-design), cutoff defaulted to fully open so a positive cutoff mod clamped
instantly, and morphX/morphY are inert by design when every scope is system-wide — in this
lab morph position gates scope, it does not blend corner parameters. A detector that
reports 53 findings, all false, is worse than no detector: it trains you to skim. The
bench now gives every destination somewhere to go, and the count fell to one real finding.

## Evidence consulted

- `docs/design/mod-lab.html:456-476` (chorus wrap), `:339` (Kboost rectifier), `:688`
  (bipolar R map), `:1111` (`rebuildMatrixRows` hiding K rows above rotor n)
- `src/time_core.h:139/160/266/291` (the four unwrapped `i0` sites), `:39-40` (buffer sizes)
- `src/fx_rack.h:273` (integer comb read), `:106-107` (`newDly` clamp)
- `./verify report` → `{"target":"full","exit":0,"git":"31049a1"}`
