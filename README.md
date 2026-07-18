# HYPERSAW

**Working title:** SWARM✱ (naming open until Phase 5; the ✱ family — SWARMSAW / SWARM✱SPECTRA / SWARM✱DYNAMICS — carries over from the prototypes; the repo answers to HYPERSAW).

**One-line pitch:** a synthesizer whose timbre, tuning, and performance gestures all emerge from a single coupled-oscillator dynamical system — the supersaw taken seriously as physics.

**Elevator version:** every existing supersaw picks a fixed detune recipe and hides it. SWARM✱ makes the swarm itself the instrument: voices are Kuramoto-coupled oscillators you can herd into lock, dissolve into cloud, splay into harmonic multiplication, or erase by interference; the same coupling law operating between *notes* settles chords into just intonation; and every behavior is deterministic, seeded, and provenance-tracked.

## What this repo is

A working CLAP-native instrument plugin (VST3 + AUv2 via clap-wrapper) built from three validated browser prototypes, driven by Claude Code under the autonomous-paradigm doctrine. Correctness is defined as **bit-level parity with the prototypes** (L0-1, ε=1e-6 RMS — in practice most golden scenarios match to the exact bit), never as plausible-sounding audio. Every claim in SPEC.md traces to a measured behavior in ACCEPTANCE.md; every measured behavior traces to a prototype you can open and hear.

## Map

| Path | Purpose |
|---|---|
| `SPEC.md` | The instrument: thesis, unified engine model, four-layer parameter surface, subsystem specs |
| `ACCEPTANCE.md` | Layer-0 and Layer-E criteria with measured numbers (+ ratified protocol notes) |
| `ROADMAP.md` | Phase-gated build plan, Phase 0 → 5 — **the single source of truth for status** |
| `DECISIONS.md` | ADR log, append-only (ADR-001…) |
| `PRIOR-ART.md` / `PARKED.md` | Competitive analysis · ideas register |
| `src/swarm_core.h` | The engine: header-only, pure, statement-level port of the reference cores |
| `src/hypersaw_clap.cpp` | CLAP shell: params, state, note handling, viz feed |
| `src/gui/` | Webview GUI (ADR-019 seam: `hypersaw_gui.h` is the swappable boundary) |
| `tools/` | The oracle: golden generator (Node, extracts the JS cores live), parity/trajectory/state checks, renderer bench |
| `traces/` | Provenance log — one entry per merged change set |
| `docs/` | Archived change notes, gate reports, (screenshots welcome: `docs/img/`) |

## Reference implementations (the oracle)

Three single-file HTML prototypes, each with a headless-testable DSP core (`SwarmSynth`, `SpectraSynth`, `DynSynth`) separable from its UI:

- `swarmsaw.html` — saw-kernel swarm: bipolar K (sync/splay), inertia, R→tone, XY pad, 8-voice poly
- `swarmspectra.html` — per-partial swarm: cascade locking, interference gating, width geography, stretch, 4-voice poly
- `swarmdynamics.html` — topology (mean-field / nonlocal ring / two-cluster), Sakaguchi α, consonance gravity, tempo-grid law, 4-voice poly

These are the reference implementation per ADR-003. The C++ port must match them (parity oracle), and their headless test harnesses are the templates for `./verify fast`.

## Repo status

*Last verified current: 2026-07-18.*

- **Phases 0–2 CLOSED** (gates ratified; see ROADMAP for the authoritative trail). The plugin is a shippable SAW instrument: full parameter surface (all four detune laws incl. host-tempo-synced tempo-grid, seeded distributions, drift, ADSR per ADR-021, density comp, width/mono, digital↔clean), webview GUI in the prototype design language (phase circle with dual R₁/Rₙ meters, seat rings, formation polygon, XY pad, COPY/PASTE STATE), resizable editor, session persistence proven by a dedicated oracle (`state_check`: exact value round-trip + bit-identical restored audio).
- **Phase 3 in flight**: the dynamics engine (topologies mean-field/ring/two-cluster + bimodal, Sakaguchi α, Daido poles, consonance gravity) is ported and parity-proven **51/51 against both references** (PR #12); the plugin surface for it (params, dynamics GUI cluster, R_q/R_A/R_B meters, gravity + grid-status readouts) is the next increment.
- **The oracle** (`./verify fast|full`): leak/structure gates → Release build → golden self-check → L0-1 parity (dual-engine golden sets regenerated from the HTML references every run) → trajectory suite (L0-2..5, 8..13 + ADR-015 anchors) → state persistence. CI mirrors it on every push (macOS + Windows build + pluginval strictness 5).
- **Validation**: pluginval strictness 10 SUCCESS · auval SUCCEEDED · Live load + play confirmed. Deferred by human direction: Windows runtime testing (until desktop coordination), Reaper/Bitwig loads.
- **Reference timeline** (ADR-011/012): prototypes update by in-repo edits with an ADR; externally-authored change notes archive to `docs/change-notes/`. Current `swarmsaw.html` is the v2 splay-legibility revision (v1 recoverable at the initial commit).
- Ecosystem briefs: Tonality intake brief due before consonance gravity ships (ADR-010); terrain-sibling brief due at Phase 4.
