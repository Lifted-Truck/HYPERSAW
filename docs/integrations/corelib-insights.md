# Core-library donor manifest — what HYPERSAW has that travels

> The core-library sibling (alias per ADR-014; real name in PRIVATE-NOTES.md)
> treats this project as its primary insight source. This file is the PULL
> SURFACE: what exists, how portable it is, and what each piece needs to leave
> home. Kept current as lessons land; exchanges beyond this file follow
> doctrine/INTEGRATIONS.md (file-based briefs, writes stay home).
> Last verified: 2026-08-08 (taper lesson added same day).

## 1 · The architecture lessons (the reason the library exists)

The distilled form is **LIBRARY.md L0027** (HIGH-PRIORITY): every axis that
*crosses* sound sources — audio context, routing, param-id namespace, preset
format, coupling semantics, macro flags, display vocabulary — is cheap to
define before the second source exists and expensive-to-impossible after,
because the first shipped build freezes it. Concrete scars, each with its PR:

| lesson | scar | where |
|---|---|---|
| the mixer DECIDES per-source vs patch | 13 of 31 "globals" were per-osc params with unreachable copies | ROADMAP § Re-order; PR #221 |
| stride IS capacity | the id scheme shipped full on day one; amendable only because no host had seen id ≥ 100 | ADR-082 Amendment 1, PR #214 |
| presets fall out of a good namespace | the per-osc preset tier was a key-prefix filter, "nearly free" | PR #217/#219 |
| parameter-sharing ≠ dynamics-sharing | "master K" (share a number) vs `link` (share phase) are different features | PR #222 |
| macros need flags at introduction | the time-scale macro's 16-param list took a scan plus hand-filtering | B25, PR #222 |
| one display vocabulary, defined once | corner colour/glyph is global across all UIs (standing convention) | ROADMAP 2026-08-05 |
| velocity/pressure from day one | the engine ignored velocity for a month; adding it late was easy ONLY because gain had one seam | ADR-084, PR #233 |
| oracles come in KINDS; parity is only one | parity certifies agreement, not correctness, and CERTIFIES any bug the reference shares — 147 scenarios passed for the whole life of the gravity bug and could not have failed. Invariant oracles need no reference, so they outlive it and donate cleanly | L0031; `subdiv_check`, ADR-086 |
| a pitch-class set is a shared control | scale quantise, arpeggiators, harmonic-snap FX and quantised mod destinations all need root+mask; carrying `{root, mask}` instead of a scale ID keeps the DSP core free of a scale table, so a new scale adds no parity surface | `hzScalePicker`, ROADMAP § Scale picker |
| consumers address a ROLE, not an instance | visuals, the XY pad, and labels each hardwired to oscillator 0; the fix is one named indirection resolved per read — **and the resolved instance must be labelled**, or mis-resolution is unobservable | L0028; PR #227, #239 |

## 2 · Portable modules, by readiness

**Ready now (C++ core + oracle, no shell entanglement):**
- **Glide/travel laws** — `src/glide_core.h` + `tools/glide_check.cpp`. Four laws
  + scale-quantise modifier; caller supplies control rate; zero deps beyond
  <cmath>. The cleanest export in the repo.
- **Force system** — `src/force_core.h` (shared by five engines already; it IS
  a mini-library and proves the pattern).
- **Oscillator-preset format** — `src/osc_preset.h` + `preset_check`. Header-only,
  callback-based, deliberately plugin-free.

**Ready with a seam cut (core is clean, shell wiring is HYPERSAW-specific):**
- **SAW engine** — `src/swarm_core.h`. Parity-proven against its HTML reference;
  exports with its golden generator or not at all (the reference IS the spec).
- **Kuro LFO / rotor** — mod-lab's `KuroSwarm` (JS). The C++ fold is pending
  (B3 blocks on rotor axes settling); export blocks on the same.
- **Voice allocation** — the ADR-083 three-tier steal policy + the arp-sustain
  oracle pattern (Goertzel-at-own-f0 vs control floor). Policy is ~20 lines;
  the ORACLE is the valuable half.
- **Humanization/ensemble** — onset scatter BY coupling (ADR-077/078), the
  Vorberg/Wing timing correction. Core-resident, exports with swarm_core.

**Design-stage (insight portable, code not yet):**
- **Quantum morph** — corner ownership, Gumbel-max territory, per-corner depths
  (A10), hysteresis-vs-chatter numbers. Lab JS only; the *rulings* are the
  portable part.
