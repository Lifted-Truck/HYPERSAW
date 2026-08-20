# 2026-08-19 — the pitch wheel never reached the engine; three sessions fixed around it

**The report.** Third report in a row: "None of the bend laws actually make the
pitch bend." Previous two fixes (arming predicate, readParam) were real bugs but
not THIS bug. The human called for the drawing board, and was right to.

**The method change that found it.** Every prior glide oracle drives SwarmCore
directly; every prior fix was verified at that layer. This time the failing
experience was reproduced from the outside in: a probe driving the REAL plugin
through the CLAP factory — param events, MIDI notes, raw MIDI — at 44.1k AND
48k, measuring the pitch of the rendered audio by autocorrelation
(zero-crossings first: useless on a saw, counts harmonics — n=1, drift 0,
autocorrelation is the recipe). Result: every note-lane scenario PASSED at both
rates. The engine was innocent. What a DAW does that the probe did not: deliver
the wheel as **raw MIDI**.

**The bug.** `hypersaw_clap.cpp`'s CLAP_EVENT_MIDI case:
`if ((data[0] & 0xF0) != 0xE0 || ch == 0) break;` — the ADR-038 MPE handler
accepts bend only on member channels 2-16. Channel 0 — the plain pitch wheel on
every ordinary DAW track — was dropped ON PURPOSE (reading a ±2 st wheel at the
±48 st MPE range would be wildly wrong) but nothing else ever picked it up. The
wheel has been disconnected since MPE landed. Every bend-law session tested the
GUI's Pitch control (param 38) and passed while the human's hand was on the
wheel.

**Why no oracle saw it.** `grep -rn CLAP_EVENT_MIDI tools/` returned NOTHING —
no tool in the repo had ever sent a raw MIDI event. The one transport a normal
Live/Logic track uses had zero coverage. An exclusion with a good reason is
still a route to nowhere unless someone owns the excluded case.

**The fix.** ch-0 0xE0 → `applyParam(38, (v14-8192) * (2.0/8192.0))` — the exact
path the GUI Pitch control takes, so the bend law shapes wheel and slider
identically; with the law off it is the same instant write it always was. ±2 st
is the MIDI 1.0 and MPE-manager default; a bend-range param can widen it later.

**Evidence, both directions.**
- Probe, pre-fix: wheel thrown to max → `220 220 220 ...` dead flat, both rates.
- Probe, post-fix: `222 225 ... 246 247` — const-time 800 ms ramp to +2 st.
- New mpe_check case (permanent gate): pre-fix RED (99.9% of energy still at
  440 Hz), post-fix GREEN (440 bin 0.32161 → 0.00809, +2 st bin 0.32105).
- One stale-binary incident inside the control cycle: `--target mpe_check` after
  a stash pop reran the OLD binary with identical numbers; forcing both targets
  recompiled 2 files and flipped the verdict. Identical numbers on a rebuild are
  the tell.

**Kept.** `tools/glidepath_probe.cpp` — the DAW-like end-to-end harness
(diagnostic target, not a gate) — and the mpe_check wheel case (gate, BND-6).

**Verify.** `./verify full` EXIT=0 · parity 156/156 (worst 4.262e-09) ·
trajectory GREEN · mpe_check GREEN.
