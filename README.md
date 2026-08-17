# HYPERSAW

**Working title:** SWARM✱ (naming open until Phase 5; the ✱ family — SWARMSAW / SWARM✱SPECTRA / SWARM✱DYNAMICS — carries over from the prototypes; the repo answers to HYPERSAW).

**One-line pitch:** a synthesizer whose timbre, tuning, and performance gestures all emerge from a single coupled-oscillator dynamical system — the supersaw taken seriously as physics.

**Elevator version:** every existing supersaw picks a fixed detune recipe and hides it. SWARM✱ makes the swarm itself the instrument: voices are Kuramoto-coupled oscillators you can herd into lock, dissolve into cloud, splay into harmonic multiplication, or erase by interference; the same coupling law operating between *notes* settles chords into just intonation; and every behavior is deterministic, seeded, and provenance-tracked.

![SWARM✱ GUI — phase circle, voice map, note monitor, full param surface](docs/img/gui-overview.png)
*The instrument as of build `86bdedd` — phase circle with live R meters, voice map (pan × pitch, target vs actual), note monitor, log spectrum, and the full four-cluster parameter surface. (Screenshot refreshed with GUI-changing PRs; the build hash in its corner says exactly which code drew it.)*

## What this repo is

A working CLAP-native instrument plugin (VST3 + AUv2 via clap-wrapper) built from seven validated browser prototypes (three oscillator engines + a three-engine effects line + an experimental swarmalator), driven by Claude Code under the autonomous-paradigm doctrine. Correctness is defined as **bit-level parity with the prototypes** (L0-1, ε=1e-6 RMS — in practice most golden scenarios match to the exact bit), never as plausible-sounding audio. Every claim in SPEC.md traces to a measured behavior in ACCEPTANCE.md; every measured behavior traces to a prototype you can open and hear.

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

## Two interfaces, and which one you get

The repo carries **two** GUIs, and they are a **succession, not a fork**: `gui.html` is the
original single-oscillator interface, and `gui2.html` is the ground-up successor that will
replace it. Worth knowing before you build:

| | reaches | shape | build |
|---|---|---|---|
| **`src/gui/gui.html`** — "GUI1" | **102 / 105** params | one long, complete column | **what a default build embeds** |
| `src/gui/gui2.html` — "GUI2" | **105 / 105** params | four pages: MAIN · MIX · OSC · FX | opt in with `-DHYPERSAW_GUI2=ON` |

**If you are just curious about the instrument, build the default and look at GUI1.** It is the
coherent one: every control the engine has, laid out as a single surface you can read top to bottom,
and it is what the screenshot above shows. Nothing extra to pass, nothing to switch on.

```bash
cmake -S . -B build-release -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j
```

**GUI2 is where the instrument is going, and it is interesting for a different reason.** Its controls are no longer
hand-placed — 144 of them are **generated** from `src/param_presentation.tsv`, an address-keyed table
of label/page/group/widget, so adding a parameter means adding a row and a control can no longer be
forgotten. That table is deliberately separate from the parameter's *structure* (ranges, ids), and it
carries **no id column at all**, because a page name that doubles as a dispatch fact re-fuses the two
things the split exists to separate. Its page assignment is a **proposal** rather than a decision —
`docs/design/layout-lab.html` is where information architecture gets settled, and changing it means
editing one column rather than moving markup.

Both are gated: `gui_reach` fails the build if any declared parameter is unreachable in *every* GUI,
which is how 29 silently dead controls were found and killed.

**Why GUI1 is still the default if GUI2 is the successor.** GUI2 was deliberately held at a partial
surface while the parameter plumbing was designed, rather than grown on top of the debt GUI1 carries
— so for a long stretch "incomplete" was the *correct* state for it to be in, not a gap. Now that the
declaration split exists, GUI2 is generated to the full surface and the default will move to it. If
you want to see the instrument today, GUI1; if you want to see where it is going, GUI2.

## Reference implementations (the oracle)

Seven single-file HTML prototypes, each with a headless-testable DSP core separable from its UI. Three oscillator engines:

- `swarmsaw.html` (`SwarmSynth`) — saw-kernel swarm: bipolar K (sync/splay), inertia, R→tone, XY pad
- `swarmspectra.html` (`SpectraSynth`) — per-partial swarm: cascade locking, interference gating, width geography, stretch
- `swarmdynamics.html` (`DynSynth`) — topology (mean-field / nonlocal ring / two-cluster), Sakaguchi α, consonance gravity, tempo-grid law

Track E (effects line, ingested 2026-07-18 — SPEC-EFFECTS.md):

- `swarmfilter.html` — resonator bank on the shared force core (`FilterLab`)
- `swarmphaser.html` — notch swarm, exact SVF nulls (`PhaserLab`)
- `swarmtime.html` — tap-swarm delay + FDN room (`TimeLab`)

Plus `swarmalator.html` — the experimental swarmalator engine (position and phase co-evolving,
ADR-048), ported to `swarmalator_core.h` and gated, but not yet in the shell.

