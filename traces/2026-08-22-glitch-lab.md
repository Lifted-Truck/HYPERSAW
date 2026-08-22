# glitch-lab — the three state-tier glitch modules, seeded, in one audition bench

- **Queue item:** ROADMAP "SWARM GLITCH MODULES INGESTED — they live INSIDE the
  oscillator (2026-08-22)" — *"The lab run the human asked for comes first:
  audition the three modules together against a HYPERSAW patch, before any C++
  exists."* (ROADMAP.md:211-249)
- **Why:** `SPEC_swarm_glitch_modules.md` arrived with a prototype
  (`horde_decoherence_lab.html`) that calls `Math.random()` seven times, so the
  spec's own acceptance tests 3 and 5 (identical seed + gesture → identical
  output) are unrunnable against it (ROADMAP.md:243). This lab is the seeded
  reimplementation: one deterministic core (`GlitchSwarm`), mulberry32 only,
  visible seed, and an offline self-test panel that actually runs the spec's §5
  shapes. It also fixes the structural point the human made — the modules are
  written INSIDE the oscillator, between the mean-field computation and the
  phase integrator, not as a post-FX chain (there is no buffer to insert into).
- **Evidence consulted:** `SPEC_swarm_glitch_modules.md` (all sections);
  `horde_decoherence_lab.html` (read-only — protected; concepts, detune shape,
  pan law, failure-model constants); `swarmfilter.html:155-300,380-450`
  (house core/UI split, `rngNext`, seed + dice control);
  `docs/design/ensemble-lab.html:33-38,490-516` (tagline + rev-badge idiom that
  `tools/gen_lab_index.py` reads); ROADMAP.md:211-249.
- **What was implemented vs cut:**
  - DECOHERE — full: `K_eff = clamp(−K·strength, −1, +1)`, burst/hold, retrigger
    restarts (no stacking), note-on trigger, seeded stochastic rhythm process,
    `dec.healboost` behind a flag defaulting off.
  - STALE FIELD — full, including the spec's Haunt extension (torque blend, not
    field blend) and both capture semantics (§0 observers, §2 instant disengage).
  - NECROSIS — full: bipolar vitality axis, seeded permutation with the first
    ⌈N/4⌉ entries constrained to interior voices, LIFO healing, all four failure
    modes, virulence, age, recovery-not-undo with a seeded 150–300 ms ramp.
  - Both K conventions shipped as the `splay law` switch (ordered splay targets /
    prototype raw repulsion) because §0 asks for exactly that A/B and a lab is
    where it is settled by ear. No module was cut.
  - **Cut from the SELF-TEST panel only (not from the modules): spec §5 test 4**
    (stale-field beat rate monotonic in |f_B − f_A|). A rectified-envelope
    detector read the 110 Hz fundamental, not the beat (measured 108/108/108/64/81
    /s for |Δf| = 0/6.5/13.5/20.8/36.8 — non-monotonic and obviously wrong), and
    shipping a green light I do not trust is worse than shipping none.
- **Alternatives rejected:** copying the prototype's structure verbatim (it has
  no seeded stream, and NECROSIS §3 supersedes its damage/heal buttons);
  smoothing the mean-field VECTOR to get ψ̄ — the vector spins at f0, so a 5 ms
  one-pole collapsed its magnitude and reported r̄ = 0.278 on a swarm whose true
  r was 1.000. The self-test's decohere case caught it; the observer is now a
  rotating-frame tracker (scalar one-pole on r, one-pole on dψ, ψ̄ advanced by
  dψ̄ and pulled to live ψ through the wrapped error).
- **Verify:** `./verify fast` → exit 0, git d6d6520 (`.harness/last-verify.json`).
  Lab evidence, all headless via node against the file's own script text:
  - self-test, 6/6 PASS — bypass null max|Δ| = 0.00e+0; determinism max|Δ| =
    0.00e+0; must-differ control (seed 1235) max|Δ| = 1.43e+0; decohere lock
    r̄ = 1.000 → 0.168 within 60 ms → 1.000; necrosis LIFO order [6,9,11,5],
    last two released; threshold collapse r̄ 0.943 → 0.573 → 0.943.
  - sample-rate invariance of the rescaled failure models: 232/233/229 flicker
    events per 2 s at 44.1/48/96 kHz; burst 150.9/150.7/150.0 ms for 150 asked.
  - robustness: ordered vs chaotic splay differ (1.13e+0); haunt sweep,
    engage/disengage, voice-count churn 16→6→28 while damaged — all finite,
    peak ≤ 0.75, no stray damaged voice beyond N.
  - headless DOM smoke test: whole script loads, every button/key handler runs,
    60 blocks render through the ScriptProcessor path (peak 0.716), one draw
    frame paints, rev badge resolves.
- **Open questions:**
  1. §5 test 4 needs a beat-rate estimator that survives a 20 Hz beat under a
     110 Hz carrier — deferred, see above.
  2. The K-normalization mapping (`kRate = K · 0.5 · 2π · f0`) is chosen to put
     this lab's K = 0.3 at the prototype's "locks hard" point; the shell's real
     normalized-K semantics may differ and should win at fold time.
  3. Voice-count change while damaged rebuilds the LIFO stack in permutation
     order, losing the original damage order of surviving casualties. Spec §4
     only requires clamping M and healing indices ≥ N; flagged in-code.
  4. `docs/design/index.html` was NOT regenerated (out of scope) — the lead
     should run `python3 tools/gen_lab_index.py` if that index is written by it.
