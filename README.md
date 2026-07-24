# HYPERSAW

**Working title:** SWARM✱ (naming open until Phase 5; the ✱ family — SWARMSAW / SWARM✱SPECTRA / SWARM✱DYNAMICS — carries over from the prototypes; the repo answers to HYPERSAW).

**One-line pitch:** a synthesizer whose timbre, tuning, and performance gestures all emerge from a single coupled-oscillator dynamical system — the supersaw taken seriously as physics.

**Elevator version:** every existing supersaw picks a fixed detune recipe and hides it. SWARM✱ makes the swarm itself the instrument: voices are Kuramoto-coupled oscillators you can herd into lock, dissolve into cloud, splay into harmonic multiplication, or erase by interference; the same coupling law operating between *notes* settles chords into just intonation; and every behavior is deterministic, seeded, and provenance-tracked.

## What this repo is

A working CLAP-native instrument plugin (VST3 + AUv2 via clap-wrapper) built from six validated browser prototypes (three oscillator engines + a three-engine effects line), driven by Claude Code under the autonomous-paradigm doctrine. Correctness is defined as **bit-level parity with the prototypes** (L0-1, ε=1e-6 RMS — in practice most golden scenarios match to the exact bit), never as plausible-sounding audio. Every claim in SPEC.md traces to a measured behavior in ACCEPTANCE.md; every measured behavior traces to a prototype you can open and hear.

## Map

| Path | Purpose |
|---|---|
| `SPEC.md` | The instrument: thesis, unified engine model, four-layer parameter surface, subsystem specs |
| `ACCEPTANCE.md` | Layer-0 and Layer-E criteria with measured numbers (+ ratified protocol notes) |
| `ROADMAP.md` | Phase-gated build plan, Phase 0 → 5 — **the single source of truth for status** |
| `DECISIONS.md` | ADR log, append-only (ADR-001…) |
| `PRIOR-ART.md` / `PARKED.md` | Competitive analysis · ideas register |
| `src/swarm_core.h` · `src/spectra_core.h` | The oscillator engines: header-only, pure, statement-level ports of the reference cores |
| `src/force_core.h` · `src/filter_core.h` | Track E: shared force system (ADR-034) + resonator bank on it |
| `src/hypersaw_clap.cpp` | CLAP shell: engine select, params, state, note/MPE handling, viz feed |
| `src/gui/` | Webview GUI (ADR-019 seam: `hypersaw_gui.h` is the swappable boundary) |
| `tools/` | The oracle: golden generator (Node, extracts the JS cores live), parity/trajectory/state checks, renderer bench |
| `traces/` | Provenance log — one entry per merged change set |
| `docs/` | Archived change notes, gate reports, (screenshots welcome: `docs/img/`) |

## Reference implementations (the oracle)

Six single-file HTML prototypes, each with a headless-testable DSP core separable from its UI. Three oscillator engines:

- `swarmsaw.html` (`SwarmSynth`) — saw-kernel swarm: bipolar K (sync/splay), inertia, R→tone, XY pad
- `swarmspectra.html` (`SpectraSynth`) — per-partial swarm: cascade locking, interference gating, width geography, stretch
- `swarmdynamics.html` (`DynSynth`) — topology (mean-field / nonlocal ring / two-cluster), Sakaguchi α, consonance gravity, tempo-grid law

Track E (effects line, ingested 2026-07-18 — SPEC-EFFECTS.md):

- `swarmfilter.html` — resonator bank on the shared force core (`FilterLab`)
- `swarmphaser.html` — notch swarm, exact SVF nulls (`PhaserLab`)
- `swarmtime.html` — tap-swarm delay + FDN room (`TimeLab`)

These are the reference implementation per ADR-003. The C++ port must match them (parity oracle), and their headless test harnesses are the templates for `./verify fast`.

## Repo status

*Last verified current: 2026-07-23.* (ROADMAP.md is the authoritative status trail; this is the human-readable snapshot.)

