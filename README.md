# SWARM✱ — spec kit

**Working title:** SWARM✱ (naming open; the ✱ family — SWARMSAW / SWARM✱SPECTRA / SWARM✱DYNAMICS — carries over from the prototypes).

**One-line pitch:** a synthesizer whose timbre, tuning, and performance gestures all emerge from a single coupled-oscillator dynamical system — the supersaw taken seriously as physics.

**Elevator version:** every existing supersaw picks a fixed detune recipe and hides it. SWARM✱ makes the swarm itself the instrument: voices are Kuramoto-coupled oscillators you can herd into lock, dissolve into cloud, splay into harmonic multiplication, or erase by interference; the same coupling law operating between *notes* settles chords into just intonation; and every behavior is deterministic, seeded, and provenance-tracked.

## What this kit is

The design-complete handoff from three validated browser prototypes to a C++ CLAP/VST3 project driven by Claude Code under the autonomous-paradigm doctrine. Every claim in SPEC.md traces to a measured behavior in ACCEPTANCE.md; every measured behavior traces to a prototype you can open and hear.

## File map

| File | Purpose |
|---|---|
| `SPEC.md` | The instrument: thesis, unified engine model, four-layer parameter surface, subsystem specs, implementation notes |
| `ACCEPTANCE.md` | Layer-0 and Layer-E criteria with the exact measured numbers from prototype verification |
| `ROADMAP.md` | Phase-gated build plan, Phase 0 → 5, with gates and exit criteria |
| `DECISIONS.md` | Seeded ADR log (append-only from here) |
| `PRIOR-ART.md` | Competitive/novelty analysis and freedom-to-operate flag |
| `PARKED.md` | Ideas register — nothing gets silently dropped |

## Reference implementations (the oracle)

Three single-file HTML prototypes, each with a headless-testable DSP core (`SwarmSynth`, `SpectraSynth`, `DynSynth`) separable from its UI:

- `swarmsaw.html` — saw-kernel swarm: bipolar K (sync/splay), inertia, R→tone, XY pad, 8-voice poly
- `swarmspectra.html` — per-partial swarm: cascade locking, interference gating, width geography, stretch, 4-voice poly
- `swarmdynamics.html` — topology (mean-field / nonlocal ring / two-cluster), Sakaguchi α, consonance gravity, tempo-grid law, 4-voice poly

These are the reference implementation per ADR-003. The C++ port must match them (parity oracle), and their headless test harnesses are the templates for `./verify fast`.

## Repo status

*Last verified current: 2026-07-17 (spin-up day).*

- **Spun up 2026-07-17** via `/spinup`: survey answers in `project.manifest.json` (rung 2 — thread + verifier), charter in `CLAUDE.md`, oracle dispatcher `./verify`, knowledge loop (`INDEX.md`/`LIBRARY.md`), `traces/`.
- **Status: Phase 0 not started.** No C++ exists yet — the only running code is the three prototype HTMLs. `./verify fast` is honestly minimal (structure/manifest/leak checks); it grows the L0 suite in Phase 1.
- **Reference note:** `swarmsaw_v2.html` supersedes `swarmsaw.html` (ADR-011, splay-legibility change note); SPEC §5.6 and ACCEPTANCE L0-3 carry the amendments.
- Ecosystem briefs (Tonality, terrain-sibling) deliberately deferred — see ADR-010.

## Spin-up (historical)

This kit was fed to `/spinup` on 2026-07-17 — see `project.manifest.json` for the resulting manifest. Original seeds: engine core is judgment-light scoped execution once the spec is fixed (Sonnet-tier); parity-oracle design, ADR resolution, and DSP debugging are judgment-bearing (Opus-tier). Phase 0 gates before any feature work, per doctrine.
