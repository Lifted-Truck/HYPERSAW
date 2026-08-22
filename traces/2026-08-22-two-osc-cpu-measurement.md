# two-osc-cpu-measurement — B18(b) min-spec CPU measurement closed with real numbers

- **Queue item:** ROADMAP.md B18(b) — "min-spec CPU measurement STILL OPEN before 2
  oscillators ship" (ADR-082 Decision 2).
- **Why:** ADR-082's CPU table was arithmetic on an estimate (measured 1-osc cost ×
  oscillator count × a borrowed ×4 derate). Since that ADR, `kNumOsc = 2` shipped
  with ADR-099/ADR-100's skip-when-disabled design, changing the actual cost shape.
  This measures the real shipped SwarmCore/shell instead of trusting the estimate.
- **Evidence consulted:** ROADMAP.md B11/B18 entries and the "CPU BENCH BUILT" note;
  DECISIONS.md ADR-082 (Decision 2 CPU table), ADR-099, ADR-099 Amendment 1, ADR-100
  (measured priors: 2.5%/6.3% single-osc, 1.81-1.97% default patch, enable_probe
  on/off numbers); `tools/cpu_bench.cpp`, `tools/shell_bench.cpp`,
  `tools/user_patch_bench.cpp`, `tools/enable_probe.cpp` (read and run unmodified);
  `src/hypersaw_clap.cpp` (`kOscStride = 1000`, `oscEnabled[kMaxOsc] = {1, 0}`,
  `kNumOsc = 2`).
- **What was built/run:** fresh `build-b18` (Release, `-O3 -DNDEBUG`, never touching
  `build-release`); ran `cpu_bench`, `shell_bench`, `enable_probe`,
  `user_patch_bench` unmodified across a voice/note matrix; added one new scratch
  tool `tools/scratch_b18_two_osc_matrix.cpp` (compiled by hand, NOT wired into
  CMakeLists.txt — out of scope) because the existing benches set osc 1's volume
  but never its `enable` (id 1150), so under the current ADR-100 default they
  measure a single active oscillator despite their comments. Full tables, commands,
  and the marginal-cost finding: `docs/research/2026-08-22-two-osc-cpu-measurement.md`.
- **Finding:** oscillator 2 OFF (shipped default) costs ~0.03% of a core — the
  ADR-099/100 skip holds. Oscillator 2 ON costs 1.8-2.0x oscillator 1's own cost at
  every voice/note count tested (linear, confirming ADR-082's assumption). Default
  and typical-heavy patches stay well inside the 50% E-6 budget even at a ×4
  min-spec derate; the polyphony-ceiling worst case (16 held notes, 16 voices/osc,
  both on) derates to ~68%, over budget — flagged as a pre-existing ceiling risk
  (a single oscillator at its absolute parameter max already derates to ~65% alone),
  not a new problem introduced by the second slot.
- **Alternatives rejected:** editing `shell_bench.cpp`/`user_patch_bench.cpp` to fix
  the stale `enable` omission — out of scope (existing files); flagged in the report
  as follow-up instead.
- **Verify:** `./verify fast`, exit 0, GREEN (presentation_check, depends_check,
  gen_gui_controls, test_table_check, gui_reach all GREEN; leak check and mailbox
  delivery SKIPPED as expected on this Mac). Git hash: d6d6520c5720b2e483ce3c07c2586ca25c3ace65
  (pre-commit; this trace's own commit follows).
- **Open questions:** no min-spec hardware (Apple M1 base / 2018 Intel ultrabook /
  Windows x64 AVX2) was available for this measurement — the ×4 derate is the same
  borrowed heuristic ADR-082 used, not a fresh fact, and that gap is reported rather
  than closed. Whether to cap poly/voice count against active-oscillator count for
  the ceiling case is a product decision left to the lead/human, not resolved here.
