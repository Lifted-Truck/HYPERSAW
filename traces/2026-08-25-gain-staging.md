# Trace — gain staging measured (B46)

**Trigger** human 2026-08-25: *"I feel like the synth is in general a little too
quiet on many settings."*

**What changed.** `tools/gain_probe.cpp` (new scratch diagnostic, wired into
CMake, NOT a gate and not run by `./verify`); `docs/research/2026-08-25-gain-staging-measurement.md`;
ROADMAP entry B46. **No DSP source was touched.**

**Evidence consulted.** `src/swarm_core.h:143,796` (the summing gain and its
defaults); `swarmsaw.html:210,583` (the reference those defaults are copied
from, verbatim); `tools/golden/gen_goldens.mjs` (no case sets `vol` or
`normExp`, so both are inherited and a default change regenerates every golden);
`src/hypersaw_clap.cpp:133,138,146,150,311` (param table).

**What was measured.** Peak/RMS dBFS out of the real plugin. Must-read-zero
control included and it reads −240. Headline: the shipped single-osc single-note
default sits at −21.39 RMS, but a four-note chord already peaks −4.37 and two
oscillators at `vol = 1` clip at +1.91 — so the deficit is headroom allocation,
not missing gain. `normExp = 0.75` was shown right at neither end of the
coherence range: 0.5 holds level flat within 0.7 dB across n = 1…16 at K = 0,
1.0 holds it flat within 0.15 dB across n = 4…32 at K = 1.

**Why no fix shipped.** All three contributing constants are reference values on
a protected path; changing any is a spec change requiring an ADR and a full
golden regeneration. Per the charter, surfacing the ruling IS the deliverable.
Option (1) in B46 — shipping factory patches at Density Comp 0.5 — needs no code
at all and should be tried first.

**Verify.** `./verify fast` exit 0, all gates green. `parity_check` 156/156
within ε=1e-6, worst 4.262e-09 — unchanged, as expected for a no-DSP change.
