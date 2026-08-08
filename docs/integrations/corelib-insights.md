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
7. **Perceptual taper declared WITH the param, applied at the knob, never in
   the core.** A spring-damping knob linear in ζ parks all audible action in a
   fraction of its travel (53%→1.5% ring between ζ 0.2 and 0.8); the fix is a
   knob linear in OVERSHOOT via the closed-form inverse — and because the taper
   lives at the knob, the core, goldens and parity are untouched (ADR-024
   lineage; bend-lab damping, 2026-08-08).
