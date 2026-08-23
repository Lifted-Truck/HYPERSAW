# bench-osc2-enable — the two stale bench tools now actually enable oscillator 2

- **Queue item:** none — direct human request (2026-08-22), landing
  recommendation 4 of `docs/research/2026-08-22-two-osc-cpu-measurement.md`
  ("Update the two stale bench tools … to set enable (id 1150) alongside
  volume, so future CPU readings from them describe what their comments
  claim"), which that report left out of scope (its brief was new-files-only).
- **Why:** ADR-100 ships oscillator 2 disabled (`oscEnabled = {1, 0}`,
  `src/hypersaw_clap.cpp:1122`) and a disabled oscillator does not run at all
  (skip path, ADR-099/100). `tools/shell_bench.cpp` and
  `tools/user_patch_bench.cpp` predate that default: their "osc2 audible"
  scenarios set osc-2 volume (id 1017) but never its enable (id 1150), so
  both measured ONE active oscillator while claiming two — the report's §4
  showed the giveaway (4.81% "osc2 audible" vs 4.70% "osc2 vol=0", noise
  apart, where a genuinely enabled second oscillator costs 1.8–2.0×, §table 3).
- **Evidence consulted:** the two-osc report §§4–5 + recommendation 4;
  `src/hypersaw_clap.cpp` (1122 default, 2576 enable handling, 2816 param
  read-back); `tools/scratch_b18_two_osc_matrix.cpp` (the scratch tool that
  set 1150 correctly and produced table 3).
- **What changed:** in both tools, `{1150,1}` is enqueued alongside the
  `{1017,0.4}` volume write in the scenarios meant to exercise two
  oscillators (shell_bench: the "+ osc2 audible" row and the
  "+ drift + width + tone tilt" row that builds on it; the "osc2 vol=0
  (default)" row stays as the one-osc control); stale comments/labels updated
  to say enable + volume. No behavior of any verify gate touched.
- **Re-measured (Release, this machine):** shell_bench "+ osc2 audible
  (enable + vol 0.4)" now 6.09% vs the 4.04% one-osc row — a real second-osc
  increment (+~2 points on a 4.1% base) instead of noise, consistent with
  table 3's 1.8–2.0× voice-loop scaling. user_patch_bench idle 0.36%,
  +note increments now ~0.98–1.45 (was 0.91→5.00 over 8 notes; now reaches
  9.90%), tail flat at ~9.3–10.9% — roughly double the report's
  one-osc ~5.1–5.3% tail, as a genuinely two-osc patch should be, and now
  close to ADR-100's DAW-shaped "8.7% flat" observation.
- **Verify:** `./verify fast`, exit 0, all gates GREEN (bench tools are not
  gates; no gate inputs changed).
- **Open questions:** `tools/notefuzz_check.cpp:185` and
  `tools/kstuck_probe.cpp` use the same volume-without-enable pattern — but
  those are verify gates, so whether their "twoOsc" arms should set 1150
  (changing what the gates exercise) is a human decision, flagged separately,
  not smuggled into this change.
