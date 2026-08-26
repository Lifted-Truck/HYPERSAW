# Trace — drop-oldest ratified; the plectrum explained; two lab defects found

**Trigger** human 2026-08-26, two items: *"The argmax boundary doesn't have a
click as much as it seems almost like a struck plectrum. I imagine this has to
do with the interaction between the comb and the drive."* and *"I want to make
sure to ratify the decision to replace drop-newest with drop-oldest."*

## 1. ADR-126 — drop-oldest, ratified and landed

`hypersaw_clap.cpp`: the 16-entry mono held stack now evicts the oldest key
instead of silently discarding the newest. Probe re-run: hold 40…55, press 70,
press 71, release 71 → **now falls back to 70** (was 55, while 70 was still
held). `parity_check` 156/156 unchanged — no golden holds 16 keys in mono.
`notefuzz_check` GREEN, 0 hangs. Test row B51-1, which is honest that the
overflow ORDER has no automated oracle: `notefuzz_check` covers hangs, not
priority, and the steal-priority suite our 2026-08-11 answer promised is still
unbuilt.

**Cross-repo:** filed `response-seam-round2-2026-08-26.md` into FOUNDATIONS'
tree (committed there, **not pushed** — pushes are the human's). It answers
their single open question: **the Aug-11 round governs on both points.** The
Aug-25 counter-answer's parity argument only held because the Aug-11 promise
had gone unkept for a fortnight — it was defending an accident and calling it
parity. Dated filename per their standing suggestion.

## 2. The plectrum — the human's mechanism was right

Measured at the ARGMAX flip, transient peak over steady mean:

| comb feedback | ratio |
|---|---|
| 0.0 | 1.6 |
| 0.45 | 1.7 |
| 0.9 | 4.3 |

It scales with feedback because **a comb with feedback IS Karplus-Strong** —
delay + feedback + damping is the plucked-string algorithm — and the flip
hands it a discontinuity. An impulse into Karplus-Strong is by definition a
pluck, so "struck plectrum" is not a metaphor. The ~1.6 floor at zero feedback
is the topology step itself; the comb turns that step into a pluck.
**B50-1's falsifier asked whether the flip is a CLICK. It is not — the
falsifier did not fire, and ADR-125 stands on that point.**

## 3. Two defects the investigation surfaced — and a caveat on ADR-125

`BiquadFilterNode` defaults **Q to 1.0**. The comb's damping lowpass was left
at that default, and a lowpass at Q=1 peaks *above* unity near its corner — so
the in-loop damper was ADDING gain at ~4.2 kHz. At feedback 0.9 that put the
comb's own loop gain over 1 and the lab reached **4.3e4 under ARGMAX, which
closes no cycle at all.** The DC blocker measured 1.0839 at its corner for the
same reason. Both are now Butterworth. The lab also lacked ADR-031(c)'s
watchdog — a lab a human rules on must not be able to deafen them — so a
hard-knee limiter now guards the output.

**The caveat, stated rather than buried.** A runaway at ~4.2 kHz is *screechy*.
The human's first reason for rejecting BLEND was "untenable screechy feedback",
and some of that may have been this defect rather than BLEND — a resonant
in-loop damper degrades every law, and BLEND's cycle would amplify it worst.
**ADR-125 is not withdrawn**: its second reason (a blended topology is an
averaged structure, which is what a quantum morph exists not to do) is
independent of any lab defect and is the one recorded as load-bearing. But the
sonic half is now uncertain, and BLEND deserves a re-listen on the fixed lab.
Filed as ADR-125 Amendment 1 and offered to the human rather than quietly kept.

## Verify

`./verify fast` — **`mailbox_delivery` RED by design**: the FOUNDATIONS filing
is committed in their tree but not pushed, and pushes are the human's. Every
other gate GREEN, including `test_table_check` (135 tests, 6 awaiting an
oracle). `parity_check` 156/156; `notefuzz_check` GREEN; `lab_load_check` GREEN.
