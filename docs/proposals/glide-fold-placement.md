# Proposal — where the bend glide advances

**Status: PROPOSAL.** Written 2026-08-19 by the lead organ after tracing the fold to the point
where it stops being mechanical. Nothing implemented. The question is one line of placement,
and picking it wrong means redoing the audio path twice.

## What is already settled

- `glide_core.h` implements all five laws, is gated by `glide_check` at the L0-1 parity bar
  against goldens sliced live from `bend-lab.html`, and is **not in the audio path**.
- `kOff` is a clean pass-through (`x = target; vel = 0; y = target`), so an inert default
  cannot move parity. That is the safety property the whole fold rests on.
- The bench ran the filter at **tick rate** (16 samples, 2756/s at 44.1 kHz) *"because that is
  where a fold would put it"*. The goldens therefore encode tick-rate behaviour.

## What bend does today

`pitchBend` (id 38) is written on the param event and `updateTuneAll()` recomputes the tune
factor immediately (`hypersaw_clap.cpp:1391`, consumed at `:823`). **Bend is instant.** The
glide fold makes that write a *target* and advances toward it.

## The fork

The render loop processes event-delimited segments `[frame, until)` and hands whole spans to
`core.render()`. There is no control-tick boundary at shell level — the 16-sample tick lives
*inside* `SwarmCore`. So the glide has two possible homes, and they are not equivalent.

### (A) Subdivide the render on a fixed time grid — RECOMMENDED

Advance the glide every `bendGridSamples()` and split the render at those boundaries, applying
the new tune at each. Construction copied exactly from ADR-086 Amendment 1:
`kBendGridSeconds = 16.0 / 44100.0`, `lround(sr * kBendGridSeconds)` — chosen so the grid is
**exactly 16 at 44.1 kHz**, which is what makes the goldens still apply.

- **For:** matches the bench, so `glide_check`'s goldens remain the oracle rather than becoming
  decoration. Keeps `glide_core` as one ported engine with one implementation. Reuses a
  construction this project has already ratified *and* probed for sample-rate drift — the
  amendment exists because the first version of that idea was sample-rate dependent, and that
  lesson is already paid for.
- **Against:** it restructures the render loop, which `subdiv_check` and the sample-rate probe
  both watch. That is a cost, not a risk: those gates exist precisely to make this kind of
  change provable rather than hopeful.

### (B) Put the glide inside `SwarmCore`'s existing tick

- **For:** no render restructure; the tick is already there.
- **Against:** `swarm_core.h` already carries its own glide machinery (ADR-063 frequency glide,
  ADR-076 poly glide). Adding a second, differently-derived glide to the same core means two
  implementations of one idea in one file — the exact shape this project keeps paying to
  remove (two GUIs, two leak gates, two default tables). It also puts a **bend-lane** filter
  inside a **per-oscillator** core, when bend is global.

## Why this is a decision and not a detail

The grid is baked into the audio. Choosing it twice is a parity break twice, and the goldens
that make the port trustworthy were measured at one specific rate. ADR-086 is the precedent
for exactly this: a grid chosen casually (256 samples) had to be amended within the hour when a
sample-rate probe found it was a duration that shrank as the rate rose.

## Recommended order, with the safety property first

1. Wire (A) with the shell's law defaulting to **`kOff`**, and prove `./verify full` unmoved —
   147/147 parity, `subdiv_check`, the sample-rate probe, 252 goldens. The glide is in the
   audio path and provably inert.
2. Only then expose the law params (id 106+) and the scale globals.
3. Then the section and its graphs.

## The one question the human owns

**Does bend inertia default ON or OFF once the params exist?** `glide_core`'s own comment calls
`kConstRate` the *"ratified default: keeps 93% of wheel vibrato"* — but that is the bench's
default for auditioning. Shipping it as the plugin default changes how every existing patch
bends. The project's own precedent points the other way: oscillators above the first default to
silent so that *"a host reset to defaults must give silence too"*. Recommendation: ship
**`kOff`**, so no existing patch changes and `kConstRate` is what you get when you turn it on.
