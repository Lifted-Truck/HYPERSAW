# SWARM✱ — roadmap (phase-gated)

Gates are blocking. "Green" = `./verify fast` passes + phase acceptance subset + trace written. Passing ≠ done; done = green + acceptance criteria + DECISIONS/trace updated.

**Status (2026-07-21):** Phases 0–1 CLOSED. Phase 2 (SAW) + Phase 3 (dynamics) gate-close proposed and shipped — **formal ratification still pending** (see per-phase gates below). Phase 4 SPECTRA ported + shell-integrated; ADR-037's P=1 gate was RULED 2026-07-18 (option (a), measured equivalence) — its only live remnant is the shared-voice-path A/B follow-up. Track E: E0 force-core + E2 time engines done; E1 frequency cores done (L0-17/18 + SWARM-FX GUI remain); **E3 internal FX rack increment 1 shipped** (ADR-054). Feature adds (2026-07-20/21): SPECTRA ADSR (ADR-055), bipolar onset lock (ADR-056), SPECTRA transposition (ADR-057), SAW waveshape morph (ADR-058), GUI column layout + fixes. **Swarmalator** core+oracle done, awaiting nondestructive shell integration — the recommended next build. Mod matrix: Kuramoto LFO design accepted (ADR-053); rotor-to-golden pending. Dev: `./install` installs the plugin locally in one command. Open housekeeping: formally close the Phase 2/3 gates. (ADR-037 ruled 2026-07-18; merged branches pruned 2026-08-03.) **This status block is historical — see the OPEN WORK REGISTER below for what is actually open.**

**Status (2026-07-18):** Phase 1 GATE CLOSED (PR #2 merged; protocol findings + free-row erratum ratified with the merge, erratum applied to ACCEPTANCE). Phase 2 in progress: SwarmCore is live in the plugin — placeholder sine replaced, 18-param CLAP surface at prototype ranges (dissolve in seconds, driftDepth in cents), versioned key=value state; pluginval strictness 10 SUCCESS, auval SUCCEEDED post-integration. Phase 2 remaining: tempo-grid law port (needs host tempo; L0-12), bimodal/clustered-pairs distributions (OPEN QUESTION — SPEC lists them, the reference doesn't implement them: extending the reference is a spec change needing a human ruling), GUI v1 + dev state button, webview smoke test, Layer-E 1/2/5 sign-off.

*(Historical status, 2026-07-17 evening:)* Phase 0 largely complete — skeleton builds (CLAP + VST3 + AUv2 via clap-wrapper, pinned submodules), pluginval SUCCESS at strictness 10 (gate asks ≥5), auval SUCCEEDED, all three formats installed locally with intact codesign seals; ADR-006 spike run (bank 66× / iFFT 216× realtime at 2560 osc on M3) with close proposed as ADR-018 (bank); GUI stack proposed as ADR-019 (choc webview). CI matrix (macOS + Windows build + pluginval) GREEN on both platforms (run for 3283ae9; Windows needed static-MSVC-runtime + M_PI portability fixes). **PHASE 0 GATE CLOSED 2026-07-17:** ADR-018 (bank), ADR-019 (webview, with the swappability amendment), and the E-6 envelope ratified by the human; Live load test passed (VST3 loads, plays sine on MIDI input — no GUI yet, as designed). **Recorded residual (human-accepted):** Reaper/Bitwig load evidence deferred — neither host is installed on this machine; CI pluginval on both platforms is the standing proxy; do a real load check when either host is available, no later than the Phase 2 gate. **Windows runtime work deferred (human, 2026-07-18):** the WebView2 backend stays CI-compile-verified only until desktop-coordination begins; Windows runtime validation moves out of the Phase 2 gate to that milestone. Phase 1 (SwarmCore port + parity oracle) is now in progress. Proposed E-6 envelope: min-spec = Apple M1 base / 4-core 2018-class Intel ultrabook, Windows x64 AVX2; 44.1 kHz @ 128-sample buffer; E-6 patch must hold < 50% of one core on min-spec. Deferred ecosystem briefs: Tonality intake brief due at Phase 3 before consonance gravity ships; terrain-sibling intake brief due at Phase 4 with the kernel abstraction (ADR-010(d) — placeholders in the meantime).

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
| A1 | **Bend inertia fold** — which travel law; flat vs distance-keyed; bend lane / note pitch / both; does it reach MPE | § Pitch-bend inertia; `docs/design/bend-lab.html` |
| A2 | **Swarmalator** — listen before shell integration (core + oracle done, bit-exact) | § Experimental engines |
| A3 | **Shape lab fold** — mandate rulings: fold mode and carrier purity both leave saw territory deliberately | § Lab campaign 2 item 6 |
| A4 | **ITD max 0.6 → 0.3** — proposed on measurement (metrics saturate above 0.15 ms); wants an ear A/B first | § Open questions 2026-08-03 #1 |
| A5 | **AP freq 700 Hz** (super-width mode D) — arbitrary, never measured; A/B in the width lab and pin | § Open questions 2026-08-03 #2 |
| A6 | **SPEC citation amendment** — protected path, awaiting approval | § Timbre-space research |
| A7 | **Law/dist widening** — state compatibility, scope, and which core-only params to expose | § Open questions for the human (4 sub-items) |
| A8 | **Phase 2/3 formal gate ratification** — shipped and evidenced, never formally closed | § Phase 2 / Phase 3 gates |
| A9 | **Mod source polarity** — ANSWERED 2026-08-05 by the reachability probe: zero routings fully unreachable, two half-unreachable (`R → Kboost`, `ENV → Kboost`), now marked in the matrix rather than rejected. Only residual question if you want it: should `Kboost` stop being half-wave rectified | § Rejected routings |
| A10 | **Morph-owned routing semantics** — RULED 2026-08-06: **per-corner depths per cell**. Each morph-owned cell holds four depths; a flip swaps which is live. Implemented + measured | § Morph-owned = per-corner depths |

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
| B11 | **Multi-oscillator ADR** (layout-lab IA) | **ADR-082 PROPOSED 2026-08-06** — id scheme (+100 stride, osc 0 keeps its ids), per-osc state keys, CPU budget. Blocks all interface-renovation GUI work; needs ratification |
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