- **Mod matrix semantics** — scope vocabulary (system/corner/morph-owned),
  per-route polarity markers, depth-of-depth (B26). Same: rulings ripe, code lab-bound.

## 3 · What the library should demand of every module (HYPERSAW-derived)

1. A **headless reference** the port is parity-checked against (ADR-003) — the
   discipline that caught every real bug here.
2. **Seconds-domain time constants** converted at the edge (ADR-009).
3. **Superset-with-inert-defaults** for every addition — default value makes the
   new path bit-inert; the old goldens are the regression proof (ADR-021 lineage).
4. An **oracle that bypasses the accessor it tests** — round-trips through one
   accessor pass on symmetric faults (the readParam lesson, PR #215).
5. **Calibrated detectors** — every new check is shown to FAIL on the defect it
   exists for before it ships (rtsafety's elided-allocation lesson).
6. **Params modulation-ready at birth**: UI range and mod range declared
   separately (the ±12-knob / ±48-mod pitch pattern), time-domain params
   macro-flagged at introduction.
7. **Role addressing for every consumer.** Visuals, meters, labels, preset
   scopes and mod destinations name a ROLE ("the oscillator being edited", "the
   master bus") resolved through ONE named indirection on every read — never an
   instance captured at wire time. Two hard requirements fall out: the resolved
   instance must be RENDERED (a label is part of the mechanism, not decoration
   — an unlabelled mis-resolution survives every visual inspection), and a
   host/GUI callback must pair its member and its binding in one declaration
   (a callback with no bind is invisible to the compiler and ships dead-green).
   See L0028.

8. **One routing layer for performance gestures.** Velocity, aftertouch/
   pressure, per-note tuning, channel bend and mod wheel are SOURCES: they
   enter the same routing table as every other source and are distributed from
   it. Never a second hand-wired path alongside the mod matrix — with E event
   types and C consumers that is E x C silent chances to miss a connection, and
   every new consumer re-opens all E. A fan-out helper per operation family
   (HYPERSAW's 2026-08-09 patch) reduces E x C to E and is honest, but it is
   not the cure. **The layer must exist before the SECOND consumer**, because
   hand-wiring is cheapest exactly while there is one and is never cheap again.
   The win is that "does pressure reach oscillator 2?" becomes *unaskable*
   rather than merely answerable. See L0029.

9. **Perceptual taper declared WITH the param, applied at the knob, never in
   the core.** A spring-damping knob linear in ζ parks all audible action in a
   fraction of its travel (53%→1.5% ring between ζ 0.2 and 0.8); the fix is a
   knob linear in OVERSHOOT via the closed-form inverse — and because the taper
   lives at the knob, the core, goldens and parity are untouched (ADR-024
   lineage; bend-lab damping, 2026-08-08).

## 4 · Oracles — the donation that transfers requirement, not implementation

**The highest-leverage thing HYPERSAW can hand over is not code.** A donated
module carries the donor's accidents (its aliases, allocation habits, scope
assumptions) and the library inherits them. A donated **oracle** carries only
the requirement, so the library can build the subsystem from scratch — the way
it should have been built — and still be held to behaviour that was paid for in
real bugs here.

This only holds for oracles written against the **public surface and observable
output**. An oracle that reads internal state is a code donation wearing a
test's clothes: it pins the recipient's architecture to the donor's. See L0030.

| oracle | states the requirement | names no internals? |
|---|---|---|
| `mpe_check` | every performance gesture reaches every voice consumer; all-notes-off silences everything; legato retarget moves everything | ✅ drives factory → activate → events → process, detects by Goertzel on the output |
| `notefuzz_check` | seeded note on/off streams always decay to silence (no hung voices) | ✅ public note events only |
| `rtsafety_probe` | the audio thread allocates nothing | ✅ counts global operator new/delete around `process()` |
| `preset_check` | a preset is slot-agnostic and globals never travel | ⚠️ uses the preset format's own API — portable only with that format |
| `subdiv_check` | rendering does not depend on how a buffer is subdivided | ✅ **the most donatable kind** — an invariant, no reference, no internals named |
| `parity_check` / the golden chains | bit-parity against a reference implementation | ❌ HYPERSAW-specific by construction — these are the donor's own gate, not a donation |

**Suggested first exchange (the human's proposal, 2026-08-09):** the library
builds gesture routing from scratch and runs `mpe_check`'s *requirements*
against it. HYPERSAW supplies the oracle; the library supplies the architecture
HYPERSAW retrofitted. That is the cleanest possible division — neither side
inherits the other's mistakes.