- **Phases 0–3 CLOSED.** The plugin is a shippable, playable instrument with **two selectable engines** (SAW / SPECTRA, engine selector param) and the dynamics layer live inside SAW:
  - **SAW**: full parameter surface — six detune laws in the core (cents / Hz / ERB / host-tempo-synced tempo-grid, plus the lab-derived **harmonic** and **stretch** laws, ADR-065/066 — those two are core+oracle only until the batched CLAP pass exposes them), seeded distributions, drift, ADSR (ADR-021), density comp, width + super-width, mono/glide/legato, digital↔clean, phase scatter, pan scatter.
  - **Dynamics** (Phase 3, in SAW): topology (mean-field / ring / two-cluster + bimodal), Sakaguchi α, Daido poles q, absolute-K, consonance gravity + basin, parity-proven **51/51 against both references**, with the full GUI surface (R_q/R_A/R_B meters, gravity + grid-status readouts, cause-AND-state warnings).
  - **SPECTRA** (Phase 4): per-partial swarm ported bit-exact (parity RMS 0.0, L0-6 cascade zipper + L0-7 interference gate), engine-gated GUI with the per-partial **strip visualizer**, up to 32 partials, and a per-voice uncoupled **sub-oscillator** (−1/−2/−3 oct, sine→triangle→square morph) — the first deliberately-original feature (ADR-042).
  - **Performance/IO**: MPE per-note pitch (needs Live's per-device MPE toggle on — see `docs/research/`), transposition suite (octave/semi/cents/pitch-bend, all live), bass-mono output stage, resizable editor, COPY/PASTE STATE, session persistence proven by `state_check` (exact round-trip + bit-identical restored audio).
- **Lab→core fold campaign (2026-07-23, in progress).** The detune-lab audition round is folding into the reference + core as parity-safe supersets, one ADR + golden set each: tone tilt, hi-tame, drift modes + keep-phase, opt-in freq glide, pan motion + centre pin, the **harmonic law** (`harmReach` — the NI-style walk down the harmonic series) and the **stretch (inharmonic) law** (ADR-060..066). Parity grew 54/54 → 102/102 across the campaign. ADR-065 also established a measured **domain limit on the parity contract** (chaotic regimes amplify 1-ULP differences; now recorded in ACCEPTANCE §L0-1). Remaining: golden distribution, octave spread + root-anchor, the divergence ADRs, and the batched CLAP param pass — see the ROADMAP workshop entry.
- **Track E (effects line), in progress** on the shared **force core** (ADR-034, E0): the position-domain home/sync/splay/gravity/drift/inertia system extracted as one module all the effect engines consume, verified seed-for-seed by `force_check`. Shipped so far: the resonator bank (`filter_core.h`, bit-exact vs `swarmfilter.html`), the notch swarm (`notch_core.h`), the time engines (tap delay + FDN room, ADR-049/050), the **SWARM-FX** effect-plugin shell (external audio + engine select, ADR-047), the experimental swarmalator engine (ADR-048), and the internal FX-rack increment 1 (routing skeleton, ADR-054).
- **The oracle** (`./verify fast|full`): leak/structure gates → Release build → golden self-check → the oracle chains: L0-1 parity (goldens regenerated from the HTML references every run), trajectory suite (+ ADR behavioural anchors), state persistence, note fuzz, **force-core** trajectories, **spectra** parity + the P=1 measured-equivalence gate, **filter**, **notch**, **swarmalator**, and **time** parity. CI mirrors it on every push (macOS + Windows build + pluginval).
- **Validation**: pluginval strictness 10 SUCCESS · auval SUCCEEDED · Live load + play + MPE confirmed. Deferred by human direction: Windows runtime testing (until desktop coordination), Reaper/Bitwig loads.
- **Scheduled: Phase F — reference-path liberation** (ADR-041). At the E1 gate the "correct == bit-parity with the prototype" contract graduates to forward performance standards (new golden references from the liberated implementation; L0 suites repointed one engine at a time; prototypes demoted to provenance). Until then the parity discipline holds and additions use the superset-with-inert-defaults pattern.
- **Reference timeline** (ADR-011/012): prototypes update by in-repo edits with an ADR; externally-authored change notes archive to `docs/change-notes/`. Current `swarmsaw.html` is the v2 splay-legibility revision (v1 recoverable at the initial commit).
- Ecosystem briefs: Tonality intake on hold (human priming directly, ADR pending); terrain-sibling brief due with the Phase 4 kernel abstraction.