These are the reference implementation per ADR-003. The C++ port must match them (parity oracle), and their headless test harnesses are the templates for `./verify fast`.

## Repo status

*Last verified current: **2026-08-15**.* (ROADMAP.md is the authoritative status trail; this is the
human-readable snapshot. A dated line that is honestly stale beats a confident one that is quietly
wrong — if this date is old, trust ROADMAP.md.)

- **Phases 0–4 CLOSED.** A shippable, playable instrument with two selectable engines (**SAW** /
  **SPECTRA**) and the dynamics layer live inside SAW. SAW carries the full surface — six detune
  laws, seeded distributions, drift, ADSR, density comp, width + super-width, mono/glide/legato,
  phase and pan scatter. SPECTRA is the per-partial swarm, ported bit-exact, with the strip
  visualizer, up to 32 partials, and a per-voice sub-oscillator (ADR-042). Performance/IO: MPE
  per-note pitch, the transposition suite, bass-mono output, COPY/PASTE STATE, session persistence.
- **The oracle — `./verify fast|full`, 30 gates.** 25 compiled probes plus 5 script gates. L0-1
  parity is **147/147 scenarios, worst 4.262e-09 RMS**, with goldens regenerated from the HTML
  references every run. Alongside parity sit the invariant probes that parity structurally *cannot*
  see: subdivision invariance, sample-rate independence, RT-safety (allocation-free audio thread),
  note-lifecycle fuzz, steal priority, forensic capture, GUI reachability, and presentation-table
  totality. CI mirrors it on every push.
- **Note-lifecycle conformance against FOUNDATIONS.** Their ruled note behaviours run as a suite
  against **our** bookkeeping (`tools/conformance_check.cpp`): **8 passed · 0 ruled failures · 3
  library-default divergences**, plus our own timing-independent END ledger — every identity issued
  comes back through an END exactly once. The three divergences are ruled **conforming** (their R8:
  the rule constrains the *path*, not the *moment*). The gate **pins** that set, so a new failure
  *and* an unexpected pass both go red — good news is still drift.
- **A test table per page and feature.** `tests/feature_tests.tsv` — **42 tests, 35 agentic, 7 human,
  4 openly awaiting an oracle.** Every row declares whether it pins a **RULING** (a decision) or an
  **ENCODING** (how it happens to be done today) and names the owner, so a table survives a harness
  change. Coverage is checked against the GUI: a feature cannot appear on screen with no test row.
- **18 design labs** in `docs/design/` — benches where behaviour is auditioned before it becomes
  code. Newest is `feedback-lab.html`, which runs its whole loop in one AudioWorklet (a node graph
  cannot express a one-sample delay, which is the quantity under test) behind three independent
  safety mechanisms: loop gain starting at zero, an always-on limiter, and an auto-kill on the
  *pre*-limiter signal. Every lab is swept by a load gate, because a lab that throws at setup still
  *looks* fine.
- **Track E (effects line)** on the shared **force core** (ADR-034): resonator bank, notch swarm,
  time engines (tap delay + FDN room), the SWARM-FX effect shell, and the internal FX rack — now
  with NOTCH selectable as slot type 6. An open design question is queued: the rack's slot contract,
  where `amount` currently means four different things across six slots.
- **Validation**: pluginval strictness 10 SUCCESS · auval SUCCEEDED · Live load + play + MPE
  confirmed. Deferred by human direction: Windows runtime testing, Reaper/Bitwig loads.
- **Scheduled: Phase F — reference-path liberation** (ADR-041). At the E1 gate, "correct == bit-parity
  with the prototype" graduates to forward performance standards. Until then the parity discipline
  holds and additions use the superset-with-inert-defaults pattern.
- **Reference timeline** (ADR-011/012): prototypes update by in-repo edits with an ADR. Current
  `swarmsaw.html` is the v2 splay-legibility revision.

## Known gaps, stated rather than omitted

- ~~`build-macos` shows as *skipping* in CI.~~ **Retracted 2026-08-16 — this was never a gap, and
  it was asserted here without being checked.** The skip is deliberate and documented in
  `.github/workflows/ci.yml`: the account hit its free Actions minutes, macOS runners bill 10×, so
  PRs run the coverage this dev Mac *cannot* produce (Linux + Windows) while **`build-macos` runs on
  push to `main` as the post-merge net** — and macOS is precisely the platform `./verify full`
  already covers locally on every change. Verified: the last six pushes to `main` all succeeded.
- Four test-table rows have **no oracle yet**: the bend-quantiser regression (behaviour fixed, gate
  unwritten), `amount=0` passthrough across every FX slot type, the master level meter (not built),
  and mute/solo + master octave.
- Feedback routing in the **modulation** graph is now **ruled** (2026-08-17): cycles are legal,
  every feedback edge carries a **unit delay at block rate**. Still lab-only in practice, but for
  a narrower reason — the *stability* bound is a separate open question, so a legal cycle is not
  yet a safe one. Feedback in the **audio** graph is a different question and untouched by that
  ruling.
