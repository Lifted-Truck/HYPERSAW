# SWARM✱ — roadmap (phase-gated)

Gates are blocking. "Green" = `./verify fast` passes + phase acceptance subset + trace written. Passing ≠ done; done = green + acceptance criteria + DECISIONS/trace updated.

**Status (2026-07-21):** Phases 0–1 CLOSED. Phase 2 (SAW) + Phase 3 (dynamics) gate-close proposed and shipped — **formal ratification still pending** (see per-phase gates below). Phase 4 SPECTRA ported + shell-integrated; ADR-037's P=1 gate was RULED 2026-07-18 (option (a), measured equivalence) — its only live remnant is the shared-voice-path A/B follow-up. Track E: E0 force-core + E2 time engines done; E1 frequency cores done (L0-17/18 + SWARM-FX GUI remain); **E3 internal FX rack increment 1 shipped** (ADR-054). Feature adds (2026-07-20/21): SPECTRA ADSR (ADR-055), bipolar onset lock (ADR-056), SPECTRA transposition (ADR-057), SAW waveshape morph (ADR-058), GUI column layout + fixes. **Swarmalator** core+oracle done, awaiting nondestructive shell integration — the recommended next build. Mod matrix: Kuramoto LFO design accepted (ADR-053); rotor-to-golden pending. Dev: `./install` installs the plugin locally in one command. Open housekeeping: formally close the Phase 2/3 gates. (ADR-037 ruled 2026-07-18; merged branches pruned 2026-08-03.) **This status block is historical — see the OPEN WORK REGISTER below for what is actually open.**

**Status (2026-07-18):** Phase 1 GATE CLOSED (PR #2 merged; protocol findings + free-row erratum ratified with the merge, erratum applied to ACCEPTANCE). Phase 2 in progress: SwarmCore is live in the plugin — placeholder sine replaced, 18-param CLAP surface at prototype ranges (dissolve in seconds, driftDepth in cents), versioned key=value state; pluginval strictness 10 SUCCESS, auval SUCCEEDED post-integration. Phase 2 remaining: tempo-grid law port (needs host tempo; L0-12), bimodal/clustered-pairs distributions (OPEN QUESTION — SPEC lists them, the reference doesn't implement them: extending the reference is a spec change needing a human ruling), GUI v1 + dev state button, webview smoke test, Layer-E 1/2/5 sign-off.

*(Historical status, 2026-07-17 evening:)* Phase 0 largely complete — skeleton builds (CLAP + VST3 + AUv2 via clap-wrapper, pinned submodules), pluginval SUCCESS at strictness 10 (gate asks ≥5), auval SUCCEEDED, all three formats installed locally with intact codesign seals; ADR-006 spike run (bank 66× / iFFT 216× realtime at 2560 osc on M3) with close proposed as ADR-018 (bank); GUI stack proposed as ADR-019 (choc webview). CI matrix (macOS + Windows build + pluginval) GREEN on both platforms (run for 3283ae9; Windows needed static-MSVC-runtime + M_PI portability fixes). **PHASE 0 GATE CLOSED 2026-07-17:** ADR-018 (bank), ADR-019 (webview, with the swappability amendment), and the E-6 envelope ratified by the human; Live load test passed (VST3 loads, plays sine on MIDI input — no GUI yet, as designed). **Recorded residual (human-accepted):** Reaper/Bitwig load evidence deferred — neither host is installed on this machine; CI pluginval on both platforms is the standing proxy; do a real load check when either host is available, no later than the Phase 2 gate. **Windows runtime work deferred (human, 2026-07-18):** the WebView2 backend stays CI-compile-verified only until desktop-coordination begins; Windows runtime validation moves out of the Phase 2 gate to that milestone. Phase 1 (SwarmCore port + parity oracle) is now in progress. Proposed E-6 envelope: min-spec = Apple M1 base / 4-core 2018-class Intel ultrabook, Windows x64 AVX2; 44.1 kHz @ 128-sample buffer; E-6 patch must hold < 50% of one core on min-spec. Deferred ecosystem briefs: Tonality intake brief due at Phase 3 before consonance gravity ships; terrain-sibling intake brief due at Phase 4 with the kernel abstraction (ADR-010(d) — placeholders in the meantime).

## FOUNDATIONS ANSWERED BOTH THREADS — and one answer exposed an overstatement of ours (2026-08-11)

**Stage 1 now extracts against BOTH shells.** Correction 2 accepted in full. Their reasoning went
further than ours: the two-consumer rule exists to stop generalizing from one shape, and HYPERSAW
is two shapes differing *in exactly the dimensions that would break a registry* — 105 params vs 17,
`coreKey` present vs absent, string-key vs positional dispatch. They recorded the swarmfx
divergence as *"the strongest single argument in this project's file for why §3.1 exists"*.

**The `coreKey` constraint is honoured as a constraint on them:** Stage 1 will not change what a
saved patch contains. They noted it is the one they could not have seen and would have broken.

**Our answer to their question 2 was adopted** — registry owns the address, core key **derived from
it by construction and asserted at build**. They took it for a reason we had not given: it keeps our
cores independently testable, and *"a registry that made invariant oracles harder would be a bad
trade at any price."*

**F2 gained acceptance criterion 1b** — at least one invariant oracle the library asserts
independently, needing no reference. Minimum content: **subdivision invariance and sample-rate
invariance**. The criterion carries its reason in place so a later session cannot drop it as
redundant: *it exists so F2 cannot certify a shared defect into the first frozen contract.* Our
oracle taxonomy was adopted verbatim as design input, and a **constitutional amendment to their
§2.5** is proposed to their human (parity **and** at least one reference-free invariant).

### The overstatement, and the correction

We told them the sample-rate probe was *"built, not yet gated"*. **It was never committed** — it
lived in a scratch directory, found the ADR-086 grid defect, and was discarded with the scratchpad.
"Built but not gated" implied a tool we had chosen not to gate; what existed was a measurement run
once. That matters because criterion 1b names sample-rate invariance as minimum content, and it was
the one of the two named invariants we could not actually have supplied.

**Now true:** `tools/samplerate_check.cpp` asserts that behaviour declared in seconds does not track
the sample rate (ADR-009 — which nothing had enforced), across 44.1/48/88.2/96 kHz on envelope
attack and gravity settle. Calibrated against the real regression, not a hypothetical: re-planting
ADR-086's fixed *sample-count* grid gives **0.419%** drift and FAILS; current is **0.163%** and
passes, at a 0.3% tolerance chosen from measurement to sit between them. Deliberately tight — a 1%
tolerance would have passed the defect it exists for.

Correction filed as `notice-samplerate-oracle-correction.md`, asking them to keep treating it as
evidence rather than a gate until it is actually gated here, so criterion 1b gets a date rather
than an assurance.

## coreKey CONFORMANCE DATA — the mapping is a dispatch, not a function (2026-08-11)

FOUNDATIONS named the `address.leaf() == coreKey` conformance tool as F2 Stage 1's live item and
the claim as unproven. Measured our real table rather than waiting.

**Injectivity: proven.** Zero duplicate coreKeys across all 105 instrument params.

**Totality: the claim was mis-shaped.** `coreKey` does not name one owner —

| owner | params |
|---|---|
| SwarmCore only | 44 |
| shell-domain only (reaches NO core) | 16 |
| SpectraCore only | 15 |
| FX rack | 8 |
| multi-owner combinations | 11 |
| unresolved (`fx1tone`…`fx4tone`) | 4 |

**16 keys reach no core at all** — shell state whose `coreKey` exists purely to be the state wire
format. A tool asserting "every coreKey is a valid core key" fails on 16 params that are working
correctly. And 11 keys deliberately fan to *two* engines (`width` → SwarmCore `width` + SPECTRA
`swidth`), so an address scheme must express "this address fans to these targets" rather than
assuming one destination.

### Three param-map idioms, not two

`k == "x"` (swarm_core) · `eq(k, "x")` (some cores) · `std::strcmp(k, "x")` (spectra_core).

Our friction list said *two* idioms made a scope audit silently report 0 findings. It is three, and
**the first pass at the table above — grepping one idiom — reported 27 params as UNCLAIMED that are
correctly owned by SPECTRA and the rack.** The analysis reproduced the exact defect it was
documenting. A conformance tool that scans source rather than the registry will under-report and
look green; that is a stronger argument for Stage 1's registry than the friction-list version was.

### Correction we owed them

Our repeated "N files awaiting their resident to commit" was **wrong** — their outbound side read
as our inbound debt. Verified independently: nothing uncommitted either direction, all nine of ours
in `origin/main`, split 9/10 exactly as they said.

Cause worth naming: we had a *convention* to report carry-state and no *derivation* for it, so a
claim formed once from a governor signal about a different repo survived every repetition. Same
failure their new `check_inbound_uncommitted` fixes on their side. The roundup memory now says:
derive it from the tree, or omit the line.
## OPEN WORK — the CLAP param-rescan host measurement (tracked here, 2026-08-11)

Moved out of `integrations/` and into the roadmap, where work belongs. The FOUNDATIONS exchange is
**closed** — they accepted, we agreed, and the ball had been "our schedule" for days; leaving it
open made an unscheduled task look like an unanswered question, which is exactly the confusion that
produced a week of phantom-debt status lines.

**What it is.** Six cases per host, run through the **legal** cycle
(`restart()` → `deactivate()` → apply → `clear(host, id, CLAP_PARAM_CLEAR_ALL)` → `rescan(ALL)` →
`activate()`), never a mid-session `RESCAN_ALL` — that call is illegal per `params.h:328`, and
measuring it could have produced "hosts do not support dynamic params" and foreclosed a flow the
spec documents at `params.h:70-77`.

| case | question |
|---|---|
| id unchanged | does an existing automation lane keep its points and its binding? |
| id added | does a new id inherit lane state from a previous instance if `clear()` was skipped? |
| id removed | does the lane disappear cleanly, orphan, or corrupt the project? |
| id **reused** for a different param | the one that decides whether append-only is a rule or a convention |
| each of the above, **after a project reload** | surviving in-session and surviving a reload are different promises |
| the **clap-wrapper VST3 path** | most hosts meet us through the wrapper, whose parameter model is not CLAP's |

**Priority order if scope must be cut: drop a host before dropping the wrapper row** (their
instruction, and right — a clean CLAP answer that dies at the wrapper answers nobody's question).

**Blocked on a human at a DAW.** The observation step cannot be agent-run. What *can* be built
without one is the instrument: a plugin that changes its exposed parameter set on command through
that cycle, so the manual pass is clicking and reading rather than building. Offered, not promised.

**Nothing depends on it.** ADR-088 §4 is ratified on the specification; this measurement only
decides whether a *future* rack could use dynamic params instead of a static block.

## F2 STAGE 1 INCREMENT 2 — both conformance reports GREEN (2026-08-11)

FOUNDATIONS shipped `registry_conformance`; we ran it on both real tables. **hypersaw 181 rows**
(105 base + 76 per-oscillator copies; 29 globals not duplicated) and **swarmfx 17** — C1 arity, C2
invariants, C3 `address.leaf() == key`, C4 wire format byte-identical to today's. All green.

**Emitter A, and the trap had a second door.** `patch_key` comes from the bytes `state_save`
actually writes. But our first emitter derived `key` by stripping the `o<k>.` prefix off
`patch_key` — the same tautology one level down, since C4 would then compare their reconstruction
against a string built from the column it was checking. Columns are now independently sourced:
behaviour for `patch_key`, declaration (`tools/registry_decl.py`) for `key`/`global`/ranges.

**Their C2 fired on our real data and was right.** First honest run: RED, *"leaf shadows an
ancestor scope — id 1002, address `osc1.dist`"*. Our declaration parser ran `\d+` over the
`kGlobalIds` block *including comments*, slurping digits from `A12`, `ADR-082`, `2026-08-11` → 36
globals where there are 29, declaring `dist` **both** global and per-oscillator, which really would
collide on load. That is the case they said they most wanted and could not predict, and it was
**calibrated on real data by accident** — fired on a table that contained the condition, green once
removed. Three deliberate plants also reported precisely: `.` in a key → C3, one param
mis-classified → C4 naming id 1001, truncated dump → C1.

swarmfx names are **proposed**, matching HYPERSAW's existing `coreKey` wherever the concept exists;
C3 there is a forward constraint only, since that shell keys state on the numeric id.

## A12 SHIPPED — and it uncovered a third fan-out bug (2026-08-11)

Implementing the ratified A12 scope changes required touching `kGlobalIds`, and checking *how*
global params reach the cores first found a live, audible defect.

### The bug: "global" meant "oscillator 1's"

`applyParam` routed every core param through `cores[oscOfId(id)]`. `oscOfId()` returns **0 for
every global id**, so a global core param was written into oscillator 0 **and nowhere else**.

**Measured before the fix:** with the Attack knob at 1.5 s, oscillator 1 reached 90% at **0.955 s**
while oscillator 2 sat at **0.007 s** — its compiled-in default. Control at the default: both
0.007 s, so the rig was sound. Every global core param behaved that way, so a two-oscillator patch
was half-configured and the second half silently ignored the panel.

**Third instance of L0028's shape** — an operation whose intent is "every oscillator" written
against one — after the note/lifecycle fan-out (PR #242) and pan motion (ADR-086). The word
*global* in `kGlobalIds` means "not per-oscillator addressable"; the *application* then quietly
made it mean "oscillator 0's".

### A12 applied

Amp envelope (19–22) and beatMult (23) left `kGlobalIds` and are now per-oscillator, as ratified.
Inert by construction: identical defaults mean oscillator 2 behaves exactly as before until the new
ids are touched.

### `paramscope_check`, gated

Two assertions, and **neither is meaningful alone**: a global param must reach every oscillator, and
a per-oscillator param must reach **only** its own. "Fan everything to everything" satisfies the
first perfectly while destroying addressing, so the second is the vacuity control.

Calibrated: reverting the fan-out gives `|L-R| mono 0.00000/**0.06737**` — oscillator 2 unfolded —
and FAILS.

### The probe was order-dependent, and that is a finding about the plugin

The per-oscillator assertion read **0.955 s standalone and 0.034 s** when four unrelated renders ran
first. Re-ordering made it pass, which is luck rather than a fix. Cause: **`plug_reset()` clears
gates and MPE bend but does not restore parameter values or core internals**, so scenarios run
back-to-back in one instance contaminate each other — the same confound that defeated three
oscillator-drift probes on 2026-08-09.

Every measurement now builds a **fresh plugin instance**, which makes the suite order-independent
*by construction* rather than by arrangement. Verified: both orderings produce byte-identical
numbers, and the two oscillators' `|L-R|` now match exactly (0.06737/0.06737) where contamination
had made them differ.

Worth carrying to F2: a registry extraction will need to know what `reset` actually restores, and
today the answer is "less than its name implies".

## A12 / A13 RECOMMENDATIONS (2026-08-11)

Both asked for by the human. Grounded in one principle rather than taste: **a parameter's scope
follows the thing it describes.** A property of a SOUND SOURCE is per-oscillator; a property of the
PERFORMANCE or the patch is global.

### A12 — which core-owned params go per-oscillator

| param(s) | recommend | why |
|---|---|---|
| **amp envelope** (19 attack · 20 decay · 21 sustain · 22 release) | **PER-OSC** | the strongest case on the list. A fast-attack oscillator layered against a slow swell is among the most basic two-oscillator moves there is, and it is per-oscillator in essentially every synth that has two. Sharing one envelope makes the second oscillator a timbre-only layer. |
| **voiceMono** (32) · **voiceLegato** (34) · **polyGlide** (89) · **glideMode** (90) | **GLOBAL — structurally** | these describe how NOTES ARE ALLOCATED, not how a source sounds. Two oscillators cannot be mono and poly at once: a note either exists or it does not. This is forced, not preferred. |
| **travel-law family** (33 glide · 11 inertia · 70 inertiaCurve · 75 freqGlide) | **DEFER to B19** | one oscillator snapping while the other slides is genuinely musical, and A1 already ruled per-destination laws linked by default. But B19's shell integration has not landed, and scoping a family before its owner exists is how the first 13 params got mis-scoped. Decide it *with* B19, not before. |
| **oversample** | **GLOBAL** | render quality is patch-level. |
| **beatMult** (23) | **PER-OSC — recommendation CORRECTED 2026-08-11** | see below. |

**Cost:** additive only, and only while the `+1000` ids stay unallocated — which they do. The
envelope move is 4 ids, beatMult 1 more.

#### beatMult — a correction, and why the first answer was wrong

Originally recommended GLOBAL, "because tempo relationship is patch-level". That was
pattern-matching on the word *tempo* without reading what the parameter does, and the human asking
*"what is beatmult though?"* is what exposed it.

**What it actually is.** `beatMult` ("Grid Cycles/Beat", 0.25–8.0) is a parameter **of the
tempo-grid detune law** (law 3, ADR-022). Under that law each voice's frequency offset is snapped
to a multiple of `u = (bpm/60) × beatMult`. Because the beat rate between two detuned voices *is*
their frequency difference in Hz, snapping every offset to a multiple of `u` makes **every pairwise
beat rate an exact multiple of `u`** — the swarm's shimmer becomes tempo-locked pulsation instead
of arbitrary drift. At 120 bpm, beatMult 1 is one beat-cycle per beat; 2 is eighths; 0.25 is a
bar-long swell.

**Why the scope flips.** `detune` (4) and `law` (5) are already **per-oscillator**. `beatMult` is a
parameter *of that law*, so today an oscillator can choose the tempo-grid law independently but
cannot choose its own grid. The genuinely global quantity here is **`bpm`**, which is host-owned
transport and correctly global; `beatMult` is the per-source *ratio* to it.

**And the musical case is the one this instrument exists for:** two oscillators both on the
tempo-grid law at different divisions — one pulsing quarters, one eighths — is a polyrhythmic
shimmer. Forcing them onto one grid deletes it for no reason.

By the same principle used for everything else in this table — *scope follows the thing the
parameter describes* — beatMult describes how **this oscillator's** detune relates to the beat.
**Per-oscillator.**

### A13 — retrig-off dead starts

**Recommend: document and expose. Do not change the physics.**

1. It is **reference behaviour**, not a port defect — the reference shows the identical 5/20. Our
   correctness definition is parity, so changing it is a spec change against a protected path.
2. **Retrigger-off exists to preserve phase continuity.** A dead start is the honest consequence of
   that choice; removing it partially defeats the feature the user asked for.
3. But 25% is high enough that it will be reported as a bug, so the fix is not silence — it is
   making the trade visible at the control, so "off" reads as *phase-continuous, occasionally quiet
   onset* rather than as breakage.

**If it is ever fixed, use the rotated even spread, not the anti-null redraw.** A redraw-until-not-null
is a rejection sampler: seed-dependent in a fragile way, unbounded in principle, and it makes
"same seed → identical output" depend on how many draws were rejected. A deterministic rotated
spread eliminates clustering while keeping the charter's determinism invariant intact by
construction. The cheap-looking option is the one that endangers the invariant.

### On A2 (swarmalator)

**Not an open question.** Tabled by human ruling 2026-08-06/07 and it has been surfacing in status
roundups as though awaiting a decision — twice the human has had to ask why it was mentioned. The
ROADMAP row now says so explicitly and instructs that it not be listed. Its core and gated oracle
stay unwired, which is the correct resting state, not a pending task.

## FOUNDATIONS THREADS CLOSED — and we walked into the bug we had reported (2026-08-11)

FOUNDATIONS reported waiting on three of ours. Two were answerable immediately and one is real work.

- **`signal-graph`** — we ratified ADR-088 two days ago and never told them. Closed with the
  ratification: topology and id block both accepted, their response cited only for what it removed
  (the retrofit risk), per their own request not to be treated as design input.
- **`oq15-clap-rescan`** — they asked whether to promote the host measurement to an F2 blocker.
  **Answered no**, with reasoning: §4 is ratified on the *specification*, so nothing here waits; what
  the measurement changes is a FUTURE option (params that exist only when their rack does) for a
  second rack that does not exist; and it would make a phase wait on a human-run, macOS-local,
  per-host manual test. Better spent on criterion 1b, which we can supply.
- **`rescan-spike`** — genuinely ours, unscheduled. Reported honestly that the observation step
  **needs a human at a DAW** and cannot be agent-run; offered to build the instrument (a plugin that
  changes its exposed param set through the legal cycle) so the manual pass is clicking and reading.

### We reproduced the exact defect we had briefed `autonomous` about

Filing `status: answered` left the thread open — *"answered"* is not in the scanner's terminal set.
Retrying with `status: closed — <reason>` **also** left it open. Only bare `status: closed` worked,
because the test is `status in TERMINAL`, an **exact string match**.

That is a third facet, and it is worse than the one we reported: **a status beginning with a
terminal keyword and adding a clause is silently non-terminal** — which is the most natural thing an
author writes, and the corpus is full of decorated statuses. Two of our own threads stayed open
through two deliberate attempts to close them, with no feedback of any kind.

It also sharpens the recommended fix in our brief: matching the **leading token** rather than the
whole string would close every historical thread that already means to be closed — including
`tonality-live-001-ratify`'s `ratified-with-refinements` — with no re-filing anywhere. Brief updated.

**Fleet is now 0 overdue.** The only HYPERSAW thread still open is the rescan measurement, which is
outstanding work rather than an unanswered question.

## PANIC PAYS ITS END DEBTS — the third gap closed (2026-08-11)

The last of the three gaps our own answer to FOUNDATIONS' seam question 4 exposed. **All three
closed; Stage 4's re-point was recorded on their side as gated on exactly these.**

`panicWithDump()` did `pendingEndCount = 0` and cleared every tag directly, **destroying every
NOTE_END the host was owed.** A host tracking `note_id`s was left holding identities that never end,
unrecoverably — the tag carrying the identity was already gone.

Same class as L0022 (an END obligation destroyed rather than delivered), reached through a different
door: there the host REFUSED the push and the tag was retired anyway; here the tag was dropped before
a push was ever attempted.

**It was invisible to every gate, and would have stayed invisible, because the AUDIO is correct
either way** — the notes do stop. Only the host's bookkeeping is corrupted. No listening test finds
this; no parity scenario touches it. It surfaced only because FOUNDATIONS asked which END cases their
seam had not modeled, and answering honestly required reading the function.

**Fix:** `for (int i = 0; i < kPoly; i++) retireTag(i);` before the clear. `retireTag` moves each
active tag into `pendingEnds` (respecting its cap) and clears `active`, so the blanket clear it
replaced was redundant as well as wrong. `emitNoteEnds` then delivers them with L0022's try_push
retry.

**Gated** in `endprobe`: hold four notes, panic, assert four ENDs arrive — with the control that
nothing leaked while the keys were still held, since "4 ENDs after panic" could otherwise be four the
hold itself emitted. **Calibrated:** restoring the old discard gives `0 delivered after panic` and
takes endprobe RED.

`./verify full` GREEN, eighteen gates; parity 147/147 worst 4.262e-09.

## STEAL PRIORITY PINNED · endprobe GATED (2026-08-11)

Two of the three gaps our own answer to FOUNDATIONS' seam question 4 exposed. Eighteen gates now.

### `steal_check` — WHICH voice dies

Nothing pinned it. `notefuzz_check` proves no voice *hangs*, and proves it whether the victim is the
oldest, the newest, or picked at random — **a seam that changed steal order would have left all
sixteen gates green.** This pins all three `alloc()` tiers as behaviour: a free slot is used before
anything is stolen; a releasing tail is taken before any held note; only when every slot is gated
does the oldest held note die.

**Two probe defects found before any code defect** — both caught by refusing a marginal result:

1. The 17th note was MIDI 59, *below* the measured octave, so its 2nd harmonic landed exactly on
   MIDI 71 — a note the probe counts as a survivor. It would have read the intruder as proof the
   victim's neighbour lived. Moved above the range; harmonics only go up.
2. Assertion 2 left release at 5 ms and idled 46 ms before stealing, so the "releasing tail" had
   already faded below the free threshold and the slot was tier-1 FREE. **The assertion was passing
   through the wrong tier and would have stayed green with tier 2 deleted.** Release stretched to
   800 ms.

**Thresholds are measured, not chosen.** A silenced bin never reads zero — neighbours leak into it —
so the floor comes from a render where the note genuinely never sounded. The first version guessed 5%
and got 5.1%, which L0024 says means the detector is wrong: the Goertzel was unwindowed, and a
rectangular window's sidelobes put ~5% of a neighbour 15.6 Hz away into the victim's bin. Hann-
windowed, the victim reads **0.00579 against a 0.00716 floor** — below the floor, i.e. genuinely gone.

**Calibrated, and the tiers fail separately.** Stealing the newest fails assertion 1 only (victim
stays at 0.125). Deleting tier 2 fails assertion 2 only, *inverted*: the held note collapses to
**0.00493** while the released tail rings on at **0.09051** — a note the player is holding dies while
a decaying tail survives. That is the exact musical harm we described to FOUNDATIONS in seam answer 1,
now demonstrated rather than argued.

### `endprobe` wired

Built and calibrated for L0022 — where `emitNoteEnds` ignored `try_push`'s return and destroyed a
NOTE_END forever under output-buffer pressure, after four rounds of wrong fixes. It has been outside
the gate set ever since. **A built, passing probe that nothing runs is worse than an absent one,
because its existence reads as coverage.**

`./verify full` GREEN, eighteen gates; parity 147/147 worst 4.262e-09.

### Still open from seam question 4

The panic-END defect: `panicWithDump()` zeroes `pendingEndCount` and clears every tag without
emitting the ENDs it owed, so a host tracking `note_id`s is left holding identities that never end.
Same class as L0022, different door. Next.

## PANIC-ORDERING BOUNDARY CLOSED (2026-08-11)

The coverage boundary recorded one commit earlier is now a gate. `panicWithDump()` extracted from the
GUI lambda so the ordering is reachable headlessly; `trace_check` assertion 5 drives the real panic
path and asserts BOTH halves — the dump sees the gated voices (capture happened first) and the synth
is silent afterwards (the clear still happened).

**Calibrated by swapping the two statements:** 0 gated voices in the dump, with the post-panic peak
UNCHANGED at 3.34e-05 — so the assertion isolates the ordering specifically, not the clearing.

**The probe corrected an assumption of mine.** The first version asserted silence two blocks after
panic and failed at 0.287. That is not a failed clear: panic RELEASES voices (`allOff`), it does not
hard-mute them, and a panic that truncated the envelope would click. Measured past the tail instead.
Asserting instant silence would have been asserting a behaviour the synth does not have and should
not.

Recording a boundary is the honest move when it cannot be closed. It is not a substitute for closing
it when it can.

`./verify full` GREEN, sixteen gates.

## FORENSIC NOTE TRACE — FOUNDATIONS ask (c) closed (2026-08-11)

The last open item from their stuck-notes brief. **Capture instead of simulate:** a fuzzer emits the
event stream it *imagines*, and ours deliberately excludes shapes no host can produce
(`notefuzz_check.cpp:14-17`), so it can never model a stream the host actually delivered. The
stuck-note bug survived weeks on exactly that gap.

A 512-entry ring records every note event (type, key, note_id, channel, port, absolute sample
position). Written from the audio thread as plain stores plus one release store — no allocation, no
lock, no wall-clock; `rtsafety_probe` stays green over block sizes 33..2048. On panic the GUI thread
writes the ring plus the live per-core voice tables and `slotOf` to a file under
`~/Library/Logs/HYPERSAW/`, path derived at runtime (never baked in — a machine-absolute path in a
tracked file is both an identity leak and wrong on any other machine).

**The dump runs BEFORE panic clears state.** That ordering is the whole feature: a dump taken after
the clear faithfully records a synth in perfect health and proves nothing.

### Gated, with controls (`trace_check`, sixteenth gate)

Driven through the real plugin via a test hook, not a reimplementation of the ring — a check that
rebuilds its own subject spans the wrong layer (L0031-B3). Every positive assertion is paired with a
negative: a dump is a text file full of plausible lines, and "the key is in the file" is satisfied by
a file that mentions every key.

| assertion | control |
|---|---|
| every event captured in order with `note_id` | virgin plugin dumps **0 rows, 0 gated** |
| voice table shows a gated voice while held | **none** after release + decay |
| ring keeps the newest 512, drops the rest | oldest `note_id` verified **absent** |

**Calibrated.** Dropping `note_id` fails assertions 2 and 4; clamping the ring index instead of
masking fails only 4, reporting "oldest dropped=no" — exactly what a clamp does. **The plants must
force a rebuild:** plant B first reported plant A's failures verbatim because the impl object was
stale. Asserting a plant's ANCHOR proves the source changed, never that the binary did — a distinct
trap from L0032's unasserted-replace, and worth its own note.

**Known coverage boundary, recorded not retried (L0033):** `trace_check` calls `dumpForensics()`
directly, so it never exercises `hostIf.panic`'s dump-before-clear ordering. Nothing would catch its
reversal. Covering it needs the GUI bridge in the harness; until then that guarantee is prose at the
panic site, not a gate.

**Interface note for the human:** `hypersaw_test_dump_forensics` was ADDED to
`src/hypersaw_clap_entry.h`. Additive and test-only — no existing signature changed, not reachable
from the CLAP surface — but it is an addition to the impl↔entry interface and is flagged rather than
slipped in.

`./verify full` GREEN, sixteen gates; parity 147/147 worst 4.262e-09.

## STUCK NOTES — FOUND, REPRODUCED, FIXED (2026-08-11)

The intermittent "notes don't die on key release" report is **confirmed, deterministic, and fixed.**
FOUNDATIONS' brief (`integrations/hypersaw/brief-stuck-notes-oracle-blindness.md`) called both the
oracle blindness and the mechanism before either was measured.

### Why nothing caught it for weeks

`notefuzz_check` gates on **rendered audio**, and the plugin constructor leaves oscillator 2 at
`vol = 0` (`hypersaw_clap.cpp:444-450`). `vol` is per-oscillator by our own A12 ruling, so raising it
needs id **1017**, which notefuzz never sent. **A voice stuck in oscillator 2 renders exactly
nothing.** The oracle was structurally incapable of failing on the entire class of hang that requires
two oscillators to exist — the class `kNumOsc = 2` introduced. Every green run said nothing about
oscillator 2. This is the corpus ruling ("parity is structurally blind to every defect that needs two
oscillators to exist") coming true one layer down, in the oracle we relied on for exactly this bug.

### The defect

`slotOf` was a **convention, not a construction**. The comment said "note fan-out keeps slot indices
aligned" (`hypersaw_clap.cpp:1421-1423`); nothing enforced it, and it is false. `alloc()`'s tiers 1
and 2 read `s.env`, and the amp envelope is **per-oscillator (A12)** — so once two cores' envelopes
differ, their release tails fade on different schedules, the same note lands on **different slots**,
and `retargetAll`/`setNoteExprAll`/`setNotePressureAll` (all indexed by oscillator 0's slot) hit the
**wrong voice** in core k. The real voice is orphaned: still gated, under a key whose note-off has
already been and gone. Only panic clears it.

Matches every symptom: intermittent (depends on tail states, so on how fast you play), computer
keyboard not piano roll (fast irregular playing forces the lower alloc tiers), unreproducible in
simulation (the oracle muted the oscillator the orphan lives in), and recent (`kNumOsc = 2` shipped
with ADR-082 increment 2).

### Measurement, with the control that makes it mean something

| mode | oscillator 2 | envelopes | result |
|---|---|---|---|
| `mono+2osc-same` | audible | **matched** | GREEN, 41 ms tail |
| `mono+2osc` | audible | **diverged** | **hang, peak 0.4519, 1498 ms** |
| `mono+legato+2osc` | audible | diverged | **hang, peak 0.4519** |

The matched-envelope control is what rules out "raising the volume caused it" and isolates envelope
divergence as the cause. Without it this is a probe confirming what it expected (L0032).

### The fix

`slotOf[s][k]` — core k's slot for the logical voice oscillator 0 holds at slot s — recorded at
note-on (the only place a core allocates) and used by every fan-out helper. Identity-initialised, so
an unbound slot degrades to exactly the old behaviour rather than to garbage: the map corrects an
assumption, so its unset state must **be** that assumption.

`./verify full` GREEN, all fifteen gates, with six new two-oscillator notefuzz modes and three
controls. The gate is calibrated by construction — it was RED before the fix and GREEN after,
same binary, same seeds.

**FOUNDATIONS Stage 2 gets its answer:** §2 is CONFIRMED, with a reproduction. "One logical note maps
to N physical voices" is now a construction here, and a library voice allocator whose identity
survives independent per-engine allocation is the right extraction — measured, not plausible.

### Still open from their brief

Ask (c), the forensic ring buffer on panic, is **not done**. It pays regardless of who was right and
turns future unreproducible field reports into replayable ones. Queued, not dropped.

## B23 INCREMENT 2 — the matrix is in the audio path, and inert (2026-08-11)

`RoutingMatrix<1, kRackSlots>` now drives the FX rack in `process()`; `rack.processStereo` is gone
from the shell. Three parts:

1. **`processSlot(idx, L, R, n)`** extracted from `processStereo` in `src/fx_rack.h` — pure
   mechanical refactor, switch body untouched, so the matrix has a block-stereo slot to call.
2. **`processBlock()`** added to `src/routing_core.h`: the same topology through the same
   predicates, for slots that a scalar callable cannot express (a compressor detects on both
   channels; a comb needs contiguous samples). Scratch is caller-owned — the audio thread allocates
   nothing. It lives in the core, not the shell, so the shell never owns a second copy of "which
   edges are live"; that duplication is the routing lab's actual bug.
3. **Shell wiring** with fixed stack scratch and a `kMixChunk` loop, matching the oscillator sum
   and for the same recorded reason: a heap buffer sized at `activate()` once made audible output
   conditional on `activate()` having run.

**A default-constructed matrix connects nothing, which means outAmount is 0 everywhere — silence.**
That is the worst direction for an init slip to fail, so `RoutingMatrix()` now establishes the
serial chain and the zero state is reachable only by asking for it.

### Bass-mono stays upstream — the reorder was measured and refused

Per-oscillator sources would force bass-mono downstream of the rack (a mid/side fold on the *sum*
does not decompose per-source). The argument for moving it was that a decorrelating slot could undo
the mono guarantee. **Measured, and refuted:** Comb at amount 0.9 scales the sub-crossover channel
difference by 2.2x whether bass-mono is on or off — residual 10.6% vs 11.4% — because it is a
stereo-*symmetric* filter. No current slot type decorrelates, so there is no correctness case, and
an audible reorder with no oracle behind it is not one to make on taste. **This increment therefore
carries one source (the summed, post-bass-mono bus).** Per-oscillator sources are a later increment
and carry this ordering question as their own decision.

(The first probe reported this backwards: a one-pole at 200 Hz is only 6 dB/oct, so its "low end"
was full of above-crossover content that bass-mono is *supposed* to leave stereo, and it failed a
correctly-working crossover. Isolating the band with a Goertzel at the note fundamental gave the
real answer. Fifth instance of L0032 — the detector shared an assumption with what it measured.)

### The goldens cannot see this, so it needed its own assertion

The 147 parity scenarios render `SwarmCore` directly; the whole plugin mix stage — bass-mono, rack,
master volume — is downstream of everything they cover. Reading a green parity run as evidence for a
mix-stage refactor would be assuming exactly the coverage that does not exist (L0031). So
`routing_check` gained assertion 8: the serial-chain block pass equals `rack.processStereo` **sample
for sample**, with all four slots active and distinct (an all-Off rack would pass trivially by
touching nothing). Result **0/1024 samples differ**, reference energy 172.3.

**Calibration — and the useful result is the plant that did not fire.** Fires at 1023/1024: a gather
coefficient off by 1e-6; zeroing the output buffer before the slots gather (the aliasing hazard, since
the shell passes the mix bus as both source and destination). **No-op:** removing the `isTerminal`
filter, because `setSerialChain` leaves `outAmount = 0` on every non-terminal, so summing them adds
zeros. Assertion 8 does not cover terminal detection at all — assertion 4 does, and that division is
recorded rather than left to be assumed the other way.

`./verify full` GREEN, all fifteen gates; parity 147/147 worst 4.262e-09; `rtsafety_probe` clean over
block sizes 33..2048 (up to eight chunks per call) with the new stack scratch.

## B23 INCREMENT 1 — routing core + oracle, not yet in the audio path (2026-08-10)

Topology and ids both ratified (ADR-088), so the build began — in the order this project has twice
proven: core plus oracle first, shell integration onto proven ground second (glide, swarmalator).

`src/routing_core.h` — framework-free crosspoint matrix. Dense coefficients, per-slot **initial
value** (`out_i = in_i + Σ g·m`, the canonical form the lab lacked), acyclicity enforced **on the
read side** through one named `edgeLive()` that every consumer calls, because the writer set is
open (preset load, morph, automation) and a guard at the write sites is bypassable by construction.
**FX-agnostic**: the caller supplies slot processing through a callable, so the matrix owns topology
and nothing else — which is what lets the oracle measure routing rather than an effect, and is the
shape that transfers.

`tools/routing_check.cpp` — 7 invariant assertions, green. Deliberately NOT parity against the lab:
scheme C carries toy effects, so sample parity would mostly measure those.

**Calibration returned two different answers, and only one is a success.**
- Removing the read-side guard makes assertion 3 **FAIL** (1 → 5). Load-bearing; the oracle catches
  its loss.
- Planting the lab's *other* bug — a terminal test that skips the legality check — is a **NO-OP**
  here. `isTerminal` loops `t > slot`, so illegal destinations are excluded by the **loop bound**,
  not the check; the bug is not expressible against this shape. Recorded as a finding about the
  design rather than counted as a second calibration, which is what it would have looked like from
  outside.

**Build hazard.** CMake did not track `src/routing_core.h` as a dependency of `routing_check`, so
the first calibration read a **stale binary** and reported two identical failures that were one
failure twice, plus a "restored" run still red. Plant/restore cycles here must delete the object
file rather than trust the incremental build — same family as L0032's four detector traps.

**Not yet wired.** The rack is still a fixed serial chain (`fx_rack.h::processStereo`). Increment 2
is the shell: separate oscillator buses into the matrix, per-slot buffers, terminals summed to
master, with `setSerialChain()` as the default so it lands inert and the goldens stay the regression
proof. `routing_check` built but **NOT gated** (`./verify` is protected).

## MOD LAB REOPENED — morph×mod built, and the matrix was dead (2026-08-05)

**Found first: the mod lab's matrix had not been rendering at all.** `wire('rN', …)`
invokes its callback during setup and that callback calls `rebuildMatrixRows()`, which
touches `mtx` — declared ~3700 characters further down as a `const`. The script died in
the temporal dead zone every load, so the entire matrix, the A/B buttons and everything
after them never existed. **Pre-existing** (the call precedes the declaration in the
original file too) and invisible, because the rest of the page renders fine. **Fourth
instance of L0026 in this project**, which is the evidence for that lesson's falsifier:
the fix is tooling (`no-use-before-define`), not care — knowing about the trap has now
failed to prevent it four times. Fixed by hoisting the handle above the wiring; matrix
now builds 13 rows × 108 cells.

**Morph × mod now exists in code.** It was specced ("modulate where you stand in the
morph field") and never built; `mod-lab.html` had zero morph references.
- **morphX / morphY are destinations** — route any source at the field position and the
  morph moves under modulation. The field draws both the base position and the live
  modulated one, joined by a line.
- **Every routing has a SCOPE**, per the human's ruling, cycled from a chip under each
  matrix cell and coloured by the global corner vocabulary: *system-wide* (neutral,
  survives every flip and reshuffle), *corner-owned* A/B/C/D (wears that corner's hue and
  glyph, and its depth blends with the corner's field weight — measured 1.00 at its own
  corner, 0.00 at the opposite one), or *morph-owned* (flips: live only while its drawn
  owner holds the slot).
- **Hysteresis added** — the missing third control against flip chatter. Measured over a
  2 s, 3 Hz sweep across the field's middle: **604 flips at hysteresis 0 → 437 at 0.12 →
  189 at 0.5**. A flips/second readout labels the regime (*flipping* / *chattering*) so
  the artifact is visible while you audition whether you want it.

**Still open in the lab** (the questions the human flagged): whether scope is per-routing
or per-source; what a corner-owned routing does when its corner owns nothing; and the
priority rule when a corner-owned and a system-wide routing hit the same destination
(currently they simply sum).

## FULL MOD-MATRIX SWEEP — the crash fix was half a fix (2026-08-05)

Human ask after the chorus crash: *"run a full deterministic probe of all mod connections
to make sure there aren't other similar issues out there."* Built as
`tools/labharness/modlab_sweep.mjs` — all **12 sources × 9 destinations × 2 polarities =
216 routings**, each from a FRESH engine, checked for non-finite output, watchdog fires,
level blow-ups, and dead routings.

**Finding 1 — the intermittent loud transients were the SAME bug, not a second one.**
The crash fix wrapped the delay-line read INDEX (`if (i0 >= len) i0 -= len`) but still
derived `frac` from the un-wrapped `rd`. So the exactly-`len` case no longer produced a
NaN — it produced `i0 = 0` with **`frac = 8192`**, and the interpolator extrapolated by
8192×. Captured live at the failing sample: two neighbours of `-0.1359` and `-0.1321`
gave `v = 30.7`, and the stage output hit **8.99 against a synth peak of 0.49**. Fixed by
wrapping `rd` itself before it is used for either purpose, so index and fraction can never
disagree. All 6 level blow-ups (every one a `choDep` routing) went to **zero**.

**Finding 2 — the same bug class exists in shipping C++, with worse consequences.**
`src/time_core.h` has four fractional-delay reads that wrap `i1` but never `i0`; a `rp`
of `-1e-13` becomes `kBuf - 1e-13`, which is inside the ulp of `kBuf` (2.9e-11 at 1<<17)
and rounds to exactly `kBuf`. In JS that is a NaN; in C++ it is an **out-of-bounds read on
the audio thread**. Guarded at all four sites; `./verify full` GREEN, worst time parity
rms 5.6e-12 (bar: 1e-6), so the guard is inert in normal operation as intended.
`src/fx_rack.h` was checked and is safe — its comb delay is integer and `newDly` is
clamped to `[2, len-1]`, so the modulo numerator cannot go negative.

**Finding 3 — one genuinely dead routing, and it is a design question, not a bug.**
`R → Kboost` at positive depth is **exactly zero output**, bit-identical to no routing.
Two mechanisms compose: `Kboost` is half-wave rectified (`kb = 8 * Math.max(0, kbMod)`)
and the `R` source is mapped bipolar (`R * 2 - 1`). At the lab's default rotor coupling
the swarm never locks — max R measured **0.334** — so the source is always negative and
the rectifier zeroes it. It revives exactly at the phase-transition knee: dead at rotor
K=0.35, alive from K=1 (max R 0.996) or at detune 0.05 (max R 0.984). The code comment
already anticipated the uni-vs-bipolar question; the sweep gives it teeth — below the knee
it is not *halved*, it is *entirely dead*, and half the R source's range is spent on the
rectifier. **Needs a human ruling (A9), not a unilateral fix.**

## STANDING BEAT — core-library insight harvest (human, 2026-08-08)

The human is building a **core library** for all their audio projects, inspired directly by
this project's sequencing scars: signal flow first, every param modulation-ready at birth,
engines born into a multi-osc context, FX as simple ports, features horizontally portable.
**HYPERSAW is the primary insight source and this is a recurring beat.** The donor-side pull
surface is `docs/integrations/corelib-insights.md` (architecture lessons with their PRs,
portable modules by readiness, and what the library should demand of every module) — keep it
current as lessons land. Cross-repo work follows doctrine/INTEGRATIONS.md; the sibling is
aliased per ADR-014. Documentation from the human's other thread is incoming.

## VELOCITY + PRESSURE → VOLUME BY DEFAULT (ADR-084) · master octave · pitch labels (2026-08-08)

Velocity now scales each voice (linear), and MPE `PRESSURE` expressions drive a ~20 ms-smoothed
per-voice gain — both default-inert at 1.0, parity 147/147 unchanged, calibrated vel 0.5 →
0.503 and pressure 0.3 → 0.302 through the real CLAP path. SPECTRA velocity, the velocity
curve param, and a DAW check of wrapper aftertouch translation are recorded residuals in the
ADR. Also: **master octave** (gOct, id 103) added, and pitch labels unified — every strip and
the master now read octave / pitch / fine.

## VOICE STEAL FIXED — sustains survive arpeggios (ADR-083, 2026-08-08)

Human: *"if I have an arpeggio running and try to play a sustained note on top of it,
eventually its voice will be stolen."* Reproduced, mechanism measured, fixed as a **deliberate
divergence** (ADR-083): three-tier steal — free slot → quietest releasing tail → only then the
oldest held note. The reference's steal-oldest survives untouched in the prototypes; goldens
never overflow the pool, so parity (147/147) is the regression proof. Follow-up: fold the
arp-sustain scenario into `notefuzz_check` (B27).

## CHORD RETRIGGER — RESOLVED AS REFERENCE PHYSICS; design question A13 (2026-08-08)

The reported intermittent retrigger failure is **real, reproduced, and not a defect in the
port**: it requires `retrig = 0`, where voices restart at seeded-random phases, and ~10-cent
detune beating can hold a fundamental null for ≥380 ms — an audibly dead start, a fraction of
the time. The JS reference exhibits the **identical 5/20** under the same experiment, so the
C++ is faithful. Full chain (including two discarded probe generations) in
`traces/2026-08-08-retrigger-hunt.md`.

**A13 (human):** leave as spec'd free-run character / anti-null redraw (reference edit + ADR +
new goldens) / random-rotated even spread. Also confirm the patch that shows it has the
retrigger toggle off.

## GUI2 — the greenfield interface, cluster by cluster (human, 2026-08-07)

Human: *"start a branch to test a new interface and build it up a cluster of components at a
time instead of trying to build backwards from the single-oscillator layout."*

`src/gui/gui2.html`, behind a build switch: **`-DHYPERSAW_GUI2=ON`** embeds it in place of the
original (default OFF — the shipped plugin is unchanged until parity). Rules of the file, in
its header: the **plumbing ports verbatim** (effId / data-fixed / paintControl / setVizOsc —
every bug in it was paid for once); the **layout starts clean** (grid pages, never CSS
multi-columns, which cost two overlap bugs); a cluster appears **only when its engine surface
is real** — SPACE/MOD/MORPH are visibly disabled tabs, not mocks.

Increment 1: MAIN (XY, active-oscillator by construction) · MIX (both strips + master incl.
global pitch) · OSC (swarm/coupling/pitch clusters with per-osc retargeting; the osc selector
is one widget class mounted per page, all instances synced). Verified in-page: strip sends
1035 fixed; OSC-page detune retargets 1004 after selecting OSC 2 *from a different page's
selector*; XY follows. Embed verified by decoding the generated header (a `strings` check
cannot see a hex byte array — the first attempt "proved" the switch broken with the wrong
detector). `lab_load_check` now sweeps `src/gui/*.html` by default so gui2 can never
load-fail silently.

Next clusters, in the mixer-first order: ~~viz~~ **viz SHIPPED 2026-08-08** (phase circle +
K vector from the per-osc snapshot — follows the OSC tab by construction — plus the master-bus
spectrum; drawing code ported from gui.html, which ports swarmsaw's drawPhase, K-vector
smoothing intact) · FX rack + routing (B23 lab first) · MOD (matrix fold) · MORPH.

## VIZ INTERMEDIARY + XY RETARGET + GLOBAL PITCH (human, 2026-08-07)

Human: *"un-wire all the visuals from OSC 1 and create an intermediary layer that points all
visuals to the active Osc… The XY is acting kind of buggy — I seem to only have control over
the detune of Osc B but not the K value."*

Not an XY problem — the diagnosis was exactly right, twice over:
- **`publishViz` built every per-swarm visual from `core` (oscillator 0) unconditionally.**
  The intermediary is one atomic index: `vizOsc`, set by the GUI on every tab click via a new
  `hzSetVizOsc` binding; `publishViz` reads `cores[vizOsc]`. Slot indices stay aligned across
  cores because note fan-out is in-order.
- **The XY pad sent raw base ids** — `setParam(4/6)` regardless of tab. Now `effId`-routed:
  verified in-page sending 1004/1006 on OSC 2's tab, 4/6 on OSC 1's. (The reported asymmetry —
  detune seemed to work, K did not — was both axes writing osc 0 while the panel repainted.)

**Global pitch added**: `gSemi` (101, ±12 st, "Pitch") and `gFine` (102, ±100 c) on the master
strip, summed into every oscillator's tune alongside its own transpose; the wheel stays global.

**FLAG (L0023, human request): mod drive beyond the UI range.** The pitch slider exposes ±12
deliberately, but the mod matrix, when it folds into the shell, should be able to drive pitch
to **±48 st, clamped** — modulation headroom past the knob. Recorded here so the fold
implements it and it does not become an invisible widened range.

Remaining from the "un-wire" audit: the spectrum/scope rings feed from the MASTER bus
(post-mix, correct as-is); the note monitor reads the viz oscillator's gates (aligned slots).

## B24 INCREMENT 1 SHIPPED — the mixer exists (2026-08-07)

The audio context, first piece. Three changes, each calibrated:

- **`width` (14) is per-oscillator** (A12, human-ruled). Removed from `kGlobalIds`; id 1014 now
  addresses oscillator 2's copy. Measured: narrowing only osc 2's width raises L/R correlation
  0.683 → 0.917 while osc 1 stays wide.
- **`masterVol` (id 100) exists** — the first id allocated above 99 under Amendment 1's stride.
  Needed because Amendment 1 made `vol` per-oscillator, leaving NO patch-level fader at all.
  One-pole smoothed (~8 ms) with a snap-to-target so unity is exactly 1.0 and the multiply is
  skipped — every pre-mixer patch stays byte-identical. Measured: 0.5 gives rms ratio 0.500.
- **The Mix cluster in the GUI**: per-osc strips (level + width) and the master fader, as
  `data-fixed` controls that pin their exact id — a strip shows BOTH oscillators at once, which
  is precisely what the tab retargeting cannot do. Verified in-page: the OSC 2 strip sends 1017
  with OSC 1's tab selected; `setControl(1017)` paints the strip and leaves the main vol
  control untouched; the main-panel width control retargets 14 → 1014 with the tabs.

**INCREMENT 2 SHIPPED 2026-08-09 — mute/solo + meters.** `oscMute` (104) and `oscSolo` (105)
are PARAMS, per-oscillator, so automation reaches them as the human asked. Shell-owned: they
gate the mix stage and never enter SwarmCore, so the parity goldens cannot see them. Mute beats
solo; any solo anywhere silences every non-soloed oscillator; the gain is the same ~8 ms
one-pole the master fader uses (a hard 1→0 on a ringing oscillator is a click), and the
1.0-exact snap keeps an untouched patch bit-identical. `anySolo` is COMPUTED from the params
every block rather than cached — a cached flag is one more thing to forget to update. Meters
ride the existing viz push as `oscPeak[]` (an array, so a third oscillator needs no serializer
change), read PRE-master and PRE-FX because a mixer strip answers "is this strip contributing?",
and a post-master reading would go dark when the master fader was down. In the GUI, a strip
silenced by ANOTHER strip's solo is dimmed — deliberately distinct from its own M being lit, or
the mixer cannot tell you which control silenced you.

Proven by `tools/mixer_check.cpp` (built, NOT yet gated — `./verify` is protected): all five
assertions green. **The probe's first run accused the mixer wrongly** and the interval turned
out to be load-bearing — see the detector note below.

Remaining in B24: **pan** (no per-osc pan-position param exists yet — panScatter/panLayout are
image laws, not a position; the law itself is an open question, below) and the rest of the A12
ruling (mono, inertia, the amp envelope).

### Detector calibration: the interval was load-bearing (2026-08-09)

`mixer_check` distinguishes the two oscillators by transposing one and reading each fundamental
with a Goertzel. The first version used an OCTAVE and reported mute as broken: muting
oscillator 1 dropped the 880 Hz bin to 67%. That was the DETECTOR. These are sawtooth
oscillators, so oscillator 1's second harmonic lands exactly on oscillator 2's fundamental —
measured, 880 Hz baseline 0.2401 = oscillator 2's 0.1603 plus oscillator 1's second harmonic
0.0803, and 0.1607/2 = 0.0803 to three figures. Any harmonically related interval makes one bin
read both sources. Switched to a TRITONE (2^(1/2), irrational, so no harmonic of either lands on
the other): baseline 0.1599/0.1599, and mute leaves the other oscillator at 100.4%. L0016/L0017
again — calibrate the detector for the signal class before letting it accuse the code.

### OPEN — per-oscillator pan needs a ruling before it is built

Two laws, materially different instruments, and the cheap one is not obviously right:

1. **Balance at the mix stage** (shell-only, zero parity risk): attenuate the opposite channel,
   `gL = min(1, 1-pan)`, `gR = min(1, 1+pan)`. Exactly 1.0 at centre, never boosts, standard for
   a stereo source. Cost: hard-panning *deletes* the far-side voices rather than moving them —
   on a swarm whose voices are SEATED across the field, half the ensemble vanishes.
2. **Image shift in the core**: offset every voice's seat, so the whole seated field slides and
   the ensemble stays intact. Musically right for this instrument, and it composes with
   panLayout/panScatter/panCurve, which are already seat laws. Cost: touches the parity-locked
   core and needs an ADR + goldens re-measured on the reference.

Recommendation: **(2)**, because HYPERSAW's stereo image is GENERATED rather than recorded, and
(1) is a law for material that arrived stereo. But it is a protected-path change, so it is the
human's call, not a default.

## MOD MATRIX: DEPTH IS ITSELF A MOD TARGET (human, 2026-08-07)

Human: *"I also want the mod matrix to expose the secondary mod target of modulation depth per
mapping: it would be great, for instance, if I could have a high R value kick in a tempo-sync'd
down-ramp sawtooth LFO on the osc volume or the filter cutoff."*

Queued as **B26**. This is second-order modulation — each ROUTING's depth becomes a
destination, so `R → (LFO → cutoff).depth` reads exactly as the example: the LFO is always
running, and R fades its *grip* in and out. Notes for the design:

- **The lab already has the scaffolding.** A routing is a cell with a depth; making depth
  addressable means the destination list gains one entry per ACTIVE routing (per the standing
  convention, surfaced only when the routing exists — not 108 phantom rows).
- **It composes with scope**: a depth-of-depth routing should itself carry the corner/system
  scope vocabulary, and A10's per-corner depths mean the target may be four values, not one —
  the ruling needed is whether depth-of-depth addresses the *live* value or the whole cell.
- **The example needs two other queued pieces**: a down-ramp saw (the reverse-saw LFO shape,
  already absorbed into B16) and tempo sync (the same substitution q·step time and beatMult
  use). Worth landing those with it so the motivating patch is buildable on day one.
- **Chatter risk is known territory**: R crossing a threshold to enable a routing is the
  flip-chatter problem the morph hysteresis already solved — reuse that, not a new mechanism.

## STEP-GLIDE TESTED IN THE LAB + B25 SCALING RULES (2026-08-07)

### Time-gated quantise is in the bend lab (the human's step-glide, testable now)

Three new controls in `bend-lab.html`: **quantise** (off / chromatic / major scale),
**q·hysteresis**, and **q·step time** (0 = free; tempo-sync replaces the ms value at fold time,
the same substitution `beatMult` makes). The gate holds the previous step until it elapses —
the law's dynamics run untouched underneath, only the *emission* is gated, so spring +
gated-quantise still lands its overshoot wobble on the grid. The timer arms on reset so a
gesture's FIRST step is never delayed (gating the onset just reads as latency).

Measured (constant-rate 12 st/s, major scale, one octave): free = 7 steps at the law's natural
~146 ms pace; qTime 120 barely bites (the gate is faster than the law); **qTime 250 = 4 steps
at 250.3 ms apart** — the gate paces only when slower than the law's own step rate, which is
the right behaviour for a musical increment control.

Goldens bit-identical (`glide_check` parity rms unchanged) — `qTime` defaults 0 = the old free
path. The C++ fold of the gate waits for the glide-module shell work (three lines once B19's
wiring lands).

**Extractor break worth recording:** adding three lines to the lab's `P` literal broke
`extract_glide.mjs` and took `./verify full` red — its helper slice kept "indented lines after
line 156", a magic index its own comment claimed it did not use. Now located by content
(`const osFromZeta` → class end). A magic number in an extractor is a delayed break.

### B25 scaling rules for clamped ranges (recommendation, per the human's ask)

Hand-tailored per family, as anticipated. Three rules:

1. **Multiplicative in the log domain, with a per-family sensitivity weight.** `t' = t · N^w`,
   w hand-tuned: envelopes 1.0, glide ~0.5, driftRate ~0.3 (drift is character more than time).
   Ratios inside a family are preserved exactly; families differ in how hard the macro pulls
   them — the hand-tailoring is one number each.
2. **Clamp-and-show, never clamp-and-hide.** A pinned param displays an at-limit marker while
   the macro keeps its position. The macro must never silently stop affecting a control — the
   pre-divided-headroom alternative warps the macro's feel for every other param and is worse.
3. **Where a cap is taste rather than physics, widen the range instead of engineering around
   it.** `freqGlide`'s 0.1 s max is a taste cap. Widen + expose together, per L0023.

## Gate ratification — `mpe_check` joins `./verify full` (2026-08-09)

**Human decision, recorded per the charter's gate rule.** Human: *"Gate ratified."*
`./verify` is a protected path; this is the explicit approval to add
`"$build_dir/mpe_check" || return 1` to `full()`.

`./verify full` now runs **fifteen** gates (the charter's "eight oracle chains" was stale by
seven and has been corrected): nine parity/trajectory chains — parity · trajectory · force ·
spectra · filter · notch · swarmalator · glide · time — plus six behavioural probes — state ·
notefuzz · rtsafety · **mpe** · preset · waveshape.

**Proven at the gate, not just at the probe.** With `allOffAll()` reverted to `cores[0].allOff()`
in the note-off path, `./verify full` exits **1**; restored, it exits **0**. That distinction
matters: a probe that prints RED while the dispatcher swallows its exit code is the failure mode
that stranded the preset tier on a dead branch — there, main stayed green precisely because the
missing piece WAS the gate.

**What it defends.** Parity renders a single core, so the whole fan-out class was invisible to
every other gate: a bend that split the oscillator pair and an all-notes-off that left half the
instrument gated (a stuck note) both passed fourteen chains. This gate is the only thing standing
between that class and a silent return — which is now especially load-bearing, since the fan-out
seam is a *helper per family* and every future consumer (third oscillator, sub-osc, per-voice FX
send) re-opens all of them until the L0029 routing layer lands.

## B23 ROUTING LAB SHIPPED — three topologies, and a cost table that decides (2026-08-09)

`docs/design/routing-lab.html`. Three candidate topologies over the SAME four slots and the same
two sources, switchable while it plays, so a difference you hear is topology and never a
different effect.

| | expresses serial? | params 2×4 | params 4×8 | slot instances 4×8 |
|---|---|---|---|---|
| **A** per-osc sends → parallel rack | **no** | 8 | 32 | 8 |
| **B** per-osc private chains | per source | 16 | 64 | **32** |
| **C** matrix (arbitrary DAG) | **arbitrary** | 24 | **120** | 8 |

**A cannot express `saw → drive → delay` at all** — the classic console limitation. **B** makes
serial free but gives each oscillator its OWN slot instances, so two oscillators through "the"
delay are two delay lines and a shared tail is impossible by construction. **C** allows a slot to
read only EARLIER slots, which makes the graph acyclic *by construction* rather than by a runtime
cycle check the audio thread cannot afford; a slot nobody reads is an output.

**Composition, which is what actually decides it.** Morph corners already target FX params, and
the mod matrix wants routing as a destination (`R → send amount`). A's sends are continuous, so a
corner interpolates cleanly. B's chain on/off is discrete — a morph between two chains is a hard
cut, and a topology bit cannot be modulated continuously at all. C's bits are discrete too, but
each slot also carries a continuous amount, so a corner blends *how much* while topology holds.

**Reading: C.** It is the only scheme that is simultaneously serial-capable, single-instance,
morphable and modulatable. Its one real cost is id count at scale (120 at 4×8), which argues for
**routing ids getting their OWN stride block** rather than being carved out of the per-oscillator
one — the same amendment ADR-082 already had to make once. **Not ruled — the human's call**, and
it wants an ADR because the id-block decision is append-only and therefore permanent.

**Verified offline, not by eye.** Identical gesture through all three: rms 0.398 / 0.792 / 0.847,
pairwise max diff 1.31–1.87, all finite, none silent. Inside C, rewiring slot 2 from slot 1
instead of from the source changes the output by 1.23 — the DAG edges do real work rather than
decorating a fixed path.

**Two lab-design corrections found while testing it.** (1) The schemes are not equally loud at
equal settings, and in an A/B the loud one always wins — so there is a per-scheme trim (remembered
across switches; −6 dB verified at 0.501×) and a live RMS readout, making the match a number
rather than a hunch. This is the calibrate-the-detector discipline pointed at the ear instead of a
probe. (2) The rack now defaults to drive/delay/lowpass/delay rather than all-bypass: all-bypass
was the honest default and a useless one, because every topology sounds identical when every slot
is a wire.

## B23 RESEARCH PROBE — the menu was incomplete, and the ruling is NOT ready (2026-08-09)

Human, at the gate: *"are we certain this is the most efficient system we can come up with?
...would it be worth running a research probe to make sure nobody has solved this problem more
elegantly?"* Yes. It was, and it did.

**The methodological fault first.** The lab compared three schemes **I authored**, then elected
one of them. A comparison whose candidate set is written by the same agent that judges it will
always produce a winner and can never produce the option that was never listed. The cost table
was honest; the *menu* was not audited. That is the class of error the doctrine's gate discipline
exists for, and the human caught it, not the process.

### What the literature actually has

- **The crosspoint matrix is the canonical primitive, and it is old.** ARP 2500 switch matrix
  (1970) → EMS VCS3 pin patchbay (1969) → NI Matrix Modular 3 → today's 16×16 hardware matrix
  mixers. Scheme C is not novel; it is the mainstream answer, which is reassuring about
  expressiveness and says nothing about cost.
- **Canonical crosspoint carries TWO values, not one:** a *scaling coefficient* and an *initial
  value* — `out_i = in_i + Σ_k (g_ki · m_k)` (Brandtsegg, Saue & Johansen, NIME 2011). The lab's
  slots have the coefficient and no initial value.
- **Three schemes the lab never considered.** (D) a **sparse connection-slot list** — N fixed
  slots each holding `(from, to, amount)`, which is the shape a mod matrix normally takes and the
  shape *HYPERSAW's own mod matrix already uses*; (E) a **reorderable chain** (a permutation, the
  Serum/Vital model), O(N) params, serial-only; (F) a **bus/aux-send** model from the console
  lineage, O(N) selectors.

### The finding that flips the analysis

D looks strictly better than C on the axis the lab used to judge: `12 slots × 3 params = 36`
params **fixed forever**, expressing any 12-edge graph, and a third oscillator or a fifth FX slot
costs **zero new ids** because it is just another value in the `from`/`to` enums. That dissolves
C's 120-param objection *and* the stride-block recommendation built on it.

**But the paper argues the other way, and its argument applies here with unusual force.**
Brandtsegg et al. keep the **dense** authored table specifically so it can be **interpolated
between whole coefficient tables** — their "dynamic modulation matrix" — and sparsify only at
*evaluation* time (§3.4: scan, drop all-zero rows/columns, run the reduced matrix until the
table changes). A dense table of continuous coefficients morphs cleanly; **a sparse edge list
cannot morph topology continuously**, because an edge appearing or disappearing is a
discontinuity. That is *precisely* the objection the lab raised against scheme B — and I did not
apply it to the sparse alternative because the sparse alternative was not on the menu. HYPERSAW's
quantum morph is a headline feature, so this is not a minor consideration here.

### The assumption underneath the whole cost table

The lab assumed **every routable quantity must be its own CLAP param**. That is what made C's
column look fatal. It conflates two different things: *what a patch can express* (table size,
saved and morphed) versus *what a host can automate* (param ids, append-only and scarce). They
need not be 1:1 — a dense table can be patch state with a bounded set of automatable routing
slots on top. Until that distinction is made explicit, every number in the cost table is
answering a question nobody asked.

### Also worth carrying (separate finding)

The paper permits **modulator feedback** — modulators modulating modulators, cycles included —
and warns it "must be applied with caution". The lab's *acyclic-by-construction* rule (a slot may
only read earlier slots) is correct for **audio** routing, where a zero-delay loop is not a
sound, and must **not** be copied into the **modulation** layer, where feedback is a feature and
is exactly what B26 (depth-of-depth) is asking for. One rule, two layers, opposite answers.

### Status

**Do not ratify.** The recommendation of C stands only against a menu now known to be incomplete.
Before a ruling: add D, E and F to the lab; separate *expressible* from *automatable* in the cost
table; and add the crosspoint initial value. PRIOR-ART.md should gain the matrix-mixer lineage —
protected path, so it needs the human gate.

**Sources:** Brandtsegg, Saue & Johansen, *A modulation matrix for complex parameter sets*, NIME
2011 (nime.org/proceedings/2011/nime2011_316.pdf); matrix-mixer lineage via Perfect Circuit and
Wikipedia *Matrix mixer*; sparse-slot mod-matrix practice via Cherry Audio Sines docs and KVR
DSP-forum implementation threads.

## B23 ROUND 2 — six schemes, a corrected cost model, and the real question (2026-08-09)

Human: *"Go for it."* D (sparse connection slots), E (reorderable chain) and F (bus/aux-send) are
now in the lab alongside A/B/C, the crosspoint **initial value** is implemented, and the cost table
is rebuilt.

### The cost model was measuring the wrong thing

Round 1 assumed **every routable quantity needs its own CLAP param**. Split into what it actually
conflated — *patch state* (saved, morphed; cheap and unbounded) versus *automation ids*
(append-only; the only scarce resource) — the picture inverts:

| scheme | patch state 4×8 | automation ids | instances | serial? | topology morph |
|---|---|---|---|---|---|
| A per-osc sends | 32 | 32 | 8 | no | continuous |
| B private chains | 64 | 0 | **32** | per source | hard cut |
| C dense crosspoint | 88 | **8** | 8 | arbitrary | **continuous** |
| D sparse slots | 36 | 12 | 8 | arbitrary | hard cut on edge add/remove |
| E reorderable chain | 8 | 0 | 8 | one path | hard cut |
| F bus model | **20** | 8 | 8 | arbitrary\* | hard cut on bus change |

**C costs 8 automation ids, not 120.** The column that killed it in round 1 was counting patch
state as if it were plugin ids.

### Topology morph is the axis that actually separates them

A morph corner interpolates *values*. In a dense table a crosspoint at 0 **is** "not connected",
so connecting and disconnecting are the same continuous motion. Every sparse scheme stores
topology as discrete structure, so adding an edge, reordering a chain or repatching a bus is a
**hard cut** — the identical objection round 1 raised against B and failed to apply to D, because
D was not on the menu. Quantum morph is a headline feature, so this is decisive here and would not
be elsewhere.

F is the cheapest scheme that still expresses serial (20 values), at the cost that a bus is a sum:
you cannot send two different amounts of one source to two places.

### A finding that separates dense from sparse on safety, not cost

D's acyclicity guard was written **in the editor**. Setting `from=slot3, to=slot1` directly on the
model — the route a preset load, a morph corner or automation would take — stuck, and produced an
undeclared one-sample feedback loop. **C cannot express a backwards edge at all**: its grid has no
cell for one. A free edge list can always express the illegal state, so either every writer is
trusted or every reader checks; only the reader-side check cannot be bypassed.

Then the fix itself was wrong in a familiar way: the legality test went into the signal sum but
**not into the terminal test**, which kept its own copy of "does anything read this slot?". The
illegal edge was correctly dropped from the audio and *still* marked its source slot consumed,
silently removing that slot from the output — measured 0.81. Same shape as the oscillator fan-out
bug (L0028), different subsystem, same day. Now one named predicate `edgeLive()` owns the rule and
the signal sum, the OUT sum, the terminal test and the graph all call it. Verified: an illegal edge
changes the output by **exactly 0**, a legal one by 1.056.

### Verified

Six schemes, identical gesture: rms 0.395 / 0.777 / 0.862 / 0.922 / 0.983 / 1.514, all finite,
**no two identical**. Each scheme's characteristic control does real work — D adding an edge 0.576,
F repatching a bus 1.876, E reordering the chain 2.299.

### The real question is above this repo

FOUNDATIONS **§3.2** already rules a MODULATION routing to be a five-tuple *(source, destination,
depth, curve, scope)* — that is scheme D, ratified. But **§3.5 (Signal Graph)** says only "slot
chain: source → per-voice processing → mix → global chain", which is **scheme E** and cannot
express what this lab demonstrates. HYPERSAW is **phase 0**, whose stated remit includes *slot
chain* seam quality, and §9's deferred-questions register does not contain audio-routing topology.

So the question is not "which scheme" but **whether audio routing and modulation routing share one
representation** — and it belongs to the mediator, not here. Ratifying a dense matrix locally would
either foreclose §3.5's doorframe or guarantee a retrofit, which is precisely what FOUNDATIONS
exists to prevent. **Still unruled; brief drafted in `INTEGRATION-STANDBY.md`.**

## EXCHANGES FILED AND CLOSED (2026-08-09)

Three documents filed into other repos' mailboxes under the INTEGRATIONS mailbox exception (write
only to `integrations/<us>/`; the **resident** commits, not us), and one thread ratified.

### FOUNDATIONS — two filings, one ack

- **`brief-signal-graph.md`** (new thread `hypersaw-signal-graph-001`, ball → provider). Asks one
  narrow doorframe question: does §3.5's signal graph stay a chain, or widen to admit a non-chain
  topology? Deliberately does **not** request a facility — Prime Directive 2 is two-consumers-
  minimum and we are one. Carries three findings judged to be the library's business: patch state
  and automation ids are different resources; topology morph splits dense from sparse (so §3.1
  morph corners + §3.2 sparse routings imply an unstated discontinuity); and a free edge list can
  express an illegal graph where a dense grid cannot, so the acyclicity rule needs an owner on the
  READ side.
- **`brief-parity-corpus.md`** — filed under **their** notice's id so the governor threads it onto
  the existing conversation and flips the ball, rather than opening a second thread that leaves
  theirs forever ball-on-us. Answers all three asks; headline is that the existing 147-scenario
  corpus contains **no multi-oscillator scenario**, plus the gravity block-dependence trap.
- **`ack-intake.md`** — closes `foundations-notice-intake-hypersaw`. Their notice says no
  acknowledgement is required, which is respected; left open it reads `ball: consumer` in the fleet
  sweep forever, which is a false signal about a settled thread. Terminal statuses are how the
  scanner learns a thread is done.

### Tonality HYPERSAW-001 — RATIFIED

Consonance-gravity ratio priors. Tonality's 2026-07-18 response ruled **(2a) gap 24 slice 1
buildable now** — the finer boundary being that a static table of rationals is *versioned prior
data*, not identity math off the 12-TET lattice, so it is **not** blocked by the Phase 6 / JI-monzo
deferral. We had assumed otherwise; the correction moves the ask earlier and is accepted. (2b)
context-weighting registered as slice 2 on the Phase 3.5 stack; (2c) determinism kinship confirmed.
All three schema counters accepted — provenance fields on kk-1982.1 discipline, fold-safety enforced
at the **producer** (our CI verifies rather than normalizes), and display names riding the artifact.

**Slice 1 deliberately not requested yet.** We are "one message away", and the reason for not
sending it is the gravity integrator bug above: swapping the ratio table while gravity's
integration is block-subdivision dependent would move the parity goldens **twice** and make it
impossible to attribute a change in settling behaviour to the right cause. The ask goes out once
that ADR lands. HYPERSAW stays named consumer on gap 24 and remains unblocked on the 13-ratio
placeholder, exactly as ADR-028 intended.

### A filing-convention finding worth carrying

`brief.md` carried `id: HYPERSAW-001`; `response.md` carried `id: hypersaw-001-response`. **The
fleet scanner threads exchanges by `id` alone**, so these were never one thread — the brief's
thread showed as awaiting a response that had been written nineteen days earlier, and the
response's thread sat separately with the ball on us. That is why the exchange surfaced as
overdue: a convention slip, not a stalled conversation. Both threads are now closed and the fleet
overdue count went 3 → 2. **A reply should keep the original `id` and add `in-reply-to` for the
human-readable link** — noted to Tonality as a suggestion, and worth applying to our own future
replies.

## F2 OPENED — extraction plan reviewed; we found a second shell (2026-08-10)

FOUNDATIONS opened F2 and filed an extraction plan **for correction, not approval**, having read
`src/` first. Plan endorsed: registry-first is right, and their diagnosis that our friction list is
symptoms of one split — metadata on the shell side, values on the core side, joined by a hand-kept
string — is right.

**Three corrections, re-derived from `src/` rather than memory.**

1. **Understated:** all **nine** cores are framework-free (they sampled four). Zero clap/juce
   references anywhere in `filter_core.h  force_core.h  glide_core.h  notch_core.h  osc_preset.h
   spectra_core.h  swarm_core.h  swarmalator_core.h  time_core.h`.

2. **Missed, and it changes their Stage 1:** `src/swarmfx_clap.cpp` is a **second CLAP shell**
   (437 lines, own factory/entry, shares filter/notch cores via `processExternal()`) — the
   dual-deployment pattern their §5 describes, already shipping. Its `ParamDef` has **already
   diverged**: 7 fields against 8, **no `coreKey`**, and **positional dispatch**
   (`indexOf(id)` → switch on index). So the registry is not "inside the shell", it is *copied
   into two shells and already forked, in exactly the field carrying core identity*. Their brief
   says three consumers independently reported positional identity failing; **we are the fourth,
   and we did it to ourselves in the newer code.** Asked them to extract Stage 1 against both
   shells — 17 params against 105, barely more work, and their own two-consumer rule satisfied
   without leaving this repo.

3. **Invisible from outside:** `coreKey` is the **state wire format**, not an internal detail — the
   literal key in every saved patch (`"%s=%.17g"`, `"o%u.%s=%.17g"`) *and* the core dispatch key.
   So HYPERSAW has **three identities and two are externally frozen**: the CLAP id by
   specification, `coreKey` by our own saved files, and only the core's internal string compare is
   free. Any address scheme must preserve `coreKey` as the serialization key or ship a migration —
   a constraint we created by using one string for two jobs.

**On their `coreKey` question** we answered against the obvious fix in both directions: the defect
was never *two representations*, it was a **hand-maintained mapping**. Collapsing to one
representation is what `swarmfx` did by dropping `coreKey`, and it landed on positional dispatch —
worse. Meanwhile the string surface is load-bearing: every core-level probe we own
(`trajectory_check`, `subdiv_check`, the block and sample-rate probes that found ADR-086) builds a
core with no shell and calls `setParam("grav", 0.7)`. Recommendation: registry owns the address,
core key **derived from it by construction and asserted at build** — two representations, one
identity, zero hand-kept mapping.

**Stage order:** registry-first agreed. We declined their offer to move voice architecture earlier
"while the code is fresh": it is fresh *because* the fan-out bug and `mpe_check` are three days
old, and `mpe_check` now pins it, so it will be no less fresh at Stage 2.

## SYNC PASS before ratifying the id block (2026-08-10) — and it found something

Human: *"it's this ID issue I'm currently chewing on with FOUNDATIONS as well. Let's take a
cautious extra pass to make sure everything is sync'd up."* Correct instinct; there was a gap.

**The gap.** ADR-088 §4 justified a permanent routing id block with "CLAP ids are append-only, so
this cannot be unmade" — stated as fact. It is FOUNDATIONS **open question #15**, unverified:
*"can shipping hosts survive `rescan(CLAP_PARAM_RESCAN_ALL)` mid-session with automation lanes
intact?"* HYPERSAW holds a `clap_host_params_t *` and has **never called rescan**; our param list
is static by assumption, never by measurement.

**Why it matters beyond tidiness.** The block allocation is safe under either answer — cheap if ids
turn out revisable, load-bearing if not. But the *justification* is not, and the difference is a
different design rather than a tidier one: if hosts survive a rescan, params could exist only when
their rack does, instead of a static block sized for the worst case. **§4 stays unratified**;
§§1–3 (the topology) are ratified and independent.

**Offered:** HYPERSAW runs the spike. It is a shipping CLAP plugin that has never called rescan —
an honest baseline rather than one already shaped around an answer — and its machine has real
hosts. Six cases (id unchanged / added / removed / **reused**, in-session and after reload, plus
the clap-wrapper VST3 path since that is also our shipping surface), reported as host × case data
with **no recommendation attached**, because five vendors converging on a macro layer should not be
overwritten by one machine's results. Filed as `offer-param-rescan-spike.md`.

**Also resynced, and both moved in our favour:**
- Their **#16 signal-graph topology** now cites **three convergent consumers** — our six-scheme
  lab, Morphos's absent bus abstraction, auricle's hardcoded `Engine::process` order. When their
  response was written it was "the second consumer decides"; it is now three, and the shape is
  still deliberately undecided. Our ratifying C locally remains exactly what they asked for.
- Their **#17 graph legality on the read side** is now a named open question with **two**
  convergent consumers (auricle's "the runtime trusts the document", plus our backwards edge via
  preset load). Our finding generalized past routing on their side, not ours.

## B23 UNBLOCKED — FOUNDATIONS widened the doorframe (2026-08-09, read 2026-08-10)

`response-signal-graph.md`, human-ratified on their side, answers the brief filed the same day.

**The ruling:** *"§3.5's chain is a default shape, not a constitutional commitment. The core will
not assume chain-only."* It deliberately does **not** choose a topology, promise a facility, or add
machinery — the two-consumer rule we pre-empted applies. The topology question enters their §9
register with our lab as its first evidence.

**Consequence:** *"ratify what HYPERSAW needs... nothing in FOUNDATIONS forecloses it, and nothing
in FOUNDATIONS should appear in your ratification rationale. Divergence between your topology and
any future library shape is information, not debt."*

**So B23 is a HYPERSAW decision again, on HYPERSAW's evidence** — and the lab's own reading stands:
scheme **C, the dense crosspoint**, on the corrected cost model (88 patch-state values, **8
automation ids**) and because it is the only scheme that is simultaneously serial-capable,
single-instance, morphable and modulatable. **Awaiting the human's ratification; nothing blocks it.**

**Where our three findings landed:**
- *Patch state ≠ automation ids* — confirmed, and we are the **second** voice: their F1 P5 found
  five vendors converging on a macro/proxy layer because host-facing ids are scarcer than state.
  That crosses their two-consumer threshold from the host side.
- *Topology morph discontinuity* — confirmed unstated and load-bearing; recorded into their morph
  semantics open question alongside the same question for curves.
- *Acyclicity needs a read-side owner* — generalized by them, better than our framing: **"legality
  is enforced where structure is consumed, not where it is written, because the writer set is
  open."** That generalizes past routing to every structure a preset can carry. Worth folding into
  the knowledge loop at the next consolidation.

Our oracle offer was accepted in principle: a routing oracle in `mpe_check`'s shape, consumer-
authored and resident-landed when F2 opens.

## ADR-086 AMENDMENT 1 SHIPPED — the gravity grid is a fixed TIME (2026-08-10, ratified)

Found by a sample-rate invariance probe written the same hour ADR-086 shipped, which is the point:
**a property oracle found a flaw in the fix, one that no golden could ever see** (goldens are only
generated at 44.1 kHz, so parity is silent about every other rate).

`kGravGrid = 256` is a fixed number of SAMPLES, so the grid's duration tracks the sample rate —
5.81 ms at 44.1 k, 2.67 ms at 96 k. Total integrated time is unchanged, but Euler truncation error
is not, so the trajectory differs slightly by rate. Measured, gravity settle time:

| rate | attack 90% | gravity settle | vs 44.1 k |
|---|---|---|---|
| 44100 | 0.23341 s | 1.56744 s | — |
| 48000 | 0.23311 s | 1.56800 s | +0.04% |
| 88200 | 0.23338 s | 1.57342 s | +0.38% |
| 96000 | 0.23314 s | 1.57400 s | **+0.42%** |

The attack column is the control: flat to ±0.13%, so ADR-009's seconds→coefficient discipline
holds. Gravity's drift is small (6.5 ms in 1.57 s — musically nothing) but **monotonic with rate**,
which is a dependence rather than noise.

**Shipped, ratified same day.** `kGravGridSeconds = 256.0/44100.0` with `gravGridSamples() = lround(sr * kGravGridSeconds)` — a fixed 5.805 ms. At 44.1 kHz it
evaluates to exactly 256, so **every golden is bit-identical and no parity moves**; at other rates
the integration step becomes constant in seconds, which is what ADR-009 asks of every other time
constant in the engine. Costs one line in `swarm_core.h` and one in `swarmdynamics.html`
(protected), and closes the dependence completely rather than relocating it.

**Measurement caveat worth keeping.** The first run of this probe reported the attack varying by
−1.4% and gravity by 0.38%, and the attack figure was entirely an artifact: the probe sampled every
256 samples, so its own time resolution tracked the sample rate — the exact confound under test. Re-run
with one millisecond of audio per step at every rate and interpolated threshold crossings, the attack
variation collapsed to ±0.13% while gravity's survived. **A probe whose resolution depends on the
variable it is testing will manufacture the effect it is looking for.**

## PAN MOTION is subdivision-dependent — the same defect, unruled (2026-08-10)

Found by `subdiv_check`, the gate written for ADR-086, on its first run. Pan motion (ADR-064) is a
per-render-call integrator exactly like gravity was: `dtB = frames/sr`, phases advanced once and
held across the block. Measured **0.191 max sample difference at chunk 333**.

**Deliberately not fixed.** ADR-086 ratified a fixed grid for GRAVITY. When the render was first
segmented, pan motion came along for the ride and took nine SAW parity scenarios red — against
goldens whose reference (`swarmsaw.html`) that ADR never touched. It is now hoisted to
`advancePanMotion()`, called once per outer call, preserving today's behaviour exactly.

**The decision, when you want it.** Pan motion is a slow LFO sampled at block rate, so the
practical symptom is milder than gravity's: the pan LFO's update rate follows the host buffer, so
the same patch moves slightly differently at 128 vs 2048 frames. Options: (a) leave it — a
block-rate LFO is a common design and the character is arguably "the sound"; (b) give it the same
fixed grid, which costs a second protected-path edit (`swarmsaw.html`) and moves the nine pan
goldens. **Recommendation: (b)**, because "the patch sounds different at a different buffer size"
is the same user-visible defect either way and there is now one mechanism to reuse — but it is not
urgent and it is not mine to rule.

Until ruled, `subdiv_check` reports it as **KNOWN** rather than asserting it, and says so in its
summary line. An undeclared exclusion is how a gate rots into decoration.

## ADR-086 SHIPPED — gravity on a fixed grid (2026-08-10)

Ratified after the ear check and implemented same day. Two things the ADR did not anticipate:

**The accumulator alone was not the fix.** Fixing the step SIZE left the step PLACEMENT wrong — in
one whole call every step fires before any audio is written. The subdivision probe rejected the
first implementation immediately (still 1.04). The working fix segments the render so gravity
advances *between* pieces of audio. Invariance now measures **0.00** across chunk sizes 64–44100,
including 333 and 127 which are not multiples of the grid.

**It moved something it should not have** — see the pan-motion section above.

Golden footprint exactly as predicted: **248 unchanged, 3 moved** (`dyn-gravity` × 3 seeds).
`./verify full` GREEN, 15 gates, parity 147/147 worst 4.262e-09.

**`subdiv_check` is built but NOT gated** — `./verify` is a protected path. Calibrated both ways:
reverting the segmenting gives FAIL at 1.093, restoring gives GREEN.

## (RESOLVED by ADR-086 — kept for the trail) gravity block-subdivision dependence (2026-08-09)

Found while answering FOUNDATIONS' parity-corpus notice. **Not fixed — the fix moves goldens, so
it wants an ADR and a human gate.**

`SwarmCore::render()` opens with `gravityStep((double)frames / sr)`: gravity advances **once per
render call, with dt = the block length**. It is explicit Euler on a nonlinear ODE — `move = err ·
rate · dt`, then `f0cur *= 2^(-move/1200)`, with `err` recomputed from the current `f0cur` each
call — so one step of dt and two of dt/2 do not agree.

Measured (bare `SwarmCore`, same seed, three notes, 1 s):

| gravity | one whole call vs 256-frame chunks | vs 333-frame blocks |
|---|---|---|
| 0.00 | **0** | **0** |
| 0.50 | **1.028** | **1.029** |

Gravity off, the engine is bit-identical under any subdivision — everything else is buffer-size
invariant. Gravity on, it is not a last-bits difference.

**Corrected 2026-08-09 (same day), by measurement:** "a different sound" overstated it. That 1.03
is **phase**, not tuning. Re-measured on `dyn-gravity`'s own settings, the interval settles within
**0.005 cents** of the same place at every step size from 16 to 2048 samples (701.926–701.931 ¢;
just 3/2 is 701.955 ¢). What varies is the trajectory, not the destination. This is a
**reproducibility** defect, not a tuning defect — real, and smaller than first stated. Full
evidence and the ratification ask are in **ADR-086 (PROPOSED)**.

**Two consequences.**

1. **Renders are not buffer-size invariant while `grav > 0.005`.** A user changing their DAW
   buffer changes the sound. That alone is worth a fix.
2. **Oscillator 0 and oscillators 1..N are integrated differently.** In the mix stage, oscillator 0
   renders in a single `n`-frame call while oscillators 1..N render in `kMixChunk` (256) chunks. The
   mechanism above therefore predicts that two *identically configured* oscillators do not track
   each other with gravity engaged.

**Honesty about what is proven.** The mechanism is measured in isolation (the table) and the
render asymmetry is plain in `hypersaw_clap.cpp`. Consequence 2 is a well-grounded prediction,
**not yet isolated end-to-end**: three attempts at a plugin-level probe were confounded, the last
because `plug_reset` does not clear core phase state, so the silently-rendering oscillator had
already advanced when it was measured. Recorded as prediction, not measurement.

**Proposed fix — now ADR-086 (PROPOSED, awaiting ratification):** integrate on a fixed
accumulator grid at **256 samples**, not the 16-sample control tick as first guessed. Measured with
10 held notes, a 16-sample grid costs **+66% CPU** (2.09% → 3.48% of a core) to buy a settling
difference of 0.001 cents; 256 samples costs **+2%** (2.13%) and removes the block-size and
subdivision dependence entirely.

**Parity impact is one scenario, not a sweep.** `dyn-gravity` is the ONLY one of the 147 that
engages gravity — `grav` defaults to 0 and `gravityStep` early-returns below 0.005 — so the other
146 stay bit-identical. An earlier note here implied a broad re-measurement; that was wrong. The
JS reference (`swarmdynamics.html:405`) has the same per-call shape and moves with it, which is
what makes this a SPEC change and a protected-path decision.

## Gesture routing — MPE belongs in the plumbing, not in the event loop (2026-08-09)

Human, on the eight-site fan-out fix: *"MPE should go to the plumbing and get routed from there
instead of messy redundancies and missed connections. This is another lesson for FOUNDATIONS.
This would be a good candidate for having the library build up from scratch efficiently and then
we can test against the oracle."*

**The shipped fix is honest but is not the cure.** PR #242 routed 14 call sites through a
fan-out seam (`allOffAll`, `setNoteExprAll`, …). That reduces `E x C` wiring (E event types x C
consumers) to `E` — it does not remove the class. Add a third oscillator, a sub-oscillator, or a
per-voice FX send and every one of the E helpers must be revisited, forever.

**The target.** Performance gestures — velocity, aftertouch/pressure, per-note tuning, channel
bend, mod wheel — are SOURCES. They should enter the same routing table as every other source
and be distributed by it. HYPERSAW currently runs TWO parallel paths for one class of signal: a
mod matrix, and a hand-wired MPE path in the CLAP event loop that reaches consumers directly.
Two paths for the same thing is the bug generator; the fan-out helpers only make the second path
tidier.

The win is not that the question gets answered — it is that **"does pressure reach oscillator 2?"
stops being askable**, because no per-consumer wiring exists to get wrong.

**Sequencing (why this is not scheduled here yet).** This is L0027 instantiated: the layer is
cheapest before the second consumer and never cheap again — and HYPERSAW is already past that
point, which is precisely why it cost eight bugs. Retrofitting it here competes directly with
the mixer/routing track (B23/B24) that the human ordered first, and it touches the mod matrix,
the event loop, and every consumer at once. **Queued behind a human gate; wants an ADR** —
specifically on whether per-note tuning needs a routed FAST LANE (bypassing depth/smoothing to
stay sample-accurate), which is the one real counter-pressure to routing everything.

**The library exchange (the human's proposal).** plugin-skeleton builds this subsystem from
scratch — the way it should have been built — and is tested against HYPERSAW's `mpe_check`,
which names no oscillator, core or alias: it drives the public plugin interface and detects via
emitted audio. **HYPERSAW donates the ORACLE, the library donates the ARCHITECTURE**, and
neither side inherits the other's accidents. Recorded in `INTEGRATION-STANDBY.md` and
`docs/integrations/corelib-insights.md` §4 as the proposed first exchange (L0029, L0030).

## Scale picker — a pitch-class set is a shared control, not a glide feature (2026-08-09)

Human: *"when it's in scale mode it will need a scale selector. It might be nice to be able to
choose the semitone pattern with a little approximation of an octave on a keyboard. This could
also be useful for effects or modulations we add down the line."*

Built as **`hzScalePicker`** in bend-lab: root selector + named-scale dropdown + a one-octave
keyboard whose keys toggle degrees. Shipped 2026-08-09.

**The gap it closed was L0023, not a missing nicety.** `scaleMask[12]` and `scaleRoot` already
existed in BOTH references (`bend-lab.html` P literal, `glide_core.h:54`) and had done since the
A1 fold — with nothing anywhere able to set them. Scale mode has therefore only ever meant C
major, and the option even said so. A reachable range with no control is an invisible feature.

**Ruling: the mask is the truth, the name is UI.** Consumers store and transmit `{root, mask}`
only, never a scale ID. That is what keeps `glide_core.h` free of a scale table: adding a named
scale is a UI-table edit that adds **no core change and no parity surface**, and hand-drawn sets
are first-class rather than a degraded mode (the dropdown reverse-matches, or reads *custom*).
This is the reason to prefer it over a `scale` enum param, which would have forced the same
table into C++ and made every new scale a parity risk.

**The keyboard is absolute; the mask is relative.** Keys show real pitch classes and the mask is
stored relative to root, so changing root TRANSPOSES the lit keys (C major → D major moves the
accidentals) — which is what "scale" means musically, and matches the core's
`((c - root) % 12 + 12) % 12`.

**Empty set is made unreachable, not handled.** The root key stays lit and the last lit degree
cannot be cleared. An empty mask is the one input the quantiser has no defined answer for: both
references fall through to plain rounding, and `Math.round(-0.5) = -0` vs `std::lround(-0.5) = -1`
disagree on exact .5 ties. Blocking it at the only control that can produce it is cheaper and
more honest than a downstream guard in two languages. *(The tie divergence itself is latent in
chromatic mode too — unreached by the current gesture. Recorded here rather than "fixed"
silently, since changing either reference's rounding is a goldens-moving act.)*

**Oracle widened to match the new reachable space.** `glide-quant-scale` had only ever rendered
C major, so every other mask was untested code the moment the picker existed. Three scenarios
added to both `gen_glide_goldens.mjs` and `glide_check.cpp` — non-zero root (`root3`,
D♯ minor pentatonic), wide-gap set (`whole`, whole tone), sparse rooted set (`sparse`, G
hirajoshi, with hysteresis). All parity **rms 0**, and calibrated as non-vacuous: the four masks
emit genuinely different step sets (−1·0·2 / −2·1 / −2·0·2 / −2·2), so a scenario cannot pass
by the mask being ignored.

**Reuse (the human's actual point).** The component's contract is `{root, mask}` in, `{root,
mask}` out, with zero dependency on lab internals — so an arpeggiator, a harmonic-snap FX, or a
quantised mod destination mounts the same control. Per FOUNDATIONS standby it is NOT extracted
to a shared module yet; it is recorded in `INTEGRATION-STANDBY.md` as a portable component with
its contract stated, which is what the first brief will need. Second consumer earns the
extraction — copying it once is the honest price of ADR-003 single-file labs.

## K vs LINK — two mechanisms, and they are not the two the question assumed (2026-08-07)

Human: *"there might be a difference between the notion of a master K and sync'd Ks: syncing
the K values preserves independent rates per cluster, while a master K forces them all into one
frequency. Am I correct?"*

**The instinct that there are two distinct mechanisms is right. The mapping is different, and
the difference matters for B22's design.**

**K is not a rate.** It is the *intra-swarm coupling strength* — `km = 4·K·|K|`, feeding a sync
term and a splay term scaled by the swarm's own frequency spread (`swarm_core.h:1178-1188`).
Within one swarm, raising K entrains its oscillators toward a common frequency; that is the
Kuramoto transition the whole instrument is built on. But K only ever acts **inside** a swarm.

So a **master K** — one knob driving several K *parameters* — does **not** force anything into
one frequency across oscillators. It makes each swarm equally coherent *internally*, while the
swarms remain at whatever pitches and rates their own detune gives them. Nothing couples across
them, so nothing can pull them together.

**The mechanism that does share timing already exists, and it is `link`.** A swarm may carry a
`master` reference and a `link` amount: at 0 it is fully independent, at 1 its phases are
entrained to the master swarm's mean phase (`mod-lab.html:91-93, 184`). That is genuine
inter-swarm coupling — the FX swarms already use it to run "participating vs independent"
against the main rotor.

**So B22 is two controls, not one:**
- **K link** — the *parameter* sharing the human asked for. One value, several K params;
  breakable per oscillator/effect. Cheap, and purely a UI/parameter concern.
- **Phase link** — the *dynamical* coupling. Already implemented for the FX swarms; extending
  it to oscillators would let oscillator 2's swarm be entrained by oscillator 1's, which is a
  real and much more interesting feature than sharing a number.

Conflating them would have shipped a "master K" that users expected to lock oscillators
together and that audibly does not. Worth an ADR before building, because they are separately
useful and the naming has to distinguish them.

**Recorded caveat on `link`'s taper:** a prior measurement found link "did nothing above 0.15
— the whole slider was one step". Whatever extension B22 makes should re-measure rather than
inherit that curve.

## GLOBAL TIME SCALE — a macro over every time-domain param (human, 2026-08-07)

Human: *"take a page out of many Ableton effects/instruments and add a global time slider which
controls all or most time settings at once. For now we can get into the habit of flagging
features this might apply to."*

Queued as **B25**, and the flagging starts now — here is the surface as it stands. **16
time-domain params today:**

| | |
|---|---|
| envelope (amp) | 19 attack · 20 decay · 22 release |
| envelope (SPECTRA) | 65 sAttack · 66 sDecay · 68 sRelease |
| swarm | 8 dissolve · 10 driftRate |
| glide | 33 glide · 75 freqGlide |
| scatter | 93 attackScatter · 95 relScatter |

(`sustain`/`sSustain` are levels, not times, and `polyGlide`/`glideMode` are behaviour
switches — listed by the scan, excluded from scaling.)

**Not yet in the shell but coming, and all time-domain:** the glide module's five travel laws
(glide time, rate, lag τ, spring frequency), the time engines' echo and room decays, and the
reverb's EDT/T30. The macro should be designed knowing those are arriving, not retrofitted
around them.

**The design question to settle before implementing:** a global time macro can scale
*multiplicatively* (every time × N, preserving ratios — the Ableton-ish behaviour and the one
that stays musical) or *interpolate toward a target*. Multiplicative is almost certainly right,
but it needs a rule for params whose range is clamped (`freqGlide` maxes at 0.1 s, so ×4 from
the top does nothing) — otherwise the macro silently stops affecting some controls partway
through its travel, which is the dead-control failure this project keeps re-learning.

**Convention going forward:** any new time-domain parameter gets flagged for the macro at the
point it is added, in the ADR or roadmap entry that introduces it. Cheaper than auditing for
them later — this list took a scan and still needed hand-filtering.

## RE-ORDER: MASTER/MIXER PAGE FIRST (human, 2026-08-07) — and 13 params are already mis-scoped

Human: *"Maybe we switch up the order so we don't build a bunch of tech debt. First we need the
audio context: the master/mixer page. Then when one Osc can send its audio through there, we
add the second Osc, and from there the routing algorithms."*

**Agreed, and there is evidence the debt is already accruing.**

### The finding: 13 "global" params are per-oscillator by construction

Audited by asking which ids in `kGlobalIds` have a key `SwarmCore` itself owns — because a
param the core owns exists **once per core instance**, so declaring it global does not make it
shared, it makes the second oscillator's copy **unreachable**:

`inertia (11)` · `width (14)` · `mono (15)` · `attack/decay/sustain/release (19-22)` ·
`beatMult (23)` · `glide (33)` · `freqGlide (75)` · `oversample (88)` · `polyGlide (89)` ·
`glideMode (90)` — **13 of 31**.

Oscillator 2 already has its own `width`, `attack`, `glide` and the rest sitting inside its
core at defaults, with no id able to address them. The human spotted this from the outside —
*"oscillators will independently need their own width controls, among I'm sure many other
things"* — before the audit found it.

**Fixable cleanly, and only while the ids are unallocated.** Making these per-oscillator
allocates their `+1000` versions; no existing id moves, so it is additive. That stops being
true the moment a build ships exposing them.

**Calibration note, since it nearly hid the finding:** the first audit reported **0
misclassified**. It searched for `eq(k, "...")`, the idiom `swarmalator_core.h` uses;
`swarm_core.h` uses `k == "..."`. A clean bill of health from a detector looking for the wrong
pattern — the same shape as the sweep's 53 phantom dead routings and the allocation the
optimizer elided.

### Which of the 13 become per-oscillator — needs a ruling (A12)

Core ownership is a *fact*; exposing it per-oscillator is a *choice*:

- **Clearly per-oscillator:** `width`, `mono`, `inertia`. Stereo image and drift character are
  properties of a sound; two oscillators that cannot differ in width cannot layer convincingly.
- **Arguably:** `attack/decay/sustain/release` — different envelopes per layer is the oldest
  trick there is, but the *voice* conventionally owns one amp envelope and SPECTRA already
  carries its own at 65-68.
- **Probably patch-level despite core ownership:** `oversample` (per-osc multiplies the CPU
  question ADR-082 already flagged as tight), `beatMult` (tempo grid), and the glide family —
  which B19's module is about to own anyway, and which A1 made a *destination-linked* system
  rather than a per-oscillator one.

### The re-ordered plan

1. **Master / mixer page — the audio context.** Per-oscillator channel strip (level, width,
   pan, mute/solo) plus the master bus. This is where the per-osc/patch boundary is decided *by
   the interface* rather than guessed in an ADR.
2. **One oscillator through it**, proving the strip with a signal that already works.
3. **The second oscillator into the same strip** — replacing today's hardcoded sum, which is a
   fixed routing that would otherwise calcify.
4. **Routing** (B23).

### New items

- **K link across oscillators AND effects (B22).** It should extend beyond oscillators: the FX
  swarms (`choSwarm`, `phSwarm`), the filter and notch cores all carry a K, and the Kuro-synced
  FX class (B17) already uses `link` as exactly this idiom. One concept — *a K value is
  independent or locked to a master K* — designed once rather than twice.
- **Routing lab (B23).** Routing is a topology question (which sources reach which slots, in
  what order, with what summing), and the FX rack is already a grid rather than a fixed chain
  (ADR-054). The lab should settle per-osc sends vs a matrix, serial/parallel per path, and how
  it composes with the morph and mod matrix — both of which already target FX parameters.

## OSCILLATOR PRESET TIER SHIPPED (B20 bottom tier, 2026-08-06)

`src/osc_preset.h` + `tools/preset_check.cpp`, gated in `./verify full`.

**The tier really was nearly free, exactly as predicted.** ADR-082 gave every per-oscillator
param the key `o<k>.name`, so one oscillator's preset is *the subset of state keys carrying one
prefix* — saving is a filter, loading into another slot a prefix rewrite. That fell out of the
id scheme rather than being designed for presets, which is some evidence the scheme is right.

**Two properties are pinned, and both are load-bearing rather than decorative:**
- **Slot-agnostic on disk** — keys are stored UNPREFIXED. A format that embedded its origin
  slot would pass a naive round-trip and fail the first time anyone copied oscillator 1 to 2,
  which is the main thing this tier is for.
- **Globals never travel** — an oscillator preset carrying the FX rack or the master image
  would silently redecorate whatever patch it was dropped into: data loss wearing the costume
  of a feature.

Also pinned: a patch blob is rejected rather than half-applied; unknown keys are skipped, not
fatal (the same forward-compatibility the patch loader promises).

**Plugin wiring deliberately NOT shipped.** Binding read/write to `readParam`/`applyParam` with
the `+kOscStride` offset has no caller until the osc-page GUI exists. Unreachable code rots
quietly — it keeps compiling while the surface it assumed drifts underneath. It lands with the
GUI that calls it, in the same change, so it is exercised the day it ships.

**Corner tier remains next**, now unblocked by A11 (corners are global). The patch tier already
exists as CLAP state.

## ADR-082 INCREMENT 2 SHIPPED — the second oscillator (2026-08-06)

`kNumOsc = 2`. `cores[kMaxOsc]` with `core` kept as a reference to oscillator 0 (the 52
existing call sites are untouched); params route by oscillator, notes fan out, oscillators
1..N-1 sum into the output. **Higher oscillators default to silent** — `vol = 0` in both the
constructed state and the reported defaults, so parity is untouched and no existing patch
changes.

Measured directly on the cores: osc1 at `vol = 0` sums to **0.08775**, bit-identical to
osc0 alone; at `vol = 0.4` with matched detune, **0.17551** (exactly 2×, correlated); at detune
0.85, **0.13621** (below 2×, decorrelated). Silent, audible, independent.

`./verify full` GREEN at 2 oscillators: parity **147/147 worst 4.262e-09**, unchanged.

**Two bugs found, both the same shape — the write path routed, the read path forgotten:**

1. **`readParam` still read oscillator 0**, so `state_save` wrote every `o<k>.` key from
   oscillator 0's values. `state_check`'s "every param round-trips exactly" **passed anyway**,
   because it compares two reads through the same broken accessor — two wrong reads agreed.
   Only the *audio* comparison caught it. **An oracle that reads through the code it tests
   cannot see a symmetric fault in it**; the audio check works because it bypasses the accessor.
2. **Audible output was conditional on a heap buffer** — the first version summed through a
   `std::vector` scratch sized at `activate()` and skipped the oscillator when it was too
   small, i.e. a voice could vanish silently. Now a chunk loop over a fixed stack buffer.

Found by **bisection**, not by reading: cutting only the note fan-out turned `state_check`
green, which located the fault in note handling. Two earlier hypotheses were wrong and were
dropped on evidence.

## ADR-082 AMENDMENT 1 — the id scheme was full on day one (2026-08-06)

Caught while **starting** increment 2, before anything was built on it. Two defects in the
ratified scheme, both free to fix at that moment and permanent a week later.

**(a) Stride 100 capped the instrument at 99 parameters, forever — and it was already at 99.**
The stride is also the capacity of oscillator 0's block. Measured: ids 1..99, **zero free slots
below 100**. A new param would need id 100, and `findParam` computes `osc = id / kOscStride`,
so 100 resolves to oscillator 1 / base 0 and is never found — silently unreachable, not merely
cramped. **Stride is now 1000** (osc 0 = 1..999, osc 1 = 1000..1999, osc 2 = 2000..2999); every
existing id unchanged. Free **only because increment 1 shipped at `kNumOsc = 1`**, so no id
≥ 100 has ever reached a host. After the first 2-oscillator build ships this is impossible.

**(b) `vol` (17) was misclassified as global.** It is the swarm's own output gain, computed
inside `SwarmCore::render`. Left global, two oscillators share one gain and cannot be balanced
— the very control increment 2 exists to add. Now per-oscillator; a patch-level master volume
is a separate future param, for which the stride amendment leaves room.

**The process point:** neither was found by re-reading the ADR. Both surfaced within minutes of
trying to build the increment it authorised, by asking "what mixes the two oscillators?" and
discovering the answer was nothing — `balance` (56) being the two-cluster *coupling* balance,
not a mixer. An ADR reads as complete right up until you execute it, which is an argument for
starting the walking skeleton early rather than perfecting the document.

Verified: `./verify full` GREEN at `kNumOsc = 1` (parity 147/147, worst 4.262e-09, unchanged);
calibrated at `kNumOsc = 2` with `state_check` fully green including the `o1.` round-trip.

## GLIDE CORE PORTED — laws 1-4 + quantise, with a trajectory oracle (2026-08-06)

A1 was fully ruled, so B19 became buildable. Increment 1 follows the swarmalator order: **core
and oracle first, shell integration separately.**

`src/glide_core.h` holds the four ratified laws and the quantise modifier, transcribed from
bend-lab's `Inertia`. **Law 5 is absent rather than commented out** — the ruling cut it, and
dead code invites resurrecting a control the measurement already rejected.

`glide_check` is green on 11/11 scenarios, worst parity RMS **3.51e-08** (bar 1e-6). The
behavioural anchors matter more than the parity: written from JS measurements taken days
earlier, the C++ port reproduces them independently — spring overshoot **+18.8¢** (JS: 18.8¢),
constant rate **+0.0¢**, hysteresis at a boundary **15 → 3 flips** (JS: 15 → 3). Parity alone
only proves the port matches a recording; the anchors pin the *character* each law was chosen
for, so a refactor that keeps parity to a stale golden still trips.

**Not in the audio path.** No param ids, no state keys, no GUI — which is why `parity_check`
is still 147/147 at the identical worst error. Nothing calls it yet.

**`./verify full` gained the chain — a protected-path edit, flagged for ratification.** It is
additive and follows the pattern of every prior core port (force, spectra, filter, notch,
swarmalator, time).

**Shell integration needs decisions A1 did not cover:** how the four destinations map onto the
seven existing glide params (11 inertia, 33 glide, 34 legato, 70 inertiaCurve, 75 freqGlide,
89 polyGlide, 90 glideMode), and whether those are superseded or re-pointed. Append-only ids
mean that wants ADR-082-level care rather than an improvised mapping.
## NOTE — CI red on PR #212 was a GitHub outage, not this change (2026-08-06)

Recorded so the history is not misread later. PR #212's checks showed FAILING while GitHub
Actions was in a **major outage** (incident `qcvjkzcs7j74`, opened 15:22:49 UTC).

The evidence that it was external, not ours:
- the jobs' conclusion was **`cancelled` with `steps: []`** — they never executed a single step;
- a Linux `verify fast` and a Windows CMake build, sharing no code path, died at the **same
  instant**, exactly 15m01s after starting;
- one run sat **28 minutes** between `created_at` and `run_started_at` waiting for a runner;
- `gh pr checks` renders anything non-success as "fail", which is what made a cancellation look
  like a broken gate;
- a fresh clone of the branch ran `./verify fast` to exit 0 locally, and `./verify full` was
  green across all oracle chains.

**Recovery was uneven:** once mitigations landed, *freshly triggered* runs got runners
instantly (PR #213: zero queue delay, `verify-fast` in 8 s), while *re-runs* of jobs created
during the outage stayed queued indefinitely. So the working move during an Actions incident is
to push a new commit rather than hit re-run.

## SWARMALATOR IS SAW + TWO TERMS — the human was right (measured, 2026-08-06)

Human: *"isn't it essentially the same thing as SAW but extended to give space and phase a
relationship? It doesn't feel different enough to be its own engine, hence the slider
suggestion."*

**Correct, and the code says so more precisely than the intuition did.** The header already
states it — *"K = ordinary Kuramoto sync (phase axis == SAW when J=0)"* — and the coupling term
is literally additive:

    couple[i] = kSync + jBack        // jBack is proportional to p.J

At `J = 0` the phase axis IS SAW's Kuramoto. But the **spatial** axis does not stop there:

    xidot = nu[i] + p.J * jRate * 0.5 * (Rp*sPlus + Rm*sMinus)

`nu[i]` is each voice's own rotation rate, seeded from **`drift`** — so at `J=0` the voices
keep circling the stereo field. Measured spatial travel over ~1.16 s at K=0.6:

| J | drift | spatial travel |
|---|---|---|
| 0.6 | 0.2 (defaults) | 0.637 rad |
| 0 | 0.2 | 0.690 rad |
| 0.6 | 0 | 0.468 rad |
| **0** | **0** | **0.000000 rad — frozen, pan is static** |

**So the reduction condition is BOTH `J = 0` and `drift = 0`** — one slider must drive two
parameters, not one. (Note for whoever builds it: `p.nu` is the *unit count*, not a rate. A
first probe set it to 0.5, rendered zero voices, and produced three identical rows that looked
like a clean result. The parameter named like a rate is the array `nu[i]`, driven by `drift`.)

**The stronger conclusion: it should not be an engine at all.** `SwarmalatorCore` has 9
parameters; SAW has ~70 (distributions, detune laws, onset/dissolve, topology, octave spread,
root anchor, drift modes, pan layout…). Keeping it as a separate engine forces a false choice
between *spatial coupling* and *every law SAW has*. Since the phase axis is already SAW's, the
right move is to fold **ξ (spatial state) and J (cross-coupling) into `swarm_core.h` as two
extra terms**, exposed as the human's spatial-blend slider driving J and drift together:
0 = today's SAW with its existing static pan, 1 = full swarmalator.

**The one constraint that makes it safe:** at slider 0 the ξ path must be *inert* — SAW's
existing pan layout/scatter/curve logic untouched — so all 147 parity goldens stay green. That
is the same superset-with-inert-defaults discipline as ADR-021/025/042/063, so there is a
well-worn precedent.

**A2 is therefore not "listen, then integrate an engine"** — it is "listen, then decide whether
the spatial coupling earns a place in SAW". Listening to `swarmalator.html` answers it
(the core is bit-exact against it: stereo parity RMS 0.0 on 9/9).

### TABLED (human, 2026-08-06) — and a correction to the "slider" framing

Human: *"I'm assuming the swarmalator behavior would have to be a toggle that switches on as an
alternative to the existing SAW pan laws. Let's table it for now and revisit down the line. I
don't think it's very high priority."*

**Tabled — and the toggle observation corrects something this section got wrong.** The text
above (and the PR that wrote it) called this a *blend slider*, inherited from the 2026-07-20
sketch. That is not quite right, and the reason is structural: **pan cannot have two sources at
once.** SAW derives each voice's pan from its static laws (pan layout, curve, scatter, invert,
spread); the swarmalator derives it from the spatial state ξ. A voice's pan is one number — so
turning ξ on means the pan laws stop determining it. There is no coherent midpoint where a
voice is half-placed-by-layout and half-placed-by-dynamics.

What CAN be continuous is the *depth* of the spatial motion once ξ owns pan (J and drift both
rising from zero, per the measurement above: at J=0 **and** drift=0, ξ is frozen at its even
initial spread — travel 0.000000 rad). So the honest shape is a **toggle** choosing which
system owns pan, plus depth controls behind it — not a crossfade between two pan systems.

That also makes the inert-default requirement cleaner: toggle off ⇒ the ξ path never executes
⇒ all 147 parity goldens hold trivially, rather than needing a "slider at 0 is bit-exact"
argument.

**Priority: low, revisit later.** No ADR is written yet; when it is, it should specify a toggle
with depth controls, not a blend. The swarmalator core stays as the reference implementation
and its oracle keeps running in `./verify full` either way.

### Standalone CPU bench for a machine with no DAW

`dist/` now carries a **universal (arm64 + x86_64) self-contained `cpu_bench`** plus
`README-cpu-bench.md`: copy the file, run it in Terminal, read one number. No DAW, no Xcode, no
install, nothing written or played. Build it with:

    clang++ -std=c++20 -O3 -arch arm64 -arch x86_64 -I src tools/cpu_bench.cpp -o dist/cpu_bench
    codesign --force -s - dist/cpu_bench

The binary itself is gitignored (a 182 KB Mach-O does not belong in git); the recipe above is
the tracked artifact. The README covers the quarantine flag, which is what will otherwise stop
it opening on another Mac.

## CPU BENCH BUILT — the min-spec question is now HARDWARE, not method (2026-08-06)

`tools/cpu_bench.cpp` (target `cpu_bench`) runs the **real `SwarmCore`**, not a model of it, and
reports % of one core against the E-6 envelope (44.1 kHz, 128-sample buffer, budget 50%).
Deterministic: fixed seed, fixed note order, a warm-up pass outside the timer, no wall-clock in
the render path.

**Measured on this machine (Apple M3, Release −O3):**

| load | % of one core | ×realtime |
|---|---|---|
| 7 voices × 8 notes (56 osc) — default patch | **1.60%** | 62.5× |
| 14 voices × 8 notes (112 osc) — two oscillators' worth | **2.98%** | 33.5× |

Scaling is near-linear (1.86× for 2× the voices), which is the assumption ADR-082's table rests
on — now checked rather than assumed. At the ×4 min-spec derate that is ~11.9% for two
oscillators at 1×, comfortably inside the 50% budget and *better* than the ADR's estimate.

**What remains is not a method problem, it is a hardware problem.** The E-6 envelope defines
min-spec as an Apple M1 base / 4-core 2018-class Intel ultrabook / Windows x64 AVX2. This
machine is an M3. Options, for the human:
- **(a)** run `cpu_bench` on an M1 or an older Intel laptop if one is reachable — the direct answer;
- **(b)** use the CI runners (`windows-latest` / `ubuntu-latest`) as a real x86 proxy — honest
  about being a cloud VM with variable neighbours, so a floor rather than a spec number;
- **(c)** accept the ×4 derate as a recorded, human-accepted residual — the precedent is the
  Phase 0 gate's Reaper/Bitwig deferral, which was accepted for the same reason (no hardware).

The derate is doing real work in ADR-082's conclusion ("3 oscillators + 2× oversampling is over
budget"), so it is worth one of (a)/(b) rather than (c) — but at two ratified slots the margin
is now large enough that this no longer blocks increment 2 on its own.

## A2 SWARMALATOR — the agreed audition path was removed by a later ruling (2026-08-06)

Flagging a queue item that a later decision silently invalidated. A2 has been waiting on "the
human's listen before shell integration", and the recorded path (human direction 2026-07-20)
was: *hear it first as a nondestructive parallel engine — engine-select, SAW byte-frozen*.

**That path no longer exists.** The SAW-first pivot (2026-08-05) removed the engine selector
from the GUI for new patches. So the plan of record for auditioning the swarmalator was
cancelled by a ruling made two weeks later, and nothing connected the two.

Options, cheapest first:
- **(a)** listen in the prototype: `swarmalator.html` is present and loads clean — zero work,
  available right now, and it IS the reference the core is bit-exact against;
- **(b)** re-expose the engine selector behind a dev flag for an in-DAW audition;
- **(c)** defer A2 until the interface renovation reaches the engine-expansion phase, when the
  selector returns anyway.

(a) is enough to answer the actual question ("is this worth shipping?"), because the C++ core is
proven bit-identical to that prototype — `swarmalator_check` renders stereo parity RMS 0.0 on
9/9 scenarios. Listening to the prototype IS listening to the engine.

## GLIDE FOLD RULED + STATE GATE WIDENED (human, 2026-08-06)

### A1 (partial): laws 1-4 ship, law 5 does not, quantise is a MODIFIER

Human: *"include all the travel laws minus 5, plus my new proposed one(s) (note/scale
quantized) — or this can be a setting attached to all of the others."*

**Law 5 (lag → constant rate) is cut.** The roundup measured it as the closest pair to law 3:
every headline metric rounds identical and the curves diverge by only 8.70 cents. It added a
control without adding a behaviour.

**Scale quantise ships as a MODIFIER on all four laws, not as a sixth law** — taking the second
half of the human's own suggestion, because it is strictly better: applied to the *emitted*
pitch while the law's dynamics run untouched underneath, it composes. Spring + quantise is an
overshooting autotune wobble; constant-rate + quantise is a stepped portamento. As a sixth law
it would have been one behaviour; as a modifier it is four, for one control instead of a whole
trajectory type. Built in `bend-lab.html` (`Inertia.quantise`) with off / chromatic / scale.

**Hysteresis: what it fixes, measured — and a claim I had to narrow.** The first version of the
code comment asserted hysteresis was "not optional". The measurement disagreed, so the claim
was narrowed to what the data supports (step changes over a 2 s window):

| case | 0¢ | 8¢ | 25¢ |
|---|---|---|---|
| spring parked ON a step boundary, ζ 0.5 | **15** | **3** | 3 |
| ζ 0.2 | 17 | 7 | 5 |
| ζ 0.08 (heavy ringing) | 20 | 18 | 13 |
| deliberate ±60¢ vibrato at 5 Hz | 20 | 20 | 20 |

Hysteresis rescues the parked-on-a-boundary case, stops helping once the ringing dwarfs the
window (a damping problem, not a quantiser one), and correctly does **nothing** to a wide
deliberate vibrato — that motion is supposed to step. It is a fix for one artefact, not a
general smoother.

### Default law: CONSTANT RATE (ruled 2026-08-06)

Law 2. The roundup's clearest playability difference is the vibrato column — constant rate keeps
**93%** of a 5 Hz wheel wobble where constant time keeps **33%** — and a fixed ¢/s is a quantity
a player can predict, where "120 ms" means a different speed for every interval. Applied to the
lab default (`P.model`), and the `<select>`'s `selected` attribute moved with it: it still said
`3` while the model ran `2`, which would have shown "lag" in the UI while constant rate was
running. Law 5's option is labelled CUT rather than deleted — the lab is a workshop and the
evidence should stay re-measurable.

### Destination matrix RULED (2026-08-06) — own law each, shared by default

Human: *"each should get its own law, but the default should be that they share a law."*

So: **four per-destination law selectors, linked by default.** One control sets all four; unlink
a destination to give it its own. This is the same shape as the FX rack's `link` (ADR/B17) —
the instrument now uses "one value, breakable per instance" in two places, which is worth
keeping consistent in the UI rather than inventing a second idiom.

Why it is the right default: a player who never opens the module gets coherent behaviour
everywhere, and the expressive case (spring on the bend wheel, lag on the mod wheel, constant
rate on note pitch) is one click away rather than four decisions deep. **A1 is now fully ruled**
— laws 1–4, constant rate default, quantise as a modifier, per-destination with link. The glide
spec is complete and B19 is buildable.

### Superseded — the open question this replaces

Four things in the instrument can travel: **note pitch** (portamento), the **bend wheel**, the
**mod wheel**, and **MPE per-note bend**. Today the lab shares ONE law across whichever lanes
`applyTo` enables (bend only / note only / both). The open question is whether each destination
picks its own law, and it matters because they want different things: note pitch wants constant
rate; a bend wheel arguably wants spring, so a flick has physical mass; a mod wheel wants lag,
because overshoot on a filter sweep is just wrong.

**(Recommendation as written before the ruling — kept for the record.)** Per-destination law
selectors, defaulting to constant rate on note pitch and *off* on the other three. The human
ruled a linked default instead, which is better: it makes the simple case coherent rather than
mostly-disabled. Glide is a module precisely
BECAUSE it has destinations — one shared law would collapse it back to a knob, which is the
thing the human said it had outgrown. Cost is four selectors instead of one; the middle option
(grouping {note pitch} · {bend + MPE} · {mod wheel}) is recorded as the fallback if four reads
as too many.

Scale SOURCE for quantise remains the deferred Tonality brief; chromatic plus a scale selector
is the interim (B21).

### State version gate widened (ratified)

`tools/state_check.cpp:222` now accepts `hypersaw-state 1` **or** `2`, unblocking ADR-082
increment 2. **Human-ratified 2026-08-06**, recorded here per the charter's rule that gates
change only on an explicit decision. Calibrated: it still goes RED on an unknown version
(planted version 9 → 4 failures), so it is a widened gate, not a removed one.

B18's remaining half — the **min-spec CPU measurement** ADR-082 requires before two oscillators
ship — is still open.

## MORPH CORNERS ARE GLOBAL (A11 ruled, 2026-08-06)

Human: *"I was thinking the morph would encompass all parameters of all oscillators. Maybe this
is too big of a thing, but it's my dream."*

**Ruled: reading (a) — corners are global.** One corner holds a value for every per-oscillator
parameter of *both* oscillators; four corner-sets total.

**Worth stating plainly, because the human framed it as the ambitious option: it is the
CHEAPER of the two.** Reading (b) (each oscillator owning its own four corners) is the
expensive one — a 2×4 grid of value sets, double the authoring surface, and a "which corner of
which oscillator am I editing" affordance on every control. Global corners are one grid, one
set of corner chips, one reshuffle. The dream is the simpler build.

**Consequences now settled:**
- A corner preset is a whole-instrument snapshot of the per-osc params across both oscillators.
- Under ADR-082's key scheme that is every `o<k>.`-prefixed key plus osc 0's per-osc keys —
  i.e. the state file minus the globals. No new format needed.
- Oscillators cannot morph independently. That is the accepted cost; if it is ever wanted, it
  is reading (b) and a format change, so it would need its own ADR.

**Still open (smaller):** whether the GLOBAL params (FX rack, output, glide) join the morph.
The human's phrasing was "all parameters of all *oscillators*", which reads as no — and there
is a reason to keep it that way: morphing FX type or master volume is exactly where the
parameter-collision problems live (the "two ring modulators on different FX slots" case).
Recorded rather than assumed.

## GLIDE MODULE — the shape visualizer is REQUIRED, and a step-glide idea (human, 2026-08-06)

**The shape visualizer ships WITH the module, not after it.** Human: *"I want to make sure the
shape visualizer is included in the module. It's crucial for understanding/predicting the
inertial character."* This is the standing lab-visual convention (B7) applied to a case where
it is load-bearing rather than decorative: the five travel laws differ in ways the parameter
names actively hide — see the roundup, where law 3 and law 5 produce identical lag/settle/
vibrato figures and different curves. A user choosing between them from a dropdown, with no
curve, is choosing blind.

**New idea to workshop in the lab: note- and scale-quantized STEP GLIDE.** Instead of a
continuous travel, the pitch moves in quantized steps toward the target — snapping to scale
degrees or semitones on the way — for an autotune-like character. Notes:
- It is a sixth travel law, not a modifier: it changes *what* the trajectory is, not how fast.
- It needs the scale/tuning source the instrument does not yet have a home for (the Tonality
  sibling brief, deferred since Phase 3). Interim: chromatic + a simple scale selector.
- The visualizer earns its keep here immediately — a stepped trajectory is unreadable from
  numbers.
- Workshop in `bend-lab.html` first (human: *"might be worth workshopping in the lab again"*),
  measured like the other five, before any fold decision.

Queued as **B21**; the fold decision for all six laws remains **A1**.

## LAYOUT: GLIDE BECOMES A MODULE + THREE PRESET TIERS (human, 2026-08-06)

Two additions to the interface renovation, both recorded in `docs/design/layout-lab.html`.

### Glide is a module now, not a knob

Human: *"glide now needs to be its own module since the inertia update."* Correct, and the
evidence is already in the repo. The bend lab settled **five travel laws** that do not sound
alike — constant time · constant rate · lag · spring (true inertia, overshoots) · lag→constant
rate — each with its own parameters plus dist→overshoot, return ×, and ζ. And it has
**destinations, not a target**: note pitch, bend wheel, mod wheel, MPE per-note bend, each able
to take a different law. That is a small matrix.

The symptom is already visible in the shipped param list: ids **33 glide, 34 legato, 75
freqGlide, 89 polyGlide, 90 glideMode, 11 inertia, 70 inertiaCurve** — seven params spread
across two clusters with no home.

**Proposed placement: the MOD page**, beside the mod matrix, because it shapes control motion
(same family as LFOs and envelopes) and is set-and-forget more often than performed; a one-line
state readout goes on MAIN. Alternatives — its own page, or living on MAIN — are recorded as
open. Which laws actually ship is still governed by fold decision **A1**.

### Three preset tiers: global · morph corner · oscillator

Human: *"Oscillators themselves should have their own presets as well (so three levels of
preset: global, morph corner, and oscillator)."*

**The oscillator tier is nearly free.** ADR-082 gives every per-oscillator param the state key
`o<k>.name`, so an oscillator preset is exactly *the subset of state keys carrying one prefix* —
saving is a filter, loading into another slot is a prefix rewrite, and "copy osc 1 → osc 2"
needs no new format. That was not designed for presets; it falls out of the id scheme, which is
some evidence the scheme is the right shape.

**The corner tier needs a ruling before anything is built (A11).** A morph corner holds a value
for every parameter it owns — but which parameters?

- **(a) corners are global** — one corner spans both oscillators; a corner preset is a
  whole-instrument snapshot minus the globals. Four corner-sets. Simple, but a corner cannot
  morph the oscillators independently.
- **(b) corners are per-oscillator** — each oscillator owns four corners, so corner × oscillator
  is a 2×4 grid. Far more expressive (osc 1 morphs while osc 2 holds) at double the authoring
  surface and every "which corner am I editing" affordance.

This must be answered **before** either preset tier is implemented, because the FORMAT differs:
under (a) a corner preset contains per-osc blocks, under (b) it lives inside one. Getting it
backwards means rewriting saved presets — the one thing a preset format may not do.

Also open: what a global preset does to per-corner mod depths (A10 gave every morph-owned
matrix cell four), and whether an oscillator preset carries its corner values with it or lands
flat into the current corner.

## GATE ADDED — labs must survive loading (human-approved, 2026-08-06)

`./verify fast` now runs `tools/labharness/lab_load_check.mjs`: every lab HTML's
`<script>` blocks are executed in a `node:vm` context with stub DOM/audio globals, and any
load-time throw fails the gate. **This is a gate STRENGTHENING and a protected-path change to
`./verify`, made with explicit human approval** ("I'll follow your advice", 2026-08-06).

**Why.** L0026 — a setup-time callback reaching forward to a `const` declared further down —
has now happened **five times**, the fifth written by the agent that had just documented the
trap. The browser swallows the throw, the rest of the script never runs, and the page still
looks fine; the mod lab's entire matrix was missing for weeks that way. The lesson's own
falsifier said the fix was tooling, not care. This is that tooling.

**Calibrated before shipping**, since a gate is worthless until shown to fail on the bug it
exists for. Both historical instances re-injected into the current mod lab: the `mtx` TDZ is
caught (`Cannot access 'mtx' before initialization`), the bare-`SR` typo is caught
(`SR is not defined`), an untouched control passes, and all 12 labs are green. The first
injection attempt was itself faulty (it put the declaration back *above* the wiring, so
nothing was reproduced and the checker "passed") — caught by asserting the injected offset.

Deliberately NOT a static `no-use-before-define` scan: that over-flags every function declared
early that references a later const, which is the common and correct case. Executing the file
has a false-positive rate of zero by construction. One false positive did surface in the
checker itself — a missing `Event` global made it blame `spectra-lab.html` — and was fixed
before shipping.

L0026 promoted **candidate → canonical**; falsifier restated to "a load-killing lab bug reaches
review while the gate passes".

## KURO-SYNCED FX — a module CLASS, not two special effects (human, 2026-08-06)

Human: *"the chorus and phaser are going to be FX modules; they have special behavior since
they can sync to the global kuro LFO, but maybe there are other FX modules that will apply to
as well."*

**Recorded as the durable design fact:** chorus and phaser are destined for FX slots, and their
Kuro sync is **not a quirk of those two effects** — it is a property they happen to be the
first to use. Treat it as a module class with a shared contract, or it will be reimplemented
per-effect and drift.

**What makes an FX eligible.** Exactly one structural property: *N parallel elements whose
parameters can be steered independently*. The chorus has N delay taps, the phaser has N allpass
stages — and a swarm of coupled LFOs steering them is what turns "N detuned copies" into a
coherence axis. K controls how the elements move **relative to each other**, which is the same
thesis as the oscillator engine, in a different domain.

**The candidates are already structured for it** — every effects core has a band/line count and
a loop over parallel elements:

| core | parallel elements | max |
|---|---|---|
| `time_core.h` | echo lines / room FDN lines (`p.nb`, `tSm[i]` per line) | 12 |
| `filter_core.h` | per-band filters (`p.nb`) | 24 |
| `notch_core.h` | notches (`p.nb`) | 12 |
| `fx_rack.h` | comb lines (`combs`, already per-line retune + gain) | — |

Reverb is the most interesting untried one: modulating FDN delay lines is standard practice,
but doing it with a *coupled* swarm rather than independent LFOs is precisely the instrument's
argument — and the ER/tail split (ADR from the orchestral research) already gives it two
element groups that could sync differently.

**The shared interface is the `link` parameter**, which is what makes the distinction *global
sync vs independent motion* rather than just "an LFO per effect": each FX swarm points at the
master rotor and `link` sets how strongly it is pulled toward it. `link = 0` is an independent
swarm, `link = 1` is welded to the global Kuro. **Status: lab-only.** `link` exists in
`mod-lab.html`'s `KuroSwarm` and in the effects labs; **no C++ core has it yet** (verified:
`grep -n link src/*.h` returns nothing). So the contract can still be designed rather than
retrofitted — which is the cheap moment to do it.

**Open, and worth an ADR before the second module is built:** whether every Kuro-synced FX
shares ONE swarm or owns its own with a link strength (the labs currently do the latter —
`choSwarm` and `phSwarm` are separate, each with `master = rotor`); whether `link` is per-FX or
per-element-group; and how this composes with the mod matrix, since `choDep` / `phDep` are
already matrix destinations and a second control path into the same parameter is exactly the
collision class that produced the chorus crash. Queued as **B17**.

## LAB BRIEF — modulator editor (LFO + envelope), queued (human, 2026-08-06)

Human: *"modern synths all have robust LFO and envelope editors. Could we please roadmap a lab
for that?"* Queued as **B16**, not built. This brief is the scope so a future session does not
have to re-derive it.

**The gap, stated plainly.** The modulators are the weakest surface in the instrument. The mod
lab's LFOs are `ClassicLFO` with exactly three parameters (`rate`, `shape`, `retrig`) and the
envelope is a plain ADSR. Every other subsystem here — the swarm, the FX rack, the morph — has
had a lab and a measurement campaign; the modulators have had neither, which is why feature
requests keep arriving against them one at a time.

**Absorbs the outstanding modulator requests** (all still untouched from the 2026-08-05
feedback message, and they belong together rather than as five separate patches):
- **reverse-saw LFO shape** — trivial, but it is the tell that the shape set was never designed
- **two S&H kinds**: a *global* S&H propagating one sampled value across every attached mod,
  and a *per-mod* S&H that syncs to the Kuramoto timing at independent levels
- **double-click to reset a mod value**
- **ownership tier** — which mods have a baked-in tier vs a modifiable one (open: the human
  flagged uncertainty about the framing itself, so the lab should make the question concrete
  before answering it)
- **polarity per route** — surfaced by the reachability probe: unipolar/bipolar is currently a
  property of the SOURCE, not the route, which is what makes `R → Kboost` half-inert

**What "modern" means here, concretely** — the survey the lab should run before building:
multi-segment envelopes with per-segment curve (beyond DAHDSR), envelope-as-LFO (loopable),
drawable/MSEG shapes, tempo sync with dotted/triplet divisions, per-voice vs global instances,
unipolar/bipolar switching, and depth-per-destination rather than one depth per source.

**The HYPERSAW-specific angle, which is the reason this is a lab and not a copy job.** Here the
LFOs are *swarm voices* — K1–K8 are the rotor's oscillators, coupled, not independent. A
conventional editor assumes independence; ours must express what the coupling does (lock,
splay, drift) without hiding it behind a shape dropdown. Same for envelopes: the VST already
has per-voice envelopes with scatter, so the editor must show the *distribution* an envelope
produces across voices, not one curve. The existing scatter-varied envelope display in the
Envelope tab is the seed for that.

**Open before building:** whether the editor is one lab or two (LFO and envelope have different
visual grammars); and whether it supersedes the mod lab's LFO panel or sits beside it.

## MORPH-OWNED = PER-CORNER DEPTHS (A10 ruled + built, 2026-08-06)

**Ruling:** a morph-owned cell holds **four depths, one per corner**; a flip swaps which one is
live. Chosen over territory-gating and over deleting scope −2 — it is the only reading where
morph-ownership does something corner-ownership cannot.

**Semantic change:** owning a routing now *makes it live* (`gd()` returns 1 for scope −2). The
flip changes **which depth applies**, not whether the routing exists. That retires the old
`owner === 0` gate, which left a morph-owned routing dead on 6–71% of the field depending
purely on the reshuffle seed.

**Measured** with the demo preset's `K1 → K` cell authored as A +0.8 · B −0.8 · C 0 · D +0.4:

| position | rms | peak | mean K mod | owner |
|---|---|---|---|---|
| A top-left | 0.1042 | 0.431 | **+0.0528** | A (100%) |
| B top-right | 0.0979 | 0.393 | **−0.0528** | B (100%) |
| C bottom-left | 0.0772 | 0.272 | **0.0000** | C (100%) |
| D bottom-right | 0.1106 | 0.540 | **+0.0264** | D (100%) |

The mean K modulation is exactly proportional to each corner's authored depth
(0.8 : −0.8 : 0 : 0.4 → 0.0528 : −0.0528 : 0 : 0.0264), which is the acceptance evidence.

**Temperature characterised — it is the field-vs-territory dial.** How often the corner you are
*standing on* actually owns the routing, over 4 corners × 6 reshuffle seeds:

| temp | 0.15 | 0.25 | 0.35 | 0.5 | 1.0 | 2.0 |
|---|---|---|---|---|---|---|
| corner matches position | 100% | 100% | 100% | 100% | 88% | 58% |

(25% would be pure chance.) The demo preset now ships at **0.5** so corners are legible; higher
temperatures are the "random territory" regime the quantum-morph work is about. This was found
because the first measurement put corner A's ownership at C — at temp 1.0 the draw wins.

**UI:** four corner-tinted values under each morph-owned cell, live one lit; the box shows the
live corner's value and follows the flip; typing edits the **current owner's** slot (the
"edit = Owner" convention from the quantum-morph lab); cycling scope *into* morph-owned seeds
all four from the single depth and *out* collapses to the live one, so scope changes never
silently zero a routing.

**Bug caught during the build, worth recording:** `commit()` — the handler for typing and
dragging, i.e. the *actual* user path — wrote `lab.depth[...]` directly, bypassing the
`setDepth` helper. Patching only `setDepth` would have shipped an editor whose typed values
went into a field nothing reads. Both now route through one `lab.setRouteDepth()`. Same class
as L0011: patching the programmatic path while the UI path diverges.

## MORPH DEMO PRESET — and the morph-owned gate is a placeholder (2026-08-06)

Human: *"show me a preset that actually takes advantage of the morph panel so I can properly
test it."* Built as two buttons in the morph cluster — **preset: four corners** and
**+ self-drive**.

**The preset was chosen by measurement, not taste.** Each corner owns a routing on a different
axis so the corners cannot be confused by ear: A slow filter sweep (LFOA→cutoff −0.9),
B detune motion (K2→detune +0.9), C chorus swell (LFOB→choDep +0.8), D phaser motion
(K4→phDep +0.9), plus ENV→Kboost +0.64 **system-wide** to demonstrate the scope that survives
every flip. Measured at the five field positions:

| position | rms | peak | zero-crossings |
|---|---|---|---|
| A top-left | 0.0886 | 0.251 | 2017 |
| B top-right | 0.0963 | 0.297 | 1920 |
| C bottom-left | 0.0772 | 0.272 | 1929 |
| D bottom-right | 0.1061 | **0.494** | 2007 |
| centre | 0.0760 | 0.268 | 2103 |

A preset whose corners measured identical would demonstrate nothing however good it sounded,
so this table is the acceptance evidence, not decoration. **+ self-drive** adds LFOB→morphX
so the field sweeps itself — the composition, with flips/s as the readout.

**FINDING — morph-owned routings are mostly dead, by a hard-coded corner index.** `gd()` gates
a morph-owned routing on `this.owner[si][di] === 0` — the literal corner 0 — while ownership is
drawn *stochastically* by Gumbel-max over the field weights. At temperature 1 a log-weight gap
of ~3.9 is routinely beaten by the random preference, so even standing exactly on corner A the
owner can be corner 2 (measured). Result: a morph-owned routing is live on

| reshuffle seed | fraction of the field where it is live |
|---|---|
| 1 | 18% |
| 7 | **6%** |
| 12345 | 71% |

— i.e. whether the routing does anything at all is decided by a random draw against a
hard-coded index. `routeGain()`'s own `-2` branch is meanwhile a no-op
(`return w[owner] > 0 ? 1 : 1`, both arms 1), with a comment deferring to "owner gates
elsewhere".

**Why this is a design question and not a fix.** Ownership already *means* "the corner the
field currently favours", so gating on it is circular — owning a routing should make it live,
not conditional. The construct only earns its keep when a cell can hold **different depths per
corner**, so that a flip switches between two versions rather than toggling one on and off.
That is the same gap as the standing open question "what a corner-owned routing does when its
corner owns nothing". **Not changed unilaterally** — raised as **A10**. The preset ships with
the morph-owned cell included and the hint says plainly that it may appear to do nothing and
why.

### Rejected routings — asked for, and the measurement says there is nothing to reject yet

Human, 2026-08-05: *"Maybe there are some routings we explicitly reject, like R → Kboost."*
Built `tools/labharness/modlab_reach.mjs` to find the rule behind that instinct rather than
hand-maintain a list (a list rots; a rule does not). It measures each source's ACTUAL
excursion and each routing's response either side of zero depth:

| source | observed range | polarity |
|---|---|---|
| K1–K8, LFOA, LFOB | −1.0000 … 1.0000 | bipolar |
| **R** | −1.0000 … **−0.3322** | **negative-only** (it is `R*2−1`, and R never reaches 0.5 below the coupling knee) |
| **ENV** | **0.0363** … 1.0000 | **positive-only** |

`Kboost` is the **only** rectified destination (`8 * max(0, kbMod)`). Result: **zero routings
are fully unreachable**, and exactly two are half-unreachable — `R → Kboost` (positive depth
inert) and `ENV → Kboost` (negative depth inert). They are the same phenomenon mirrored.

**So a reject-list would be wrong here, and the reason matters.** `R → Kboost` works fine at
*negative* depth, and above the coupling knee it works at positive depth too — the sweep
measured it alive at rotor K=1 (max R 0.996) and at detune 0.05 (max R 0.984). Its mirror,
`ENV → Kboost`, is the routing the lab's own onset A/B deliberately uses. Rejecting the pair
would delete capability that works to suppress a half that doesn't.

**Built instead: polarity markers.** A matrix cell is hatched when its source has so far only
gone one way and its destination rectifies the other way, with a tooltip naming the observed
range and which half of the knob is inert. Computed from the LIVE excursion, so raising rotor
K past the knee clears the `R → Kboost` marker on its own instead of leaving a stale warning.
Seeded at load from a throwaway probe instance (a separate lab, ticked 1 s — ticking the live
one would advance its rotor/LFO/env and break the deterministic start the spec pins), so the
markers are right on a fresh page rather than only after you have already been bitten.

**The rejection mechanism itself is NOT built** — with zero fully-dead routings it would be an
empty abstraction, which the charter's "reduce, never invent" forbids. If a genuinely
incoherent pair appears (a new rectified destination, or a source that cannot leave one sign),
the marker logic is where it goes. Register item **A9** is answered by this: no ruling needed
unless you want `Kboost` unrectified, which is a separate question.

**Calibration note (L0016 again).** The sweep's first run reported 53 dead routings and
every single one was my bench, not the lab: K5–K8 are hard-zeroed above the rotor's
oscillator count (the lab's own UI hides those rows), cutoff defaulted to fully open so a
positive cutoff mod clamped instantly, and morphX/morphY are inert BY DESIGN when every
scope is system-wide — in this lab morph position gates scope, it does not blend corner
parameters. The bench now configures each destination to have somewhere to go, so a dead
reading means something.

## STANDING CONVENTION — corner colour is global, and it means ONE thing (human, 2026-08-05)

Human: *"I want to make sure the color mapping (corners to parameters) stays consistent
across the whole UI so it's always clear which parameters are owned by which corner. The
osc page, when morph is somewhere intermediary, will have each parameter colored (and
glyphed) according to its source."*

**The rule.** The four corner hues + glyphs (◆ ▲ ● ■) are a **global vocabulary**, not a
morph-page decoration. Any parameter anywhere in the interface that is currently OWNED by
a morph corner is tinted and glyphed by that corner — the OSC page, the FX rack, the
envelope tab, everywhere. Ownership is answerable on sight from any page, without
navigating to the morph.

**Consequences that fall out of it, and are the reason to write it down now rather than
discover them at fold time:**
- **No other feature may claim those four hues** for an unrelated meaning. The colour is
  spent; a future "engine colour" or "MPE colour" must use a different channel (border
  style, icon, brightness). This is the kind of decision that is free now and expensive
  after three pages ship.
- **Glyph rides with colour everywhere**, per the already-ratified pairing — so the
  vocabulary survives colour-blindness and small controls on every page, not just the
  morph.
- **A parameter with no corner owner needs a defined neutral** (currently: no tint). That
  includes every parameter when the morph is disabled entirely, which must not read as a
  bug.
- The morph lab's **tint mode** (Dominant vs Mixture) is now a GLOBAL setting, not a
  morph-page preference, since it changes how the whole interface reads.

## MOD SCOPE — corner-owned vs system-wide (human direction, 2026-08-05)

Human: *"Different mods will clearly have to have different domains/priorities, and should
probably be colored to show it. Sometimes a mod belongs to a corner, sometimes to the whole
system. These will need to coexist elegantly."*

Recorded as the design frame for the reopened mod lab. A modulation routing has a **scope**:
- **Corner-owned** — the routing belongs to one corner's patch. It is subject to the morph:
  its depth blends where corners agree and its topology flips where they differ (the (g)
  ruling). It carries that corner's colour and glyph.
- **System-wide** — the routing belongs to the instrument, not to any patch corner. It
  survives every flip and every reshuffle untouched, and reads in a neutral colour.

**Why the distinction is load-bearing:** a performance macro (mod wheel → filter) that
vanished because the morph flipped to a corner that never defined it would be a bug, not a
feature — whereas a patch-defining routing that *didn't* change with the patch would make
the morph a lie. Both failure modes are real, so scope has to be explicit rather than
implied. The colour is the whole legibility mechanism: corner-tinted routings move with
the morph, neutral ones do not.

**Open for the lab:** whether scope is per-routing or per-source; whether a corner-owned
routing whose corner currently owns nothing is silent or falls back; and the priority rule
when a corner-owned and a system-wide routing target the same destination (sum, or does
one win).

## FLIP-BOUNDARY CHATTER — the question restated concretely (2026-08-05)

The human asked what this means, so it is restated here in full rather than left as jargon.

A discrete parameter (filter type, wave, routing topology) is owned by whichever corner
wins at your current field position. Near a boundary that win is by a hair. **Route any
modulator to the morph X/Y position and it will sweep you across that boundary
repeatedly** — every crossing is an instant discrete flip. At 0.1 Hz that is a dramatic
slow switch; at 5 Hz it is ten filter-type changes a second, which is a stutter. And the
smaller the win margin, the smaller the modulation depth needed to cause it — so the
artifact appears exactly where the morph is most delicately balanced.

**It may be a feature** (rhythmic glitching is a legitimate sound) but it must be a choice.
**Two existing controls already answer part of it, which is worth noticing before building
anything new:** `flip glide` (8 ms crossfade) softens each flip, and `flip timing → next
note` converts continuous chatter into at most one flip per note — a strong answer, since
it makes the artifact musical by construction. **The missing piece is hysteresis**: a
deadband so that crossing back requires a margin, not a hair. Nothing in the lab implements
that yet, and it is the natural third control.

## ADDITIVE-SYNTH RESEARCH BRIEF — for the SPECTRA return (human, 2026-08-05)

Human: *"let's look through the Loom II documentation (and any other commercial additive
synths) to find how they managed to add modern dynamism to their additive synth so we can
take inspiration."* Roadmapped, NOT run now — SPECTRA is deferred behind the SAW-first
renovation, and research whose findings cannot be acted on for weeks goes stale.

**The question to answer** (sharper than "what do they do"): SPECTRA measured as *dark and
static* — centroid 562 Hz vs SAW's 2449, and mean R pinned at 0.96 within a second of the
attack. Both are additive's classic failure modes. So: **what do shipping additive synths
do about staticness specifically?**

**Targets:** Air Loom II (the human's lead), and for contrast the other live approaches —
Razor, Harmor, Alchemy's additive mode, Iris/Chromaphone-adjacent resonator hybrids, and
the historical Kawai K5000 / Kyma lineage.

**What to extract, per product:** (1) the *animation* mechanism — per-partial envelopes,
spectral morphing, partial-index-dependent LFOs, noise/formant layers; (2) how brightness
is reached without unbounded partial counts (band-limited spectra, resynthesis, filter
layers on top); (3) which controls are exposed vs derived, i.e. how they avoid 400 partial
knobs; (4) explicitly, what they DON'T do — the absence of coupling physics is our claimed
differentiator and it should be verified rather than assumed.

**Convention reminder** (ratified earlier): factual naming is fine in process docs;
comparative not imitative verbs; never commit competitor-rendered audio, presets or
captured data; SPEC.md stays competitor-free.

## RESURFACED — the other quantum thread, and where it now converges (2026-08-05)

Human: *"buried somewhere in our documentation is a conversation we'd had about a
different kind of quantum behavior; let's resurface that."* Found — it is **ADR-052**,
`docs/proposals/2026-07-19-kuramoto-entangled-mods.md`, accepted in DIRECTION with phases
gating individually. Distinct from the quantum MORPH (which is Gumbel-max preset
flipping); this one borrows the *structure* of entangled systems for coupling:

- **Phase A — observable extraction.** Publish the swarm's own emergent quantities as a
  mod-source bus: `R`, `ψ`, `drift` (dψ/dt co-rotating), `direction` (signed, hysteretic),
  `R₂`, per-voice `lock_ratio`, and `slip` events (θᵢ−ψ crossing ±2π). ~70 % are already
  computed every control tick as viz readouts; the new work is the *bus* (unwrap,
  smoothing, slip events) on the existing 2756 Hz tick.
- **Phase B — coherence budget.** A conserved [0,2] budget makes a second bank's
  *effective K* — not its gain — anticorrelate with the first bank's R. This IS the
  cross-coupled multi-oscillator variant.
- **Phase C — membership spinors.** Per-voice (a,b) ∈ ℂ², equal-power two-path render,
  phase carried across so a migrated voice beats against its new ensemble. C.1 stochastic
  tunneling, C.2 Pareto blinking (quantum-dot statistics — long stable stretches punctuated
  by chatter), C.3 coherent Rabi.
- **Phase D — measurement bus.** Slip/note/transient events collapse spinors by Born rule,
  then relax back.

Honest framing preserved from the ADR: classical coupled oscillators cannot literally
entangle — these are structural analogues, and that honesty stays in README/PRIOR-ART.

**Why it resurfaces NOW rather than as trivia.** Three live threads converge on it:
1. **Multi-oscillator** (the current renovation target) was ALREADY specced to carry "an
   initial concept PROPOSAL for quantum interference between banks — how superposed banks
   interfere rather than merely sum," with ADR-052 Phase C named as the nearest precedent.
   The multi-osc work cannot be designed without answering it.
2. **Phase B is literally the cross-coupled multi-osc variant** — so "how do two swarm
   banks relate" already has a proposed answer on file.
3. **Phase A is the source side of the mod matrix** the human just asked to reopen.

**MORPH × MOD — already has a recorded position, and it answers the human's worry.**
ROADMAP already states: the quantum morph is the MACRO/preset layer, the Kuramoto-LFO
matrix is the CONTINUOUS layer, and *"the two compose: modulate where you stand in the
morph field"* — i.e. field position (x, y), temperature, coupling and the reshuffle
trigger are automatable macros and natural mod DESTINATIONS. **Confirmed gap:**
`mod-lab.html` (1145 lines) contains **zero** morph references — its destinations are
`K, Kboost, detune, cutoff, level, choDep, phDep`. So the composition is designed on paper
and entirely unbuilt, which is exactly the human's instinct that "the mods may interact
strangely with the morph." Specific unresolved collisions to work in the reopened lab:
morph flips are DISCRETE and instant while mod is continuous (what does modulating toward
a flip boundary sound like — chatter at the edge?); (g)'s ruling says depth blends and
topology flips, so a mod routing that IS morphed changes shape mid-modulation; and the
morph's own reshuffle can re-own a parameter a mod is actively driving.

## ATTRACTOR-BASIN SEARCH — abstract direction, recorded (human, 2026-08-05)

Human: *"eventually doing a deterministic sweep of different feature sets and searching
for attractor basins. Some kind of gradient descent system for finding oases of coherence
in parameter combinations that tend toward incoherence at most settings. Maybe this could
simplify the cooperator and other future engines and replace the complex interface."*

Recorded as a research direction, not a queue item. **The observation that makes it
tractable: this project already computes its own coherence scalar.** R (the order
parameter) is exactly "how coherent is this configuration", it is already produced every
control tick by every engine, and ADR-052 Phase A proposes publishing it. So a search has
a ready-made objective function without inventing a perceptual metric — which is normally
the hard part of automated patch search. Candidate objectives beyond R: slip rate (peaks
near K_c — the interesting edge), R-variance over time (an oasis that MOVES is more
musical than a static one), and the COOPERATOR ratio-error measure (distance to the just
lattice, already implemented).

**Why it is a strong fit for COOPERATOR specifically:** the human's verdict there was
"too many complicated novelties, hard to control." A basin search inverts that problem —
instead of exposing eleven controls and asking the user to find the good regions, find the
good regions offline and expose *those* as the interface. That is a genuinely different
answer to the complexity problem than "simplify the panel."

**Honest caveats before anyone builds it:** (1) R measures coherence, not *musicality* —
R = 1 is a locked, possibly boring drone, so maximising R naively finds silence-adjacent
attractors; the objective probably wants a band, not a maximum. (2) A deterministic sweep
of an 11-D space is combinatorially hopeless — this needs gradient descent, or coarse
sampling plus local refinement, and the parameter space is not smooth (phase transitions
are knees, as the K measurements repeatedly showed). (3) Verification would need the
oracle discipline this project already has: a found basin must be reproducible from a
seed, and "it sounded good" is not the gate.

## STRATEGIC PIVOT — SAW-first renovation (human, 2026-08-05)

Human: *"I would actually like to remove spectra from the VST for now (at least from the
UI for new patches) while we renovate the interface with the new pages and the morph and
the mod matrix and the full suite of FX, etc., and just focus on getting multiple
oscillators and the morph and the mods right for the SAW engine which is, frankly, the
most broadly functional engine as it stands."*

**Done today, minimally and reversibly:** the SPECTRA option is hidden from the engine
selector for NEW patches. Nothing was deleted — param id 43 still exists, `spectra_core`
still runs, state still round-trips, host automation still reaches it, and
`spectra_check` stays in `./verify full` (all ten chains green, parity 147/147). A patch
saved as SPECTRA **puts the option back, labelled "SPECTRA (legacy patch)"**, so old work
stays loadable and visibly explains itself. Reversing the hide is one line.

**The new order of operations.** Interface renovation first, on SAW only:
1. **Layout lab** — resume the IA audition; it is the next step (human).
2. **Multi-oscillator** — the open ADR (B11). The layout lab already stages the question.
3. **Morph** — feature-complete in its own lab; needs a page and the fold path.
   **RULED (human, 2026-08-05): MAIN gets its own compact morph XY**, with the full
   editor (territory, capture, copy-from, tint/glyph) on the morph page. This closes the
   layout lab's open question "morph pad (compact) — full editor on its own page?".
4. **Mod matrix** — design accepted (ADR-053), right-click access queued (B8), no page yet.
5. **Full FX suite** — rack exists; reverb/delay slots queued (B2).

**Consequence for the register:** SPECTRA-facing items are now *behind* the renovation.
B5 (ADR-037 shared voice path) and the SPECTRA parity-audit gaps (MPE per-note bend,
mono/glide for SPECTRA) are **not cancelled but deprioritised** — they only matter when
SPECTRA returns to the UI. The SPECTRA lab's findings (K taper, cloud spacing, lock wave)
stay recorded for that return.

**Intelligent-randomness ruling (human, same message):** *"Some kind of automatic
conditional rulesets could be applied, but likely these are more cases for hand-tailoring
the distributions for those parameters."* So the resolution leans **hand-tailored
per-parameter distributions**, not an automatic constraint solver — which also fits the
territory-authorship tools already built (corner weight + pin are exactly hand-tailoring).
The guard case (`lfoDest` off making rate/depth inaudible) may still deserve a mechanical
rule since it is logic rather than taste; everything else is authored. Human also noted
the analogous collision they had in mind: *double ring modulators on two different FX
slots*.

## OPEN WORK REGISTER (reconciled 2026-08-03)

**Why this exists.** ROADMAP is a narrative record — excellent for *why* a decision was
made, useless for *what is open right now*, because an item's status is buried in the
paragraph that created it. Two stale claims were found in the status block above during
this reconciliation (ADR-037 recorded as unruled when it was ruled 2026-07-18; branch
pruning listed as pending when it was done). **This register is the index; the sections
below remain the evidence.** Update it in the same change that changes an item's status.

### A · Waiting on the human (no build work possible until answered)

| # | Item | Where |
|---|---|---|
| A1 | **Bend inertia fold** — FULLY RULED 2026-08-06: laws 1–4 (5 cut), constant-rate default, quantise as a modifier, per-destination laws linked by default. B19 buildable | § Pitch-bend inertia; `docs/design/bend-lab.html` |
| A2 | **Swarmalator — DEFERRED by human ruling 2026-08-07** ("table it for now and revisit down the line, not very high priority"). NOT an open question and NOT awaiting the human; do not surface it in status roundups. Core + `swarmalator_check` stay gated and unwired, which is the correct resting state. Revisit only on a fresh human ask | § Swarmalator tabled |
| A3 | **Shape lab fold** — mandate rulings: fold mode and carrier purity both leave saw territory deliberately | § Lab campaign 2 item 6 |
| A4 | **ITD max 0.6 → 0.3** — proposed on measurement (metrics saturate above 0.15 ms); wants an ear A/B first | § Open questions 2026-08-03 #1 |
| A5 | **AP freq 700 Hz** (super-width mode D) — arbitrary, never measured; A/B in the width lab and pin | § Open questions 2026-08-03 #2 |
| A6 | **SPEC citation amendment** — protected path, awaiting approval | § Timbre-space research |
| A7 | **Law/dist widening** — state compatibility, scope, and which core-only params to expose | § Open questions for the human (4 sub-items) |
| A8 | **Phase 2/3 formal gate ratification** — shipped and evidenced, never formally closed | § Phase 2 / Phase 3 gates |
| A9 | **Mod source polarity** — ANSWERED 2026-08-05 by the reachability probe: zero routings fully unreachable, two half-unreachable (`R → Kboost`, `ENV → Kboost`), now marked in the matrix rather than rejected. Only residual question if you want it: should `Kboost` stop being half-wave rectified | § Rejected routings |
| A10 | **Morph-owned routing semantics** — RULED 2026-08-06: **per-corner depths per cell**. Each morph-owned cell holds four depths; a flip swaps which is live. Implemented + measured | § Morph-owned = per-corner depths |
| A11 | **Morph corner scope** — RULED 2026-08-06: **global** — one corner holds every per-oscillator parameter of *both* oscillators. Unblocks B20 | § Morph corners are global |
| A12 | **RECOMMENDATION FILED 2026-08-11** (§ A12/A13 recommendations). Envelope per-osc; mono/legato/polyGlide/glideMode global by structure; travel-law family deferred to B19; beatMult+oversample global. **Which of the 13 core-owned params become per-oscillator?** — width/mono/inertia clearly yes; the amp envelope arguably; oversample/beatMult/glide-family probably patch-level. Additive only while their +1000 ids stay unallocated | § Re-order: master/mixer page first |
| A13 | **RECOMMENDATION FILED 2026-08-11**: document + expose, do not change the physics; if ever fixed use rotated even spread, never anti-null redraw. **Retrig-off dead starts** — reference physics (random-phase nulls under slow detune beating; reference shows identical 5/20). Options: document / anti-null redraw (reference edit + ADR) / rotated even spread | § Chord retrigger resolved |

### B · Queued build work

| # | Item | Status |
|---|---|---|
| B1 | **Baseline saw to Nyquist** | recommended next DSP fold; caveat recorded (buys air, not the fullness already solved by drift) |
| B2 | **FX rack: reverb + Kuramoto-modulated delays as slots** | labs done, not folded |
| B3 | **Modulation lab → golden + matrix** | **deliberately blocked**: rotor axes still moving, a golden measured now would churn (and its ACCEPTANCE rows are protected-path) |
| B4 | **E1 remainder** — SWARM-FX GUI + L0-17/18 | cores parity-proven, shell incomplete |
| B5 | **ADR-037 follow-up** — shared voice path behind a switch, for an A/B against the frozen cores | ruling done, follow-up open |
| B14 | **COOPERATOR** — Kuramoto-FM engine candidate | **DEFERRED (human verdict 2026-08-05)** — pare to basics before building complexity; see the verdict note in the COOPERATOR section |
| B6 | **Lab campaign 3** — SPECTRA expansion · swarm filters · quantum morph | **SPECTRA lab BUILT 2026-08-04** (findings below); **swarm-filters lab BUILT 2026-08-04** (`docs/design/filter-lab.html`, findings below); SPECTRA + quantum morph not yet built |
| B7 | **Lab-visual fold backlog** — bend step-response · width scope+cliffs · reverb ER/tail · ensemble raster | per the convention below; bend ships with A1 |
| B8 | **Mod matrix reachable by right-click on every parameter** | design accepted, not built |
| B9 | **Pan motion speed + bipolar position weighting** (subsumes `motionCenter`) | requested 2026-07-31 |
| B10 | **Slider units/naming pass** + feature-by-feature visual breakdown for docs | deferred until the interface settles |
| B11 | **Multi-oscillator** | **ADR-082 RATIFIED (2 slots); Amendment 1 (stride 1000); increments 1 AND 2 SHIPPED** — `kNumOsc = 2`, second core summing, silent by default, parity 147/147 unchanged. Remaining: GUI (osc page) + B20 preset tiers — id scheme (+100 stride, osc 0 keeps its ids), per-osc state keys, CPU budget. Blocks all interface-renovation GUI work; needs ratification |
| B18 | **ADR-082 increment 2 blockers** — (a) state version gate WIDENED + ratified 2026-08-06 (accepts 1 or 2, still red on unknown). (b) min-spec CPU measurement STILL OPEN before 2 oscillators ship | § ADR-082 increment 1 |
| B19 | **Glide/travel module** — **CORE + ORACLE SHIPPED 2026-08-06** (`src/glide_core.h`, `glide_check` in `./verify full`, parity 11/11 worst 3.5e-08, not yet in the audio path). Remaining: shell integration (MUST ship the overshoot-linear damping taper from bend-lab 2026-08-08 — knob domain, never glide_core) — destination mapping onto the 7 existing glide params (11/33/34/70/75/89/90), which wants ADR-082-level care since ids are append-only | § Glide core ported |
| B20 | **Three preset tiers** — **oscillator tier SHIPPED 2026-08-06** (`src/osc_preset.h` + `preset_check` in `./verify full`; format slot-agnostic, globals excluded; plugin wiring deferred to the GUI that calls it). Corner tier unblocked (A11 ruled global), patch tier already exists as CLAP state | § Layout: glide + preset tiers |
| B21 | **Step glide** — **TESTED IN LAB 2026-08-07**: quantise + q·hysteresis + q·step-time controls in bend-lab; gate paces steps onto a time grid (measured 250 ms commits at qTime 250) only when slower than the law. Remaining: tempo sync + C++ fold with B19's shell wiring. **Scale source answered 2026-08-09**: `hzScalePicker` (root + 12-bit mask, no scale enum) — the shell exposes root + mask, not a scale ID, so named scales stay a UI table |  § Step-glide tested |
| B22 | **K link AND phase link — two mechanisms** — K link shares a *parameter* (does NOT lock oscillators together); `link` is the *dynamical* inter-swarm coupling that actually does. Wants an ADR before building so the naming distinguishes them | § Re-order |
| B23 | **RULED 2026-08-10 (ADR-088): dense crosspoint matrix, id block ACCEPTED** (routing at 10000+, topology as patch state). Both halves settled; ids are append-only *by CLAP spec* (`params.h:212`) while the param SET is revisable (`params.h:70-77`) — dynamic params are a live option, not a closed door. **Increment 1 (core+oracle) and increment 2 (in the audio path, inert, one source) both landed 2026-08-10/11**; per-oscillator sources are the next increment and carry the bass-mono ordering decision. Lab **SHIPPED 2026-08-09** (`docs/design/routing-lab.html`): three topologies + cost table + morph/mod composition. Initially recommended **C (matrix DAG)**; a 2026-08-09 research probe found the **menu incomplete** (missing sparse connection-slots, reorderable chain, bus model) and the cost table built on an unexamined assumption that every routable quantity needs its own CLAP param. Round 2 (2026-08-09) added D/E/F, the crosspoint initial value, and a corrected cost model (C is **8 automation ids**, not 120). **Still unruled — escalated to FOUNDATIONS**: §3.2 rules modulation routing sparse, §3.5 leaves the signal graph a plain chain | § B23 routing lab · § research probe · § round 2 |
| B24 | **Master/mixer page** — **INCREMENT 2 SHIPPED 2026-08-09** (mute/solo params 104/105 + per-osc meters, `mixer_check` built-not-gated). **INCREMENT 1 SHIPPED 2026-08-07**: per-osc strips (level+width, fixed-id, both visible at once) + masterVol (id 100, first stride-1000 allocation, unity-exact). Remaining: per-osc pan (LAW UNRULED — balance vs image-shift, see § OPEN), rest of A12 | § B24 increment 1 · § increment 2 |
| B25 | **Global time scale macro** — one control over the 16 time-domain params (plus the glide laws, echo/room decays and reverb EDT still to land). Multiplicative, with a rule for clamped ranges so it cannot silently stop affecting some controls | § Global time scale |
| B26 | **Depth-of-depth (mod-on-mod)** — each active routing's depth becomes a destination (`R → (LFO → cutoff).depth`); surfaced per active routing, carries scope, reuses morph hysteresis against threshold chatter; wants the reverse-saw + tempo sync (B16) so the motivating patch works day one | § Mod matrix: depth is a target |
| B27 | **Arp-sustain gate** — **SHIPPED 2026-08-08** in `notefuzz_check`; calibrated (naive steal-oldest fails 55× below threshold, ADR-083 policy passes). Retrig toggle also now exposed in gui2 | § Voice steal fixed |
| B12 | **BLEP aliasing re-measure at incommensurate f0** | earlier measurement used a commensurate f0 |
| B13 | **Granular-sibling intake** | gated on that sibling maturing; INTEGRATIONS.md route |
| B15 | **Promote the mod sweep to a gate?** — `tools/labharness/modlab_sweep.mjs` is runnable but not wired into `./verify` (adding it is a gate change, human call). ~3 min for 216 routings | § Full mod-matrix sweep |
| B16 | **Modulator editor lab (LFO + envelope)** — queued 2026-08-06; absorbs reverse-saw shape, the two S&H kinds, double-click reset, ownership tier, per-route polarity | § Lab brief — modulator editor |
| B17 | **Kuro-synced FX module contract (ADR candidate)** — chorus/phaser are the first two of a CLASS (any FX with N steerable parallel elements: time/filter/notch cores + fx_rack combs all qualify). Design the `link` contract before the second module lands; `link` is lab-only today, no C++ core has it | § Kuro-synced FX |

### C · Closed during this reconciliation
- **Divergence ADRs** (root-pivot topology · pan default image · saw retarget) — each is
  recorded, but *inside* the ADR that made the change rather than as three standalone
  entries: root-pivot in the fold ADR + `traces/2026-07-24-fold-root-pivot.md`; the pan
  default's mono-fold consequence explicitly "accepted with the divergence"; retarget in
  ADR-026. Tracked as open for weeks because the task expected three separate documents.
- **Prune merged branches** — 91 local branches deleted 2026-08-03 after verifying every
  one was fully contained in `main` (no stashes, no dirty files, no remote-only commits).
  95 remote branches remain on GitHub.

## COOPERATOR — Kuramoto FM, engine candidate (ratified 2026-08-05)

Human ruling on the FM proposal: **both architectures** (CLOUD and NETWORK), **full force
system from day one**, name **COOPERATOR**. Lab built: `docs/design/cooperator-lab.html`
(tracked like the campaign labs, not gitignored). Breaks the saw mandate knowingly — an
ADR-045 (Γ,W) kernel argument plus an explicit mandate line is owed AT FOLD TIME, same
shape as the swarmalator's path; no SPEC document until the audition says it survives.

**The premise** (from the design discussion): FM is already phase coupling — strong,
unidirectional, dumb; Kuramoto is weak, mutual, self-correcting. And FM makes the physics
MORE audible than the saw bank: every cent of ratio error sprays enharmonic sidebands, so
drift→lock is a dramatic timbral event rather than subtle chorus.

**Measured at birth (all in the lab, in-browser):**
- **Ratio gravity is the headline and it works**: modulator ratio set to 1.48, gravity on →
  mean error to the just lattice **23.2 ¢ → 0.0 ¢ in 2 s** — captured to 3/2 exactly, and
  capture acts on the HOME so drift orbits the captured ratio instead of escaping it.
- **Cloud lock**: R **0.394 (K=0) → 0.933 (K=0.5) → 0.941 (K=1)** at 12 ¢ spread; the
  critical transition sits between K 0.25 and 0.5. At 50 ¢ spread lock is PARTIAL
  (R 0.803) — by design, the coupling ceiling (40 ¢ at K=1) is commensurate with the
  spread knob so over-spread is an audible regime, not a bug.
- **Bounded chaos**: 2–3 s of the worst case (all 12 edges at 1.0, edge law full FM,
  index 8, two notes) stays finite with **zero** watchdog resets — tanh + the clamp hold
  it without the safety net firing.
- **Carrier participation in network mode is real and bounded**: Kuramoto edges into op 1
  bend the note ≤ 33.5 ¢ measured (clamp ±80) — documented in the lab, zero op 1's row to
  silence it.

**Two of my bugs fixed before shipping, both caught by measuring:** the first coupling
implementation added its correction to state the next tick overwrote (R flat 0.394→0.395
across the whole K range — a dead knob), and was ~170× too weak to cancel the detune it
fights; rewritten as fresh-per-tick frequency offsets with a ceiling expressed in CENTS.
And the network carrier's `lCur` was never rebuilt, so coupling would have random-walked
the note's pitch.

**Honest limit, queued as increment 2:** network R is low (0.2–0.3) because 1:1 phase
pull cannot lock ops at different ratios — true cross-ratio locking needs **n:m edges**
(sin(n·θj − m·θi), Arnold tongues). Stated in the lab rather than faked.

### VERDICT — deferred (human, 2026-08-05)

Human, after playing it: *"this system may be starting with too many complicated novelties
in its first swing — it's very hard to control and most of the sounds aren't particularly
interesting. My instinct is to pare it down to basics until it makes a decent sound at
all, and then build the complexity on top."* Rest of the build deferred; quantum morph
prototyped next instead.

**The verdict is fair, and the "edge law does nothing" report diagnosed it precisely.**
Verified before recording: in NETWORK mode the slider transforms the sound (zero-cross
111 → 1381 Hz between its extremes); in CLOUD mode — **the default** — the two extremes
are byte-identical, because edge law is network-only and the lab leaves it visible,
draggable, and dead. The user's experience was correct even though the DSP wasn't broken:
a control that does nothing in the mode you're in IS a dead control. That is the
first-swing problem in miniature — eleven force/coupling controls presented flat, no
mode gating, no hierarchy, novelties (R→index, edge morph, mutual edges) stacked before
a plain 2-op patch sounds good.

**Return path, recorded for the restart:** begin from the SUBTRACTED version — carrier +
ONE modulator, ratio, index, envelope, nothing else — and make that sound genuinely good
first (FM's bread and butter: index envelopes, velocity→index; the lab has none of
these, which is much of why the sounds were uninteresting). Then add ONE novelty at a
time in auditioned increments: gravity capture first (it measured best: 23.2 ¢ → 0 in
2 s), then the modulator cloud, then the matrix. Mode-gate the panel so only live
controls show. The measured mechanisms all survive — nothing here invalidates them; the
lesson is about ORDER of assembly, not the physics.

**Open (human):** which architecture survives (or both); does R→index earn its place;
n:m edges as increment 2; fold path (engine selector, SAW byte-frozen) if the ear says yes.

**When work resumes — build the WAVEFORM READOUT first (human, 2026-08-05):** *"more than
any other engine, it needs a detailed waveform readout."* This is a build-order note, not a
nice-to-have. The rest of the deferral verdict says pare COOPERATOR to one modulator and
make it sound good before adding novelty — and the reason the lab's sounds were hard to
judge is that FM's character lives in a waveform whose shape you cannot infer from a
spectrum or an R meter. Every other engine here is auditioned against a swarm visual;
COOPERATOR's equivalent is the wave itself, at enough time resolution to see the index
envelope bite. Ship it BEFORE the pared-down engine, so increment 1 is auditioned with the
instrument that can actually show what changed.

## SPECTRA feature-parity audit (human direction, 2026-08-05)

Human: *"make sure all the feasible decisions from across the envelope and pitch bend
(and obviously FX) modules work with this engine too."* Audited against the shell and
both cores rather than recalled — the gaps below cite where each one lives.

**Already works with SPECTRA (no action):**
- **FX rack, entirely** — bus-side post-oscillator, engine-blind by construction; the
  comb's note feed fires at the common note-on point for both engines (ADR-071), and the
  new per-slot tone (ADR-080) rides the same path.
- **Global pitch suite** — octave/semi/fine/pitch (ids 35–38) reach SPECTRA through its
  `tune` factor (ADR-057). A future WHEEL-lane bend inertia folded at this seam is
  engine-agnostic for free.
- **Its own ADSR** (ids 65–68, ADR-055) with its own reference constants.
- Shared coupling surface: K / onset / dissolve / seed / vol / retrig / width.

**Feasible and cheap (queue):**
- **MPE per-note bend.** SPECTRA has no per-voice `noteTune`; the shell comments the gap
  explicitly (`hypersaw_clap.cpp` NOTE_ON path: "No MPE bend re-apply here — SpectraCore
  has no noteTune"). The fix is the exact ADR-036 pattern: a per-swarm factor,
  multiplicatively inert at 1.0, so parity holds by construction. **This is the
  prerequisite for the bend-inertia fold (A1) reaching SPECTRA's per-note lane.**

**Feasible, medium (queue behind a ruling):**
- **Mono / legato / glide + poly glide.** The mono held-stack and `retargetNote` live in
  the shell + SAW core only (`monoSlot` → `core.retargetNote`, no spectra branch).
  SPECTRA needs a `retargetNote` + an `f0cur` glide slew — straightforward, inert at
  glide 0, but it touches note lifecycle, so it ships with its own notefuzz coverage.

**Different concept, needs design rather than porting (do NOT copy blindly):**
- **Per-voice envelopes + scatter (ADR-077/078).** SAW's "voice" is a swarm member;
  SPECTRA's nearest analog is per-PARTIAL (or per-cloud-voice) envelopes. But SPECTRA
  already owns a better-fitting version of onset scatter: **cascade IS per-partial onset
  staggering**, produced by the physics instead of drawn from a jitter distribution. The
  liveliness increment (lock wave / partial drift / R→tone, PR #187) is the
  SPECTRA-native answer to what ADR-077/078 did for SAW. Recommendation: audition those
  first; only design per-partial ADSR scatter if the ear still wants it after.

## SPECTRA lab — BUILT, and the brief's premise was only half right (2026-08-04)

`docs/design/spectra-lab.html`. Campaign 3 item 1 asked to *"make the engine worthwhile —
find the features that give SPECTRA its own identity rather than 'the other engine'."*
Measured the shipped core first.

**SPECTRA does not sound like SAW. It sounds DARKER than SAW.** Spectral centroid
**562 Hz vs 2449 Hz** at matched pitch (A2), because 12 partials at 110 Hz stop at
1.3 kHz. It is not competing with SAW and losing — it is playing a quieter game. That
reframes the brief: the question is not "how do we differentiate it" but **"is dark-and-
evolving the identity, or should it reach for brightness?"** Partial count is the lever
and it costs CPU linearly.

**K spends 85 % of its travel doing nothing.** Measured on the core at 0.05 steps: R sits
at the free-run floor (~0.28) from K 0 to 0.45 and drifts slightly DOWN (0.282 → 0.251),
then the entire lock happens between **0.65 and 0.85**, then saturates. In the lab's own
port the usable band is 0.50–0.65 = **15 % of travel**; a piecewise taper that sprints
through the dead zone and crawls through the band takes it to **40 %** — a 2.7× gain.
**Honest limit:** Kuramoto lock is a genuine phase transition, so the knee is physics, not
a taper bug. A taper can put the knee mid-knob; it cannot make the transition gentle.
*Fourth taper failure in this project* (ADR-059 inertia, filter-lab K, bend-lab, this).

**`seed` cannot affect the spectrum, by construction.** `rebuild()` builds cloud offsets
as `x[m] = 2m/(M−1) − 1` — a perfectly even ramp — so every partial's cloud is identically
regular and seed only touches phase (measured: **0.00 dB** spectral distance across seeds).
SAW draws its swarm from seeded gaussian/cauchy distributions and gets much of its life
from that irregularity. The lab offers even / gaussian / cauchy spacing as a candidate
identity feature.

**cascade and dissolve are healthy, and they are the actual identity.** Cascade staggers
*which partial locks when* (measured 5.7–11.2 dB of sustained spectral change, R climbing
0.32 → 0.52 over seconds); dissolve sets how long a coupling burst survives (0.05 s → gone
immediately; 8 s → R still 0.974 after 4.5 s, smooth throughout). **Nothing in a detuned
saw bank can do either.** They are currently buried at the bottom of the panel.

**Method note worth keeping.** Three instruments were wrong before the right one: a
steady-state FFT audit (blind to timing knobs), a zero-crossing proxy (blind to spectral
ones — dominated by the fundamental), and a time-resolved FFT (blind to phase lock, which
magnitude spectra average away). The correct instrument was the **order parameter R**,
which the engine already computes. For a coupling engine, measure coupling. *L0017 for the
fourth and fifth time.*

### Liveliness increment + the CPU blocker (human, 2026-08-04)

Human: *"What can we do to make spectra more lively and interesting? Also any polyphony
with high partials is overloading the lab."*

**The overload was mine, and it was 234 % of real time.** The render loop made THREE trig
calls per oscillator per sample — and two of them computed a CONSTANT (pan depends only on
`x[m]`, partial parity and width, none of which change per sample). Measured on the inner
loop at 2016 oscillators: **2336 ms to render 1 s of audio = 234 % real-time**, i.e. it
could not keep up at all. Precomputing pan and replacing `Math.sin` with a 4096-point
interpolated table (max error 3e-7): **435 ms = 43 %, a 5.4× speedup.**

**Then an oscillator budget that thins the CLOUD on the highest partials first**, rather
than dropping partials — partial count is the brightness lever we want free to raise, and a
top partial with fewer beating voices is far less audible than a missing one. Worst case
(6 voices × 48 partials × 7 cloud) goes 2016 → 636 oscillators, **76 % → 36 %** measured in
the real engine. Cost is displayed, not hidden: the panel shows the live oscillator count
and flags when the budget is biting.

**Why the engine feels static, in one measurement.** With sustained K, mean R climbs to
0.96 in the first second and then **sits at 0.96 forever**. Nothing changes after the
attack. That is the whole complaint, and it points at the fix: the engine's distinctive
mechanism (cascade) already reorders the spectrum, but it fires once and is over.

**Three candidates added, all default off, all measured:**
- **Lock wave** — cascade made CYCLIC. A travelling band of coupling ping-pongs along the
  partial series, so WHICH partials are locked keeps changing. Measured: R-spread across
  partials oscillates 0.57 → 0.94 → 0.83 continuously, against the static case's flat 0.96.
  **This is the headline candidate** — it is the one mechanism a detuned saw bank
  structurally cannot imitate, turned from a one-shot into an animator.
- **Per-partial drift** — SPECTRA has none at all; SAW's driftDepth/driftRate is most of
  why SAW feels alive. Each partial gets its own slow rate. Measured −8.4 ¢ of movement.
- **R → tone** — coupling made audible in TIMBRE rather than only in beating; a partial
  lifts or ducks as its cloud locks. Measured peak 0.496 → 0.737 (lift) / 0.366 (duck).

**NaN at cloud 7 — root-caused and fixed (human report, 2026-08-05).** Not cloud-7-specific:
the liveliness rewrite of `rebuild()` dropped its tail responsibility — the loop resizing
LIVE notes' arrays — so growing partials or cloud mid-note indexed past the old `vf`/`phase`
arrays; typed-array OOB reads return `undefined`, and `undefined/sr` is NaN from there on.
Reproduced by simulated slider abuse (clean in 9 static configs, NaN at block 52 under
live dragging), fixed by restoring the resize (preserving surviving phases so a drag does
not restrike the note), and verified clean over 3000 blocks of the same abuse with an
ADR-032-style watchdog now in place that never fired. *Same lesson as L0023: a rewrite
must diff the responsibilities the old code carried, not just the ones being changed.*

**Open (human):** brightness direction (raise partial count / re-tilt, vs commit to dark);
whether cloud spacing becomes a real parameter; whether cascade/dissolve get promoted in
the GUI; whether the K taper folds.

## Swarm-filters lab — BUILT, and the "not quite there yet" verdict is now three numbers (2026-08-04)

`docs/design/filter-lab.html`. The human's verdict on the E1 cores was *"not quite
there yet"*, so the bench began by **measuring the shipped core** (`filter_core.h`,
`processExternal`, swept steady-state at 48 kHz) rather than guessing at a fix.

**Three defects, measured:**

1. **The resonance knob is a backwards volume knob.** Peak output falls
   **+0.98 → −3.21 → −9.32 dB** as `qbase` goes 0.1 → 0.5 → 0.9. Cause is structural,
   not a bug: N summed *unity-gain* bandpasses capture less total power as they narrow.
   Turn up resonance, get quieter and thinner — almost certainly the feel behind the
   verdict.
2. **No low end, and it worsens with Q.** 40 Hz sits **24.2 dB** below peak at default,
   **28.4 dB** at high Q. The bank has no DC path at all, so anything it processes loses
   its body.
3. **It is a band-pass hump, not a filter.** Every configuration rolls off on BOTH
   sides; there is no LP/HP/notch topology and no cutoff-with-slope. Between bands the
   response nulls hard — **27.1 dB** deep at the default 16-band spread, worse with
   fewer bands, where it is frankly a comb.

**Plus a gap rather than a defect: no key tracking on the effect path.** `setNoteFreq`
moves only the gravity centre, and only when placement is harmonic — so in the rack the
filter does not follow the note at all. The lab adds a `track` control (0 = shipped) to
audition what it should be.

**Two candidate fixes, both auditionable and both measured:**
- **Q compensation** (normalise by √Q, since summed power ∝ 1/Q): level swing across the
  whole Q range **9.0 → 1.0 dB**. Resonance becomes a character control.
- **LF preserve** (one-pole at the lowest band, added back): LF deficit **22.6 → 4.7 dB**.
- Combined, plus a conventional multimode alongside and a bank→conventional series
  option, for the brief's "how would this sit next to a conventional filter" question.

**Fidelity, stated honestly.** The lab's band POSITIONS come from its own seeded draw,
not `forcecore::buildOffsets`, so its absolute curve is not the core's curve
sample-for-sample. What was cross-checked is what the bench is for — the structural
diagnostics: LF deficit **24.2 dB in C++ vs 22.6 in the lab**, Q swing **10.3 vs 9.0**.
Both defects follow from summing unity-gain bandpasses and survive any particular draw.

**Three lab bugs found by the human on first play, all mine, all fixed 2026-08-04:**
- **Sound skipping.** `redraw()` blocked the main thread for **299 ms**, and
  ScriptProcessorNode runs its audio callback on that same thread — an audio block is
  23.2 ms, so every redraw starved ~13 consecutive blocks, and every knob move triggered
  one. Fixed by replacing the simulated sweep with the **analytic** transfer function
  (the TPT SVF is a bilinear-transformed analog prototype, so `s = j·tan(πf/fs)/g` gives
  it in closed form; bands summed COMPLEX because the phase between them is what carves
  the inter-band nulls). **299 ms → 0.9 ms.**
- **New notes killing old ones.** The source did `src.notes = [one note]` — monophonic by
  construction. Replaced with a held stack and per-key release. *This is the same defect
  fixed in bend-lab.html hours earlier and then written fresh here.*
- **K audible but invisible.** The old `measure()` built a **fresh** bank per call, and a
  fresh bank has never run `controlTick` — so coupling could not appear in the measurement
  at all, by construction. The curve now reads the live bank and animates while the swarm
  is in motion. Verified: at K=1 the band spread collapses 5.396 → 0 octaves and the comb
  becomes a single +15.5 dB peak.

**K was unusable outside ±0.1 — and the taper was the smaller half of why (2026-08-04).**
Human: *"K is only usable about 0.1 on either side of 0, and really only as a kind of YOY
filter."* Two causes, and the second was the real one:
- **Taper.** The lab's law was a raw per-tick gain (`K*0.05`), which at tick rate spends
  the whole knob below |K| ≈ 0.1. Re-expressed as a collapse TIME CONSTANT in seconds
  (ADR-009), log-spaced: |K| = 0.1 → 2.35 s, 0.5 → 0.28 s, 1.0 → 0.02 s. *This is
  ADR-059's taper lesson recurring for the third time in this project.*
- **No restoring force — the actual reason it read as "only a YOY filter".** The bench's
  coupling was a pure attractor with nothing to pull against, so ANY non-zero K collapsed
  the bank to a single frequency and K only set how *fast*. What the human was hearing was
  the transient; the steady state was identical everywhere. The real core has this term
  (`pop.tHome` + the force system) and the bench had dropped it. Restored, K now settles at
  an **equilibrium** between coupling and home, so it controls depth: measured equilibrium
  spread **5.40 (K=0) → 4.70 → 4.31 → 3.78 → 3.12 → 2.41 → 1.18 → 0.29 octaves** at
  K = 0 / 0.1 / 0.2 / 0.3 / 0.4 / 0.5 / 0.7 / 1.0. Smooth and monotone across the whole
  knob. **Honest limit:** the splay side saturates around K = −0.6 (7.61 → 8.10 oct), where
  the bands hit the 40 Hz / 11 kHz clamp — less usable travel than the lock side.

**Animation chop fixed by splitting cheap from expensive.** Each frame was also rebuilding
three throwaway Banks for the Q-swing probe — ~4× a frame's work plus allocation churn.
The curve and band map now animate alone (**0.06–0.14 ms/frame**, ~119× headroom at 60 fps)
and the diagnostics run self-throttled at ~3 Hz. No accuracy was traded for the speed: the
analytic response is exact, so the "less accurate but faster" fallback the human offered
was not needed.

**The analytic path is verified against the simulation**, which stays as the oracle:
worst |analytic − simulated| = **0.01–0.02 dB** across all six topologies. Getting there
exposed a fourth issue worth recording — the first comparison showed an 87 dB disagreement
in the deep stopband and **the simulation was the wrong one**: 8 cycles of warm-up left
transient energy that set a ~−65 dB floor, and in a stopband that floor *is* the reading.
With warm-up scaled to the actual ring time, the deep stopband agrees exactly (−151.9 vs
−151.9 dB at 16 kHz). An oracle can be less accurate than the thing it checks.

**Not yet decided (human):** whether the bank becomes a proper rack filter (fixes 1+2, or
1+2 in series with a conventional multimode), whether key tracking is added and at what
default, or whether the bank stays a *resonator/formant* effect and a conventional filter
is built beside it. The measurements argue it is currently neither one thing nor the
other, which is a plausible reading of "not quite there yet".

## STANDING CONVENTION — lab visuals ship with the feature (human, 2026-08-03)

Human: *"I want to set a precedent that the best visual elements from each lab are
included in the synth itself, though many will probably want to just be on their own
tabs instead of on the global visualizer. The bend lab visuals will be helpful in
demonstrating to users what these unusual controls actually do."*

**The rule.** A lab is not just a design bench — it is where the *explanation* of a
control gets built. When a lab feature folds into the plugin, its best visual folds
with it, and "no visual" is a decision that must be argued, not a default from
forgetting. Placement is per-feature: the **global visualizer** stays reserved for
things true of the whole instrument (phase circle, scope, voice map); feature-specific
displays live on **their own tab beside the controls they explain**.

**Why it matters more here than in a normal synth.** HYPERSAW's controls are unusual
enough that a user cannot infer them from the name — `dist→overshoot`, `onset α`,
`super-width mode`, `glide model`. A knob whose meaning is only discoverable by
careful listening is, in practice, a knob most users will leave alone. The step-response
plot answers "what does this do" in one glance, and it already exists.

**Backlog of visuals worth folding** (each with its lab source):
- **bend lab** — step-response plot (target vs actual) + the vibrato-cost readout.
  Highest value: it makes the glide models legible at a glance. *Ships with the bend
  fold, whenever that lands.*
- **width lab** — the L/R scope and the cliff counter (side/mid, correlation).
- **reverb lab** — the ER/tail envelope display.
- **ensemble lab** — the onset-scatter raster (shows corrected vs i.i.d. timing).
- **detune / shape / mod labs** — pending their own folds.

Not a queue item to do now; a **rule applied at each fold**, recorded so it is not
re-litigated per feature.

## FX fold status — what IS and IS NOT in the plugin (recorded 2026-08-03, human asked)

Human asked for the comb's fold status to be recorded clearly, believing it was
"only in a lab". **Checked rather than recalled, and the truth was a third thing —
plus a live bug.**

- **Karplus-Strong comb: SHIPPED** in the internal FX rack as slot type 5
  (ADR-071), ids 57/59/61/63, `src/fx_rack.h`. Not a lab-only feature.
- **…but it was UNREACHABLE from the plugin's own panel.** The rack params were
  widened 0..3 → 0..5 when ADR-071 landed; the four GUI dropdowns in
  `src/gui/gui.html` were never widened with them, so **Comp (4) and Comb (5)
  were shipped, automatable from the host, and invisible in the interface**.
  Fixed 2026-08-03. Neither of us would have found this by memory — the human's
  wrong recollection was pointing at a real defect from the wrong direction.
- **Divergence already on record:** the rack's comb is BUS-side (8 tuned lines
  fed the whole mix, sympathetic-string posture), not the lab's per-swarm comb.
  ADR-071 records this honestly; if an A/B says the per-voice isolation matters,
  the comb moves core-side as its own decision. **Still unaudited by ear** —
  because until now it could not be selected in the GUI.
- **NOT folded: the Track E1 swarm filter + notch/phaser.** `filter_core.h` /
  `notch_core.h` are parity-proven but live in the **separate SWARM-FX plugin**
  (`src/swarmfx_clap.cpp`), not in HYPERSAW's rack. That is the "whole lab not
  folded in yet" — `swarmfilter.html` / `swarmphaser.html`.

**Lesson worth the ink:** a param range widened without its GUI control is a
feature that ships invisible. Every future rack type must widen both, in the same
change. (Candidate LIBRARY entry.)

## Pitch-bend inertia — EXPERIMENT, awaiting audition (human direction, 2026-08-03)

Human: *"I want to try adding an inertia option to pitch bend (with various settings
to key it in)."* Bench built first (`docs/design/bend-lab.html`); **no core change** —
the fold decision is open and belongs to the ear, not the meters.

**Why the bench offers four models.** "Inertia" is three different physical claims,
and they do not sound alike; choosing one silently would have decided the feature by
accident. A **lag** (one-pole) is proportional — every move takes the same time no
matter its size. A **rate limit** is constant-velocity — a −12 st dive takes twelve
times as long as a 1 st nudge, which is what a physical mechanism actually does. Only
the **mass-spring** is inertia literally: it overshoots and rings, because a mass in
motion does not stop when the force does. A fourth (lag → rate limit in series) is the
practical combination. Plus a `return ×` asymmetry — a real spring snaps home faster
than you can push it — applied to the bend lane only, since a note has no home pitch.

**The bench runs the filter at tick rate**, not sample rate, because that is where a
fold would put it (ADR-027's live-tune factor is read once per tick at law evaluation).
Measuring a filter the plugin would never have would measure the wrong thing.

**The finding that matters before any fold.** Every model is a low-pass on the
player's hand, so inertia *costs wheel vibrato*: a 60 ms lag already keeps only 47 % of
a 5 Hz wobble, arriving 34 ms late. If both expressive gestures are wanted, flat
inertia cannot give them — that is an argument for keying the amount to bend
*distance* (slow travel on a big sweep, near-instant on a small one). Untested and
deliberately unbuilt; it is a design decision, not an implementation detail.

**Also live in the bench, worth an opinion:** `applies to → note pitch` routes the
same filter through note-to-note pitch, where the mass-spring puts a **pitch blip on
every note onset** — what a struck resonator does. That is a different feature from
bend inertia wearing the same math, and it may be the more interesting one.

**Open questions for the human.** (1) Which model, and does the ringing spring earn its
place or is it a novelty? (2) Flat amount, or keyed to bend distance? (3) Bend lane
only, or note pitch too? (4) Does this belong on the wheel *and* on MPE per-note bend
(ADR-036/038), which is a much more expressive surface and would need per-note state?
No fold, no ADR, and no param ids until these are answered.

### Increment 2 — human direction, 2026-08-03 (bench updated, still no fold)

Human's ideal set: *"constant-time glide, constant cents glide, lag, spring (with all
the current spring settings and possibly an extra slider for the extent to which glide
distance influences overshoot)"*, plus *"note pitch was actually the whole crux of my
initial idea"* and *"apply it to MPE as well"*. All now in the bench:

- **Constant time added as its own model** — it was genuinely missing. The old "lag"
  is a one-pole, which is asymptotic and *never arrives*; constant-time portamento
  latches a velocity from the move distance and arrives exactly on schedule. Verified
  by the property that defines it: a 2 st and a 12 st move both reach 50 % at T/2
  (99.77 ms measured for T=200).
- **dist→overshoot slider.** Answering the human's "if that isn't already how it
  works": **partly, yes.** A linear spring overshoots by a fixed *percentage*, so
  absolute overshoot in cents is *already* proportional to distance — that is k=1, and
  the knob generalises it to overshoot ¢ ∝ distance^k. Implemented by solving the
  closed-form ζ↔overshoot relation for the damping that *produces* the wanted
  overshoot, not by scaling the output. Measured ratios over a 6× distance change:
  k=1 → 6.00, k=0 → 0.97 (constant absolute), k=2 → 9.07 vs 3²=9.
- **Note-pitch lane promoted to the default** (`applies to` now defaults to note
  pitch), since it is the crux of the idea.
- **MPE lane is real, not a note.** Each note carries its own bend inertia *and its own
  latched target*; the wheel drives the newest note while the others hold. Verified:
  bend note A to +2, play note B, move the wheel to −1 → A stays at +2, B follows.
  **It maps naturally at fold time** — `setNoteExpr` already writes per-voice
  `noteTune` (ADR-036/038), so per-note bend inertia is one filter instance per voice
  with no new plumbing.

**Bug fixed in the bench:** releasing any key gated every sounding note off, which made
glide unauditionable (glide is inseparable from what "still held" means). Replaced with
a real held stack: per-key release in poly, last-note-priority with fallback-to-still-held
in mono. Verified for both.

**Mechanism bug the calibration caught:** the move-distance tracker rearmed the moment
the error crossed zero — but a spring crosses its target at full speed *on the way to
overshooting*, so it was re-deriving its own damping mid-overshoot. dist→overshoot read
distance^2.5 instead of distance^2. Rearm now requires arrived AND stopped.

## Mono note-hang FIXED (2026-07-29, tasks #24 + #28 closed)

The human's 2026-07-26 report — "notes get stuck for longer than they ought to when I
play quickly ... hasn't happened with preprogrammed MIDI in the piano roll" — was **two
real bugs in the mono held-stack**, both reproduced and both fixed. `./verify full` green
(all nine oracle chains).

**Why the piano roll never triggered it.** A piano roll emits a clean NOTE_OFF before the
next NOTE_ON for a key, and stamps events sample-accurately. A computer keyboard played
fast does neither. `notefuzz_check` modelled only the piano-roll shape: it explicitly
`continue`d past a duplicate key and gave every event a distinct sorted time. Those two
skips **were** the blind spot — the oracle could not have caught this.

**Bug 1 — phantom held key.** The mono note-on pushed to `heldStack` with no duplicate
check, and note-off removed only the *first* match (it `break`s). So `on(C) on(C) off(C)`
left a phantom C on the stack; the off path saw `heldCount > 0` and **retargeted the voice
to the phantom** instead of releasing it. Fixed: a re-press moves the key to the top
(last-note priority), and note-off removes every entry for the key as an invariant restore.

**Bug 2 — orphaned gated voice (introduced by the fix for bug 1, then caught).** Evaluating
`anotherHeld` *after* the duplicate removal sent a re-press down the fresh-strike path while
the previous mono voice was still gated, orphaning it — and every release path keys off
`monoSlot`'s current midi, so nothing could ever release it. Fixed by enforcing the actual
mono invariant: **at most one gated voice**, force-releasing a still-gated `monoSlot` before
a fresh strike. Only a *gated* voice is touched, so the intended release-tail overlap
(gate == 0) is unaffected.

**Behaviour change worth a human eye:** re-pressing an already-held key in mono now
re-articulates the note (fresh strike) rather than doing nothing. That is the standard mono
reading of a re-press and it is what removes the hang, but it is a decision, not a
mechanical fix — flag if you want it silent instead.

**Oracle strengthened, never weakened.** The five original modes are unchanged and still
run; six new modes cover `restrike` (duplicate note-on) and `live` (every event stamped at
frame 0) across poly/mono/legato. A second metric was added — the tail length after the last
note-off, because the original gate only caught *permanent* hangs and the reported symptom
was a *finite* over-hold (41 ms normal vs 1498 ms hung). Result: 75 hangs before, **0 after**
at 12 modes x 40 seeds.

**`--minimal` added and kept.** A delta-debugging search over all balanced event sequences
up to length 6 on two keys, which turned "a hang exists somewhere in 400 random blocks" into
the exact five-event repro `on(61) on(61) on(60) off(61) off(60)`. Random fuzzing found
*that* a hang existed; this found *what it was*. Retained in the oracle for the next
note-handling regression.

## Task #18 batched CLAP param pass — BUILT 2026-07-29 (ADR-072; proposal below ratified)

Human rulings: remap delegated (resolved: state stores RAW values by key, so sessions are
immune by construction; VST3 automation-lane rescale on law/dist accepted + recorded), full
roster minus `lpOut`, `toneTilt` approved. Ids landed at **71..86** — NOT 70: id 70 is a
ghost (ADR-059 dev taper hook intercepts it by number with no table row; found because
toneTilt's writes were silently swallowed there — the new paramfunc_smoke caught it).
All 16 params ACT at extremes (smoke), all 16 inert on SPECTRA (leak probe, both
controls firing), `./verify full` green. Original proposal kept below for provenance.

### (ratified proposal, 2026-07-29)

Survey done 2026-07-29. Param ids are append-only, so this is one-way: **69 params exist
today (ids 1..69); next free id is 70.** Everything below is proposed, nothing built.

### Verified gap — 16 core keys with no CLAP param

Established by comparing SwarmCore's `setParam` key table against the shell's param table
and then checking each candidate individually, because name-substring greps produced false
results in BOTH directions three separate times during this survey:

`anchor · driftMode · freqGlide · harmReach · hiTame · keepPhase · lpOut · motionCenter ·
panCurve · panInvert · panLayout · panMode · panMotion · pivotMode · spread · stretchB`

Two candidates that LOOK like gaps and are not: `tune` is derived in the shell from
octave/semi/fineCents, and `bpm` is written directly from the host transport
(`core.p.bpm = tr->tempo`) rather than through `setParam` — I nearly reported the
tempo-grid law as broken on that basis and it is fine.

### Enum widenings (the compatibility-sensitive part)

- `law` (id 5) 0..3 → **0..5**, adding *harmonic* (law 4, ADR-065) and *stretch* (law 5,
  ADR-066) to `kLawLabels`.
- `dist` (id 2) 0..3 → **0..4**, adding *golden* (ADR-067) to `kDistLabels`. Note golden is
  implemented as the trailing `else`, not a `dist == 4` branch — grepping for the latter
  finds nothing and looks alarming.

Widening a stepped param's max is the one genuinely risky edit here: a host that stored a
normalised value re-reads it against the NEW range, so existing sessions can shift law/dist
under the user. **Needs a ruling** — accept the shift, or add the new values as a separate
param, or version the state chunk and remap on load. My recommendation is remap-on-load:
the state chunk is already versioned, and it is the only option that is silent for the user.

### A LATENT TRAP the survey found — the `tilt` key collides across cores

`applyParam` mirrors most ids into BOTH cores by key name ("unknown keys no-op"), and
ADR-060 added a `tilt` key to SwarmCore while id 45 "Amp Tilt" already used `tilt` for
SPECTRA. That is safe **today only because of a positional guard** —
`if ((id >= 44 && id <= 55) || (id >= 65 && id <= 68)) { spectra…; return; }` — so id 45
never reaches the SAW core. Safety is by ID RANGE, not by name.

PROVEN, not assumed: a black-box probe (`tools/paramleak_probe.cpp`, diagnostic only, NOT
wired into `./verify`) drives each SPECTRA-only id at non-default extremes and measures the
SAW engine's rms. With a positive control confirming the probe is sensitive at all
(detune 0.28 → 0.6 moves rms 0.086957 → 0.090925), id 45 at both 0.5 and 2.0 leaves the SAW
output bit-identical. A direct-core parity oracle could not have shown this either way —
L0011 is exactly that lesson.

CONSEQUENCE for this pass: SwarmCore's tone tilt **cannot** be exposed under the key
`tilt`. Any new id ≥ 70 falls through to the mirror path, so a `tilt` param would write
SPECTRA's amp tilt as well. Expose it as a distinct key (`toneTilt`) and add that alias to
SwarmCore's `setParam`. Retiring the collision beats perpetuating it behind a guard whose
correctness depends on nobody renumbering.

### Open questions for the human

1. **State compatibility** on the law/dist widening — remap on load (recommended), accept
   the shift, or separate params?
2. **Scope**: all 16 at once, or only the ones the fold ADRs need reachable
   (harmReach, stretchB, spread, anchor, pivotMode, panLayout) and defer the rest?
3. `lpOut`, `panCurve`, `panInvert`, `panMode`, `motionCenter`, `keepPhase` — expose as
   user params, or leave core-only as implementation detail?
4. Confirm the `toneTilt` rename approach for the colliding key.

## Richness round 5 (2026-08-02): clean-mode ear test WEAKENS droop-as-whole-story

Human A/B'd digital 0 by ear: "helps the frequency curve a little", does NOT restore
the richness, adds messy LF (consistent with the measured −44..−79 dB dense aliasing
— folded products land low), and STILL no corner wiggles. Two consequences:

1. **Wiggles are a spectral-shape signature, not a droop signature.** Gibbs corner
   ripple requires FLAT-TO-NYQUIST harmonics with a BRICK-WALL cutoff (wavetable/
   additive-class saws). Both our modes roll off GRADUALLY (droopy BLEP or flat-ish
   clean) — gradual rolloff cannot ring. If the wiggle look (and its HF sheen) is
   the target, the fix is a band-limited-additive/wavetable saw path, not just
   flattening the rolloff. Moves that option UP the fix menu, ahead of plain
   oversampled BLEP.
2. **Droop may not be the whole fullness story.** Clean recovers most HF (−1.35 vs
   −7.56 @ 20 kHz) yet the richness gap persists by ear — so either the richness
   lives in Serum's spectrum EXCEEDING the 1/k law (brightened wavetable), or in
   animation (drift/phase motion), not in static response. NEXT MEASUREMENT: the
   human renders a Serum single-voice saw locally (unison off, FX off, one note,
   few seconds) — we analyze it against the 1/k law LOCALLY (per the competitor
   convention: never committed) and answer "is Serum's saw brighter than ideal?"
   directly. Also still owed: BLEP aliasing at incommensurate f0.
## README screenshot + visual-breakdown docs (human direction, 2026-08-02)

1. **README carries a GUI screenshot** (`docs/img/gui-overview.png`, embedded above
   "What this repo is") — **refresh discipline is part of the PR protocol**: any PR
   that visibly changes the GUI updates the screenshot in the same change, exactly
   like TESTING.md items. The L0020 stamp closes the loop: the build hash visible in
   the screenshot's corner states which code drew it, so a stale image is
   self-incriminating.
2. **Roadmapped: feature-by-feature visual breakdown** — once more labs are folded
   and the multi-page IA lands, a documented tour (annotated per-cluster captures:
   what each section is, what it sounds like, which ADR built it). Natural home:
   `docs/` + README link; builds on the layout-lab's cluster map. Deferred until
   the interface complexity warrants it (human: "once we've integrated more of the
   labs").

## ADR-078 SHIPPED (2026-08-03): per-voice envelopes (increment 2 done)

voiceEnv id 94 + relScatter id 95. Shared env demoted to BOOKKEEPING (= max
per-voice env) so liveness/steal/NOTE_END machinery is untouched — deliberately
avoiding a rewrite of the note-lifecycle code we spent three rounds stabilising.
Measured: uniform when unscattered (spread 0.0e+00), genuinely spread at 0.8
(0.237 at 150 ms), always reaches silence. Per-voice envelope level is now
available as a future mod SOURCE — the human's stated motivation.

## ADR-077 SHIPPED (2026-08-03): ensemble onset timing, increment 1

The L0019 research reaches the instrument. Vorberg/Wing correction folded into the
core (ids 91/92/93), inert at onsetScatter 0. Reproduces the regimes: lag-1 +0.985
(alpha 0, random walk) -> +0.679 (0.25, structured) -> -0.072 (1.0, i.i.d. jitter).
Oracle gates the STRUCTURE, not the variance. Increment 2 (per-voice ADSR) deferred
— needs the per-swarm -> per-voice envelope rework.

## STICKY NOTES: measured — the release knob is a TIME CONSTANT, not a time-to-silence

Human, 2026-08-03, still: "sticky notes (take a little too long to end after I stop
pressing)". MEASURED (per-block envelope, after a first attempt that thresholded raw
samples and reported nonsense — a waveform crosses zero every cycle):

| release setting | −40 dB at | −60 dB at | ratio |
|---|---|---|---|
| 0.005 s | 35 ms | 46 ms | 9.3× |
| 0.050 s | 209 ms | 337 ms | 6.7× |
| **0.160 s (default)** | **801 ms** | **1057 ms** | **6.6×** |
| 0.500 s | 2345 ms | 3390 ms | 6.8× |

The envelope is a one-pole (`env += (0−env)·rel`, rel = 1−exp(−1/(release·sr))), so
the knob is a TIME CONSTANT and silence takes ln(1000) ≈ 6.9 of them. At the default
a note is still audible ~1 second after key-up. That is very likely the whole
remaining "sticky" complaint — nothing to do with note-offs.

DISCRIMINATING TEST (the note monitor exists for exactly this): after key-up, is the
cell **FILLED** (gate stuck — a real bug, our side) or **HOLLOW AND SLOWLY DIMMING**
(envelope tail — this finding)? One glance settles which.

FIX OPTIONS (human's call — this is taste + compatibility, not correctness):
1. **Shell-side knob taper** (ADR-024 inertia precedent): applyParam divides the knob
   by ~6.9 so "release 0.16 s" means audible-silence in 0.16 s. Parity-safe — the
   core keeps its semantics, only the mapping changes — but existing sessions get
   ~7× shorter releases, which is a big audible change to saved work.
2. **Leave the law, fix the LABEL** in the units pass: display the knob as its
   time-to-silence (0.16 s → "1.1 s"), so the number stops lying.
3. Do nothing; document in TESTING.
Recommendation: (2) now — it is honest, breaks nothing, and folds into the
already-planned units pass — with (1) offered as an opt-in curve later if the human
wants Serum-like snap.

## Coherence gain compensation — PROPOSED (human, 2026-08-03: "tame the big additive saw without changing its shape")

At high K the voices phase-align, so the sum's peak rises with COHERENCE, not with
voice count — and `normExp` (density comp) only compensates the latter. The tanh
then bends the peaks, which is the shape change the human wants to avoid.

The elegant fix is already sitting in the engine: **scale output gain by the order
parameter R**, which the core computes every control tick. Coherent (R→1) = quieter
by design, splayed (R→0) = unchanged, so a K sweep holds level without touching the
waveform. Sketch: `gain *= 1 / (1 + cohAmt·R·(n^a − 1)/…)` — the exact law needs
auditioning (an R-follower with a time constant in SECONDS per ADR-009; instantaneous
R would pump).
DESIGN NOTES: (a) must be opt-in/default-off — it changes level under K, which is
audible and golden-visible; (b) it is arguably the most on-brand feature yet — the
instrument compensating itself using its own physics observable; (c) alternative
framing is a proper limiter in the FX rack (already ruled a rack slot, ADR-016), but
that CHANGES SHAPE by construction, which is exactly what the human asked to avoid;
(d) pairs naturally with the mod matrix, where R becomes a first-class source.

## ADR-075 SHIPPED (2026-08-03): opt-in 2x oversampling

Built with its oracle. Droop 15 k −4.50 → −2.13 dB, 10 k −2.17 → −1.23; CPU 2.5% →
6.3% of one core (E-6 budget 50%); parity 147/147 untouched; gates: OS-off
determinism + 15 kHz recovery ≥ 1.5 dB (got +2.37). CLAIM IS BOUNDED: "roughly
halves the droop through 15 kHz", not flat — the spike predicted −0.83 at 15 k and
the core reaches −2.13, with the 1x R→tone output pole the prime unverified
suspect for the gap (OPEN, see the trace). Spike record kept below.

## 2x-oversampling SPIKE measured (2026-08-03) — build it, with a bounded claim

Prototype (headless, polyBLEP saw + windowed-sinc halfband decimation) vs the ideal
1/k law, droop in dB at 5/10/15/20 kHz:

| path | 5 k | 10 k | 15 k | 20 k |
|---|---|---|---|---|
| 1× polyBLEP (shipping) | −0.36 | −1.51 | −3.44 | −6.30 |
| 2× OS + 31-tap halfband | −0.09 | −0.37 | −0.95 | −5.78 |
| 2× OS + 63-tap | −0.09 | −0.37 | **−0.83** | −4.36 |
| 2× OS + 127-tap | −0.09 | −0.37 | −0.83 | −2.55 |

READING: 2× OS recovers essentially ALL the droop through 15 kHz (−3.44 → −0.83,
and 10 kHz becomes −0.37) with a modest 63-tap filter. **20 kHz is intrinsically
hard** and NOT an oversampling failure: it sits at 0.91× Nyquist, inside any
decimator's transition band, so it costs filter length (127 taps only reaches
−2.55) for a band at/above most listeners' limit. 4× OS would move the transition
band clear of 20 kHz at ~2× the CPU of 2×.

DECISION RECORDED: build **2× OS + ~63-tap halfband**, claim "flat to 15 kHz",
explicitly DO NOT claim flat-to-Nyquist. Design constraints for the fold: opt-in
param (default off = bit-exact, the ADR-063 precedent) so all 147 goldens stay
green; C++-only superset → per L0021 it ships with its own droop gate in
waveshape_check (assert ≤1 dB at 15 kHz when on, and assert OFF is bit-identical);
CPU measured against the E-6 envelope before ratification — the voice loop doubles,
the decimation FIR is negligible (2 ch × 63 taps ≈ 5.6 M MAC/s).

## Open questions answered / opened (2026-08-03)

**1. ITD max default — measured, and the measurement says LOWER it.** Natural max
interaural delay is 0.51 ms (head width / c). Our law `0.6 · (w−1)·2 · |pan|` gives
0.60 ms at width 1.5 and 1.20 ms at width 2 — past natural ITD into Haas territory.
Cost curve at width 1.5: side/mid and mono-fold both SATURATE at ITD ≥ 0.15 ms
(−0.4 dB / −2.8 dB, identical from 0.15 through 1.2 ms). So everything above ~0.15 ms
buys ZERO measured width while still paying delay costs: mono-sum comb nulls move
DOWN with delay (first null ≈ sr/2N: 3.3 kHz at 0.15 ms, 833 Hz at 0.6 ms, 417 Hz at
1.2 ms — the per-voice spread smears them, but the trend is real), plus transient
smearing. **PROPOSED: drop the coefficient 0.6 → 0.3** (width 1.5 → 0.30 ms, inside
natural ITD; width 2 → 0.60 ms). Honest limit: the metrics saturate, so this is a
"stop paying for nothing" argument, not a measured-benefit one — the EAR should
ratify, ideally A/B at 0.6 vs 0.3 in the width lab before the change lands.

**2. AP freq 700 Hz (mode D smear) — arbitrary, no measurement behind it.** Picked
by feel when the lab was built. Options: measure a coloration/motion metric across
300 / 700 / 1500 Hz, or expose it (id churn) — recommend the human A/B in the lab
(the knob is already there) and pin whatever wins.

**3. Baseline saw to Nyquist — RECOMMENDED NEXT DSP FOLD, with a caveat.** Measured
droop vs ideal 1/k: BLEP −0.60/−2.17/−4.50/−7.56 dB at 5/10/15/20 kHz; clean mode
−1.35 @ 20 kHz but with −44…−79 dB aliasing. CAVEAT FROM ROUND 5: flattening HF did
NOT restore the richness by ear (drift 30¢ did) — so this buys AIR and the
brick-wall Gibbs "wiggle" character, NOT the fullness that is already solved. Fix
menu, ordered by cost: (a) **2× oversampled BLEP** — flat AND clean, CPU cost to
measure against the E-6 envelope, the standard answer; (b) higher-order BLEP kernel
— cheaper, partial; (c) band-limited additive/wavetable saw path — the only option
that gives a true BRICK WALL (and thus the wiggle), but it is a second oscillator
architecture beside the phase-accumulator core, i.e. a big fold. Acceptance baseline
is the droop table; the aliasing midpoint protocol (at INCOMMENSURATE f0 — still
owed) is the other gate.

**4. Wordmark — asterisk removed from the GUI (human, 2026-08-03):** HYPER✱SAW read
as adjacent to NI's SUPER✱SAW styling. Now plain "HYPERSAW". NOTE for the Phase-5
naming pass: the ✱ is still the SWARM✱ house mark across docs/prototypes — decide
there whether it survives at all, and give the final name a proper clearance check
(the repo is public; the competitor-reference convention already governs prose).

## ADR-074 SHIPPED (2026-08-02): super-width 3-mode fold (F/A/D)

Ship list built same-day: superMode id 87 (wide/pulse/smear), F default and clean
(gated 0 cliffs), A/D pinned as documented character (1,867 / 14,300 cliffs at the
parity patch — the pin fires if a future change silently linearizes them). C/E
retired. Parity 147/147 untouched; verify full green; TESTING.md carries the
audition items. Details: ADR-074, traces/2026-08-02-fold-superwidth.md.

## Width characterization measured (2026-08-02) — F confirmed "best of both worlds"

Full study (6 algos × 7 widths × 10 s, parity-recipe swarm, drone D2):
`docs/reports/2026-08-02-width-characterization.html`. At width 1.5:

| algo | S/M dB | corr | mono-fold | cliffs/s |
|---|---|---|---|---|
| A M/S boost | −0.7 ±3.4 | 0.08 | −3.0 | 864 |
| B mid-duck | −1.5 | 0.16 | −2.6 | 466 |
| **C ITD** | **−0.1** | **0.01** | −3.3 | **0** |
| D allpass | −2.7 | 0.29 | −2.2 | 8,869 |
| E steep | −4.5 | 0.44 | −1.6 | 0 |
| **F ITD+steep** | **−0.1** | **0.01** | −3.3 | **0** |

READINGS: C/F are the WIDEST of all six (beating A) at zero cliffs — the human's
"C feels wider than E" and "F is probably the best of both worlds" both confirmed
numerically (E alone is the narrowest and is dominated by F, which subsumes it; B is
a worse A). The clean candidates pay ~1.3 dB more mono-fold loss than E (Haas
combing on fold) — the one tradeoff to keep an ear on for PA use. D is the
narrowest AND the cliff-heaviest — its value is purely the freq-smeared character
the human likes, not width.

RECOMMENDED SHIP LIST (matches the human's leaning): **A + D + F** as a 3-mode
super-width selector — F the default (clean + widest), A "pulse" and D "smear" as
named character modes with the polarity behavior documented. C and E retire (both
subsumed by F). Fold = ADR-025 revision: mode enum + F's ITD/steep params into core
(C++-only superset AGAIN → per L0021 the same commit must extend waveshape_check to
gate F-at-1.5 clean AND pin A/D's cliff behavior as the documented exception.)

## Width lab OPEN (2026-08-02) — ADR-025 alternatives bench, pre-calibrated

Human ruling: super-width is NEEDED (Serum-class baseline wideness) and "the pulse
effect isn't the worst sound" — so the bench keeps ADR-025 as baseline A and
auditions five alternatives. Design fact the menu encodes: ANY linear M/S boost past
unity has a negative cross-feed coefficient, so truly non-inverting width must come
from the time domain or seat redistribution. `docs/design/width-lab.html`, with the
ADR-025 cliff detector running LIVE (same physical bound for every algo).

Headless pre-calibration at width 1.5 (cliffs per ~4.6 s, drone D2):
A ADR-025 M/S 3,695 · B mid-duck 1,975 · **C per-voice ITD 0** · D allpass side
39,504 (worst — constant freq-smeared inversion) · **E seat steepening 0** ·
**F ITD+steep 0**. Width 1.0: all zero (calibration held).

The audition question for the human: do C / E / F reach algorithm A's side/mid
number at the same knob position — and which SOUNDS widest without the pulse?
(E alone caps at hard-pan width; C buys precedence-effect width beyond the gains;
F stacks both.) Detector note for the record: the first bench build multiplied algo
A's own side-boost into the legal-slope bound — the known-bad case read ZERO and
L0016's planted-bad discipline caught it headlessly before the lab shipped. A
detector whose bound depends on the suspect's gain proves nothing.

## CLIFF MYSTERY SOLVED (2026-08-02, human isolation + probe): super-width's negative cross-feed

The human isolated it — cliffs appear exactly when **width > 1** — and their standing
hypothesis ("there's one phase-inverted saw mixed in there") was LITERALLY correct.
Mechanism, src/swarm_core.h:513-525 (ADR-025 super-width): at width > 1 the M/S
side-boost `sideGain = 1 + (width-1)*2` gives per-channel algebra
`L' = L*(1+g)/2 + R*(1-g)/2` — at width 1.5, `L' = 1.5L − 0.5R`: every
opposite-side voice enters PHASE-INVERTED. An inverted saw ramps down and wraps UP =
the vertical up-cliffs, and the inverted cross-feed comb-filters against same-side
content = a chunk of the persistent "notching" at the parity patch (width 1.5).

DOSE-RESPONSE (C++ probe, parity patch minus drift): width 0.8/1.0/1.01 → **0**
cliffs; width 1.2 → 432; width 1.5 → **1,414**, worst rise 8× legal slope.

WHY EVERY EARLIER "CLEAN" VERDICT MISSED IT — the blind spot is STRUCTURAL and worth
a lesson: ADR-025 is a **C++-only superset** ("no swarmsaw.html reference — the
reference range is bit-untouched"), so (a) every JS-reference render I cliff-tested
was width-clamped by construction, and (b) the parity goldens AND waveshape_check
all run width ≤ 1 — the superset region had ZERO oracle coverage. The parity oracle
cannot see superset-only regions BY CONSTRUCTION; every superset needs its own
invariant coverage (L0011's corollary; lesson to bank).

DESIGN DECISION (human's call — ADR-025 revision):
1. Replace the M/S boost with a non-inverting widener (keep cross-feed coefficient
   ≥ 0, e.g. sideGain ≤ 1.4 cap ≈ coefficient −0.2… still negative; truly
   non-inverting needs a different mechanism: per-voice Haas micro-delays, or
   side-boost with a mid floor);
2. Cap width at 1.0 and retire super-width (the fan + pan motion may make it
   redundant);
3. Keep it, documented as a "beyond-100% = polarity play" zone.
After the ruling: extend waveshape_check to width 1.5 as a GATED regime (must be
clean under the new design), and re-audition the parity patch — the notching
verdict may change entirely at width ≤ 1 + a different widener.

## Richness BREAKTHROUGH (2026-08-02, human ear): drift ~30¢ closes the Serum gap

The human matched HYPERSAW to the basic Serum supersaw by ear — the missing
ingredient was **driftDepth ≈ 30 cents** (with their patch otherwise as posted).
This is round 6's mechanism confirmed from the listening side: per-voice random
frequency wander continuously DECORRELATES the pair phases, so the n(n-1)/2 comb
sweeps never align — no coherent hollows, stationary spectrum, "Serum richness".
Serum ships analog-style unison drift ON by default; our default driftDepth is 0.

METRIC STATUS — honest null: a 1-8 kHz BAND-energy hollow detector showed nothing
(0.9 dB dips in every config, and critically its KNOWN-BAD case — two-saw analytic
notch sweep — never fired, so per L0016 the metric is invalid for the phenomenon,
not the phenomenon absent). Comb notches cut INDIVIDUAL harmonics; a band sum over
~100 harmonics averages them away. Next-session instrument: per-harmonic amplitude
tracker (bin-exact, frame-wise), calibrated on the two-saw analytic sweep (must show
full-depth swings), then drift 0 vs 30¢ becomes a number.

**PARITY RECIPE CAPTURED (human, 2026-08-02):** `docs/presets/serum-parity-reference.json`
— audible parity with Serum at 16 voices / default detune+blend, EXCEPT the notching,
which persists and is now the single remaining delta. Levers: n=16 · drift 30.4¢ @
0.4 · detune 0.143 (cents/gaussian) · **digital 0.37** (a BLEP↔clean BLEND — partial
HF recovery with partial aliasing, an interesting middle the droop/alias tables
bracket) · retrig 0 · K=0 · freqGlide 42 ms. The persisting notches at drift 30 +
K=0 sharpen the per-harmonic tracker's job: identify WHICH pair alignments survive
that much decorrelation (gaussian commensurate cluster? the 16-voice even-spacing
statistics?) — that is the hunt's last open door.

DESIGN DECISIONS OPENED (human's call):
1. Default driftDepth — ship a Serum-class subtle drift ON by default? Changes
   default output → ADR + golden updates; the alternative is a "Classic Supersaw"
   factory preset carrying drift 30¢/rate 0.4 and leaving defaults bit-stable.
2. The richness↔breathing axis is now a designed CONTROL, not a defect: drift up =
   stationary/rich (Serum-class), drift down + small K = coherent breathing (ours
   alone). Worth a named macro once the mod matrix lands.
3. Slider-units pass gains a datapoint: driftDepth already reads in cents — the
   one knob whose units let the human FIND this. Evidence for finishing that pass.

## Richness round 6 (2026-08-02): THE UNIFYING HYPOTHESIS — coherent comb-notch breathing

Human, on recreating the "jumping" waveforms at low f0: "it's exactly the distinction
I'm hearing: the effect of PWM notches closing and opening, which cuts a hollowness
into the rich sound of the supersaw." Mathematically exact, and it unifies the hunt:

- Two detuned saws sum to a waveform sweeping through pulse-like configurations, and
  the PAIR SPECTRUM is a sweeping comb |cos(pi*k*tau/T)|: as relative phase tau
  drifts at the beat rate, notches sweep the harmonic series. A supersaw is
  n*(n-1)/2 such pairs. The waveform "jumps"/staircases ARE near-aligned
  configurations; the audible hollows ARE the notch sweeps.
- WHY SERUM STAYS RICH: free-running voices, decorrelated beat rates → pairs sweep
  independently → instantaneous spectrum statistically STATIONARY. Richness =
  spectral stability over time, not average response.
- WHY OURS BREATHES HOLLOW: (a) small K > 0 near-critical coupling makes R breathe —
  ALL pairs sweep through alignment together = deep coherent hollows (the human's
  K=0.028 patch sits in this regime); (b) commensurate spreads (even/JP) make beat
  rates harmonically related → periodic collective alignment even at K=0;
  (c) retrig/keepPhase correlate initial phases. Low f0 + slow beats make it
  audible as PWM motion — matching exactly when it reproduces.

DETERMINISTIC METRIC (waveshape_check increment): frame-wise FFT over a long
render; per-harmonic amplitude variance over time + hollow-event count (frames
where a band drops >X dB under its own median). CALIBRATE: single saw → zero
variance; two free-running saws → the analytic notch sweep. Compare: even vs
gaussian vs GOLDEN dist (ADR-067 exists for exactly this) × K ∈ {0, 0.03, 0.3} ×
retrig on/off. Prediction: golden + retrig 0 + K=0 minimizes hollow events; small
positive K maximizes them.

IMMEDIATE EAR TEST: dist → golden, retrigger off, K → exactly 0, same low-f0
patch — does the breathing disappear? Design consequence if confirmed: richness is
a PHASE-STATISTICS property; the fix menu becomes spacing law + free-run defaults +
K-taper near 0 (+ per-voice drift) — not oscillator brightness. The Serum reference
render (round 5) stays wanted to close the static-spectrum question independently.

## Clean-mode aliasing measured (2026-08-01) — with a protocol limit caught mid-run

Midpoint aliasing, dB rel h1 (worst/mean): E3 — BLEP −51.9/−69.9, clean −44.0/−46.6;
660 Hz — BLEP −180/−186, clean −78.1/−78.8; 1763 Hz — BLEP −173/−180, clean
−69.6/−70.2.

**PROTOCOL LIMIT (caught before concluding, L0017 again):** these renders used
BIN-COMMENSURATE f0 (right for the droop test, wrong here) — folded aliases of a
commensurate saw land ON the harmonic grid, so midpoints are structurally blind to
them. The BLEP "−180" rows are the protocol seeing nothing, not the saw being that
clean; BLEP's true aliasing needs a re-run at detuned/incommensurate f0 (the
shape-lab protocol's original design). What IS valid: **clean mode's aliasing is
dense/inharmonic so midpoints do see it — it sits at −44 to −79 dB re h1, audible
territory at high notes.** Clean mode is therefore NOT a free flat-response win;
the digital↔clean tradeoff is droop-vs-aliasing, quantified on one side only.

DECISION INPUT still owed: BLEP aliasing at incommensurate f0 (expected very clean
per shape-lab's earlier −149 dB reading, but measure, don't assume). Then the fix
menu chooses: oversampled BLEP (flat AND clean, at CPU cost) is the likely winner
if BLEP verifies clean.

## HF-rolloff hypothesis CONFIRMED (2026-08-01) — the measured Serum-gap lever

Calibrated harmonic-droop measurement (ideal band-limited saw reads 0.00 dB at every
probe; bin-exact FFT, f0 164.8 Hz): HYPERSAW's default BLEP saw droops **−0.60 dB @
5 kHz, −2.17 @ 10 kHz, −4.50 @ 15 kHz, −7.56 @ 20 kHz** versus the ideal 1/k law —
the polyBLEP kernel's sinc²-ish rolloff, exactly the missing "air" vs Serum's
flat-to-Nyquist wavetable saws (their corner Gibbs ripple, their −60 dB analyzer
range). SURPRISE with design value: clean mode (digital 0) is far FLATTER (−1.35 dB
@ 20 kHz) — the default is the droopy mode; measure clean's ALIASING before drawing
conclusions (flat + aliased is not a free lunch — the aliasing midpoint protocol
from the shape-lab work is the calibrated tool).

DECISION FOR THE HUMAN (fix menu, already auditioned on the fold map): 2×/4×
oversampled BLEP · higher-order BLEP kernel · wavetable path · or re-tune the
digital↔clean blend once clean's aliasing is measured. Any of these touches the
reference → ADR + parity discipline; the droop numbers above are the acceptance
baseline to beat.

NOTE: the S-zag/up-jump behavior did NOT reproduce for the human this session
(cause of the earlier sightings still unidentified — their settings diff is
pending). waveshape_check now guards the K=0 invariants permanently either way.

## Competitor-reference convention (ratified 2026-08-01) + HF-rolloff hypothesis

**Convention** (candidate for promotion to doctrine CONVENTIONS.md §Audio plugins via
the human's flagpole): naming competitors factually in process docs (ROADMAP, labs,
ADRs, commits) is fine and normal — nominative use, benchmarking culture. Enforced
rules: (1) NEVER commit competitor-rendered audio, presets, wavetables, or captured
data — goldens are self-generated, A/B material stays local; (2) framing verbs stay
comparative, never imitative — "close the perceptual gap", not "match/clone X";
(3) SPEC.md stays competitor-free — the invention is defined on its own terms
(patent posture); (4) marketing copy never names competitors. No retroactive
scrubbing of merged history — the honest record is the better look.

**Serum gap, round 4 — the HF hypothesis (best-evidenced lead yet).** Human's
analyzer shots: Serum's saws carry visibly MORE corner ripple (Gibbs = harmonics
preserved to Nyquist) and MAnalyzer auto-ranges to −60 dB on Serum vs only −30 dB
on HYPERSAW; our spectrum visibly rolls off faster above ~2 kHz. Mechanism
candidate: **polyBLEP is a 2-sample correction whose kernel imposes a sinc²-ish HF
droop**, several dB down well below Nyquist, where wavetable/minBLEP saws (Serum)
stay flat to the top. "Depth and body" = the missing top two octaves.
DETERMINISTIC TEST (waveshape_check increment or standalone): render single saw,
FFT, compare harmonic levels to the ideal 1/k law — report droop at 5/10/15 kHz;
calibrate the measurement on a synthetic ideal band-limited saw first. If confirmed,
the fix menu is already on the roadmap: the fold-map's audition included 2×/4×
oversampling and anti-alias stages; higher-order BLEP and a wavetable path are the
alternatives. ALSO re-check `digital` (clean mode) — the human's patch ran digital 0,
which may roll off further.

## S-zag round 3 (2026-08-01): DOES NOT REPRODUCE HEADLESSLY — suspects all refuted with the exact patch

Human's patch state reproduced in the JS core (+ shell sub simulated in both
topologies). Refuted with numbers: tanh at vol 1 (26% squash — real compression,
but monotone: cannot create falls); sub sine −1 oct @ 0.43 (its steepest fall
0.00225/sample loses to the saw sum's rise, and post-tanh order changes nothing —
zero gradual-fall runs, longest run 1 sample); clean-mode edge width (3.0 samples
vs BLEP 3.9 — vertical at scope zoom); core curvature (numerically straight).
UNVERIFIED ASSUMPTION flagged: shell sub at subWave 0 was simulated as a pure sine
— confirm the actual shell waveform next session.

CONSEQUENCE: the S-zags are introduced DOWNSTREAM of the synth or by the scope's
display path. DISCRIMINATING TEST for the human (one minute): freeze/render the
track to an audio clip and inspect the RAW clip waveform in Ableton. S-zags absent
in the clip → MScope display processing (case closed); present → a device between
HYPERSAW and the meter (walk the chain, cf. the bass-mono find).

FULLNESS note: at the patch's vol 1 the tanh squashes 26% — Serum does not
saturate by default, so this alone is a real punch/fullness difference. A/B at
vol ≈ 0.5 with loudness matched before judging timbre.

## Serum gap round 2 (2026-08-01): rootWeight test was VOID; S-zag anomaly opened

1. **The rootWeight audition was void, not negative.** In the lab, `vgain = (1 −
   rootWeight·aw·up)` with `aw = p.anchor` for every law except harmonic — at the
   default anchor 0 the knob multiplies by ZERO. "Doesn't have much of an effect"
   was the gate, not the hypothesis. RE-RUN: root anchor → 1 (root pinned to f0),
   THEN sweep rootWeight on the matched patch. Fold consequence if it works: the
   folded control must NOT be anchor-gated (or the gate must be visible) — a knob
   that silently no-ops is a repeat of this exact confusion.
2. **Bass-mono curvature RESOLVED (human found it):** the ADR/M-S elliptic filter's
   phase rotation curves ramps — real physics, benign, but it means A/Bs against
   Serum must run with bass mono OFF. Worth a GUI hint at fold time.
3. **NEW ANOMALY — S-shaped zags.** Human scoped segments that a sum of rising
   ramps + downward jumps cannot produce: GRADUAL falling stretches / S-curved
   transitions (MScope, D2 ~73.7 Hz, both screenshots on file in the PR). A sum of
   ideal saws must rise between wraps (all voice slopes positive) and fall only by
   near-instant jumps. Candidate mechanisms to test headlessly, in order: (a) any
   LP in the path bending the jump into an exponential (tone tilt / rtone / hiTame
   / scope's own display filtering — test by scoping a SINGLE full-scale voice
   through the same chain); (b) saw-shape morph > 0 (the ADR-058 two-saw machinery
   creates genuine falling segments); (c) eff-frequency clamp at 0 freezing ramps;
   (d) width/pan summation in whatever channel MScope displays. DETECTOR to build:
   sustained negative-slope runs (>5 samples, excluding BLEP corners) on headless
   renders across a param grid — calibrate on a known-clean single saw FIRST
   (L0016). NEEDED FROM THE HUMAN: the exact patch state (save the Live set or use
   the dev state dump) so the render matches the scope shot.
4. **Fullness gap stays open** pending the valid rootWeight test + S-zag resolution.
   If both close and the gap remains, next suspects: per-voice level trims at Serum
   defaults, unison phase relationships at note-on, and Serum's built-in drift.

## Serum A/B diagnosis: the fullness gap is CENTRE-VOICE WEIGHTING (2026-07-31)

Human A/B'd a Serum 2 supersaw against ours: Serum "slightly fuller", its scope trace
a consistent big-tooth saw, ours wavy with "bends". MEASURED at the human's settings
before concluding (both suspects refuted): the tanh guard squashes peaks only 2.4%
(0.73% rms distortion — invisible on a scope), and the summed core output is
numerically PIECEWISE-STRAIGHT (median |2nd diff| ~2e-6 of peak, equal to a pure-saw
control). Nothing in our chain bends ramps.

The remaining explanation fits every observation: **Serum's supersaw (JP-8000
architecture) mixes the CENTRE voice louder and the sides down** (its detune-mix/blend
knob), so one strong saw skeleton survives summation — visually a consistent tooth,
audibly a solid fundamental = "full". HYPERSAW mixes all 7 voices EQUAL (only global
density comp), so the sum is a democratic interference pattern — piecewise straight
but meandering, with the fundamental carried by no one.

**The lever already exists and is already queued: `rootWeight`** — prototyped in the
detune lab, deliberately excluded from ADR-068 as a gain-domain feature ("its own
fold later"). Promote it: fold rootWeight as the centre/side mix control, audition
target = close the Serum fullness gap at matched settings. Confirmation test for the
human meanwhile: (a) in Serum, pull its detune-mix toward equal — its scope should go
wavy like ours and lose the fullness; (b) in detune-lab, raise rootWeight on a
matched patch — fullness should return. Either result confirms; both together settle
it. Also worth noting for the naming pass: normExp ("Density Comp") interacts — it
rescales TOTAL level by voice count but never re-weights voices.

## Pan-motion expansion (human direction, 2026-07-31; reference-first fold)

Two new controls ratified for the ADR-064 pan-motion system:
1. **Speed** — the rates are currently HARDCODED (sweep 0.1 Hz-ish; per-voice drift
   0.08 + i·0.021). Expose a rate knob scaling both modes.
2. **Position weighting (bipolar)** — one end: centre wiggles, sides still; other
   end: sides wiggle, centre still. NOTE FOR THE ADR: the positive half DUPLICATES
   `motionCenter` (centre pin) — per the consolidation principle the new bipolar
   knob should SUBSUME motionCenter (map old values onto the new axis, retire id 78
   from the GUI, keep the CLAP id as an alias) rather than ship as a third
   overlapping control.

Reference-first (swarmsaw.html carries ADR-064) + core parity + 2 new CLAP ids +
GUI; inert defaults (speed = current hardcoded feel, weighting = uniform).

## Poly glide + glide-from-last (human direction, 2026-07-31; "as long as trivial")

1. **Poly glide**: portamento in poly — each new voice glides INTO its pitch from the
   most recently played note's frequency. Likely genuinely small: the core already
   has per-swarm glide machinery (glideActive/glideTarget, ADR-026 mono retarget);
   poly noteOn seeds f0cur from a shell-tracked lastNoteFreq and glides to target.
   Core+shell only — glide never touched the JS reference, so NO reference edit and
   no parity exposure (verify with inert-default goldens anyway). One new toggle
   param (polyGlide); TIME reuses the existing Glide knob (id 33), which then stops
   being mono-gated in the GUI.
2. **Glide-from-last / always-bend mode**: a third glide state where every note —
   including after all keys are up — begins at the remembered last-played pitch and
   bends in. Design questions: single lastNoteFreq or per-voice nearest-prior-voice
   mapping for chords; does the memory decay or persist indefinitely; interaction
   with retrigger/keepPhase.
Ship both behind inert defaults; abort the "trivial" claim honestly if the chord
mapping (2) grows teeth — (1) alone is still worth it.

## Test round 1 results (2026-07-31) — NEXT SESSION'S BRIEF

**1+5. NOTE_END timing is still wrong — now in the OTHER direction (top priority).**
Stuck-forever is gone, but release lag is inconsistent ("minimum duration of played
notes varies seemingly at random") and mono re-press doesn't fire until the prior
key-up registers. DIAGNOSIS SKETCH: Live withholds retriggering a pitch until it
receives NOTE_END for the prior note (the 2026-07-18 finding that motivated emission
in the first place). We emit END at ENV DEATH (~1.1 s after release at default
settings), and the #135 deferral pushes some ENDs later still — so retrigger waits on
a tail the player can't see. REDESIGN QUESTION for next session: emit NOTE_END at
NOTE-OFF/steal time (prompt host bookkeeping; the DSP tail still sounds — hosts don't
gate our audio) vs at env death (today, laggy). Emitting promptly on release likely
fixes 1 AND 5 and lets the #135 deferral be DELETED rather than patched. Check CLAP
spec intent + what other CLAP instruments do before committing.

**2. Voice-map amber jumps between voices; pivot pinning invisible.** The GUI marks
lowest-vf PER FRAME, so drift/coupling makes the crown hop. Fix: publish the core's
STABLE root index (ADR-068 rootIdx) in the viz snapshot and mark that. Re-test pivot
after — pinning may already work and be unobservable under a hopping marker.

**3. Ruling recorded:** harmonic/oct-spread extremes are sound-design terrain, kept
as-is (per-law usable-range table remains the eventual answer, already roadmapped).

**Hi-tame audit RESOLVED (2026-07-31, formula-level evidence):** gain is
(f0/vf)^hiTame, so its bite is proportional to pitch SPREAD — at the human's
±28¢ default the max cut is −0.14 dB (inaudible, exactly as reported), at
octave spread it is −6 dB, at harmonic reach 4 it is −28 dB on the top voice.
NOT broken; spread-proportional by design (ADR-061 is an equal-loudness law).
GUI tooltip now says so. A RESCALE (e.g. normalizing to the current spread) would
change reference behavior → it is a fold-discipline decision, parked unless the
human wants the control to bite at cents-level detune too.

**Quick fixes queued (all GUI-side):**
- Double-click any slider → default (use kParams defV; trivial, do first).
- Retrigger soft-gate is wrong: grayed in SPECTRA and whenever scatter==0 is FALSE…
  human ruling: retrigger should effectively NEVER gray (only inert case is SAW with
  scatter>0 — verify then simplify the gate).
- Hi tame inaudible at defaults — audit: gain (f0/f)^hiTame only bites with WIDE
  spreads; at ±28¢ the ratio ≈1 so it does ~nothing. Either rescale the curve for
  small spreads or gate/label it as a spread-dependent control.
- SPECTRA should feed the voice map too (partial-0 cloud, or per-partial seats —
  design at fix time).

## Human-test protocol (ratified 2026-07-31)

**TESTING.md at repo root is the living human test checklist.** Every PR that changes
human-testable behavior updates it (agent refreshes items; human checks off in Live,
reports failures by item number). Prioritized, ~15 min; stale items pruned, verified
items move to known-good. This replaces ad-hoc "try it and see" handoffs.

## STUCK NOTES: FIRST HARD EVIDENCE (2026-07-31) — poly, computer keyboard, GATE STAYS ON

The note monitor did its job on day one: the human reports "almost every note is
getting stuck (in polyphonic mode, using the computer keyboard)" — cells staying
FILLED with keys up. Filled = the core still sees gate=1, i.e. **the note-off never
reaches the plugin.** Combined with the CLAP layer being probe-clean (tailprobe, 60
runs; notefuzz 12 modes), the fault is between Live's computer keyboard and our
process() input queue — the wrapper translation layer.

PRIME SUSPECT for next session: **our CLAP_EVENT_NOTE_END emission.** clap-wrapper
keeps a note bookkeeping table to translate VST3/AU note streams; if we emit
NOTE_END for a voice that is still HELD (voice steal, re-press, tag aliasing), the
wrapper may drop the note from its table and then SWALLOW the eventual note-off —
which would produce exactly this: gate stuck on, poly, fast typing. Audit
tags[]/NOTE_END emission against steal/re-press first; then instrument the wrapper
if clean. (The monitor's own skip condition `!gate && env<1e-4` is worth a
5-minute sanity check too, but filled-cell-persists implicates gate, not the viz.)

## Even-voice pan fan — symmetric image (human direction, 2026-07-30; needs ADR + fold)

Human: "even numbers of voices should have no voice centered (right now 2 with any
width is unlistenable)." Correct — the ADR-070 fan seats rank 0 at dead centre and
steps out at d = r/(n−1), so n=2 degenerates to one voice centre + one voice HARD
side: a lopsided image at any width. Direction ratified:

- **Even n: no centre seat.** Symmetric pairs balanced across L/R — proposed law:
  pair k sits at ±(k + 0.5)/(n/2) · width (n=2 → ±0.5·w; n=4 → ±0.25, ±0.75). Pitch
  ranking and alternating sides unchanged; only the distance law forks on parity.
- **Odd n unchanged** (root keeps the centre seat — the ADR-070 image the human asked
  for is explicitly the odd-n case).
- **Scatter's role in the new mode**: human sketch — scatter OFFSETS the symmetric
  seats rather than "what it does now"; exact behaviour is an ADR question (offset
  pairs together to keep balance, or per-voice with a balance re-center?).

This CHANGES DEFAULT OUTPUT for even voice counts → reference-first fold (protected
swarmsaw.html edit under the human gate above, which this direction constitutes),
its own ADR, golden updates for even-n scenarios, voice-map verification after.

## Lab campaign 3 (human direction, 2026-07-30)

Three labs ratified, extending the campaign-2 pattern (audition first, fold with ADR +
parity after):

1. **SPECTRA robustness + expansion lab.** Activates campaign-2 item 4 with a sharper
   brief: make the engine *worthwhile* — find the features that give SPECTRA its own
   identity rather than "the other engine". Candidates to audition: richer partial-amp
   laws, per-partial coupling topologies, cascade behaviors, transposition interplay,
   whatever the lab surfaces.
2. **Swarm-filters lab.** Human verdict: the E1 filter/notch cores are "not quite
   there yet". Audition bench over `filter_core.h` + `notch_core.h` character —
   resonance behavior, key-tracking, how they'd sit in the rack next to a conventional
   multimode (this pairs with, but is distinct from, the layout-lab's
   conventional-filter-topology question).
3. **Quantum-morph lab — ACTIVE; campaign-3 increment BUILT 2026-08-05** (human: "prototype
   the quantum morph first, then perfect the global interface and all the sub-pages").
   Built onto the existing flip-morph prototype, all verified in-browser:
   (a) **both tint modes** auditionable — Dominant (crisp allegiance, flips read as color
   flips) vs Mixture (the weight vector as a blended hue; the glyph still shows the OWNER,
   so hue answers "where am I" and glyph answers "who owns this" — two channels, two
   questions); (c) **glyph pairing shipped** per the ratified ruling — ◆▲●■ on every chip
   and in the legend, with Always / Hover / Off auditionable; (b) **edit routing made
   legible**: chips are now EDITABLE (vertical drag), the write goes to Owner / Nearest /
   Armed per a selector, and the edit flash is the TARGET corner's color — where the edit
   landed is answered by sight; (e) **copy-from shipped**: arm a corner in the legend,
   copy any other corner's preset into it. Remaining: (f) cross-engine blending needs the
   real engines (deferred to fold time). **(g) RULED (human, 2026-08-05): mod-matrix
   collisions resolve as "blend depths on agreement, flip on topology changes"** — where
   two corners share a routing's source and destination, the DEPTH morphs continuously
   like any continuous param; where the topology itself differs (different source,
   destination, or a routing that exists in one corner and not the other), the routing
   FLIPS through the same quantum machinery as the discrete params. Convergent with the
   agent's proposal — arrived at independently, which is the strongest ratification the
   process produces. Design can now proceed at fold time. **TERRITORY AUTHORSHIP (human, 2026-08-05): "I can't actually seem to edit the
   territory for each setting, which would be a useful level of granularity for making
   sure the whole morph produced good sounds."** The flip map was a pure lottery —
   reshuffle until you like it, with no way to guarantee a parameter never flips
   somewhere ugly. Two authored terms now enter the SAME score the audio path uses:
   **corner weight** (per-parameter, per-corner thumb on the ballot — hand a corner more
   or less of the grid) and **pin** (hard override; one corner owns the field and the
   parameter never flips). Deliberately part of the score rather than a post-hoc
   override, so temperature/coupling/reshuffle keep working on top; reshuffle re-rolls
   the lottery and leaves authorship intact (verified). Measured: cutoff baseline
   A 60.0 / B 23.3 / C 6.3 / D 10.4 % → corner-C weight +2.5 gives 26.1/18.2/**54.0**/1.7,
   −2.5 gives 63.7/23.3/**0.1**/12.8; pin B gives 0/**100**/0/0 and the audio path shows
   corner B for all 200 sampled field positions. **Refactor that made it safe:** the map
   and the audio had two copies of the scoring law; they are now one `pickCorner()` (the
   L0011 trap — a map that can disagree with the sound). Discoverability fixed alongside:
   an explicit territory selector + live grid-share readout, since the only way in was
   clicking a rack chip. **RESHUFFLE POLICY + CLEAR-ALL (human, 2026-08-05):** "Reset this parameter" does
   return a parameter to the pure lottery and re-eligibility (verified), and there is now
   a **Clear all authorship** button plus a policy toggle deciding whether reshuffle
   **Keeps authorship** (default — re-rolls only the lottery underneath) or **Re-rolls
   everything** (wipes bias and pins first). An authored-count readout sits beside it,
   because authorship is otherwise invisible state and easy to forget three patches later.

   **INTELLIGENT RANDOMNESS — human design note, 2026-08-05, NOT YET BUILT.** *"Most users
   are just going to want to run with the random settings (or, possibly, an intelligent
   randomness that we predetermine for at least a subset of parameters based on which
   features depend on which others to be musical)."* This is the right long-term default
   and it is a genuine design problem, so it is recorded rather than improvised.
   **What already exists:** `module coupling` is a coarse first version — parameters in
   the same module share a Gumbel draw, so they tend to flip together. **What it misses,**
   from the actual 22-parameter list:
   - **Guard dependencies** — `lfoRate` / `lfoDepth` are meaningless when `lfoDest` is
     `off`; flipping them changes nothing audible, so a flip "spends" randomness that the
     listener never hears. Ties them to the guard is logic, not taste.
   - **Joint-musicality pairs** — `cutoff`+`res` (high resonance at a low cutoff is a
     scream), `atk`+`dec`, `dlyTime`+`dlyFb`+`dlyMix` (long time × high feedback × high
     mix = wash). Independent flips can land on combinations no corner authored.
   - **Anti-degenerate constraints** — `levA`+`levB` both landing on low-level corners is
     near-silence; both on high is a level jump. Neither is a state any corner contains.
   **Proposed shape:** declare dependency GROUPS (guard / joint / anti-degenerate) that
   share a draw or constrain each other, sitting under the existing coupling knob as a
   smarter default rather than replacing authorship. **Needs a human ruling on the
   groupings themselves** — which pairs are genuinely coupled is a taste judgement about
   this instrument, and the agent should not invent it.

   **Also added same day (human):
   CAPTURE — arm a corner and overwrite it with the current resolved settings, so a
   mixture found by ear on the pad becomes a corner you can morph back to.** Continuous
   params capture their blended value; discrete params capture the owning corner's value.
   Verified: centre-pad mixture cutoff 2017 (corners 6500/700/1400/2600) captured into
   the armed corner exactly.
   (exists; STAYS gitignored for now — human ruling 2026-07-30,
   revisit once the lab has settled). NEW INTERFACE CONCEPT to prototype there —
   **corner colors**: each morph corner owns a color; every control tinted by the
   corner it currently controls, so allegiance flips are visible as color flips, and
   "which preset am I editing?" is always answered on sight. Design questions for the
   lab: (a) mid-morph, tint by the *mixture* (proportional blend of corner colors —
   the weights made visible) vs by dominant corner only; (b) the edit-routing rule the
   color must make legible — does an edit write to the dominant corner, the nearest,
   or an explicit armed corner?; (c) color+shape pairing RATIFIED (human,
   2026-07-30): every tinted field pairs a small corner glyph with the color —
   possibly revealed on hover rather than always-on (audition both in the lab); (d) keep the palette to 4 highly-separable hues.
   FURTHER MORPH ROADMAP (human, 2026-07-31): (e) each corner gets a "copy from
   <other corner>" action (all three sources, per corner); (f) design an elegant
   blend for oscillators that are INACTIVE or a DIFFERENT ENGINE across corners —
   options to discuss: cross-engine parameter mapping so shared axes (detune, K,
   width…) morph continuously and only engine-specific residue jumps; level-fade
   an osc whose engine flips; treat engine identity as a collapse-only (never
   blended) property. (g) mod-matrix collisions across corners acknowledged as a
   challenge — the human has ideas; capture them at the next morph session before
   designing.

## GUI information architecture + full-product fold plan (human brief, 2026-07-30)

The human wants ALL labs folded into the plugin, gated on visual-hierarchy decisions
first: an uncluttered primary view, multiple pages, dropdown/right-click homes for the
long tail. **`docs/design/layout-lab.html` is the audition instrument** — a clickable
mock of the full product built from the REAL inventory (all 86 shipped params + every
lab feature with a fold path), chips marking shipped-new vs lab-only. Its decisions
table is the deliverable:

1. **Page count/names** — mock proposes 5: MAIN (play) · OSC (per-osc deep edit) ·
   SPACE (image+FX) · MOD · MORPH. Principle: MAIN is what you touch while playing;
   nothing lives ONLY on MAIN. Right-click = mod-assign/reset/units; ⚙ = global prefs.
   **Ratified requirement (human, 2026-07-30): EVERY parameter's right-click menu
   reaches the mod matrix** ("map to…" → pick source, or jump to that param's matrix
   column) — mapping must never require a trip to the MOD page first. This is the
   same right-click surface the per-param curve editor (below, 2026-07-21) will live
   in — one context menu, growing.
2. **Multi-oscillator architecture** (2–3 full oscillators, each independently
   SAW/SPECTRA/…, per-osc levels; maybe sub stays global). NOT a GUI change — N core
   instances, per-osc param namespace (ids are append-only: design ONCE), per-osc
   preset format, CPU budget. **ADR before any GUI work**; then a 2-osc walking
   skeleton behind the existing balance param.
3. **Lab-needs matrix** (full table in the lab): ready to fold now — SwarmVerb,
   E2 delays, ensemble timing stack, Kuro chorus/phaser + LFO/matrix. Need a lab
   first — saturation/drive (drive curves × placement), conventional filter topology
   (how it meets the swarm filters), sequencer (or park it). Needs an ADR not a lab —
   multi-osc. Presets: browser is GUI work; per-osc format lands with the multi-osc
   ADR. Quantum-morph lab stays iterating (NOTE: that lab is gitignored — decide
   whether that stays true as it matures).
4. **Human-readable units pass** (human, 2026-07-30, "not a rush"): display-only —
   cents σ / ms / Hz / semitones where physical (trigger example: core detune 0.20
   ≙ gaussian σ 8¢, unknowable from a 0–1 knob), dev params hidden. CLAP ids and
   stored values unchanged (only value_to_text + GUI outputs), so sessions and
   automation survive. Pairs with the deferred naming pass.

**Fold queue once IA is ratified** (each with ADR + parity discipline, sequenced by
the human): ensemble timing stack (strongest evidence, inert at 0) → SwarmVerb +
E2 delays as rack slots → Kuro LFO system + matrix → shape-lab axes (sync/warp/
ripple/windowed-carrier, which also unlocks #29 formant scatter and the ADR-058 saw
retarget) → morph. Slider-units pass can ride any of these.

## Phase 0 — Platform gate & renderer decision

- CLAP-native skeleton; VST3 via clap-wrapper. Empty plugin builds on macOS + Windows, loads in target hosts (Live, Reaper, Bitwig), passes pluginval at strictness ≥ 5.
- CI: build matrix + pluginval + `./verify fast` wiring (initially trivial-green).
- **ADR-006 spike:** oscillator-bank vs iFFT additive renderer. Benchmark: 128 partials × 5 voices × 4 notes on target min-spec CPU; measure headroom both ways; decide and record. (Architecture note: coupling already runs at control rate, so iFFT frames are a natural fit if the bank loses.)
- **GUI stack decision (ADR-013):** pick the plugin GUI framework in Phase 0 so Phase 2 can ship a real GUI that reproduces the prototype design language (canvas-style phase circle, meters). Record as an ADR.
- Define target hardware envelope for E-6.
- **Gate:** hosts load it, CI is real, ADR-006 closed, GUI stack chosen.

## Phase 1 — SwarmCore port + parity oracle

- Port `SwarmSynth` (SAW core) to C++: mulberry32, seeding scheme, 16-sample control tick, σ-normalized bipolar K with slews, splay (3× authority, center anchor), inertia, R→tone, envelopes, voice stealing, tanh guard.
- Build the parity harness: JS reference renders (Node, checked into repo as golden generators, not binaries) vs C++ output; L0-1 green across the matrix.
- Port the headless trajectory tests: L0-2 through L0-5, L0-13.
- **Gate:** L0-1..5, L0-13 green. No UI exists yet and that is correct.

## Phase 2 — SAW mode feature-complete

- Distribution menu (even / JP / Gaussian / Cauchy / bimodal / clustered-pairs), detune laws (cents / Hz / ERB / tempo-grid with host-tempo sync), onset-lock/dissolve, retrigger, density comp, width + mono audition, digital↔clean, XY pad as macro pair.
- **GUI v1 (ADR-013, pulled forward from Phase 5):** phase circle with dual R₁/Rₙ meters, seat rings, formation polygon, XY pad, live R/σ/pull readouts — the SPEC §5.6 contract, styled to match the prototype design language as closely as possible (extract palette/treatments from the prototype CSS, don't reinvent).
- **Dev state button (human request, 2026-07-17):** a GUI-v1 affordance that copies the current full parameter state as JSON to the clipboard (for debugging / pasting into a session) plus a manual "save preset" action. Design position: the debug dump IS the preset format — one Layer-schema JSON with provenance metadata (SPEC §5.7), no second serialization mechanism. [SHIPPED]
- **Tempo-grid audibility experiments (human request, 2026-07-18):** hard to find settings where the grid lock is clearly audible. Explore: default-detune interactions, u ranges that put beat rates in the 0.5–4 Hz sweet spot, a "grid-forward" preset. Note the Phase 3 grid-status readout (ADR-016/017) directly attacks the legibility half of this — the populated-but-inaudible state becomes visible. Revisit alongside it.
- **Detune workshop (`docs/design/detune-lab.html`; PR #70) — FOLD CAMPAIGN IN PROGRESS (2026-07-23).** The audition phase produced a reviewed fold map (`docs/reports/2026-07-22-lab-to-core-fold-map.html`) and the reference/core folds are landing per its sequence, each a parity-safe superset with its own ADR + goldens: **FOLDED** — tone tilt (ADR-060), hi-tame (ADR-061), drift modes + keep-phase (ADR-062), opt-in freq glide (ADR-063), pan motion + centre pin (ADR-064), harmonic law + harmReach (ADR-065, incl. the chaotic-regime parity domain limit now in ACCEPTANCE §L0-1), stretch law as law 5 (ADR-066). Also folded since: golden distribution as dist 4 (ADR-067), octave spread + root-anchor across every law (ADR-068 — the placement-block rewrite, inertness proven by manifest diff). **SUPERSET + NEW-LAWS PHASE COMPLETE**: nine folds, ADR-060..068, parity 54/54 → 117/117, every scenario rms 0. **RESOLVED (human, 2026-07-24):** comp/limiter → **FX-rack slot**, not a core fold (HPF precedent — don't freeze a stopgap param); polyphonic KS comb → **FX-rack slot**, approved. Both land as E3 rack increments. **REMAINING** — the divergence ADRs (APPROVED 2026-07-24: root-pivot topology, alternating-pan default image; saw-shape retarget NO LONGER blocked — see the phase-shape axis unblocking below); rootWeight (gain-domain, excluded from ADR-068's scope); and last the **batched CLAP param pass** (public-interface gate — must widen `law` 0..3 → 0..5 and `dist` 0..3 → 0..4 + labels, since harmonic/stretch/golden are currently core-only and unreachable from the host, and expose tilt/hiTame/driftMode/keepPhase/freqGlide/panMotion/panMode/motionCenter/harmReach/stretchB/spread/anchor). Master HPF stays lab-only (see E3). Original audition scope, for provenance: **harmonic law** (unison→series morph — the coherent-metallic "spread" the NI/AG Cook instrument uses; voices land on the harmonic series, root-anchored) + **reach** (decouple top harmonic from voice count); **octave spread + root-anchor**; **stretch (inharmonic) law**; **golden distribution**; **alternating pan fan** (root-centred, voices step out on alternating sides) + pan scatter; voice-tone/anti-alias/power stages (tone tilt weighted to highs, 2×/4× oversample, drive, envelope-normalize, hi-tame equal-loudness); **per-voice + per-sample frequency smoothing** (de-zipper the sweep); **Karplus-Strong comb** (human keeper). Real-time **voice map** (pan × pitch, target vs actual) added for auditioning. All default-inert (mono fingerprint Δ=0) except the deliberate new pan default. **Forward — per-mode parameter limits (human, 2026-07-21):** the expanded space reaches a lot of unusable terrain (extreme reach/spread/detune combos); a dedicated session to set per-law usable ranges (clamp/curve table, folded in at port time). **Forward — scale/pitch quantization (human, 2026-07-21):** an optional post-detune quantizer that snaps each voice's frequency to a chosen musical scale (set the song key + scale; voices quantize into it). Design questions: quantize the target or the smoothed frequency (former = clean intervals, latter keeps glide); interaction with the harmonic law (harmonics are already a "scale" — quantize probably applies to the spatial laws); per-voice vs. whole-swarm; how it reads on the voice map (snap targets to scale gridlines). Prototype in the detune lab first. Winners fold into swarmsaw.html (reference, ADR-011/012) → port to swarm_core.h with parity (freq/tilt slews as seconds→per-tick coeffs, ADR-009). **Forward — saw-shape direction (human, 2026-07-22):** the real instrument should NOT ship the saw↔square morph (ADR-058, id 69 "Saw Shape") — **saw is a design constraint**; rounding toward "glass" (the lab's round / round×hi bench, 2026-07-22) stays acceptable for now, may revisit. Instead the shape control should morph through *subtle sawtooth variations* — profiles analysed from existing synths' saws, plus more experimental saw shapes at the far end. A discrete-algorithm sweep is acceptable; smooth interpolation between shapes is the ideal. Add a **Saw Shape visualizer** showing the currently-selected waveform. Reconsiders ADR-058's square target (keep the two-saw machinery, retarget the morph); prototype-first in the lab. **Two-slider design (human, 2026-07-22, prototyped in the lab):** a `saw base` slider selects the top-level saw shape and a separate `roundness →` slider selects the shape it rounds toward (roundness = depth, round×hi = pitch-weight) + the live Saw Shape visualizer. **UNBLOCKED 2026-07-25 by the timbre research:** the base bank no longer waits on measured synth-saw captures — a **ripple / phase-shape axis** (variable-slope phaseshaping; the same phase-domain family as the sync and formant candidates) IS a continuum of subtle sawtooth variants, which is exactly the brief. Captured profiles remain a *nice-to-have* for naming/anchoring presets, not a prerequisite. Retarget accordingly: keep ADR-058's two-saw machinery, aim the morph at the phase-shape axis, and prototype it in the sync/formant bench below rather than waiting.
- L0-12 green (grid law); Layer-E 1, 2, 5 sign-off.
- **Gate:** SAW mode is a shippable instrument on its own — playable through its own GUI. **GATE CLOSED (ratified 2026-07-21, human).** Layer-E 1/2/5 signed off — E-2, E-5 passed; E-1 passed with two parked UI-mapping refinements (NOT DSP changes; the tapers are parity-frozen). CAVEAT CORRECTED 2026-07-21 (the original "steep on both sides of K=0" was a mischaracterization): (1) the cloud→order K-transition has a sharp edge around K≈0.6–0.8 (settings-dependent) — human has adapted; optional tune-then-lock curve slider; (2) the INERTIA knob's response is steep just after 0 (ADR-024 sqrt taper), pronounced at low detune + retrigger-on — the human's real concern, addressed via a tune-then-lock inertia-curve slider. Retrigger fix confirmed in Live. Remaining distribution scope moved per the reference-first principle: **bimodal** relocates to Phase 3 (the dynamics reference implements its placement tied to two-cluster topology — port them together, with parity); **clustered-pairs** has no reference implementation anywhere — awaiting a prototype update from the design session (human is asking the original agent), then ports with parity. Ratify to close.

## Phase 3 — Dynamics integration

- Topology (mean-field / ring+reach / two-cluster+μ), Sakaguchi α, absolute-K mode, consonance gravity + basin + ratio readout.
- **Root-pinned pacemaker topology (human-validated in the lab, 2026-07-22).** A `sync pivot` option: mean-field (collapse toward the swarm mean) OR **root** (every voice entrains to the fundamental — the voice nearest f0 — so it stays pinned and the rest fold onto it; pitch-stable collapse). Human keeps BOTH as a toggle: "the root sync is a great option; totally different, but a more musical sound in general at a lot of settings." It's a coupling-law divergence from pure mean-field → its own topology entry here; prototype-first fold into swarmsaw.html + an ADR before the core port. Lab caveat: the pacemaker drops the R (order-parameter) scaler, so its onset off K=0 is a touch stronger — revisit the taper at port.
- Daido poles q (1–4) with R_q meter (ADR-015); tempo-grid status readout + cause-AND-state lock warning (ADR-016/017).
- Formalize L0 criteria for q-cluster formation / demographics / bistability from the ADR-015 anchors (R_q = 0.97 at q∈{2,3} across seeds; 2f0 projection ~0.080 seed-invariant) and add them to ACCEPTANCE.md at this gate as L0-22+ (L0-14..21 are taken by Track E, ingested 2026-07-18).
- L0-8..11 green; **Layer-E 3 SIGNED OFF (human, 2026-07-18: "I hear it. Sounds great")**.
- **Tonality brief ON HOLD (human, 2026-07-18):** the human will prime Tonality directly; integration scope under discussion (see traces — possible outcome: HYPERSAW owns a richer static ratio table itself and only context-weighting ever involves Tonality, or the integration is skipped). Gravity ships on the default set either way.
- **Gate:** the dynamics lab's verified states are reproducible in-plugin from preset recall. **GATE CLOSE PROPOSED (2026-07-18):** engine parity 51/51 both references; L0-8..12 green; ADR-015 anchors formalized as L0-22 and enforced in trajectory_check (q-cluster R_q, bistability, split-as-timbre projections); surface complete (params 24-31, meters, gravity + grid readouts per ADR-016/017); Layer-E 3 signed off; preset-recall reproducibility guaranteed by state_check's bit-identical-restored-audio requirement. **GATE CLOSED (ratified 2026-07-21, human).** (Bimodal placement confirmed shipped via two-cluster topology + goldens dyn-twocluster/dyn-cluster-balance + L0-10/L0-23 anchors; tempo-grid audibility remains a parked legibility item with its own readout, not a blocker.)

## Experimental engines (parallel track; ingested on drop)

- **Swarmalator** (SPEC-SWARMALATOR.md, swarmalator.html; ADR-048, 2026-07-19). Phase θ ↔ spatial position ξ coupled to each other — timbre and stereo image as one dynamical system. **Ported bit-exact** (src/swarmalator_core.h; swarmalator_check: stereo parity RMS 0.0 on 9/9 + the §5 acceptance anchors, in ./verify full). STATUS: core + oracle done; **EXPERIMENTAL — awaiting the human's listen before shell integration** (may not survive; may be joined by other new engines). Shell path when greenlit: an engine in the instrument's selector, or a slot in SWARM-FX-style hosting. Under ADR-045 it's a (Γ,W) point (ring spatial topology × two-term Γ); it also delivers the parked grain-swarm's spatial dynamics as a special case. **NEXT (human direction 2026-07-20):** hear it first as a nondestructive parallel engine (engine-select, SAW byte-frozen). **Spatial-blend slider idea (human 2026-07-20):** rather than a separate engine long-term, a single slider — 0 = the engine behaves as it does today, 1 = full swarmalator spatial-swarm behavior — that could apply to SAW *and* SPECTRA (the spatial coupling as a shared, per-engine characteristic, not a copied core). Open design problem to resolve for a fluid morph: how the spatial swarm interacts with the existing static pan/spread logic (pan scatter, width, SPECTRA swidth) as the slider crosses from static → dynamical pan. Decide after hearing the swarmalator. SPECTRA-spatial specifically needs its OWN formulation (its multi-partial structure ≠ the swarmalator's single θ-swarm + W± math) — a prototype-first addition of its own weight.

- **Granular-sibling engine intake (human roadmap note, 2026-07-20).** The human wants to eventually package a trimmed-down version of the **granular sibling's** engine inside HYPERSAW — reporting "really surprising sounds" from the two together, with potential especially at **the intersection of granular and dynamical**. This is the same family as the already-referenced parked grain swarm (SPEC-SWARMALATOR §; the swarmalator delivers its *spatial* dynamics as a special case, ADR-048) — a granular layer whose grain population would live under the same force/coupling physics as the swarm. **Not yet actionable:** gated on the granular sibling maturing first; when ready, intake follows INTEGRATIONS.md (brief→response, writes stay home — like the terrain-sibling Phase 4 intake) and ports prototype-first per ADR-003 against a reference clone. Design questions for that session: which grain parameters become swarm coordinates (onset/rate/position/pitch), whether grains couple to the carrier swarm or run parallel, and CPU against the E-6 envelope. (Alias note: "granular sibling" per PRIVATE-NOTES.md — the real name is never written in tracked files.)

- **Kuramoto chorus** (human direction 2026-07-19; prompted by Chiral Audio's Foxfire, chiral.audio/kuramoto-audio-synchronization — see PRIOR-ART §1). A chorus/ensemble engine where N **modulation LFOs** are Kuramoto-coupled: each voice reads a short base delay (~5–30 ms ensemble body) whose offset is moved by its LFO phase; coupling K sweeps the modulators from broad/incoherent (lush, statistically wide) to correlated/locked (the field "tightens") — K as the single performance gesture, R as the readout. DISTINCT from the E2 tap-swarm delay (which herds the delay TIMES via the force system on log-time coordinates for rhythmic/long delays); the chorus couples the LFO PHASES modulating short delays. Prior art is a SHIPPING product (Foxfire) — cite it; HYPERSAW's contribution is the integration (shared force-core coupling, the instrument's own K/gravity vocabulary, seeded determinism, and it living in the same swarm-FX shell). Prototype-first per ADR-003 when built. Reuses the force core's phase-coupling (it's an LFO-rate Kuramoto — the same sin coupling SwarmCore runs at audio rate).

## Architecture expansion — parallel oscillators & multi-page device (forward; under external prototyping)

Two coupled directions the human is developing on separate threads (2026-07-19). Roadmapped, not yet designed; ingest-and-port on drop like the engines.

- **OSC2 / OSC3 — parallel Kuramoto oscillator banks.** Today the instrument is ONE swarm voice engine (SwarmCore, with SPECTRA as an alternate). This adds two more parallel banks — each a full independent Kuramoto swarm — layered into one voice, the way a classic synth stacks oscillators, except each "oscillator" is a whole swarm. Motivating a "more complex idea" the human is prototyping separately. **Design questions to resolve before building:** (a) per-osc surface — each osc its own engine (SAW/SPECTRA/dynamics?), K, distribution, seed, and a tuning offset (octave/semi/fine/level/pan) so they can be detuned/stacked; (b) **independent vs cross-coupled** — are OSC1/2/3 independent swarms summed (straightforward layering) OR Kuramoto-coupled *to each other* (a swarm-of-swarms — this is PARKED #5, and likely what the complex idea needs)? The cross-coupled case is the novel one and needs its own reference/oracle; (c) mixing/routing (per-osc level, osc→FX send); (d) **CPU/E-6 re-check** — 3× the voice cost against the min-spec envelope is significant and gates how many banks × voices are allowed. Architecture: the shell holds N core instances summed; param ids append per osc (a large frozen block); state grows. Prototype-first per ADR-003; cross-coupling wants a swarmdynamics-style clone to measure against.

- **Multi-page device GUI — a high-level control page.** The webview goes multi-page (a page/tab switcher, prototype design language preserved). A **high-level page** controls the oscillators from the top (osc on/off, mix, tuning, per-osc engine) — and is the natural home for the **mod-matrix interface** (Phase 5) and the eventual **FX routing** (E3, effects-as-sections). Likely page structure: Overview/Oscillators · per-osc Detail (today's dense single-page view becomes the detail page) · Mod Matrix · FX Routing. Design questions: page navigation model, how per-osc detail is reached, keeping the swap cheap (all pages share the one param bridge). This unblocks presenting OSC2/OSC3, the mod matrix, and FX routing without cramming one flat page. Depends on nothing shipped; buildable once the osc-bank or mod-matrix surface is decided.

## Phase 4 — SPECTRA mode & kernel abstraction

- Per-partial engine at the ADR-006 renderer: amp tilt, stretch, width tilt, width law, cascade, splay-as-interference-gate with per-partial stereo narrowing.
- Kernel abstraction landed: saw / sine share one voice path; wavetable kernel stubbed (terrain-sibling crossover parked until here).
- L0-6, L0-7 green; Layer-E 4 sign-off.
- **STATUS (2026-07-18):** SpectraCore ported (verbatim, own goldens): parity RMS 0.0 on 9/9 scenarios vs the live-sliced JS reference; L0-6 (monotone front, 7.21 s / 1.81 s) and L0-7 (−15.06 dB, narrowing engaged) GREEN, enforced in ./verify full (spectra_check). **ADR-037 RULED (human, 2026-07-18) — option (a):** the P=1 gate is a MEASURED-equivalence check (tick-for-tick R-trajectory match, implemented + green in spectra_check; at P=1 the two references' dynamics coincide, the kernel being the only difference — exactly SPEC §2's claim). This resolves the Phase 4 gate interpretation. Optional follow-up only: try a shared voice path behind a switch for an A/B listen (nice-to-have, not a blocker). Shell integration SHIPPED (2026-07-18): engine select id 43, SPECTRA surface 44-51, note/render dispatch, state round-trip, GUI engine gating; SPECTRA v1 viz = partial-0 cloud (per-partial lock-front display and Layer-E 4 sign-off remain, then the shared-voice-path A/B follow-up).
- **SPECTRA routing parity + new params (human sweep 2026-07-20).** DONE: transposition (octave/semi/fine/pitch) now transposes SPECTRA (ADR-057). REMAINING SAW→SPECTRA routings, when wanted: voice mono/glide/legato (needs glide/retarget in spectra_core), MPE per-note bend (per-voice noteTune), drift/rtone/scatter/panScatter (core additions). FORWARD (human interest 2026-07-20): SPECTRA is also a target for *new, SPECTRA-native* parameters (beyond porting SAW ones) — the per-partial structure has design space SAW doesn't (per-partial coupling shaping, inharmonicity curves, cascade variants, spatial-partials per the swarmalator-spatial idea). Collect ideas as they surface; each is its own prototype-first increment.
- **Gate:** SAW provably = SPECTRA at P=1 (parity between modes on equivalent settings).

## Phase 5 — Performance layer & face

- **Design language — visual & intuitive (human, high-level, 2026-07-22). GUIDING PRINCIPLE for the whole face.** The final version should be as visual and intuitive as possible. Two rules: (1) **Naming by feel.** A parameter's label is literal ONLY when the parameter is conventional (cutoff, attack, mix); otherwise it is named for what it *feels* like, in the instrument's metaphor — e.g. K (Kuramoto coupling) means nothing to most people and should be something like **"cooperation"** (the swarm sticking together); the "swarm" itself may eventually get a weirder name (e.g. **"horde"**). (2) **Every control carries a visual.** No naked slider for a non-obvious parameter — each pairs with an intuitive live visual (the lab's voice map, phase circle, saw-shape scope, level meter are the seeds of this). Applies across the face: the dense engineering names used through Phases 0–4 (K, σ, R, dissolve, onset, Daido q, …) get a translation layer for the player-facing UI while the internal/param-id names stay stable. Sequenced with GUI completion; the metaphor/naming pass is its own design task (a glossary: internal name → felt name → visual).
- GUI completion (v1 shipped in Phase 2 per ADR-013): phase carpet, partial strips, gravity readouts, mod-matrix UI — the full §5.6 thesis, same prototype design language.
- **Per-parameter custom response-curve editor (human idea, 2026-07-21).** Right-click any param → a Serum-2-style draggable curve editor that remaps its knob→value response. RATIONALE (why essential HERE, not overkill): this instrument drives a chaotic dynamical system, so params have narrow, nonlinear sweet spots (the perceptually-alive range is rarely the linear middle — cf. inertia steep-after-0 ADR-059, K cloud→order edge ~0.6-0.8); and the XY performance SWEEPS params, so a param's curve shapes the performance trajectory through the chaos, not just its resting value. GENERALIZES the tune-then-lock tapers (ADR-059 inertia curve is a one-param special case) into one user-editable mechanism that RETIRES the hardcoded per-param tapers (inertia sqrt, dissolve/attack log). ARCHITECTURE (parity-safe): the remap already lives shell-side in applyParam; store a small curve (control points/spline) per param, default = identity/current-taper (bit-inert → goldens never see it → core untouched); apply in applyParam; persist per-param in the preset (curves are part of the sound). COST is the editor UI (webview canvas + right-click context menu), not the plumbing. SYNERGY: Serum puts curves on mod CONNECTIONS; here per-PARAM curves are the foundation and per-mod-routing curves fall out of the same editor. SEQUENCING: after the swarmalator + mod-matrix foundations — it's the layer that makes both direct params and mod routings performable.
- MPE: pressure→K, slide→detune, per-note routing. Mod matrix with R and σ as sources. K envelopes/macros.
- **Movement / arp layer (human long-horizon, 2026-07-22).** A generative movement engine that random-walks scales or chords across the swarm's parallel voices, in two variants: (a) **scale-quantized** — the walk snaps to a chosen key + scale (ties to the Phase-2 scale-quantization forward note); (b) **relative-to-played-note** — the walk moves in scale degrees / intervals around the held note, key-agnostic. Each is effectively a per-voice or per-cluster pitch sequencer feeding the detune / harmonic law. Deterministic per the core invariant (seeded walk, no wall-clock). Opens direct **Tonality** integration (Tonality is public — named directly): scales, keys, and voice-leading supplied by Tonality. Prototype-first in an HTML lab; sequenced with the mod matrix (the walk is itself a mod source) and scale quantization.
- **Forward — consolidation review (human, 2026-07-24).** As general systems arrive, dedicated mechanisms they subsume should be RETIRED into them rather than accreted alongside: the canonical example is **onset lock / dissolve**, which may reduce to straightforward envelope modulation of K once the mod matrix ships (an envelope → K route with attack/decay IS the onset-lock gesture, generalized and routable). Same lens applies to the master HPF (already ruled: superseded by the E3 filter module), comp/limiter (ruled 2026-07-24: FX-rack slot, never a core param), and any future one-off that a mod route or rack slot could express. Schedule an explicit consolidation pass whenever a general system lands (mod matrix, FX rack completion, arp/movement layer), BEFORE the CLAP surface freezes the dedicated params at 1.0 — param ids are append-only, so a mechanism shipped as a dedicated param must be deprecated-in-place forever; a mechanism consolidated before exposure costs nothing. Reduce, never invent — applied to the parameter surface.
- **Mod-matrix polarity — unipolar vs bipolar (human note, 2026-07-24).** A first-class design axis for the real matrix, not an afterthought: sources and destinations each have a NATURAL polarity, and a route that ignores the mismatch is either unusable or silently wrong. Bipolar sources (LFOs, the rotor's shaped `lfo[]`, ±1) swing both ways around a base; unipolar sources (envelopes, velocity, R) only rise from 0. Destinations differ too: `K` is genuinely bipolar (sync ↔ splay through zero), while a coupling BOOST, level, or detune-depth is unipolar-by-meaning (negative is either clamped away or means something else entirely). Decisions the matrix owes: (a) does each SOURCE declare its polarity, or does each ROUTE carry a uni/bi selector (the flexible answer — the same envelope usefully drives a unipolar boost and a bipolar pitch offset); (b) how a bipolar source reaches a unipolar destination — offset-and-scale (`(x+1)/2`, keeps full range, adds a DC floor), rectify (`max(0,x)`, halves duty), or clamp (loses the bottom half); (c) how a unipolar source reaches a bipolar destination — at what point in its range does it cross zero (this is the "attenuverter with offset" that classic matrices expose as depth + bias); (d) whether negative depth means INVERT (the usual reading) and whether it composes sensibly with a unipolar source. **Existing instance, already live in the mod lab:** the rotor's `R` source is currently mapped `R*2−1` (`mod-lab.html`) to force a bipolar reading of an inherently unipolar quantity — which quietly changes what depth 0.5 means and injects a DC offset at R=0. That hack IS the ambiguity this note is about; it stays as a marker until the polarity model is decided, then becomes a route setting rather than a hard-coded map. Sequence: decide with the matrix UX in the modulation lab (campaign 2.2), before any param ids are frozen (append-only — a polarity model retrofitted after 1.0 is a compatibility problem, per the consolidation-review note).
- **Mod matrix design (human request, 2026-07-18). Kuramoto LFO design ACCEPTED (ADR-053, 2026-07-20; brief `docs/proposals/2026-07-20-kuramoto-lfo.md`).** The signature mod source, distinct from N independent LFOs: a swarm of phase oscillators where every routed parameter becomes a member of one coupled population, syncing/desyncing under a bipolar pull-K (K>0 locks to unison motion, K<0 splays to even interleave — past free, unreachable by unipolar competitors like Foxfire). **Accepted design (ADR-053):** ship it as a **routable modulation primitive** (published to the mod bus), NOT a hardwired chorus — the chorus is one demo destination. **Ship the rotor first** (4 phase-coupled LFOs → morph/cutoff/chorus/saturation, bipolar K, shape selector, rotor viz), then add **rate → depth → destination** axes behind it, one at a time, each a routable extension with its own Layer-0 rows. **Coupling domain is axis-dependent** (grounded in the prototype code): the rotor is PHASE-domain (reuses SwarmCore's law, not force_core); the rate/depth/dest axes are POSITION-domain springs (force_core's domain) — and the audible spine is always phase (rate coupling alone is inaudible; phase must ride with it). This is the GENERATOR side of the mod matrix; ADR-052 Phase A (audio-swarm observables) is the emergent-source side — same bus. **PORT GATE:** the four attached prototypes are NON-GOLDEN concept tests (human's word overrides the packet's "parity oracle" claim); the rotor must first be hardened into a golden reference (headless core + measured anchors + the multi-LFO-cycle mod-test rule, §5c) then ingested per ADR-003/ADR-052 prototype-first. Stand by for that golden drop — do NOT port the concept tests. Matrix scaffolding (sources × destinations, depth, curve) is standard; destinations are the existing frozen param ids; a source only needs a per-block scalar.
- **Velocity routing (human request, 2026-07-18):** the synth is currently velocity-insensitive (deliberate through Phases 1–4 — velocity would have muddied the parity contract). Add velocity as a first-class mod SOURCE in the matrix, routable to at least amplitude, K, and onset-lock, with per-destination depth and a global on/off (default off preserves the current velocity-flat behavior and every golden). Note the wrapper interaction: ADR-039 remaps NOTE_ON velocity ≤ 0 to note-off, so the live velocity value feeds the mod matrix only for velocity > 0.
- **Daido-pole center-of-gravity slider (human request, 2026-07-20).** In mean-field mode (topo 0) with poles q > 1, the coupling currently pulls every voice toward the q-th order parameter uniformly (`couple[i] = KsmS·R_q·sin(psi_q − q·θᵢ − α)`, swarm_core.h), so the q clusters populate evenly. Add one slider that shifts the center of gravity from **even distribution** (all q clusters equal) toward **weighted onto one pole** (one dominant cluster). This is the ADR-051 cluster-balance idea generalized from 2 clusters to q, and it is naturally a *coupling-function* mix under ADR-045 Γ: pure Daido-q is `{(K_q,0)}`; biasing toward one pole blends in a 1st-order term `{(K_1,φ),(K_q,0)}` that pulls toward the mean-phase cluster. Superset-with-inert-default (slider 0 → current q-even behavior bit-exact; the goldens are the proof). Gate: new golden on the biased path + an L0 trajectory anchor (dominant-cluster R rises, others fall, with a measured split). **Prototype-first per ADR-003** — like cluster-balance, this wants a swarmdynamics-style clone to measure the biased coupling against before the C++ port. Relates to PARKED #16 (per-cluster controls) and is a clean point in the ADR-045 (Γ) space.
- **Emergent mod sources — Phase A of the entangled-mods proposal (ADR-052; `docs/proposals/2026-07-19-kuramoto-entangled-mods.md`).** The SOURCE layer of the mod matrix above: publish the swarm's own observables as a smoothed, phase-unwrapped mod-source bus — `R`, `ψ`, `drift` (dψ/dt in the co-rotating frame), `direction` (sign(drift) w/ hysteresis), `R₂`, per-voice `lock_ratio`, and `slip` events (θᵢ−ψ crossing ±2π). ~70% of these are already computed each control tick (R, ψ, RN, RQ, RA, RB) as viz readouts — the new work is the bus (unwrap, per-source one-pole smoothing, the slip-event bus) on the existing 2756 Hz `controlTick` (no new rate; §8.3 self-answered). Rides existing prototypes (swarmsaw/dynamics), so it needs no new HTML lab; gated prototype-first only where a source's definition is new. Sequenced with the Kuramoto LFO (this is its source side). Acceptance: deterministic replay bit-identical; K=0 → R~N^(−½) band; K≫K_c → R→1, slip→0; slip-rate curve peaks near K_c (matches the squareness experiment's K≈0.7 metastable dip); visual R(t)/drift(t)/slip-raster trace.
- **Entangled-structured coupling — Phases B/C/D (ADR-052; same proposal, forward/parked).** B: coherence-budget coupling (a conserved [0,2] budget makes a second bank's *effective K* anticorrelate with G₁'s R) — this is the CROSS-COUPLED variant of OSC2/OSC3 above (swarm-of-swarms, PARKED #5); §8.1 leaves G₂'s identity (second bank vs a modal bank, possibly cross-project) as the human's call. C: membership spinors (per-voice (a,b)∈ℂ², equal-power two-path render, phase carried over; tunneling → Pareto-blinking → Rabi). D: measurement bus (events collapse spinors by Born rule, then relax back). C/D are net-new with no precedent → prototype-first per ADR-052 (an HTML lab for the audible membership-render choice before any port). Each phase ratified and sized individually when reached; ~6-7 gated phases total, behind Phase A + the Kuramoto LFO.
- Presets with full provenance metadata; deterministic recall test added to L0-13.
- Layer-E full pass; naming decision; demo patches (including the validated recipes: shimmer-K, zipper, erasure, gravity-settle, broken-symmetry pad).
- **Gate:** release candidate.

## Session feedback — 2026-07-27 (reverb · delays · note-off report)

- **REVERB LAB BUILT 2026-07-28 — `docs/design/reverb-lab.html`.** A full chain rather than a bare FDN, because each stage answers a different part of "sounds like a room": pre-delay · **early reflections** (12 panned taps over ~80 ms, with their own send) · diffusion allpasses · 8-line FDN with Householder mixing, per-line damping and low cut · line-length modulation. **The ER stage is a first-class hypothesis under test**, not decoration: the roadmap's guess is that the "acoustic strings section" quality came from the early pattern rather than tail length, and ER send → 0 is the A/B that decides it. **The swarm question is built in as a control:** the eight line-modulators are Kuramoto-coupled, so `coupling K` sweeps from eight independent drifts (the conventional answer) to one coherent breath (the instrument's own idiom applied to its room) — the human decides by ear which is right. **MEASURED, and the measurement did real work.** Per L0016 the RT60 estimator was CALIBRATED first on known exponentials (0.1–0.2 % error) — and it then exposed two genuine defects: the decay knob undershot badly (6.0 s set → 3.48 s at 1 kHz) and **size changed the decay** (1.47/1.69/1.94 s at 250 Hz for size 0.1/0.5/1.0), violating the design's own claim. Band-limiting confirmed these were real rather than instrument error. Root cause for BOTH: the damping LP and low-cut HP sit inside the feedback loop and lose a little per pass, and longer lines mean fewer passes per second, hence less accumulated loss. Fixed by dividing the loop gain by the filters' magnitude at a 1 kHz reference. **One instructive error on the way:** the first compensation computed the highpass magnitude as `|1 − |H_lp||` instead of `|1 − H_lp|` — `H_lp` is complex at 1 kHz, so the loss was over-estimated (0.881 vs 0.982), the compensation hit its safety clamp, and the decay ran **2.4× LONG**. AFTER: decay 2.2 s → 1.98 s and 6.0 s → 5.62 s at 1 kHz (90–94 %), HF decaying faster than LF as a real room does, size spread cut from 32 % to 8 %. REMAINING for the port: A/B against the human's Ableton reverb (the roadmap's definition of "robust"), the ER-hypothesis ear-check, the coupling-K decision, then the E3 rack slot.

**INTEGRATED REVERB — human priority (2026-07-27).** "A robust reverb in addition to integrating the kuro delays… I've been able to make this synth sound like an acoustic strings section [with Ableton reverbs] and I see an integrated reverb as a necessary step toward making this a properly impressive instrument." That observation is a finding in itself: the swarm's detuned-ensemble character reads as *acoustic strings* once a real space is around it, which is the strongest argument yet that the reverb is not a garnish but part of the instrument's identity. **Scope:** a genuinely good algorithmic reverb, not a token FDN — the `time_core.h` FDN room already exists (ADR-049/050) and is the starting point, but "robust" means it must survive A/B against the Ableton reverb the human is currently reaching for. Belongs in the **E3 FX rack as a slot** (ADR-071 precedent: real cores as slot types), with the **Kuramoto-modulated delays** landing alongside — the tap delay is already ported and the Kuro chorus/phaser proved the rotor-as-modulator pattern, so "kuro delay" = tap delay with per-tap times steered by a swarm, the same construction. **Open design questions:** does the reverb get its own swarm (per the mod-lab per-effect-swarm precedent) so its modulation is coherent with the instrument, or stay a static space? Pre-delay/size/damping/diffusion surface. Whether the strings-section quality wants an *early-reflection* stage specifically rather than a longer tail. Prototype-first per ADR-003 — a reverb lab, or an increment on the effects labs.

- **NOTE-OFF REPORT — INVESTIGATED 2026-07-27, our side measured CLEAN; most likely cause is upstream.** Human report: notes stick "longer than they ought to" when playing fast **from the computer keyboard**, never from piano-roll MIDI. Investigation (headless probe driving the REAL CLAP plugin, `stuck_probe`, scratch): 3539 note events, up to **14 keys held simultaneously against 8-voice polyphony** (so voice stealing is exercised hard), same-key retriggers inside the release tail, then all keys released — **every gate cleared and the instrument decayed to exact silence** (rms 0.0000 by t = 2 s). So the shell + core do not hang notes when the note-offs actually arrive. THREE candidate explanations remain, in order of fit:
  1. **Dropped key-up events (keyboard ghosting / N-key rollover)** — best fit for *intermittent*, *computer-keyboard-only*, *fast-playing-only*. If the OS never delivers the keyup, Ableton never sends note-off and the plugin is innocent by construction. **Decisive 30-second test for the human:** record the computer-keyboard performance into a MIDI clip and inspect it in the piano roll — if a stuck note is *long in the recorded clip*, the host never received the key release and the problem is upstream of the plugin entirely.
  2. **The release tail is a slow exponential and reads as "stuck."** Default release is 0.16 s, but it is a one-pole: −8.7 dB at 0.16 s, −27 dB at 0.5 s, and not truly inaudible until ≈1.5 s (measured: rms 0.034 at 0.5 s, 0.0041 at 1.0 s). Playing fast piles 8 voices' tails on top of each other, which smears in a way spaced piano-roll notes never do. This is real and ours, but it is *consistent* rather than intermittent — so it may be a contributing factor rather than the reported bug. **Candidate fix if it is the culprit:** a faster final segment (or a release curve that terminates rather than asymptotes), which is an envelope change and needs its own decision since it touches the reference-exact AR path (ADR-021).
  3. A genuine shell bug the probe's event pattern does not reach — held open, but it is now the *least* supported of the three, and any further work here should start from the human's recorded-MIDI test rather than from more speculative fuzzing.
  **Owed regardless:** the probe pattern (overlap past polyphony + same-key retrigger inside the tail) is stronger than what `notefuzz_check` currently generates and should be folded into it as a permanent gate.

## Orchestral-space research (2026-07-29) — what a hall has that an FDN cannot

105-agent narrow re-run (task #27), after the broad 2026-07-28 pass returned ZERO on this angle. **19 of 105 agents FAILED** (connection-closed mid-response; agents stalling through all 6 retries) — the run completed on the surviving 86, so treat coverage as partial. Six findings survived; the field is also unusually paywalled (29 × HTTP 403 on acoustics journals), which bounds what any pass can reach.

**STRUCTURAL GAP, MEASURED (high confidence).** A real hall decouples EDT from T30, position-dependently; **our FDN forces EDT = T30 everywhere by construction.** In the Northern Alberta Jubilee Auditorium T30 sits flat at ~1.65 s from 10 m to 47 m while **EDT falls 2.4 s → 0.6 s (~4×)** over the same span. Hall-dependent: Boston Symphony Hall is near-flat (2.25 → 2.4 s) *because* it is reverberant and diffuse; Salzburg 2.1 → 1.7 s. Even for ±30 cm source/receiver moves, T30 varies 0.06 s vs EDT 0.15 s — EDT is ~2.5× more position-sensitive at centimetre scale. **Actionable: an EDT/T30 ratio control** (early-decay shaping distinct from tail RT60) targets a real measured hall property our reverb currently cannot express. [Bradley, NRCC-46097]

**TWO PERCEPTUAL AXES, NOT ONE (medium).** Apparent source width and envelopment are driven by physically different cues and are therefore **independently controllable**: sub-50 ms lateral energy contaminates the onset ITD and widens the SOURCE (reads frontal); **>50 ms** — and for clear note endings **>160 ms** — spatially diffuse decorrelated energy produces ENVELOPMENT. [Griesinger] **Actionable and cheap: treat the ER stage and the late tail as two independent controls with independent decorrelation, rather than one "space" amount.** This is a routing change, not new DSP — the strongest-supported item in the whole pass.

**MIXING TIME IS DESIGNABLE (high).** An FDN's echo density follows a polynomial in time whose coefficients derive in closed form from the delay-line lengths; mixing time can be predicted from it and the design **inverted** so a target mixing time yields the required mean delay length. So our 8 line lengths are currently a guess where they could be a specification. [Schlecht & Habets, TASLP 2017]

**LATERAL ENERGY — direction supported, magnitude not (medium).** Identical anechoic Beethoven convolved with SRIRs from six European halls produced measurably different **physiological** arousal (skin conductance), strongest in halls with high low/mid strength and lateral energy. Proposed mechanism is binaural-spectral: fortissimo playing puts >15 dB more energy above 2 kHz, and binaural hearing adds 1–5 dB at 2–10 kHz for lateral vs median incidence, so lateral energy *enlarges perceived dynamic range*; shoebox halls measured ~2 dB more of it. Direction supported; mechanism hypothetical and effect size unverified. [Pätynen & Lokki, JASA 2016]

**NOT ESTABLISHED, despite direct attempts** — recorded so nobody assumes these were answered: numeric ISO 3382 target ranges (LF, IACC, C80, G) for good halls; whether the **80 ms window** is a perceptual threshold rather than a definitional convention; and **whether per-section source placement beats a single wide source** — which was the specific question about spreading an orchestra across a stage. That last one stays genuinely open.

**BUILT 2026-07-29 (task #28).** All three items are in the reverb lab. **(1) Split axes**: `erSend` (source width) and `envelop` (envelopment) are now independent, plus `tailDecorr` — at 0 the tail is mono and cannot envelop however loud it is, which is the point the research makes about diffuseness being required rather than level. **(2) EDT/T30 — and the two items turned out to be the SAME LEVER**, which is also why halls behave this way: EDT is the first 10 dB (dominated by early energy) while T30 is the later slope (the tail), so the ER/tail balance IS the early-decay control. Measured, with the estimator calibrated on a pure exponential first (EDT/T30 = 1.00 exactly, as it must be): ER 0 → **0.96** (an FDN alone has EDT ≈ T30, as predicted); ER 1.0 → **0.52**; ER 1.5 with envelop 0.5 → **0.13**. That spans the measured hall range (NAJA far seats ≈ 0.36; Boston ≈ 1.0). **(3) Mixing time specified**: the 8 delay lengths now derive from a target. First implementation was wrong — I used t ∝ m, but FDN echo count is the lattice-point estimate t^(N−1)/((N−1)!·Πd), so **t ∝ m^(N/(N−1))**; the linear version collapsed every short target onto the clamp, which is how the error surfaced. Fixed version round-trips exactly (25→25, 45→45, 180→180 ms). Honest note recorded in-lab: the absolute anchor (stock lengths declared to mix at 90 ms) is a design choice, not a measurement, so the control is relative — and the underlying closed form is Schlecht & Habets, which is paywalled, so this is the standard approximation rather than their exact expression.

**CONSEQUENCE FOR THE CONVOLUTION QUESTION (human, 2026-07-28: "convolution obviously opens up a whole new terrain").** Partly borne out: a measured IR carries position-dependent EDT/T30 structure and real directional information that a single-RT60 FDN cannot synthesise. But the two highest-value fixes — split ER/tail perceptual axes, and a specified rather than guessed mixing time — are **reachable inside the existing algorithmic chain**, no convolution required.

## Orchestral-ensemble research (2026-07-28) — what a section has that a detuned bank lacks

104-agent swarm, 3-vote adversarial verification, 11 findings. Full report: `docs/reports/2026-07-28-orchestral-ensemble-research.html`. Brief was deliberately NARROW (the 2026-07-25 pass proved a broad brief dilutes).

**THE HEADLINE VALIDATES THE ARCHITECTURE, SPECIFICALLY.** Listeners judge ensemble "togetherness" not from the VARIANCE of onset asynchrony but from its **serial micro-structure** — the lag-1 autocorrelation produced by players mutually correcting each other's timing error (Wing et al. 2014: detection threshold fell 64.3 → 18.2 ms² when structure differed; lag-1 autocorrelation 0.84 vs 0.39, p<0.0005). **A Kuramoto coupling K is formally that error-correction gain.** So ensemble timing should be generated BY the coupled dynamics at low-to-moderate K — and independent per-note jitter, which is what every conventional "humanize" does, is the WRONG mechanism. This is the strongest architecture/evidence match the project has found.

**THE COUNTER-INTUITIVE ONE: static detune should be TIGHTER, not wider.** Real unison ensembles measure **13–30 cents SD**; in controlled tests expert listeners set maximum TOLERABLE static scatter at ~14 cents SD but PREFERRED **0–5 cents**. Larger instantaneous figures (39–55 cents) are inflated by vibrato, flutter and note-transition spikes, not static offset. Caveat that matters: the stimuli already carried per-voice vibrato/flutter, so this bounds STATIC mean-F0 offset only — it is NOT evidence for a phase-locked unison. **The variation belongs in time, not in tuning** — the reverse of the usual supersaw instinct, and a likely retarget for our detune defaults.

**THE LARGEST VERIFIED GAP: onset scatter.** Professional ensembles show between-player onset SD of **24–73 ms** (49 ms string trio at 79 bpm; 24–28 ms quartets at 157 bpm — strongly tempo-dependent), against a discrimination threshold of **~8 ms**. Our swarm has **exactly zero** — every voice of a note starts on the same sample, which by this evidence is reliably distinguishable from an ensemble.

**ATTACK IS CHAOTIC, AND THAT IS PHYSICAL.** Bowed attacks occupy a narrow wedge in the bow-force/acceleration plane with categorically different failure modes either side (over-force = raucous/scratchy; under-force = loose multiple-slipping). Decisively: **machine-bowed repeats under nominally identical conditions produce different transients** — sensitive dependence on initial conditions. So per-note attack-CHARACTER randomisation along a bidirectional scratchy↔loose axis is physically grounded, and is a different thing from attack-TIME jitter.

**TWO DIMENSIONS ORTHOGONAL TO PITCH.** (a) **Spectral smear** — dispersion of formants 3–5 was manipulated as an INDEPENDENT dimension from detune; our windowed-carrier VOSIM/FOF axis with absolute-Hz lock is its faithful analogue (per-voice tone tilt is a cruder proxy, not an equivalent). (b) **Vibrato is NOT reliably uncorrelated** — a measured 16-singer choir showed partial vibrato SYNCHRONISATION via shared note onsets acting as a common phase reset, contradicting the standard uncorrelated-modulation assumption behind chorus models. Our note-on phase scatter already IS such a phase-reset mechanism. (Preliminary; choir not strings.)

**HONEST GAPS — two of five questions returned NOTHING.** The mechanism question (which of incoherent summation / roughness / spectral smearing / spatial decorrelation dominates) had its leading candidate voted down 0–3. The entire **space & seating** angle produced **zero** surviving claims: no orchestral impulse-response, seating-spread, per-section-placement or stage-vs-hall result. So the report says nothing about what an orchestral IR has that an FDN lacks — precisely where the human's convolution instinct sits. **Owed: a narrow third pass on orchestral space.**

**ENSEMBLE LAB BUILT 2026-07-29 — `docs/design/ensemble-lab.html` (task #25).** Implements the Vorberg/Wing linear phase-correction model per voice: `off_i ← off_i − α·(off_i − mean_off) + motorNoise_i` — every voice hears the ensemble and corrects toward it by gain α. **VALIDATED, with the estimator calibrated first per L0016** (i.i.d. → lag-1 −0.010, random walk → 0.999, AR(1) φ=0.5 → 0.513, φ=0.8 → 0.811 — so the measurement is trustworthy before it is used):

| α | onset SD | lag-1 | reading |
|---|---|---|---|
| 0 | 427 ms | 0.983 | no correction — drifts without bound |
| 0.25 | **39.8 ms** | **0.701** | the near-optimal quartet gain — lands INSIDE the measured 24–73 ms band with strong serial structure |
| 0.50 | 30.3 ms | 0.456 | correlated, ensemble-like |
| 1.00 | 26.2 ms | 0.010 | i.i.d. — **this is what a humanize control gives you** |
| 1.50 | 30.3 ms | −0.437 | over-corrected, alternating early/late |

**THE DEMONSTRATION THAT MATTERS:** α 0.5 and α 1.0 produce *comparable variance* (30.3 vs 26.2 ms) but *opposite serial structure* (0.456 vs 0.010). That is precisely the distinction the research says listeners respond to — and it is unreachable by any per-note random draw, however well tuned. At the literature's own optimal gain the model lands in the measured ensemble band without that being fitted. Also in the lab: detune restated as a **gaussian σ in cents** (research target 5–15) rather than a supersaw-style spread, and per-voice attack-time scatter (chaotic-attack finding). **SMEAR + ATTACK CHARACTER ADDED 2026-07-29 (completing task #26).** Per-voice **spectral smear** — each voice gets its own two-slope phase warp, so its formant peak sits at its own frequency. Verified as a genuinely INDEPENDENT axis: f0 reads 110.36 Hz at warp d = 0.50/0.30/0.15 alike (smear does not detune), while at smear 1.0 the seven voices' formants disperse to 990/330/220/220/220/550/990 Hz. Honest limit recorded in the lab: at low smear most voices sit near the identity warp, their envelope is nearly flat, and the peak estimator has no formant to find — so the low-smear dispersion figure is an artifact, not a measurement. **Attack character** is bidirectional per the physics (over-force → scratchy/raucous, under-force → loose multiple-slipping with longer settling), with per-voice spread, since machine-bowed repeats under identical conditions differ. REMAINING: ear-check; then whether the timing layer folds into the core as its own coupled system (it is NOT the existing audio-rate K — that distinction is recorded in the lab, and claiming otherwise would overclaim).

**FEASIBILITY:** nothing verified is structurally out of reach for a coupled-oscillator design — the gaps are evidentiary, not architectural. Already reachable: static scatter (retarget to 5–15 cents gaussian), slow drift, **serially-correlated timing via K**, note-on phase scatter, spatial fan. Add a swarm parameter: **per-voice onset-time scatter** (the big one, tempo-scaled, drawn from a coupled process), per-voice attack character, per-voice formant offset. FX rack: a resonance field to convert per-voice FM into AM+timbre — but ONE shared body correlates AM across all voices unlike N real instruments, so per-voice resonance offsets are likely preferable.

### Spectral smear — RETIRED 2026-07-29 (queue item #29 replaces it)

Finding 10's per-voice formant dispersion was built, widened once, and **removed** after
three independent human reports of total inaudibility. The removal is evidenced, not a
concession — two measured reasons it could never work as built:

1. **The two-slope phase warp is spectrally sign-blind.** `warpD = +1` and `warpD = −1`
   produce *identical* magnitude spectra, because the map at `+d` is the time reversal of
   the map at `−d`, and a magnitude spectrum is invariant under time reversal (only the
   phase flips). A dispersion symmetric about zero therefore folds onto `|warpD|`: half
   the intended spread collapses onto the other half, so voices meant to differ are equal.
2. **The warp produces a tilt, not a peak.** At full depth it moves harmonics by 2.73 dB
   rms, monotonically (h1 −5.5 dB rising to h16 +0.9 dB). A gentle high-shelf, dispersed
   across seven voices already beating against each other, is indistinguishable from
   nothing.

The finding itself is not refuted — only this realisation of it. **Queue #29:** re-introduce
it as *scatter on the shape lab's formant control*, whose windowed-carrier axis is confirmed
audible ("I am certainly hearing some formant qualities", 2026-07-26), once those axes fold
into the engine — exactly as `charScatter` scatters `character`. Do not rebuild a standalone
mechanism in the ensemble lab.

**This is the consolidation principle in action** (human, 2026-07-23: certain dedicated
mechanisms should be replaced by general ones as those land). A weak bespoke mechanism
duplicating a strong general one is the case for retiring the bespoke one, not for tuning it.

## Timbre-space research — supersaw discourse & mechanisms (2026-07-25)

103-agent research swarm (5 angles → source fetch → 3-vote adversarial verification); 9 findings survived. Full report: `docs/reports/2026-07-25-supersaw-timbre-research.html`. Brief was: widen the reachable space toward (A) metallic/glassy hyperpop and (B) organic low growls, without breaking the saw mandate.

**HONEST LIMIT FIRST — half the brief came back empty.** Direction (B) produced **NO surviving claims**: reese phase cancellation, growl/talking bass, unison beating rates, filter self-oscillation, wavefolding and low-end FM/PM were unsourced or refuted. Their feasibility is **UNDETERMINED, not assessed**. The competitive-landscape angle (Serum/Vital/Phase Plant/Massive/Falcon/Pigments/Hive) also returned nothing verifiable — that space is **unsurveyed, not clear**. A second, narrower research pass is owed on both.

**What survived, and what it changes:**
- **The JP-8000 origin story changed under us.** The SuperSaw was read off the silicon (39C3, Dec 2025; bit-accurate `JE-8086` emulator) and the finding is **deflationary**: seven NAIVE sawtooths + a high-pass filter at an 88.2 kHz internal rate — no phase tricks, no chorus, no modulation. There is no lost Roland secret to recover, so every metallic reach must come from our own additions. (Caveats: emulator needs a user ROM dump — weak as a CI oracle; bit-accuracy is developer-asserted, no published third-party null test.) **Szabo 2010 survives as the DETUNE-LAW reference** (non-linear 11th-order curve, asymmetric ratios, centre oscillator unmoved) — which is what we already implement as the JP-8000 distribution.
- **Direct academic support for the project thesis:** peer-reviewed work states that static spectral richness is NOT sufficient for a convincing supersaw — the missing ingredient is **time-varying timbral variation** from the detuned bank. In HYPERSAW the Kuramoto coupling term *is* that modulator. **Worth citing in SPEC** (protected path — human gate).
- **TWO CONCRETE CANDIDATES, both category (ii) — add a parameter to the saw swarm, mandate intact:**
  1. **Hard sync as a pure phase operation** — `y = 2·[a₁·x mod 1] − 1`, `a₁ = f_slave/f_master`, operating on the normalised modulo-1 phase the swarm ALREADY maintains, and yielding a hard-synced *sawtooth* slave specifically. Becomes a per-voice sync-ratio parameter the coupling can modulate. Cost: alias-prone; the source's own remedy is polyBLEP, which the saw oscillator already has (SMC 2010 §3.3).
  2. **Formants without a filter** — three published mechanisms (variable-slope phaseshaping; Vector Phaseshaping with formant centre `f_f/f₀ = 2v−1`; phase-synchronous ModFM). Gets vowel/"talking" character INSIDE the oscillator. **Two design-shaping caveats from the papers:** aliasing where `2v−1` is non-integral (the published fix crossfades TWO oscillators, so "single oscillator" holds for a static formant, not an alias-free glide); and **bandwidth is coupled to position**, unlike a filter's independent fc/Q — so a formant control here must not be labelled or shaped like a filter.
- **SAW-SHAPE RETARGET UNBLOCKED (task #17's last item).** A ripple / phase-shape axis IS a family of subtle sawtooth variants — exactly what was asked for when ADR-058's saw↔square morph was rejected. It no longer waits on measured synth-saw captures.
- **Third engine: case NOT made.** Best candidate premise is the **Janus oscillator network** (ring of nodes, each holding two internally coupled phase oscillators with equal-magnitude OPPOSITE-SIGN natural frequencies; β internal, σ external). It satisfies the mandate structurally — Kuramoto is a *reduction* of it — but the claim that it yields wider dynamical regimes was **REFUTED**, so the timbral payoff is unevidenced. **Recommendation: exhaust category (ii) first**; if hard sync + formants still leave the glassy/growl space unreachable, that failure is the actual evidence for opening (iv).
- **Prior art is emptier than expected.** The nearest published coupled-oscillator synthesis work (Kuroscillator) is phase-coupled SINE waves capped at 0.25–30 Hz — no sawtooth, no supersaw, no metallic/growl mechanism. Prior art for coupled-oscillator additive/rhythmic synthesis, NOT for a coupled detuned-saw swarm.
- **One verified artist datum:** SOPHIE's own statement — everything but vocals synthesized from raw waveforms, samples explicitly rejected, "metal" named as a synthesized target on a Monomachine. The metallic palette is documented as **synthesis-primitive-reachable**, not sample-derived. All other AG Cook / PC Music process claims the swarm surfaced were folklore and did not survive verification.

**PARKED with explicit re-open criteria — third engine (Janus).** The Janus oscillator network (ring of nodes, each holding two internally coupled phase oscillators with equal-magnitude OPPOSITE-SIGN natural frequencies; β internal, σ external) is the best-attested premise that satisfies the mandate structurally — Kuramoto is a *reduction* of it, so it extends rather than replaces. But its wider-dynamical-regimes claim was **REFUTED** under verification, so there is no evidenced timbral payoff. **Re-open condition, stated now so the decision is not made by drift:** build the sync + formant bench first; if the metallic/glassy target is still unreachable with those axes exhausted, that documented failure IS the evidence for opening (iv) — and it should be opened with a specific unreachable sound as its justification, never as a speculative engine.

**PROTECTED-PATH ITEM — SPEC citation (human gate).** The research surfaced direct academic support for the project thesis: static spectral richness is not sufficient for a convincing supersaw; the missing ingredient is time-varying timbral variation from the detuned bank — which in HYPERSAW is supplied by the coupling term. SPEC.md is protected, so this is recorded as a proposed amendment awaiting approval, not applied.

**Sequencing:** the two category-(ii) candidates are prototype-first per ADR-003 — they belong in a lab (a saw-shape/sync/formant bench, or an increment on the detune lab) before any reference or core change. Neither needs a new engine, and both target direction (A) directly; direction (B) stays open pending the second research pass.

## Lab campaign 2 — five fresh labs (human direction, 2026-07-24)

The detune-lab campaign (ADR-060..070: audition lab → reviewed fold map → reference-first folds, parity 54→141) is the TEMPLATE; these five run the same loop on the next tier of the instrument. Each is a fresh single-file HTML lab (spec-in-code, ADR-003) whose job is to IRON OUT BEHAVIOR before anything touches a protected reference or the core — labs are audition instruments, not references; winners fold with their own map, ADRs, and goldens. Sequencing is the human's call per lab; they can run in any order and in parallel.

1. **Multi-oscillator interface + initial quantum-interference concept.** A lab that makes the OSC2/OSC3 design questions (Phase-5 entry above: per-osc surface, independent-vs-cross-coupled, mixing/routing, CPU envelope) AUDIBLE — multiple swarm banks stacked/detuned/cross-fed with a top-level interface sketch. Second deliverable: an initial **concept PROPOSAL for quantum interference between banks** (how superposed banks interfere rather than merely sum — nearest existing precedent is ADR-052 Phase C's membership spinors / equal-power two-path render; whether this is that, a relative of it, or new is exactly what the proposal must pin down). Concept doc + demo first; no engine work until ratified.

2. **Modulation lab — Kuro LFO + traditional LFOs/envelopes + mod matrix (+ novelties).** The ADR-053 rotor's port gate requires hardening to a GOLDEN reference — this lab is where that happens (headless core + measured anchors + the multi-LFO-cycle mod-test rule). Alongside it: conventional LFOs and envelopes as first-class matrix sources (the bread-and-butter the signature source sits beside), the matrix routing/depth/curve UX, and room for additional modulation novelties as they surface. Also the natural venue for the CONSOLIDATION-REVIEW candidate already flagged: does envelope→K modulation reproduce onset lock/dissolve well enough to retire the dedicated params before CLAP freeze? **STATUS (2026-07-24): lab live (mod-lab.html); A5 splay RESOLVED (rank lattice + rate entrainment — gap 0.250, R 0.000, sd 0); per-voice Kn shapes added; consolidation A/B measured NEAR-EXACT (rms 0.002 vs peak 5.12) — but only via a COUPLING-DOMAIN destination (Kboost): the clamped K knob cannot reach onset's strength (km max 4 vs onset 0.8's 5.12), which is itself the review's first finding. FINDING #2 (human-heard, then measured): onset lock is PER-NOTE; a global env re-surges every sounding note on each key (staggered test: note1 jumps 2.10→4.93 when note2 arrives, rms 1.274 vs onset lock) while PER-NOTE env instances match at rms 0.002 on both notes — the matrix must distinguish per-note sources (envelopes) from global ones (LFOs/rotor). With #1+#2 met the retirement case is CLOSED at lab level, pending ear-check. EAR-CHECKED AND CONFIRMED (human, 2026-07-24): splay behaves correctly at K=−1, and the staggered-chord A/B holds — the A5 golden gate is satisfied on the audition side, so the rotor is CLEAR to graduate (gen_kuramotolfo_goldens.mjs → kuramotolfo_check.cpp → ACCEPTANCE rows, the last being a protected-path gate).** **KURO CHORUS AUDITIONED (2026-07-24):** the rotor drives 4 delay taps (mono line + panned taps, Juno/Solina ensemble topology), so K is the character knob. Measured: tap spread sweeps 0.58 -> 9.00 ms (15x, monotone in K); wet-only L/R correlation 0.990 (K=+1, near-mono unison vibrato) -> 0.549 (K=-1, even-lattice decorrelation). **Finding:** correlation is NOT a smooth width fader — it is near-mono while the rotor is LOCKED and wide everywhere else, the cliff sitting between K +0.7 and +0.35 exactly where the rotor's sync transition is (R 0.990 -> 0.648); the useful width range lives below the lock threshold. Chorus depth is also wired as a matrix destination, to keep the ADR-053 point that the chorus is a *routed destination*, not a hardwired effect. Ear-check pending; a shipping version belongs in the E3 FX rack as a slot (cf. ADR-071 comb precedent), not the core. **KURO PHASER AUDITIONED (2026-07-24):** all-pass chain, stage j steered by rotor voice j mod 4, stereo from a rotated voice assignment. Measured: notch spread 0.186 -> 2.879 octaves across K; wet-only L/R correlation 0.866 -> 0.287. **It sweeps stereo more smoothly than the chorus** (whose width collapses only in the locked regime) because phaser width comes from the voice-rotation assignment rather than the lock state — a design lesson for any future rotor destination: derive stereo from WHICH voice drives a channel, not from how coherent the swarm is. Stable at max feedback (peak 1.076, NaN-clean) but hot enough to want a limiter downstream. **ROTOR → SWARM (human direction, 2026-07-24 evening):** the mod source now has the SAW oscillator's surface — variable voices (1..8), a rate-detune law with anchor (mean/slowest/fastest on the global rate), and the topology family (mean-field / ring+reach / two-cluster+μ+balance) — plus per-effect swarms for chorus and phaser with a `link` slider (independent ↔ entrained by the main swarm), and drag-editable matrix cells. **This supersedes three lines the golden spec froze (NV=4, one global shape, no seed axis), so the rotor must NOT be hardened to a golden until the axes settle** — a golden measured now churns immediately, and its ACCEPTANCE rows are a protected-path edit. **Deeper consequence, ADR-worthy at port time: the mod source and the audio engine are now the same phase-domain Kuramoto swarm at two rates — the port should reuse SwarmCore rather than grow a parallel implementation.** Measured: splay gap exactly 1/n for n∈{2,4,6,8}; free-run floor 0.631→0.360 tracking 1/√n; ring coherence monotone in reach; A/B balance splays cluster B (0.988→0.365); link taper made quadratic after a linear one locked by 0.15 and wasted the rest of the slider (ADR-059's taper lesson, recurring). **Degenerate-equilibrium trap found and documented:** detune 0 + even phases = an exact fixed point (R=0 → zero coupling force at any K); the default detune is non-zero so the rotor doesn't present as broken. **Source visualizers (human, 2026-07-24):** every source now has a shape preview in its own colour with a live playhead (K1-K4 waveforms, LFO A/B incl. S&H hold, ADSR drawn in proportional time), all colours read from ONE table shared by the rotor circle, scope, previews and matrix row headers — which fixed a real collision (ENV and K4 were both green).
3. **Quantum Morph lab.** The dropped QM materials (section below: QM-0 core spec, QM-2 integration spec, quantum-morph-lab.html demo) come under version control as a tracked lab, upgraded to QM-0's mask formulation (the demo's noise-blending coupling is superseded — QM-0 §mask keeps the marginal census exact). Iron out: census honesty at temperature extremes, salience/coupling feel, discrete-flip musicality (the half every vector synth punts on), and the QM-2 integration contract against the real param surface. Behavior ratified in the lab BEFORE any engine binding.

4. **SPECTRA expansion lab.** A swarmspectra-derived audition lab for making SPECTRA a more robust, more capable engine — the Phase-4 forward note already collects the seams: SPECTRA-native params the per-partial structure uniquely affords (per-partial coupling shaping, inharmonicity curves, cascade variants), the missing SAW routings (mono/glide/legato, MPE per-note, drift/rtone/scatter), and the swarmalator-spatial idea's SPECTRA-specific formulation (its own math — the single-θ swarmalator does not transfer). New functionality auditioned here; robustness gaps (e.g. the anti-cancellation floor, Phase-F territory) get characterized here even where the fix waits for reference-path liberation.

6. **Sync + formant bench (NEW, from the 2026-07-25 timbre research).** The two verified category-(ii) candidates, prototyped together because they are the same phase-domain family and share one aliasing budget. Deliverables, prototype-first per ADR-003: (a) **per-voice hard-sync ratio** `a₁` applied as `y = 2·[a₁·x mod 1] − 1` on the swarm's existing normalised phase — with polyBLEP on the new modulo discontinuities, and with the coupling term allowed to modulate `a₁` (the thing no other synth's sync can do, since our phase is already a coupled state); (b) an **oscillator-internal formant axis** (variable-slope phaseshaping and/or VPS `f_f/f₀ = 2v−1`), giving vowel/"talking" character with no filter in the path; (c) the **phase-shape / ripple axis** that retargets ADR-058's saw morph — same machinery, so it costs almost nothing extra here. **Design constraints carried from the papers, not to be discovered again the hard way:** aliasing appears where `2v−1` is non-integral and the published fix crossfades TWO oscillators (so "single oscillator" holds for a *static* formant, not an alias-free glide); and **formant bandwidth is coupled to position**, unlike a resonant filter's independent fc/Q — so this control must not be shaped, labelled, or visualised as a filter. Measure aliasing explicitly (the lab's honest-measurement discipline: an FFT null test against an oversampled render, not "sounds clean"). Targets direction (A) metallic/glassy directly; also the most likely route to the growl direction via moving formants, pending the (B) research pass. **BUILT 2026-07-25 — `docs/design/shape-lab.html`.** All three axes live, with the mandate argument made explicit in code: a sawtooth is *phase, read as a rising ramp, reset once per cycle*, and each axis touches a different term without replacing the ramp — sync resets it early, warp changes its rate within the cycle (two-slope CZ-style distortion through (d,v); d = v is a plain saw), ripple perturbs it with monotonicity guaranteed by a depth cap. That is how a formant axis fits under the saw mandate at all: **warp the phase, do not replace the waveform.** `syncCouple` is the differentiated control — a₁ tracks the swarm's order parameter R, so the stack hardens as it locks; sync driven by the physics rather than by an LFO, which a conventional sync oscillator cannot do because its phase is not a coupled state. **ALIASING MEASURED** (f0 1760 Hz, inter-harmonic midpoint sampling): plain saw −149.5 dB BLEP vs −48.6 naive; **integer sync −136.0 dB but FRACTIONAL sync −62.4 dB** (the master truncates the ramp mid-cycle and the partial-height correction is only approximate); steep warp −60.0 dB; ripple −129 to −142 dB; **fractional sync + steep warp COMPOUND to −51.6 dB, only ~7 dB better than naive** — confirming the roadmap's reason for one shared budget. Opt-in integer snap added (fractional ratios are the musically interesting ones, so it is not forced). Port mitigations in preference order: snap to integers · a BLAMP/multi-BLEP treatment of the truncated wrap · oversample only while fractional sync is engaged. **EAR-CHECK PASSED (human, 2026-07-28): "getting some amazing sounds out of this lab"**, formant character audible. **CONSEQUENCE — the third-engine question stays closed.** The research's recommendation was "exhaust category (ii) before opening (iv)"; category (ii) is now delivering audibly, so Janus remains parked and its re-open condition (a documented unreachable sound) is not met. **CHAIN ORDER + COUPLING VIZ (human, 2026-07-28).** The human heard that engaging sync makes the warp/ripple formant "less formanty… it multiplies the notches/peaks up and down the spectrum" — correct, and now measured. The three axes were refactored into composable stages (each a [0,1)→[0,1) map plus a rate multiplier), which makes chain order a parameter AND makes the polyBLEP fall out generically instead of being hand-derived for one order. Detrended harmonic envelopes (saw −6 dB/oct removed, f0 110 Hz, sync 2.5×, warp d.15/v.65): **sync→warp = 23.7 dB bump @ H10 with a visibly PERIODIC pattern** (the replication the human heard); **warp→sync = 8.0 dB, flat**. **This REFUTES the tidy hypothesis** that warp-first would preserve a single formant: it removes the replication but also removes most of the formant, because the following sync re-reads the warped phase and its own resets dominate. **Neither order gives a strong single formant under sync.** WHAT WOULD: a **windowed-carrier** construction (VOSIM / FOF / PAF family, and the ModFM the timbre research already surfaced) — sync'd ramp as CARRIER, a separate master-rate WINDOW setting the formant, so position is fixed by the window while sync sets brightness. That is a fourth mechanism, not a reordering, and it is **AMPLITUDE-domain rather than phase-domain** — a mandate question for the human (a windowed saw is arguably still saw-based, but it is a different class from the three phase axes). **BUILT AND TESTED 2026-07-28 (human greenlight).** Implemented in the shape lab as axis 4: a Hann grain at the MASTER rate (width = formant bandwidth) multiplying the sync'd ramp as CARRIER (its frequency = formant centre), plus **formant lock** — the carrier tuned to an absolute Hz (a₁ = fHz/f0 per voice) rather than a fixed ratio, which is what makes it a formant musically. **MEASURED** (same patch at f0 110/165/220/330 Hz, peak of the 1/3-octave spectral envelope; a fixed formant should NOT move while pitch moves 3×): ratio sync tracks pitch at **3.03× spread** (the baseline — it follows the note, as a ratio must); **formant lock at 1600 Hz holds at 1.29×**; lock at 800 Hz was erratic (3.03×) until **carrier purity** was added, improving it to **1.64×**. **Carrier purity is a measured necessity, not a preference:** a saw carrier spreads energy across its OWN harmonic series so "the formant" is not one region — FOF/VOSIM use a near-sine carrier for exactly this reason. HONEST LIMITS: the remaining spread at low formant frequencies is a mix of genuine harmonic-grid quantization (a formant in a harmonic sound can only be expressed by the harmonics that exist) and my estimator's coarseness (±12 % bands), so the ear-check is the real arbiter; and the earlier attempt with a +20log10(k) detrend reported nonsense (3190 Hz) because that tilt over-weights the top octaves — the envelope-peak method replaced it. **MANDATE:** this axis is amplitude-domain and the carrier-purity control leaves saw territory outright; the human accepted that alongside the fold mode and the re-admitted square morph.

**FORMANT LOCK — TWO BUGS FOUND AND FIXED (human report 2026-07-28: "it seems like there's something wrong with the formant lock, hard to explain").** Both were silent-failure modes, which is why they resisted description: (1) `a1` was derived from the COUPLING-PERTURBED effective frequency, so the formant jittered with the swarm's motion instead of sitting still — now derived from the voice's base frequency, since a formant is a property of the note, not of the swarm's instantaneous state; (2) `a1` clamps at 1, so whenever the formant frequency fell BELOW the voice frequency the sync disengaged entirely and the formant vanished — at 800 Hz that is every note above ~G5, fading out as you approach it. Clamping is correct physics (a formant below the fundamental cannot be expressed by any harmonic); doing it silently was the bug. Now reported and painted red in the new visualiser. Verified: f0 110 → carrier exactly 800 Hz; f0 880 → clamped and flagged. **WINDOWED-CARRIER VISUALISER added** (human ask): three stacked traces over one master cycle — grain window, the carrier running inside it, and their product — plus a live carrier-Hz readout that turns red on clamp. The mechanism is now visible rather than inferred, which is what let the second bug be explained rather than just felt. Both visualisers also now follow the NEWEST sounding voice rather than the first slot (with a drone or chord held, "first gated voice" is whatever you played earliest, not what you are listening to). **LAYOUT: clusters reflowed into COLUMNS** (fill top-to-bottom, then next column) per the human's request to see everything at once — the same pattern the plugin GUI uses.

**CLICK DIAGNOSED + PANIC ADDED (human, 2026-07-28: clicking "especially when I turn the carrier toward sine").** Root cause: with a FRACTIONAL sync ratio the carrier is truncated mid-cycle by the master reset. For a saw that step is already BLEP'd; for a SINE it is an uncorrected discontinuity — which is exactly why turning the carrier toward sine exposed it. Measured (2nd-difference outliers): integer ratio + sine = **0/s**; ratio 4.37 = **300/s**; formant lock (which always yields a fractional ratio) = 169/s. **The fix was a bug in the existing mitigation:** `snap to int` ran BEFORE the formant lock, so with lock engaged it was silently a no-op. Moving it after gives **169/s → 0/s** at the cost of quantising the formant to whole ratios of the note. Also added per-sample a₁ smoothing (4 ms, seconds-in per ADR-009) since `y = frac(a1·ph)` DERIVES the slave phase from the master — a change in a₁ teleports the phase rather than changing its rate. **PANIC button added to the lab** (releases all voices, clears held keys, cancels drone, suspends the graph) — there was no way out of a stuck note but closing the tab. **MEASUREMENT-PROTOCOL LESSON, third of its kind:** the first click detector thresholded |x[n]−x[n−1]| and reported 2700 "jumps" in a clean 1200 Hz sine — that is just its slope (~0.17/sample at 44.1 kHz), not discontinuity. A click is a SECOND-difference outlier against the robust local norm. Same failure family as the aliasing metric (L0014) and the formant detrend: **a threshold on the wrong derivative measures the signal, not the defect.**

**SAW↔SQUARE RE-ADMITTED, CAPPED AT 0.5 (human ruling, 2026-07-28).** Reverses the 2026-07-22 direction that the morph should not ship: "I like the saw-square slider but it should max out at about 50% — enough saw integrity to still count while also giving access to the pleasing hollow sound." Implemented with a hard 0.5 clamp in the DSP (not just the slider range, so a preset or automation cannot exceed it) and polyBLEP on BOTH saws of the morph, which is ADR-058's original correctness point.

**Recorded, not built — awaiting a ruling.** **RIPPLE FOLD MODE RESTORED (human, 2026-07-28):** the human reported losing a "compelling pulse wave character" with notches at high ripple depth, and explicitly accepted it leaving saw territory. Diagnosed by diffing the pre- and post-refactor phase maps across a grid: they are IDENTICAL except when warp is engaged, where the old code produced **465 fold-backs per cycle** (ripple .9 H3 + warp d.2/v.6) versus 0 after. Cause: the old code indexed the ripple by the phase entering the WARP stage while ADDING it to the warped phase — where warp compresses, the excursion exceeds the local slope and the map folds backward. That was strictly a composition bug, and also a musically valuable waveform class, so it is now an explicit **`ripple index` mode**: `shaped` (monotone, stays a saw) or `fold` (reproduces the old map EXACTLY — verified phase Δ 0.0, slope Δ ~1e-15). **Both coexist in one engine**, which was the human's actual ask. **Aliasing cost of fold: none** — −108.4 dB vs −104.1 dB shaped, because a fold leaves the phase map CONTINUOUS (slope reversals, not new discontinuities); only the cycle boundary is a step, so it yields spectral nulls without broadband fold-down. That is also why it reads notchy rather than dirty. **Consequence for the port:** the fold mode is the first axis that deliberately leaves the saw mandate, so it needs its own ADR line at fold time rather than riding in as a superset. Also added: a **coupling visualiser** (phase circle with per-voice dots, the R arm, an R history trace, and a live a₁ readout that turns amber when `← coupling` is driving it) — the swarm-driving-sync relationship made visible rather than inferred.

**NEXT: fold the three axes into swarmsaw.html + swarm_core.h with parity**, per ADR-003 and the ADR-060..070 fold discipline — each axis inert at its identity default (sync 1.0, d = v, ripple 0), so every existing golden must stay bit-identical; the aliasing numbers above become the acceptance evidence, and the fractional-sync hot spot needs its mitigation decided AT fold time rather than discovered later. **HARNESS LESSON worth keeping:** the first aliasing metric summed every non-harmonic bin and reported a clean polyBLEP saw at −27.7 dB — that was Hann-window leakage from a dozen strong harmonics accumulating across thousands of bins, not aliasing. Sampling the inter-harmonic midpoints (nearest harmonic ~650 bins away) is the protocol that measures the thing it claims to.

7. **Second research pass — direction (B) + competitive landscape (OWED).** The 2026-07-25 swarm returned NOTHING verifiable on organic low growls (reese phase cancellation, growl/talking bass, unison beating rates, filter self-oscillation, wavefolding, low-end FM/PM) or on the commercial landscape. Both are **undetermined, not cleared**. Re-run narrower: one pass on low-end mechanism DSP specifically (the broad brief diluted it), one on the named synths' actual architectures. Until then, do not assign feasibility categories to (B) techniques — the map has a hole there and should keep showing it.

5. **Novelty scratch lab.** An explicitly open exploration bench for ideas that fit none of the above — the campaign's rule is only that a novelty that graduates must exit through the same gate: concept → lab behavior ironed out → proposal/ADR → reference-first fold. Candidates already parked that could start here: scale/pitch quantization (workshop forward item), per-mode parameter limits, performance-history modulation (design-first, after the mod matrix exists).

## Quantum Morph — macro-morph layer (forward; specs dropped 2026-07-22)

A stochastic **preset-morph** engine, distinct from the mod matrix: a 2-D field with up to 4 "corner" patches, and a Gumbel-max selection law that decides — per parameter, independently — which corner currently owns it. The point is that **discrete** params (wave, filter type, routing, sync) morph by *stochastic flipping* rather than snapping at 50% or being excluded — the half that every vector synth punts on. Per-slot **salience** (how hard a param pins to the dominant corner), **module coupling** (params flip as coherent groups so you never hear osc-A-detune with osc-D-level), a temperature control (hard 4-way switch → honest proportional census → uniform), and per-slot mode overrides (frozen / pinned / quantum / gradual).

**Dropped at root (untracked):** `QM-0-core-engine-spec.md` (normative core engine), `QM-2-instrument-integration-spec.md` (integration contract), `quantum-morph-lab.html` (511-line prototype demo — note QM-0 *supersedes* its noise-BLENDING coupling with a mask formulation that keeps the marginal census exact). A QM-1 (Max-for-Live device) is referenced but not dropped here.

**Fit — strong.** QM-2 §5 names HYPERSAW the best pilot (cleanest coupling metaphor, smallest param count). §8 **polyphonic superposition** — each voice draws its own corner assignment at note-on, `Voice spread` 0→1 — maps directly onto HYPERSAW's coupled-oscillator ensemble ("the same idea one level up the hierarchy"). Its core invariants ALIGN with ours out of the box: pure-function determinism (no wall-clock, no free-running RNG), bit-identical state recall, and — the stated acceptance gate — "the instrument works with morph disabled, byte-identically to before," which is our superset-with-inert-default discipline verbatim.

**Real integration work (QM-2):** a per-parameter **manifest** carrying a `morph_class` (SAFE / VOICE_BOUND / STRUCTURAL / FORBIDDEN) and **authored salience** per param — the field no current instrument has, and the actual cost of integration. VOICE_BOUND params latch at voice allocation (running voices keep their birth patch — the "morph becomes an ensemble" payoff). STRUCTURAL params (voice count, coupling topology — exactly what a user most wants to morph here, and the most likely to glitch) need a declared deferral policy. Master gain / tuning reference / oversample factor are FORBIDDEN, non-overridable.

**Interaction with the mod matrix (why it is not redundant):** QM is a MACRO / preset layer; the Kuramoto-LFO mod matrix (Phase 5) is the continuous-modulation layer. QM's field position (x, y), temperature, coupling, and reshuffle trigger are automatable macros — and natural mod DESTINATIONS for the swarm-observable bus (ADR-052) or the movement/arp walk above. The two compose: modulate *where you stand* in the morph field.

**Open decisions (QM-2 §10 — human's to make):** host-automation conflict (is `FROZEN` the whole answer, or a takeover mode?); which STRUCTURAL params participate in v1; editing behaviour under GRADUAL; per-patch coupling overrides; pilot ordering across the catalogue. **Sequencing:** Phase 5+, after the mod-matrix foundations; **prototype-first per ADR-003** — the demo exists but QM-0 supersedes its coupling, so a golden reference must be hardened before any C++ port. **File hygiene (DONE, PR #71):** specs relocated to `docs/proposals/`, demo to `docs/design/`, and both `.gitignore`d — they name a private sibling (the terrain sibling) plus other catalogue instruments, so they stay local-only until the names are aliased for tracking (ADR-014).

- **Performance-history-influenced modulation (human long-horizon, 2026-07-22).** Modulation whose behaviour adapts to what has been played (recent pitches, gestures, density). A big choice-architecture question: what history is observed, how it is summarised into a bounded *deterministic* state (the no-wall-clock / seeded-determinism core invariant must survive), how it feeds the mod bus, and how much it should surprise vs. stay predictable. Deep **Tonality** integration territory (history → key/scale inference → modulation bias). Explicitly a design-first discussion, not a near-term build; sequence after the mod matrix + Quantum Morph foundations so there is a bus to feed and a macro layer to bias.

## Phase F — Reference-path liberation (scheduled at the Track E1 gate; ADR-041)

The prototypes were always a gesture toward the instrument, not its final form. Once E1 closes, the "correct == bit-parity with the prototype" contract graduates to forward performance standards:
- Author the successor acceptance standard (behavioral/perceptual targets + new golden references generated from the liberated implementation, versioned).
- Migrate the L0 suites off parity-to-prototype onto the new references, one engine at a time — each migration its own ADR + gate; the bit-parity harness is repointed, not deleted.
- Re-scope the protected prototype HTMLs to historical provenance.
- **Only then** do reference-path DSP modifications (e.g. the SPECTRA anti-cancellation floor, low-energy body work) proceed against the new standards rather than as guarded-inert additions.

## Prior art & positioning

Maintained in PRIOR-ART.md; revisit at Phase 3 (before gravity ships) for the freedom-to-operate check flagged there, and at Phase 5 for marketing claims accuracy.

## Track E — effects line (parallel track; ingested 2026-07-18, packet UPDATE-001)

Track E depends only on Phase 0 platform infrastructure and the control-tick scaffolding from Phase 1; it does not depend on the third oscillator engine and can proceed alongside it.

**E0 · Force-core module.** Port the shared force system (home/sync/splay/gravity/drift/inertia on log2 coordinates, per-tick) as a standalone, engine-agnostic module consumed by all four effect engines. Gate: force-core unit tests reproduce the JS labs' population trajectories (collapse σ ratios, gap CVs, equilibrium-law residuals) within L0-14/15/19/20 tolerances, seed-for-seed. **GATE CLOSE PROPOSED (2026-07-18):** src/force_core.h (labs' force system verbatim; Profile per engine; three attractor kinds) + force_check in ./verify full — 17 scenarios seed-for-seed vs Node-extracted lab cores (worst |Δv| 1.3e-14, tolerance 1e-9), population halves of L0-14/15/17/19/20 all green, drift-off measurement protocol discovered and pinned (ADR-034). SwarmCore↔effects unification scoped honestly per ADR-034 (phase-domain vs position-domain: only the RNG is genuinely shared, now delegated, parity 51/51). Merging the PR = ratification; E1 (frequency engines) unblocks.

**E1 · Frequency engines.** Resonator bank + notch swarm on the force core, external audio input. Gate: L0-14 through L0-18 green; notch-exactness regression guard (L0-16) in CI. **STATUS (2026-07-19):** E1.1 resonator bank (ADR-043) + E1.2 notch swarm (ADR-046) ported bit-exact (filter_check/notch_check, RMS 0.0; L0-14 collapse→Q + L0-16 notch nulls 158 dB in CI). E1.3 SWARM-FX audio-effect plugin (ADR-047) — external audio in, both engines selectable — validated (pluginval 10 / auval) and installed for human testing. REMAINING for the gate: L0-17 audio (tuned harmonic rejection) + L0-18 family-stability long-runs as oracle rows; SWARM-FX webview GUI. Merging closes E1 once those land + human sign-off.

**E2 · Time engines.** Tap-swarm delay (host tempo sync replaces the bpm field) + FDN room swarm. Gate: L0-19 through L0-21 green, including the LF-stability and DC-boundedness long-run checks; matrix-sign regression guard (L0-20) in CI. **STATUS (2026-07-19, ADR-049):** both ported bit-close (time_check: parity worst RMS 5.6e-12 within eps; L0-19/20/21 stability rows green, in ./verify full) and WIRED INTO SWARM-FX (engines Tap Delay / FDN Room) — validated + installed for human listening. Remaining: stereo + host-tempo + GUI for whichever survive testing.

**Effects visualization + experimentation (human direction 2026-07-19).** The effects need MORE VISUAL FEEDBACK and license to experiment with how they work — the human likes the time engines, thinks the frequency engines can become valuable with rethinking, but "we haven't landed on it yet." Priority for the SWARM-FX survivors: a webview GUI with a live visualizer (the swarm state — delay-time / notch / resonator population moving, the order parameters, gravity targets), and rapid iteration on the DSP behavior (not just parity-frozen ports — these are experimental, and under Phase F the reference-path graduation applies to the effects too). Stereo shipped (ADR-050). Host-tempo sync for rhythmic gravity is the next mechanical win.

**E3 · Integration — internal FX rack (ADR-054, human direction 2026-07-20).** Bring the effects inside the instrument as post-oscillator sections; the standalone SWARM-FX plugin then offers *all of it* over the same cores (one rack, two shells), possibly with a MIDI sidechain input for note-context engines (e.g. consonance-gravity attractors). The near-term target: a **routing GRID, not a fixed chain** — FX order is expected to be particularly significant to this instrument — built **placeholder-first** (rack + routing + mod-destination wiring against trivial FX to get the feel/UX/param-plumbing right, since the DSP mostly already exists; then drop in the real cores + a new **saturation engine** whose pre-shaper *drive* is the lever the squareness experiment identified). Effects run first as ordinary mod destinations (XY / Kuramoto LFO / velocity → drive/size/feedback — the by-hand orchestral↔organic↔metallic morph, made internal + routable). **Deferred behind the mod matrix:** FX driven by the synth's own emergent observables (reverb size ← R, feedback gated by slip, drive ← σ) and FX cores cross-coupled to the carrier swarm — E3's "collapse/comb-regularity/in-basin-error as mod sources," the novel/experimental layer. Global-bus vs per-voice FX placement is an open build decision. Visualization per SPEC-EFFECTS §7, warnings per ADR-017 (cause AND state). **STATUS (2026-07-20): increment 1 shipped** — src/fx_rack.h (4 series slots, placeholder Off/Drive/Filter/Gain, all-Off bit-exact passthrough) + shell params 57-64 + GUI cluster; ./verify full green (state_check confirms rack params round-trip), pluginval 10 SUCCESS. Next increments: real cores as slot types + saturation engine, FX params as mod destinations, true parallel/matrix grid. **Master HPF is provisional (human, 2026-07-23).** The lab's master high-pass (a single one-pole low-cut, ADR-063-era lab work) is a stopgap for mud/rumble; it will **probably be replaced by a proper filter module — or more than one** — living here in the rack (the resonator bank `filter_core.h` and notch swarm `notch_core.h` already exist as slot candidates, alongside a conventional multi-mode filter). CONSEQUENCE for the fold: do NOT freeze the stopgap HPF as a reference/CLAP param we would then have to deprecate (param ids are append-only) — keep it lab-only until the real filter section is designed, and let that section supply the low-cut.

**Sequencing note:** E0 is small and high-leverage — the force core is the same mathematics the dynamics engine already needs, so if the third original engine is the dynamics engine, build E0 first and have both consume it. If the third engine is already underway with its own force implementation, unify at E0 rather than maintaining two.

**Local sequencing ruling (per the packet's own note):** the dynamics engine is already built inside SwarmCore with its own force implementation — so E0 is a UNIFICATION: extract/share the force mathematics rather than build a second copy.
