# modulator-lab — the LFO + envelope editor bench, built (B16)

- **Queue item:** B16 (`ROADMAP.md:6915`), scoped by § LAB BRIEF — modulator editor
  (`ROADMAP.md:6456-6494`). Composition targets read but not built: B26 (`ROADMAP.md:6910`,
  § MOD MATRIX: DEPTH IS ITSELF A MOD TARGET, `ROADMAP.md:5126`).
- **Why:** the brief's five outstanding modulator requests belong together rather than as five
  patches, and two of them are design questions rather than features. The lab answers them as
  design claims with measurements attached, and leaves the third (ownership tier) explicitly
  unanswered because the human flagged the framing itself as uncertain — so the deliverable
  there is a concrete question, not a ruling.

  Three claims the lab makes, each measured offline on its own core:
  1. **reverse-saw is skew = 0 on the ramp axis, not a sixth shape.** One phase warp gives
     saw-down / triangle / saw-up on the ramp family and pulse width on the pulse family;
     shape is therefore continuous and is itself a routable destination.
  2. **S&H is a sampler, not a shape**, and the two kinds differ in the CLOCK. Global = one
     clock, one value, broadcast. Kuro = each mod samples on its bound rotor voice's wrap,
     from its own stream. Measured (seed 1234, rotor 6 Hz, 60 000 control ticks):
     global → value corr **1.000**, timing lock **1.00**; kuro → corr **−0.23** at every K
     while timing lock rides |K| from **0.04** (K=0) to **0.83** (K=+1) and **0.75** (K=−1).
     Both mods off → no sample instants, lock reads nothing (the must-read-nothing control).
  3. **polarity belongs to the route.** LFO A → drive (a rectified destination standing in for
     Kboost), discarded fraction by route polarity: as-source **43.1%**, bipolar **43.1%**,
     uni+ **0.0%**, uni− **100.0%**.
- **Evidence consulted:** ROADMAP § LAB BRIEF (`6456-6494`), § MOD MATRIX: DEPTH IS ITSELF A
  MOD TARGET (`5126`), § THE SHAPE LABS, PLACED (`260-276`); `docs/design/mod-lab.html`
  (`KuroSwarm` 95-241, `ClassicLFO` 242-262, `ModEnv` 265-278, scope/per-corner-depth ruling
  and the reachability/polarity finding 590-800); `docs/design/shape-lab-mod.html` (breakpoint
  grammar and `segCurve`, 119-127); `swarmtime.html` and `docs/design/bend-lab.html` for house
  lab conventions; `tools/labharness/lab_load_check.mjs`; `tools/gen_lab_index.py`.
- **Alternatives rejected:**
  - *Add a `reverse saw` enum entry.* Rejected: the brief calls the missing shape "the tell
    that the shape set was never designed", so one more entry leaves it exactly as undesigned
    and keeps shape unmodulatable.
  - *Rebuild shape-lab-mod's MSEG breakpoint editor here.* Rejected as a fork of an existing
    surface. The envelope is staged DAHDSR seconds (a gate has no total duration) and reuses
    that lab's `segCurve` unchanged; the two time grammars are named as an open question
    rather than silently unified.
  - *Port mod-lab's full `KuroSwarm`.* Rejected: this lab needs coupled TIMING, not the
    topology/link/splay-lattice surface. The compact rotor is mean-field only and says so in
    the panel; negative K is plain repulsion, not the rank-lattice splay law.
  - *One correlation meter for the two S&H kinds.* Rejected after measurement — see below.
- **Verify:** `./verify fast`, exit **0**, git `d6d6520` (`.harness/last-verify.json`). The new
  lab is gated: `lab_load_check.mjs` runs inside `fast()` (`verify:159-161`) over every
  `docs/design/*.html`; direct run prints `OK modulator-lab.html · GREEN`.
- **Two defects the offline probe caught, recorded because both are the same class:** a
  detector that agreed with itself for the wrong reason (MEMORY: detector-shares-assumption).
  1. The determinism check hashed only mods/env/rotor R and its seed+1 control came back
     IDENTICAL — correctly, since with S&H off and scatter 0 none of those touch a stream.
     Now it hashes rendered audio (per-voice envelope scatter is drawn at note-on) and
     `seedIsLive()` says out loud when a patch legitimately has no randomness in it.
  2. The correlation meter ran over 256 control ticks = 93 ms, over which a 0.8 Hz sine is a
     straight line and a 4 Hz S&H is a constant. It read ~1.0 for two unrelated free LFOs and
     0.0 for two mods holding the identical value — exactly inverted. Now 1024 decimated
     points (~6 s). A first replacement for the timing meter — "did both fire in the same
     12 ms window" — read 0.01 at K=1 with R=0.86: a true number answering the wrong question,
     because mean-field coupling locks voices at CONSTANT OFFSETS, not simultaneously. The
     shipped meter is the order parameter of the A→B sampling offset in units of A's period.
- **Open questions:**
  - **ownership tier is deliberately unanswered.** The panel poses three readings —
    (i) editability, (ii) preset-state ownership, (iii) precedence between writers — and
    demonstrates (i) by refusing to make K1 modifiable and (ii) by dropping a baked mod's
    parameters from the serialized JSON. (iii) is the morph scope vocabulary, already ruled;
    if that is what was meant, B16 owes nothing here. Needs the human.
  - **two time grammars unreconciled**: staged seconds (this lab's envelope) vs normalised
    t + duration (`shape-lab-mod.html`). Which one a preset stores is unruled.
  - **`docs/design/index.html` is not regenerated** — out of scope for this dispatch. The lead
    should run `python3 tools/gen_lab_index.py` (note: its `main()` only counts; it does not
    write the file, so the index card for this lab must be added by whatever writes it).
  - **B31 does not exist in ROADMAP.md.** The dispatching brief cited "B31 (XY as modulator)"
    as a downstream target; `grep -n B31 ROADMAP.md` returns nothing. Read as either a typo or
    an unwritten item; nothing in this lab depends on it.
  - **Not built, on purpose:** depth-of-depth (B26's own scope — the lab ships only the two
    pieces B16 owes it, reverse-saw and tempo sync, as the `preset: B26 patch` button);
    per-voice modulation destinations (routes here are global); MPE/per-note modulator
    instances (mod-lab already recorded the per-note finding).
