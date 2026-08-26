# Trace — armed-view polish: four rulings landed, one CPU report triaged

**Trigger** human 2026-08-26, five items in one message.

**1. Knob baseline hop (FX-rack screenshot).** `.row.kcell.owned` offsets its
content by border 3 + padding 3 = 6px; my #457 fix left ghost cells at
padding 3 only, so a cell hopped 3px up the moment it went ghost. Fix:
`padding-top:6px` on the ghost cell — stripe still paints at 0–3 of the
padding box. Verified: knob-top delta owned-vs-ghost = 0 shipped, 3 with the
old padding re-injected (control fired).

**2. Ghost-flicker on release ("only sometimes").** The arm click writes
`lastParams[159]` optimistically; `syncFromEngine`'s 500ms `Object.assign`
can land a snapshot taken before the engine applied the write, regressing 159;
the 700ms poll re-ghosts every row; the next cycle clears it. Intermittent
because it needs a stale snapshot inside the window. Fix: `armEcho` local-echo
guard — the clicked value is authoritative until the engine echoes it once or
1.5s passes (so host automation of 159 is never masked for long). Verified by
simulating the race: echo holds through a regressed snapshot, clears on
agreement, expires for automation.

**3. Empty gaps — RULING.** Human: "I know it would make them a little less
visible but I preferred the empty dotted lines." The deep-shade underlay is
removed from both stripes and the rail; `--armc-deep` computation deleted.
The 1.31–1.77:1 pale-corner measurement stands and is recorded at the rule —
the human accepts the trade knowingly. Comment forbids quietly reintroducing
an underlay.

**4. Logo outline — RULING.** "Keep the logo bordered in light mode." The ink
outline was already the shipped light-mode state; what the human compared was
the click-to-audition `logo-bare` toggle. Ruling recorded; the toggle removed
(a logo that drops its outline on a stray click is a liability once settled).

**5. "Drive module uses unreasonable CPU" (30% → 10% across a morph line).**
Triaged, not fixed — filed as ROADMAP B47. The FX-rack Drive is two tanh per
sample (~0.3% of a core, arithmetic) and cannot explain 20 points. Prime
suspect: **Oversample 2x in the same corner preset** — ADR-075's own
measurement is ~2.5× core CPU, and 30/10 is a ×3 ratio; a stepped toggle
snaps at the morph boundary, matching "morphed over the line". Plan in B47:
diff the two corner presets first; A/B cpu_bench with OS toggled; only then
profile the drive slot.

**Verify.** `./verify fast` exit 0 each step. CSS+JS only, no DSP, parity
untouched. Built, installed both formats, re-signed, seals verified, markers
(`armEcho` present, `armc-deep` absent) confirmed in both installed binaries.
