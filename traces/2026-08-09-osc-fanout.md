# Oscillator fan-out — eight sites that meant "every oscillator" and said "core"

**Date:** 2026-08-09
**Trigger:** continuing the multi-oscillator thread; checking my own earlier note
that "MPE note expressions reach only oscillator 0" rather than trusting it.
**Verify:** `./verify full` GREEN, 13 chains. L0-1 parity **147/147 worst
4.262e-09 (unchanged)**. New `mpe_check` GREEN, calibrated red on all three bugs.

## What was wrong

Eight sites in the CLAP event loop applied a per-voice or lifecycle operation to
`core` — the alias for oscillator 0 — where the intent was every oscillator.

| class | sites | audible result with 2 oscillators |
|---|---|---|
| `setNoteExpr` (TUNING, MPE latch, MIDI channel bend) | 3 | a bend moves osc 1 and strands osc 2 at the old pitch — the pair SPLITS mid-gesture |
| `allOff` (mono/poly toggle, engine switch, MIDI all-notes-off, `plug_reset`, GUI panic) | 5 | osc 2 never released — **a stuck note** |
| `retargetNote` + mono force-release | 3 | mono legato slides osc 1 only; the mono force-release orphans osc 2's gated voice |

My starting note was half stale, which is why it got checked: PRESSURE already
fanned out (ADR-084 added it correctly), TUNING did not. **That asymmetry was the
tell** — when one member of a family fans out and its sibling does not, the port
was done call site by call site rather than by family.

## Root cause: the compatibility alias

`hypersaw::SwarmCore &core = cores[0];` was introduced so the ADR-082 two-slot
port would not have to touch every legacy call site. That convenience is exactly
what hid this: eight wrong sites read as correct C++, compiled clean, and were
genuinely correct while `kNumOsc == 1`.

An alias over a newly-multiplied resource converts "unported call site" from a
**compile error** into a **silent runtime bug**. It should carry a deadline.

## The fix: one named indirection, not sprinkled loops

Added a fan-out seam on the plugin — `allOffAll`, `noteOffAll`, `retargetAll`,
`setNoteExprAll`, `setNotePressureAll` — and routed all 14 affected call sites
through it (including two places that already had a hand-rolled
`for (k = 1; k < kNumOsc)` loop, which is the same idea written twice). There
are now **zero** per-voice calls on the `core` alias.

Removing the alias entirely is the real fix, since it would make the compiler
force the choice at every future site. Not done here: `core` still has
legitimately-oscillator-0 readers (`core.p.bpm`, `core.focus()`,
`core.swarmAt()` for the mono bookkeeping), so it is a broad refactor of a
public-ish surface. Queued behind a human gate in ROADMAP.

## The oracle: `tools/mpe_check.cpp`

Drives the **real CLAP event path** (factory → activate → events → process), not
a core in isolation — the bug lived in the event fan-out, which no core-level
test can see. Measures emitted audio with a single-bin Goertzel rather than
reading state: there is no per-voice tuning getter, and adding one to test with
would be testing the accessor (the `state_check` trap, where a round-trip
through one broken accessor agreed with itself).

Both cores are set to one voice, zero detune, zero width, so each oscillator is
one clean partial and "did oscillator 2 come along?" becomes "is there still
energy at 440 Hz after a +7 semitone bend?". **Oscillator 2's volume must be
raised explicitly** — it defaults to 0 (inert-by-default), and a fan-out probe
that lets it stay silent passes for the worst possible reason.

### Calibration — every assertion proven red on its own bug

| planted | probe says | verdict |
|---|---|---|
| `allOffAll` → `cores[0].allOff()` | rms `0.142 → 0.142 → 0.142`, decay **×1.0** | plateau |
| `setNoteExprAll` → `cores[0].setNoteExpr()` | 440 Hz **49.8% left** | half stranded |
| `retargetAll` → `cores[0].retargetNote()` | 440 Hz **50.0% left** | half stranded |
| fixed | 0.2% / 0.1% left; tail decays to 0 | GREEN |

**A threshold tuned until it passes is not a gate.** The all-notes-off check
initially failed at rms 0.0021 and the tempting move was to loosen it. Measuring
three successive windows instead showed it *decaying* ×18 — and reading
`SwarmCore::allOff()` confirmed it only clears gates, so voices ring out through
release + dissolve and a tail there is CORRECT. The settle was lengthened rather
than the threshold relaxed, and the discriminator became the shape, not the
loudness: a stranded oscillator keeps its gate set at sustain 1.0 and **plateaus**
(measured: ×1.0), which no amount of settling would ever hide.

**A calibration loop that is not itself checked will report success.** The first
calibration run silenced the build with `>/dev/null`; the tuning case's build
did not produce a fresh binary and the previous iteration's executable ran
instead, reporting the *previous* bug's failure. It looked like a pass because
something failed. Re-run with build output visible, it fired correctly at 49.8%.
Same class as the lab-load-gate near-miss.

## Knowledge loop

L0028 promoted **candidate → canonical** on a second independent occurrence in
an unrelated subsystem (voice lifecycle, not visuals), same day. Two corollaries
added: (4) a compatibility alias over a newly-multiplied resource is the hazard;
(5) partial fan-out means the port was done by call site, not by family.

## Open — human gate

`mpe_check` is **built but NOT gated**: adding it to `./verify` touches a
protected path. Requested in the PR. Until gated it is a probe, not an oracle,
and the three bugs it covers can silently return.

## Evidence consulted

- `src/hypersaw_clap.cpp` — 14 sites; `core` alias at the cores[] declaration
- `src/swarm_core.h:449` — `allOff()` clears gates only (why a tail is correct)
- `tools/rtsafety_probe.cpp` — the CLAP host-stub pattern this reuses
- `./verify full` — 13/13 GREEN, parity 147/147 worst 4.262e-09
