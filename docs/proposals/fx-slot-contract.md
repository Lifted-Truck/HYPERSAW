# Proposal — a universal FX slot contract

**Status: proposal, awaiting the human.** Written 2026-08-15 by the lead organ at
the human's direction: the Notch finding *"looks like the shape of an issue that
we need a universal solution for instead of a patchwork… let's go back to the
drawing board and consider the most hygienic alternatives that can be applied
universally so we avoid bumping into similar edge cases again."*

That reading is correct, and the survey below is worse than the Notch case
suggested.

## The actual state: `amount` means four different things

From `src/fx_rack.h`, as documented today:

| slot | what `amount` means | is there a no-op value? |
|---|---|---|
| `Drive` | dry/wet | 0 = passthrough |
| `Filter` | cutoff severity | 0 = open |
| `Gain` | level | **0.5** = unity; 0 = **silence** |
| `Comp` | strength | **none** — a 0.98 brickwall is *always on* |
| `Comb` | wet mix | 0 = dry (early-out below 0.001) |
| `Notch` | core's internal `mix` | **none** — measured −5.4 dB, mono, at 0 |

**Three different identity points** (0, 0.5, none) across six slots, and two slots
that cannot be bypassed at any setting. A patch author has no way to know which
they are holding, and neither does the GUI.

**And the evidence is one-sided:** five of those six rows are *documentation*.
Exactly one was ever measured — Notch — and it was wrong. We should not assume
the other five are right; we should assume they are unmeasured.

## The proposal: the rack owns dry/wet, slots only produce wet

One rule, applied to every slot type:

```
out = (mix == 0) ? in : lerp(in, slot.wet(in), mix)
```

- **`mix` is a rack-level control, identical on every slot.** `mix = 0` is an
  early-out, so passthrough is **bit-identical by construction** — not by each
  slot's implementation remembering to honour it. A rule enforced by the rack
  cannot be broken by a new slot type, which is the property the current design
  lacks and the reason Notch could ship wrong.
- **`amount` stops carrying bypass duty** and becomes purely per-slot character.
  `Gain`'s 0.5-is-unity and `Comp`'s always-on brickwall stop being anomalies:
  they are just what those slots *do* at `mix = 1`.

This is the same shape as the two artefacts landed this week — declare the
contract as data, enforce it with a gate that reads the declaration — and the
consistency is a reason to prefer it, not a coincidence.

## Where a naive crossfade is WRONG, and what the declaration must carry

A universal rule that ignores these would be a patchwork with extra steps.

1. **Blending dry is not "less effect" for every slot.** Dry + compressed is
   *parallel compression* — a different effect, not a gentler one. Dry + lowpass
   is a shelf, not a gentler lowpass. So `mix` and `amount` are genuinely
   different axes and both must exist. The proposal keeps both; it does not
   collapse them.
2. **Latency comb-filters the blend.** Crossfading a dry signal against a
   *delayed* wet one (Comb, and any future reverb) produces comb filtering — a
   colour nobody asked for. A slot with latency must declare it, and the rack
   either delay-compensates the dry path or the slot declares itself **wet-only**.
3. **Image and level are separate promises.** Notch collapsed stereo to mono; a
   slot must not change channel count, stereo image, or loudness *unless that is
   its effect*. `Gain` legitimately changes level. That is a declaration, not an
   exception.

So each slot type declares:

```
{ identity_at, blends_dry, changes_image, changes_level, latency_samples }
```

## The gate, which is the part that actually prevents recurrence

`slotcontract_check`: for **every** slot type, drive the real rack through the
CLAP factory and assert:

- at `identity_at` → output is **bit-identical** to the no-slot case;
- with **decorrelated stereo in** (L 220 Hz, R 330 Hz — the input that caught
  Notch) → `|outL − outR|` does not collapse unless `changes_image`;
- broadband level within a stated tolerance unless `changes_level`;
- declared `latency_samples` matches measured group delay.

Calibrated by planting: a slot that ignores `mix = 0`, one that mono-sums, one
that misdeclares latency — each must turn it red naming the slot.

**This is what makes the answer universal rather than a Notch patch.** A new slot
type cannot ship without a declaration, and a declaration that lies fails the
build. The current design had six prose comments and one measurement; this
replaces both with a declaration and a probe.

## Honest costs

- **It is a public-interface change.** Every slot grows a `mix` param, and
  `amount`'s meaning narrows on four of six types. Param ids are append-only, so
  this is additive — but saved patches must keep sounding the same, which means
  `mix` defaults to 1 and a migration is needed only if any slot's `amount`
  semantics actually move.
- **CPU.** A dry/wet crossfade per slot is trivial; delay-compensating the dry
  path for a latent slot is not free.
- **It touches `fx_rack.h` broadly** — worth doing before more slots exist, and
  the FX overhaul is queued anyway, so the cost is largely already committed.

## What we need from the human

1. **Ratify or counter the rule** — rack-owned `mix`, slots produce wet only.
2. **Wet-only vs delay-compensated** for latent slots (Comb, future reverb).
   Recommendation: **declare wet-only** first; delay compensation is a bigger
   change and can arrive when a slot actually needs the dry blend.
3. **Confirm `mix` defaults to 1** so existing patches are unchanged.

Nothing is built until these land. **The immediate next step regardless of the
ruling is the probe**, because it costs little and it tells us how many of the
other five documented contracts are also fiction — a question we currently cannot
answer, and the one that made this a shape problem rather than a Notch problem.
