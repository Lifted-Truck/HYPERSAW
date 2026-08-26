# Trace — the lab was breaking our own feedback law; the human heard it first

**Trigger** human 2026-08-26: *"blend is currently resulting in feedback; does
it follow our decided feedback law?"*

**Answer: no, it did not — and we do have one.** ADR-031 (`DECISIONS.md:114-118`)
promotes two stability laws from measured bug post-mortems:

- **(a)** feedback paths normalise **/N** assuming worst-case correlation — a
  √N norm gave the echo loop LF gain `regen·√N`, unstable above regen ≈ 0.35,
  with a *measured* runaway (buffer mean 0.04 → 0.42 over 10 s) "followed by
  tanh-saturation squashing the audio to near-silence";
- **(b)** a **DC blocker inside every feedback loop**, corner clear of the
  lowest musical comb, because in-loop filters compound per pass.

Both are Layer-0 guarded (L0-19/L0-21) in the engine. **Rev 1 of the lab obeyed
neither**, and its symptom — runaway then tanh squash — is verbatim the
phenomenology ADR-031's own post-mortem records. The engine's law was fine; the
lab simply did not inherit it.

**Consequence: rev 1's headline measurement is RETRACTED.** "BLEND's midpoint
runs +9.0 dB above corner A and clips (peak 1.166)" was measuring a diverging
cycle, not a parallel blend. It was reported to the human as a finding that
"moves the ruling"; it moved it in the wrong direction. Retracted in the lab
panel, in ROADMAP B50, and here.

**Rev 2 applies the law.** Per-module deterministic makeup, in-loop DC block at
8 Hz, /N feedback and /√N output normalisation, an explicit loop-gain ceiling
with a live readout, and a lawful/lawless A/B so the failure stays audible on
demand. Verified at the matrix rather than inferred: outputs ×0.707, feedback
×0.500, exactly as the law specifies; loop gain 0.06 at the defaults.

## Three wrong module-gain schemes, each recorded because each looked right

1. **Small-signal unity** — tanh's effective gain depends on operating level,
   so a saturating drive landed ~30 dB down.
2. **Analyser-measured adaptive makeup** — correct in principle, but a feedback
   loop interacting with the crosspoints, so the level *wandered while you
   listened*. A lab whose gain moves under you is worse for ruling than one
   that is slightly off.
3. **Deterministic, integrated from the shaper curve over the actual source
   level** — shipped. No loop, no settling.

## The probe lesson (seventh instance this session)

A single analyser buffer is 46 ms; the source is seven detuned saws whose beat
period is *seconds*. Short-window level readings drifted **9.42 dB on identical
states**. Only a repeatability control — measure the same state twice, demand
agreement — caught it; 2.5 s averaging brings drift to 1.18 dB. Every level
number taken before that control is untrustworthy, which is why rev 1's table
is retracted rather than adjusted.

**Verify.** `./verify fast` exit 0; `lab_load_check` GREEN. No engine code;
parity untouched by construction.
