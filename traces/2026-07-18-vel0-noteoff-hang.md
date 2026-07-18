# 2026-07-18 — Envelope hang: vel-0 note-on remap (ADR-038)

**Report.** "Doesn't always stop when you let go even when release is all
the way down; something isn't updating on a tight enough schedule."

**Diagnosis (layered exoneration, ADR-032 method).**
1. *Core:* primary hypothesis (sustain>=1 path ignores p.releaseS) refuted —
   render() derives atk/rel/dec from the params every call
   (src/swarm_core.h:322-324); headless measurement: tail at release=0.005 s
   is 40 ms, scales with the knob (0.16 → 1.32 s, 2.0 → 11 s). Env updates
   per-sample; events are sample-accurate — no coarse schedule exists.
2. *Shell:* 120 seeded causal random note streams (poly / mono / mono+legato)
   through the real CLAP process() — all silent after all-keys-up.
3. *Wrapper layers read:* VST3 passes note-offs unconditionally; AUv2
   translates MIDI status 9 → CLAP_EVENT_NOTE_ON **without the MIDI 1.0
   vel-0-is-note-off remap** (libs/clap-wrapper auv2/process.cpp), and
   handleEvent ignored velocity → a controller's 0x90-vel-0 release struck a
   fresh full-gain voice that nothing ever releases. Headless repro: 50/50
   vel-0 scenarios hang at sustain level. Controller-dependent → "doesn't
   always."

**Fix.** handleEvent: NOTE_ON with velocity <= 0 routes to the shared
note-off path (extracted handleNoteOff — identical mono/held-stack routing).
Shell-only; core untouched.

**Oracle.** tools/notefuzz_check (new, CMake target linking the impl like
state_check): 5 modes × seeds, release at knob min, requires silence 0.5 s
after all-keys-up. GREEN 75/75 post-fix; pre-fix the vel-0 modes fail 50/50.
Not wired into ./verify (protected) — wiring proposed in ADR-038.

**Evidence.** ./verify full exit 0 @ 4968493 (pre-commit tree + fix):
parity 51/51 (worst 4.262e-09), trajectory/state/force/spectra GREEN.
Harness trap recorded: fuzz timestamps must be drawn sorted per block, or
the generator emits OFF-before-its-own-ON orders no host can produce (first
run "found" 94 fake hangs that way).

**Open.** (a) Wire notefuzz_check into ./verify full (human gate).
(b) Residual: voice steal / mono retarget overwrite tags[] without NOTE_END
for the displaced note — host bookkeeping leak, spawned as follow-up.
(c) Upstream: consider reporting the missing vel-0 remap to clap-wrapper.
