# Trace — Echo and Room reach the rack; and I destroyed the work once doing it

**Trigger** human 2026-08-27: *"I also want to hear the reverb and the delays
ASAP."*

## They were already built

`src/time_core.h` — the Track E2 port of `swarmtime.html` — has been
oracle-covered since it landed (`time_check`: L0-1 parity plus the L0-19/20/21
stability laws, all running in `./verify`), and reachable only from SWARM-FX,
the standalone effect. **Zero references in the plugin.** This wires it in; it
does not write a reverb.

Two slot types, one core: `Echo` (7, tap-swarm delay) and `Room` (8, FDN room
swarm), differing only by `mode`.

## Measured before claiming it works

Tail RMS with the note long gone, so only an effect can be sounding:

| | regen 0.3 | 0.6 | 0.9 | 0.97 |
|---|---|---|---|---|
| Echo | −69.2 dB | −59.9 | −52.3 | — |
| Room | −102.7 dB | −95.5 | −62.7 | **−34.8** |

Room read only 12 dB above the floor at regen 0.75 and my first probe called it
"no tail" — that was my threshold, not the engine. The sweep shows 68 dB of
range and clean monotonicity: Room is a *room*, short at moderate settings.
**Usability finding worth having early: Room's useful range is the top third of
the knob**, which argues for a per-type `amount` curve when the per-slot pages
land.

`Echo` at `mix = 0` differs from no Echo in **0 of 88,064 samples** — the
passthrough contract holds. Parity **156/156**.

## RT-safety

Follows the NotchCore precedent exactly: `TimeCore`'s constructor allocates
(~1.75 MB each), so instances are built in `setSampleRate` on the main thread
and never touched by `processSlot`. Mode changes *are* audio-thread safe —
`setParam("mode")` calls `rebuild(false)`, which writes pre-existing arrays and
allocates nothing. **Verified by reading it, not assumed.** Allocation is
unconditional because `setType()` runs on the audio thread from param events, so
lazy construction there would be the exact allocation the rule forbids.

## The mistake, recorded because it was expensive and entirely mine

Mid-way through, while repairing an unrelated scratch probe, I ran
`git checkout -q -- .` — a command I had no reason to include and did not think
about. It **discarded every uncommitted source edit**, destroying the whole
feature. The ADR and test rows had been written afterwards, so for several
minutes the repo held documentation and tests for code that no longer existed,
and I had installed a build without it.

Recovered by re-applying the edits from the transcript, rebuilding, re-running
every measurement (identical numbers), and re-installing. **Nothing was lost
permanently**, because the edits were reconstructible — but that was luck about
this particular change, not a property of the process.

The lesson is not "be careful with git checkout". It is that **a bulk-discard
command has no place in a turn that is not explicitly about discarding**, and
that uncommitted work should be committed before any unrelated repair step. Both
are cheap; neither was done.

## Verify

`./verify fast` exit 0 · `parity_check` 156/156 · installed to both formats via
`./install`, labels confirmed present in both binaries, AU seal verified,
**`auval -v aumu Hsaw LfTk` → AU VALIDATION SUCCEEDED**.
