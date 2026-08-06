# ADR-082 increment 2 — the second oscillator, and two read/write asymmetries

**Date:** 2026-08-06
**Verify:** `./verify full` GREEN at `kNumOsc = 2` — parity **147/147, worst 4.262e-09**
(identical to the one-oscillator build), `state_check` green, all 12 chains.

## What landed

`kNumOsc = 2`. The plugin holds `cores[kMaxOsc]`, with `core` kept as a reference to
oscillator 0 so the 52 existing call sites keep meaning what they meant. Params route by
oscillator, notes fan out to every oscillator, and oscillators 1..N-1 sum into the output.

**Higher oscillators default to SILENT** — `vol = 0` both in the constructed state and in the
default `params_get_info` reports. Without both, a second oscillator would sound the instant
`kNumOsc` rose, changing every existing patch; with them, parity is untouched.

Measured, driving the cores directly:

| config | osc0 alone | summed |
|---|---|---|
| osc1 `vol = 0` (default) | 0.08775 | **0.08775** — identical |
| osc1 `vol = 0.4`, same detune | 0.08775 | **0.17551** — exactly 2×, correlated |
| osc1 `vol = 0.4`, detune 0.85 | 0.08775 | **0.13621** — below 2×, decorrelated |

Silent by default, audible when asked, independently controllable.

## Two bugs, both the same shape: I routed the write and forgot the read

**1. `readParam` still read oscillator 0.** `applyParam` was routed by oscillator; `readParam`
was not, so `state_save` wrote every `o<k>.` key by reading oscillator 0's value, and load
applied those to oscillator 1.

The instructive part is *why the oracle nearly missed it*. `state_check`'s "every param value
round-trips exactly" **passed** — because it compares `get_value` on A against `get_value` on
B, both going through the same broken accessor. Two wrong reads agreed. Only the audio
comparison ("restored instance renders bit-identical audio") caught the divergence, and only
because the note fan-out made oscillator 1 audible. **An oracle that reads through the code it
is testing cannot see a symmetric fault in it** — the audio check works precisely because it
bypasses the accessor entirely.

**2. Audible output was conditional on a heap buffer.** The first render summed oscillators
through a `std::vector` scratch sized at `activate()`, guarded by
`if (mixL.size() < n) break;`. That makes a *voice disappear* when the buffer is unsized or the
block is larger than expected — a silent failure with no error. Replaced with a chunk loop over
a fixed stack buffer: cannot allocate, cannot depend on block size, cannot drop a voice.

## How the divergence was actually found

Not by reading the diff. By **bisecting**: cutting only the note fan-out and re-running
`state_check`, which went green — proving the fault was in note handling rather than
param/init state, and pointing straight at what the notes made audible. Two prior hypotheses
(the scratch guard, then activate ordering) were both wrong and both discarded on evidence.

## Not done

No GUI, no per-oscillator preset UI, no mixer control beyond `vol` itself. The layout lab's
osc page and the B20 preset tiers are the next increment.
