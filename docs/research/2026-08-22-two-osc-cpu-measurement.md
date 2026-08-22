# Two-oscillator CPU measurement — closing B18(b)

**Date:** 2026-08-22 · **Feeds:** ROADMAP.md B18(b) ("min-spec CPU measurement
STILL OPEN before 2 oscillators ship"), ADR-082 Decision 2's CPU budget table.

## Why this measurement, now

ADR-082's CPU table (DECISIONS.md, Decision 2) is arithmetic on an *estimate*:
measured single-oscillator cost × oscillator count × a ×4 min-spec derate
borrowed from the ADR-018 spike. `tools/cpu_bench.cpp`'s own header calls this
out — a derate is not a measurement. Since that ADR was written, ADR-082
increments 1 and 2 have SHIPPED (`kNumOsc = 2` in `src/hypersaw_clap.cpp`,
osc 1 silent by default per ADR-099/ADR-100), and ADR-099/ADR-100 landed a
skip-when-disabled optimization that changes the "silent second oscillator"
cost story entirely. This report replaces the estimate with numbers from the
actual shipped SwarmCore/shell, run through the existing bench harnesses.

## Method

- Fresh build directory `build-b18` (Release, `-O3 -DNDEBUG`), never touching
  the repo's existing `build-release`:
  ```
  cmake -S <repo> -B <repo>/build-b18 -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
  cmake --build <repo>/build-b18 --target cpu_bench shell_bench user_patch_bench enable_probe -j 8
  ```
- Four tools already in `tools/`, run **unmodified**: `cpu_bench` (SwarmCore
  alone, no shell), `shell_bench` and `user_patch_bench` (through the real
  CLAP factory), `enable_probe` (ADR-100's on/off/tail probe).
- One **new scratch tool**, `tools/scratch_b18_two_osc_matrix.cpp` — not
  wired into `CMakeLists.txt` (out of scope), compiled by hand with the same
  flags `shell_bench` uses:
  ```
  c++ -O3 -DNDEBUG -std=gnu++20 -arch arm64 -mmacosx-version-min=11.0 \
    -I<repo>/libs/clap/include -I<repo>/libs/clap-wrapper/include \
    -c tools/scratch_b18_two_osc_matrix.cpp -o build-b18/scratch_b18_two_osc_matrix.o
  c++ -O3 -DNDEBUG -arch arm64 -mmacosx-version-min=11.0 \
    -Wl,-search_paths_first -Wl,-headerpad_max_install_names \
    build-b18/scratch_b18_two_osc_matrix.o -o build-b18/scratch_b18_two_osc_matrix \
    build-b18/libHYPERSAW-impl.a -framework WebKit -framework Cocoa
  ```
  It exists because `user_patch_bench.cpp` sets osc 1's volume (id 1017) but
  never its `enable` (id 1150) — see the caveat below — so no existing tool
  gives a clean "osc 2 explicitly on vs explicitly off" comparison at matched
  voice/note counts. The new tool sets `enable` directly and reports the
  delta. All measurements: 44.1 kHz, 128-sample block, % of one core against
  the E-6 budget (<50% on min-spec), same convention as `cpu_bench`/
  `shell_bench`.
- All figures below use the plugin's shipped defaults except where a table
  states otherwise (voices/note = param 1 default 7, range 1-32; held notes
  ≤ `kPoly` = 16).

**Machine context, stated honestly:** Apple M3 (this Mac), macOS 15.3.2,
arm64, Release build. **This is NOT min-spec.** The E-6 envelope defines
min-spec as an Apple M1 base / 4-core 2018-class Intel ultrabook / Windows x64
AVX2. No such hardware is available in this worktree or session. Per the
brief, extrapolation below reuses the same ×4 derate ADR-082 itself used (the
ADR-018 spike's precedent) **for comparability with that ADR's own table
only** — it is not a fresh measurement and should not be read as one. A real
min-spec measurement is still not done; that gap is reported, not closed.

## Results

### 1. `cpu_bench` — SwarmCore alone, single oscillator, no shell overhead

| voices/note | notes | oscillators | % of one core | realtime multiple |
|---|---|---|---|---|
| 7 (default) | 8 | 56 | 2.25% | 44.4x |
| 8 | 4 | 32 | 1.12% | 89.7x |
| 16 | 8 | 128 | 4.42% | 22.6x |
| 16 | 16 (kPoly ceiling) | 256 | 8.88% | 11.3x |
| 32 (param max) | 16 (kPoly ceiling) | 512 | 16.23% | 6.2x |

The 32×16 row is the absolute worst case the parameter ranges allow for a
**single** oscillator — already worth flagging on its own: at ×4 derate that
is 64.9%, over the 50% E-6 budget, independent of the two-oscillator
question. Not new; recorded for context.

### 2. `enable_probe` — ADR-100's on/off/tail probe, through the shell, 4 held notes at 16 voices/osc

```
both on, 4 held:        rms 0.28403  cpu  4.43%
osc2 OFF:                rms 0.12445  cpu  2.15%
both OFF:                rms 0.00000  cpu  0.03%   (tails? KILLED)
re-enabled + new note:   rms 0.13837  cpu  1.23%   (SOUNDS)
osc1 vol 0, osc2 off:    rms 0.00000  cpu  0.58%
osc2 back ON mid-hold:   rms 0.06918  cpu  1.24%   (HELD NOTES RE-STRIKE)
```

Confirms ADR-099/100's shipped design still holds under the current build: a
disabled oscillator costs essentially nothing (0.03%, not merely reduced),
and audibly re-enabling it re-strikes held notes correctly.

### 3. New scratch matrix — marginal cost of oscillator 2, explicitly on vs off, matched patch

| voices/osc | notes | 1 osc active | 2 osc active | marginal cost of osc 2 | ratio |
|---|---|---|---|---|---|
| 7 (default) | 8 | 2.15% | 4.05% | +1.90 pts | 1.9x |
| 16 | 8 | 4.18% | 8.29% | +4.11 pts | 2.0x |
| 16 | 16 (kPoly ceiling) | 8.57% | 16.90% | +8.33 pts | 2.0x |
| 8 | 4 | 1.31% | 2.39% | +1.08 pts | 1.8x |

Patch shape: spring-law pitch bend, 5 s release, comb + drive (matches the
"human's patch" shape from `user_patch_bench.cpp`/ADR-100's diagnosis).

At ×4 min-spec derate (comparability-only, see caveat above): the 16v/16-note
row (kPoly ceiling, a realistic worst case for held polyphony) derates to
**34.3%** with one oscillator and **67.6%** with both active — over the 50%
budget. The default patch (7v/8 notes) derates to 8.6% / 16.2% — comfortably
inside. The 16v/8-note "typical heavy" row derates to 16.7% / 33.2% — inside.

### 4. `shell_bench` — existing scenario suite, unmodified, run for context

```
default patch, idle (no notes)          0.12% of a core
default patch, 8 held notes             2.16% of a core
16 voices/osc, 8 notes                  4.54% of a core
+ bend law on (const-time)              4.99% of a core
+ osc2 audible (vol 0.4)                4.81% of a core
+ drift + width + tone tilt             5.01% of a core
osc2 vol=0 (default) but 16v            4.70% of a core
16v + ALL saw-shape at 0.5              5.39% of a core
```

**Caveat, evidenced:** `oscEnabled[kMaxOsc] = {1, 0}` (`src/hypersaw_clap.cpp`
line 1122) — osc 1 ships OFF by default per ADR-100. `shell_bench`'s
`"+ osc2 audible (vol 0.4)"` row sets id 1017 (osc-1 volume) but never id 1150
(osc-1 `enable`), so under the CURRENT shipped default that row is **not**
actually running two oscillators — it is a single active oscillator with an
irrelevant volume set on a disabled one (row cost 4.81% sits right beside the
"osc2 vol=0" row's 4.70%, well inside noise, not the ~2x jump table 3 shows
for a genuinely enabled second oscillator). This bench predates ADR-100 and
was not touched (out of scope) — flagged here so its numbers aren't
misread as two-oscillator figures.

### 5. `user_patch_bench` — same stale-default caveat applies

```
idle                  0.27%
+note 1 .. +note 8    0.91% -> 5.00%
tail t+1s .. t+8s      ~5.1-5.3% (flat, matches ADR-100's "release doesn't decay CPU" finding)
```

Same issue as (4): this tool sets osc-1 volume (id 1017) but never `enable`
(id 1150), so under the current default it also measures a single active
oscillator, not two. Its numbers reproduce ADR-100's own "8.7% flat during
the tail" finding closely enough (this run: ~5.1-5.3%, at 44.1 kHz/128
blocks vs ADR-100's 48 kHz/64 blocks) to be a trustworthy general release-tail
sanity check, but should not be cited as a two-oscillator number until (or
unless) the tool is updated to set `enable` explicitly — table 3's scratch
tool does that.

## The comparison that matters: marginal cost of oscillator 2

- **Off (shipped default):** effectively free. `enable_probe`'s "both OFF"
  measurement is 0.03% of a core — the ADR-099/100 skip path holds; a
  disabled oscillator is not merely quiet, it does not run.
- **On:** linear, as ADR-082 assumed — table 3 shows oscillator 2 costs
  **1.8-2.0x** oscillator 1's own cost at every voice/note count tested, i.e.
  turning it on roughly **doubles** total voice-loop CPU. This matches
  ADR-082 Decision 2's stated assumption ("the voice loop scales linearly
  with oscillator count") — now confirmed against the real shipped core
  rather than assumed.
- **The one real risk found:** at the polyphony ceiling (16 held notes) with
  a heavy per-note voice count (16/osc), both oscillators active derates to
  ~68% of one core on the ×4 min-spec heuristic — over the 50% E-6 budget.
  The default patch and a "typical heavy" patch (16v/8 notes) both stay
  inside. This is the same shape of finding ADR-082's own table already
  flagged ("3 osc + 2x OS... over budget") — a genuine ceiling case can
  exceed budget; the common case does not.

## Recommendation for B18(b)

1. **Close B18(b) on the evidence above for the SHIPPED configuration**
   (2 oscillator slots, osc 1 off by default, oversampling opt-in/global).
   The default patch and typical heavy patches measure well inside the E-6
   budget even under the conservative ×4 derate; the ADR-099/100 skip design
   means most users pay nothing for the second slot until they deliberately
   enable it.
2. **Do not close it as "min-spec verified"** — no min-spec hardware was
   available for this measurement. The ×4 derate is the same borrowed
   heuristic ADR-082 already used, not a new fact. If a real min-spec machine
   becomes available, re-run `enable_probe` and the new scratch matrix on it
   directly; both are cheap (seconds) and require no code changes.
3. **Flag, don't block on:** the kPoly-ceiling + max-voice worst case (16
   held notes, 16+ voices/osc, both oscillators on) derates over budget. This
   is a pre-existing shape of risk (even a single oscillator at its absolute
   parameter ceiling — 32 voices, 16 notes — derates to 64.9% alone) rather
   than something the second oscillator introduces; worth its own ROADMAP
   line if the human wants a hard poly/voice cap tied to active-oscillator
   count, but that is a product decision, not a blocker for shipping 2 slots
   as configured today.
4. Update the two stale bench tools (`shell_bench.cpp`, `user_patch_bench.cpp`)
   to set osc 1's `enable` (id 1150) alongside its volume, so future CPU
   readings from them describe what their comments claim. Left out of this
   change set — both files are outside this brief's scope (new files only).

## Files touched by this measurement

- `tools/scratch_b18_two_osc_matrix.cpp` (new, not wired into `CMakeLists.txt`)
- `docs/research/2026-08-22-two-osc-cpu-measurement.md` (this file)
- `build-b18/` (new build directory; not committed — see `.gitignore`)
