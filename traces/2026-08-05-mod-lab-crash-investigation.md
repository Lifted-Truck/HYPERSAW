# 2026-08-05 — mod-lab crash: NOT root-caused, and why

## Human report
"I did just crash something by making CHODEP dependent on R while chorus was linked
about 50% to main."

## Status: REPRODUCED ONCE, NOT DIAGNOSED. A watchdog was added; the cause is open.

## What is solid
- **The crash is real.** The first measurement of the session — on the only genuinely
  fresh page load I got — reproduced non-finite output with the human's exact
  configuration (R→choDep at depth 1, chorus mix 0.6, choSwarm link 0.5).
- **The mod lab's matrix was dead before any of this**, by static reading rather than
  measurement: `wire('rN', …)` calls `rebuildMatrixRows()`, which touches `mtx`; the call
  sits at character 1647 of the script block and the `const mtx` declaration at 5351.
  Confirmed in BOTH the pre-change file and the current one. Fixed in the previous PR.
- **That fix has a consequence I did not anticipate:** the TDZ abort had been killing the
  script partway, so roughly forty `wire()` initialisation callbacks — everything declared
  after the matrix — had **never run**. Un-breaking the script means they now run for the
  first time. That is a large, real behavioural change and a plausible source of new
  non-finite state, but it is a HYPOTHESIS; it is not verified.

## Why it is not diagnosed: my instrumentation was wrong four times
1. `lab.noteOn(midi, f)` takes a FREQUENCY. I called `lab.noteOn(45)`, so `f0` was
   `undefined` and every voice rendered NaN. That poisoned the synth's persistent phase
   state for the rest of the page's life.
2. I then hand-rolled a render harness (`controlTick` + `advance` + `synth.render`) that
   did not match the real `render()` path, and drew conclusions from it.
3. I "isolated" the chorus while its delay line already held NaN from (1), so the chorus
   looked guilty; with a cleared line and clean input it is provably fine.
4. **The decisive error: `navigate` did not actually reload the document.** Same-URL
   navigation reused the live JS context, so every "fresh page" test still carried the
   poisoned state and my earlier extreme settings (`fb 0.9`, `depth 0.02`, `link 0.5`,
   `R→choDep 1`) — visible in a freshness check I should have run first, not last. A
   copy under a new filename did not help; the pane served the old context anyway.

Each step produced a confident-looking measurement that was an artifact. The pattern is
L0017 again — the instrument, not the subject — but the specific new lesson is
**verify the reset before trusting the experiment**: a probe that cannot prove it started
from a known state cannot support any conclusion.

## What was done anyway
An ADR-032-style **NaN watchdog** in `render()`, the same pattern already carried by the
spectra and cooperator labs. It is a mitigation, not a fix: a non-finite sample otherwise
persists forever, because it is written into the delay lines and swarm phases — one
blow-up silences the lab until reload and poisons every later reading. The watchdog clears
the FX memory, releases voices, zeroes the block and counts the event.

## Next step, and it is tooling not care
Build a **headless Node harness for the labs** — extract the DSP classes and run them in a
fresh process per test, the way the C++ cores already are. Every failure above comes from
testing DSP through a browser context I could neither reset nor verify. The project's own
oracle discipline exists for exactly this and the labs have been exempt from it.
