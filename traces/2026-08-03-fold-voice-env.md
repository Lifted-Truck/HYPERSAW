# 2026-08-03 — ADR-078: per-voice envelopes (ADR-077 increment 2)

## Changes
- swarm_core.h: voiceEnv/relScatter params; per-voice relC[]/vAtk[]; full ADSR per
  voice in the render loop; shared s.env demoted to bookkeeping (= max per-voice
  envelope) so liveness/steal/NOTE_END are untouched; coefficient setup hoisted so
  it runs for voiceEnv OR onsetScatter.
- hypersaw_clap.cpp: ids 94/95. gui.html: rel scatter + per-voice toggle in Drift.
- waveshape_check.cpp: ADR-078 gate (sounds · uniform when unscattered · spreads
  when scattered · always ends).

## Evidence
voiceEnv on, scatter 0: peak 0.211, spread 0.0e+00, silent after 106 blocks.
voiceEnv on, scatter 0.8: spread 0.237 at 150 ms, silent after 174 blocks.
verify full green; parity 147/147.

## Bug found by the probe
Coefficients were computed only inside the onsetScatter branch — voiceEnv alone
left them at 0 and rendered SILENCE. Fixed by hoisting; the gate now pins
"per-voice envelopes must produce output" so it cannot regress silently.
