# horde — decisions (append-only)

## ADR-001 · One kernel-agnostic engine — ACCEPTED
SAW mode is the per-partial swarm engine at P=1 with a saw kernel; SPECTRA is the same engine at P>1 with a sine kernel; wavetable kernel reserved (terrain-sibling crossover). Rationale: the three prototypes converged on identical state (phases, vf, couple, R/ψ/σ, slewed K) differing only in kernel and partial count. Consequence: Phase 4 must prove mode parity at P=1.

## ADR-002 · CLAP-native, VST3 via clap-wrapper — ACCEPTED
Carried from prior platform exploration (JUCE alternatives). Revisit only if a Phase 0 host-compat blocker appears.

## ADR-003 · JS prototypes are the reference implementation — ACCEPTED
swarmsaw/swarmspectra/swarmdynamics HTML cores are the spec-in-code. C++ correctness is defined as parity (L0-1) plus trajectory regressions, not as independent reimplementation. The prototypes' headless Node harnesses are the templates for `./verify`. Divergence requires an ADR here.

## ADR-004 · σ-normalized K with squared taper; absolute-K escape hatch — ACCEPTED
K_Hz = 4·K·|K|·σ_measured places the phase transition mid-knob at any detune/law/register (validated). σ floored at 0.05–0.08 Hz. Normalization collapses for identical oscillators, so an absolute-K advanced mode is required for strict-chimera experiments (L0-10 note).

## ADR-005 · Tempo-grid quantization is a detune law — ACCEPTED
Not a subsystem: voices are placed by the distribution, then Hz offsets snap to the nearest grid multiple. First implementation (forced ladder) ignored the detune knob and produced ±23 Hz spreads; quantize-to-grid fixed spread, exactness, and coherence simultaneously (L0-12). Unit is cycles-per-beat.

## ADR-006 · Additive renderer: oscillator bank vs iFFT — OPEN
Phase 0 spike. Bank is simpler and parity-friendly; iFFT scales past ~200 sines and fits control-rate coupling naturally. Decide on measured min-spec headroom at 128 partials × 5 voices × 4 notes.

## ADR-007 · Splay is assigned-slot, not emergent N-th-harmonic coupling — ACCEPTED
N-th-harmonic mean-field coupling rotates at N·f and cannot be held across a 16-sample control tick; it also admits full-sync as a spurious solution. Assigned slots anchored on the center voice (argmin |x|) are stable, tick-holdable, and avoid pitch sag. Splay authority is 3× sync (detune fights slot precision; at 1× the multiplication was only 2.4× baseline vs 9× at 3×). Detune under splay is reframed as "looseness."

## ADR-008 · Consonance gravity is an audio-engine force — ACCEPTED
Runs on per-note f0cur inside the engine (per-render-block pairwise pulls, basin-limited, octave-folded ratio snap), not as MIDI preprocessing. This is the substantive distinction from the Hermode/Pivotuner/Alt-tuner category (see PRIOR-ART) and what makes settling an audible physical event. Note-on resets to ET so the settle is per-chord.

## ADR-009 · Coupling slew coefficients are per-tick at 2756 ticks/s — ACCEPTED (recorded as a trap)
16-sample ticks at 44.1 kHz. τ ≈ 1/(coef × 2756). A 10× mental error here silently destroyed the cascade zipper once. All time-constant math in the C++ port must be expressed in seconds and converted, never hand-tuned per-tick.

## ADR-010 · Spin-up ratifications (2026-07-17) — ACCEPTED
Survey conducted via /spinup; answers in project.manifest.json (provisional pending final ratification). (a) Project/repo name is HYPERSAW; SWARM✱ branding stays open until the Phase 5 naming decision. (b) Architecture rung 2 — single thread + fresh-context verifier/subagents; earned by the parity oracle, fleet not earned (one engine per ADR-001). (c) Model pins per doctrine: lead/critic Opus, implementer Sonnet, verifier Haiku; judgment-bearing DSP/oracle work stays with the lead. (d) Tonality and terrain-sibling intake briefs deferred to Phase 3 / Phase 4 respectively; the default ratio set and stubbed kernel are the visibly-degraded placeholders. (e) Kit docs (SPEC/ACCEPTANCE/ROADMAP/PRIOR-ART/PARKED + this log) adopted as canonical; ACCEPTANCE.md numbers are the oracle contract.

## ADR-011 · swarmsaw_v2.html supersedes swarmsaw.html as the SAW reference — ACCEPTED
Per CHANGE-NOTE-splay-legibility.md (2026-07-17): v2 = v1 + R_N (Daido) order parameter in controlTick + splay-legibility visualization (dual meters, seat rings, formation polygon). Diff-verified additive; no audio-path changes. SPEC §5.6 and ACCEPTANCE L0-3 amended accordingly. R_N is parity-relevant for the C++ port (displayed AND asserted). swarmsaw.html retained for history until the human approves deletion; ADR-003's reference list should be read with this substitution.

## ADR-012 · Reference timeline consolidated; change notes archived under docs/ — ACCEPTED
The definitive timeline of the SAW reference, for any agent reconstructing it: (1) 2026-07-17 morning — `swarmsaw.html` v1 arrives with the spec kit; (2) mid-spin-up — `swarmsaw_v2.html` + the splay-legibility change note arrive (v2 = v1 + R_N readout + splay visualization; diff-verified additive), amendments folded into SPEC §5.6 / ACCEPTANCE L0-3 (ADR-011); (3) post-ratification cleanup — v1 deleted (human-approved), **v2 renamed to `swarmsaw.html`**, so the ADR-003 reference names hold again and both v1 and v2 remain recoverable in git history (v1 and the pre-rename v2 live in the initial commit). Policy from here: reference-implementation updates land as in-repo edits under version control with an ADR; externally-authored change notes are archived at `docs/change-notes/YYYY-MM-DD-<topic>.md`, folded into SPEC/ACCEPTANCE in the same change, never left as loose root files.

## ADR-013 · GUI is an early priority, not a Phase 5 deliverable — ACCEPTED
Human direction at ratification (2026-07-17): a GUI matching the prototypes' visual design as closely as possible is an early priority. ROADMAP amended: Phase 2 now includes GUI v1 (phase circle with dual R₁/R_N meters, seat rings, formation polygon, XY pad, live readouts — the SPEC §5.6 contract, styled from the prototype design language); Phase 5 keeps the full face (phase carpet, partial strips, mod-matrix UI, MPE surface). The prototypes' CSS/canvas code is the design reference — extract tokens (palette, meter treatments, layout) rather than reinventing. Consequence for Phase 0: the CLAP skeleton must pick the GUI stack early enough that Phase 2 can ship a real GUI; renderer spike (ADR-006) and GUI-stack choice are both Phase 0 exit criteria now.

## ADR-014 · Public repo: private siblings are aliased in tracked files — ACCEPTED
Pre-push sweep (2026-07-17) found one referenced sibling project whose repo is private; its name appeared in the kit docs. Human ruled scrub-before-push: all tracked-file references use a neutral alias ("terrain sibling"), the unpushed history was rewritten so the name never reaches the public remote, and the alias→name map lives in PRIVATE-NOTES.md (untracked, gitignored). Standing rule: never write a private sibling's real name into a tracked file; public siblings (e.g. Tonality) are named directly. The verify leak gate covers paths/identity; this alias rule covers project names — both are push-blocking.

## ADR-015 · Daido multi-pole coupling verified; ADR-007 partially amended — ACCEPTED
*(Arrived 2026-07-17 as an external ADR drop numbered 010; renumbered here — this log was already at ADR-014. Claims independently re-verified headless on the v2 dynamics core before merging: (a),(d),(e) reproduced exactly, (c) reproduced to three decimals, (b) reproduced qualitatively.)*
Measured in the dynamics lab (mean-field path, `poles q` select, q-harmonic mean field sin(ψ_q − qθ − α), tick-held):
(a) **q-cluster formation**: q=2 → R₂ = 0.97 across seeds with R₁ low; q=3 → R₃ = 0.97; clusters lock hard.
(b) **Spontaneous symmetry breaking**: cluster population splits are seed-dependent (measured 8/16, 12/12, 8/16, 14/10, 11/13 at identical parameters) — demographics are preset character.
(c) **Split is a timbre dimension**: 2f0 energy constant (~0.080 projection) across seeds while residual f0 tracks cluster imbalance (12/12 → 0.006; max imbalance → up to 0.064), i.e. the split ratio is a fundamental-vs-octave mix knob the seed rolls.
(d) **Bistability**: identical parameters, aligned start → full sync persists (R₁ = 0.99); scattered start → clusters. Two coexisting attractors, confirming hysteresis potential.
(e) **ADR-007 amendment**: the claim that q-harmonic mean-field coupling cannot be tick-held was WRONG — ψ_q and qθ both advance at ~q·f, so their difference drifts at q·detune (slow); tick-holding locks cleanly at q ≤ 4. ADR-007's *other* rationale stands: q-coupling admits full sync as a coexisting solution (that is now (d), a feature), so assigned-slot splay remains the correct mechanism where a deterministic multiplication outcome is required. Daido coupling ships alongside it as the emergent sibling.
Meter note: under q-coupling, sync shows R₁ AND R_q high; clusters show R₁ low + R_q high (unlike SWARMSAW's R_N, where N=7 magnification keeps sync-side R_N low).
Spec placement: `poles q` joins Layer 2 (coupling); generalizes the drawn-coupling-function direction (Daido harmonics ⊂ arbitrary Γ(Δθ)).

## ADR-016 · Coupling × beat-grid interaction is physics; the UI must disclose it — ACCEPTED
*(Arrived in the same drop as external ADR-011; renumbered. Suppression direction re-verified headless — locking a populated grid suppresses grid-rate envelope power; the ~80× magnitude is the design session's measurement and is harness-dependent (a coarser independent method gave ~5× at detune 0.6): treat the direction as blocking, the magnitude as reference.)*
Measured: with the tempo-grid law active and populated (3–7 rungs, exact multiples verified), raising K to lock (R = 0.97) suppresses envelope power at the grid rate ~80× (6.0e-4 → 7.7e-6); same under Daido q=2. Locked voices share one frequency → no gaps → no beats. This is the intended "K loosens/overrides the grid" negotiation, not a defect — but it is invisible without instrumentation and was reported as a regression in testing. Requirement: any surface exposing the tempo-grid law MUST show live grid status (unit, rungs occupied, and a lock warning when coherence is high) so the populated-but-overridden state is legible. Reference implementation: `beatinfo` readout in swarmdynamics.html. Hierarchy note for presets: grid-forward patches want K subcritical; document K ≲ 1×σ as the compatible range. Warning condition as originally stated here is superseded by ADR-017.

## ADR-017 · Amendment to ADR-016: lock warnings require cause AND state — ACCEPTED
*(Arrived in the same drop as external ADR-012; renumbered, internal reference updated from "ADR-011" to ADR-016. Falsifier scenario re-verified: retrigger zeroes phases, so R = 1.00 at note-on with Ksm = 0.000 by construction; the corrected gate is implemented verbatim in swarmdynamics.html drawBeatInfo.)*
ADR-016 specified the grid-suppression warning condition as coherence alone (R or R_q > ~0.8). Falsified in testing: with K = 0 and retriggered phases, R reads 1.00 with Ksm = 0.000 — the swarm is coherent from initial conditions, not from coupling, and no suppression is occurring (nothing holds the voices; the grid reasserts as they disperse). Corrected condition, verified both directions: warning fires iff coupling is engaged (Ksm > ~0.05 Hz) AND coherence is high (R, R_q, or both cluster orders > 0.8). General principle for all such displays, here and in the VST: order parameters measure state, not cause — any UI claim about what the coupling is *doing* must gate on the coupling being nonzero. Reference implementation updated in swarmdynamics.html; UI additionally reserves fixed height for the warning line so its appearance cannot reflow the layout.

## ADR-018 · ADR-006 closed: oscillator bank renderer (iFFT recorded as escape hatch) — ACCEPTED (ratified 2026-07-17)
Spike measured 2026-07-17 on Apple M3, single thread, Release -O3, 8 s renders (tools/renderer_bench.cpp): at the target load (128 partials × 5 voices × 4 notes = 2560 sinusoids) the bank runs 66× realtime, vDSP iFFT 216×; at the prototype ceiling (480 osc) 403× vs 947×. Both have order-of-magnitude headroom even derated ~4× for min-spec. With throughput not the binding constraint, the decision falls to fidelity: (1) **parity** — L0-1 demands ε=1e-6 equivalence with the JS reference's per-sample phase accumulators, which the bank preserves structurally and an iFFT/windowed-OLA pipeline cannot; (2) **control-rate semantics** — the bank honors the 16-sample tick; iFFT quantizes control to the hop (~23 ms at 4096/1024), smearing fast K slews and per-tick coupling. Decision: oscillator bank (table + linear interp per SPEC §6.6). iFFT remains the recorded escape hatch if partial budgets grow past ~1k sines per note or min-spec measurements (E-6 hardware envelope) fall short; revisit trigger written into ROADMAP Phase 4. Caveats: M3 is not min-spec; benchmark excludes control-tick coupling cost (identical for both paths).

## ADR-019 · GUI stack: embedded webview (choc), reusing the prototype design language directly — ACCEPTED (ratified 2026-07-17, with amendment)
ADR-013 requires the GUI to match the HTML/canvas prototypes as closely as possible; a webview can run the prototype's canvas/JS/CSS nearly verbatim instead of reimplementing it in native drawing code. Research (2026-07-17, scout report in traces): choc::ui::WebView (Tracktion; actively maintained; WKWebView/WebView2/WebKitGTK; permissive license) is the de-facto C++ plugin webview — JUCE 8's own WebView UI feature is built on it; CLAP itself added a draft webview extension in 1.2.7; Cmajor's CLAP exporter defaults to webview UIs; a Rust webview+CLAP stack reports 15k+ production users. Known risks, each with a mitigation owner: (1) keyboard-focus stealing in Live/Logic on macOS (documented JUCE-forum pattern) → implement key-forwarding in the platform view from day one; (2) webview-through-clap-wrapper-to-VST3 unconfirmed in the wild (VSTGUI/ImGui confirmed; a webview's NSView/HWND should pass through identically but this is inference) → Phase 0/1 smoke test before GUI work starts; (3) multi-instance memory behavior → measure with 8 instances during Phase 2. Fallback recorded: iPlug2 IGraphics (CLAP-native since 2024) or Dear ImGui, at the cost of a full visual reimplementation. Consequence: GUI v1 (Phase 2, ADR-013) is built as HTML/canvas assets shared with — and diffable against — the prototypes.
**Ratification amendment (human, 2026-07-17): build for swappability.** The webview is an implementation behind a seam, not an assumption the codebase absorbs: (1) engine↔GUI communication goes through a transport-agnostic message/parameter layer (state snapshots out, gestures in — no webview types outside the GUI module); (2) the CLAP gui-extension implementation lives in one module that owns all platform-view and webview specifics; (3) visualization data (R/R_q/σ/phases for the meters) is produced by the engine as plain structs, so a native fallback (iPlug2 IGraphics / ImGui) consumes the same feed. Swapping GUI systems must never touch the DSP core or the plugin shell.

## ADR-020 · Param-surface may exceed prototype UI ranges; display units are host-facing — ACCEPTED
Human request (2026-07-18, post-GUI test): drift depth widened 0–25c → 0–100c and drift rate displayed in physical units. Principle: the REFERENCE pins the DSP mapping (a cents value is a cents value at any magnitude — parity and goldens unaffected), while the prototype's UI ranges are ergonomic defaults the plugin may exceed deliberately, recorded here when it does. Drift rate keeps its 0–1 knob domain (state/parity compatibility) but value_to_text and the GUI display the actual random-walk speed 0.2–8.2 /s (rate = 0.2 + knob·8, the controlTick mapping). Caveat recorded: widening a param's range rescales NORMALIZED automation in VST3 hosts — acceptable pre-release, never again after 1.0 without a migration shim. Also: GUI window is now resizable (720×440–1600×1000, default 920×600, responsive layout); ADR-019 risk #1 downgraded — no keyboard stealing observed in Live with sliders/pad; the state textbox is the remaining watch item (only text-enterable field).

## ADR-021 · ADSR envelope as a parity-safe superset of the reference AR — ACCEPTED
Human request (2026-07-18): traditional ADSR control. The reference has a fixed AR (~3 ms attack, ~160 ms release, per-swarm one-pole; SPEC §6.5) sitting in the parity-critical render loop, so the design constraint is: DEFAULTS MUST REPRODUCE THE REFERENCE BIT-EXACTLY. Implementation: attack/decay/sustain/release params (defaults 3 ms / 160 ms / 1.0 / 160 ms); when sustain >= 1 the envelope takes the reference's exact code path (same expressions, same operands — decay never engages), so L0-1 goldens are the regression proof that defaults are unchanged; sustain < 1 enters an attack→decay phase machine that is new, deliberately divergent behavior. Time params are expressed in seconds and converted per-rate (ADR-009 discipline). Param IDs 19–22 appended (never renumbered); state schema gains keys additively (old states load with reference defaults). The SAW prototype is NOT extended — this is a plugin-surface superset per the ADR-020 principle, recorded here because it touches the DSP core.

## ADR-022 · Tempo-grid law ported into the SAW core from the DYNAMICS reference — ACCEPTED
The grid law (ADR-005) lives in swarmdynamics.html, not the SAW reference, so adding law 3 to SwarmCore is a cross-reference port: the snapping expression is taken verbatim from the DYNAMICS beatQ path (u = bpm/60 · cyclesPerBeat; cents placement, then round(df/u)·u on the Hz offset), with L0-12 as its oracle (the SAW goldens cannot cover it; laws 0–2 remain proven untouched by L0-1 staying green). bpm is host-owned — CLAP transport tempo, read per block and from transport events, never a user param (fallback 120 without a host tempo); cycles-per-beat is param 23. Drift applies after the snap: turning drift up deliberately loosens the grid (the L0-12 protocol runs at drift 0). Placement protocol note: the DYNAMICS reference has no distribution selector (even spread implicit) — L0-12's rung/spread numbers reproduce at dist=even, M=16, and the harness pins that protocol.

## ADR-023 · One unified core proven against BOTH references — ACCEPTED
Phase 3 structural finding: ADR-001's "one engine" held to the expression level — DynSynth's unipolar K taper 4K²σ IS the SAW sync branch at K ≥ 0, and the two references share σ measurement, slews, RNG, envelope, and render paths verbatim. The dynamics layer (topologies mean-field/ring/two-cluster, Sakaguchi α via Math.PI-precision radians, Daido poles, consonance gravity on f0cur, bimodal placement tied to two-cluster) was ported expression-for-expression into SwarmCore such that (a) SAW defaults leave every SAW expression bit-untouched (topo 0/α 0/poles 1/grav 0; sin(ψ−θ−0.0) ≡ sin(ψ−θ)), proven by the SAW golden set staying 30/30 with the identical worst case; (b) a DYN config (dist even, lpOut 0 — the output pole is the ONE structural difference between references, exposed as a param) reproduces DynSynth, proven by a new 21-scenario DYN golden set (7 scenarios × 3 seeds; 9 bit-exact, worst 4.3e-9 on the chaotic ring). Splay exists only on the mean-field path (SAW has no topologies; DYN has no splay). L0-8..11 green with one protocol note (L0-11's snapshot values reproduce as a 2–8 s observation window — the locked/drifting ring regions wander; reference-probed) and one probe correction (a 350 Hz "outside basin" test note actually folds inside the basin; 240 Hz is outside). ADR-015 anchor criteria (R_q ≥ 0.95 at q∈{2,3} across seeds; aligned-start bistability) now enforced in trajectory_check; formal ACCEPTANCE text proposed at the Phase 3 gate.

## ADR-024 · Inertia knob taper at the surface; XY pad two-way binding — ACCEPTED
Human report (2026-07-18): inertia feels binary. Measured landscape (R tail-mean over K × w): at K ≥ 0.85 the response is smooth, but at near-critical K (0.6–0.7) the FIRST epsilon of inertia drops the marginal lock a regime (0.60→0.45 at w=0.02) and then w ∈ 0.02..0.3 is a dead plateau (ω0 = 2π(8(1−w)+0.6) stays far above the beat rates there). Two separate phenomena: the initial step is reference physics (SPEC §5.3 keeps the hunting deliberately — softening it would be a DSP divergence, offered to the human as a reference-update question); the dead plateau is a knob-mapping defect fixed here parity-safely: the CLAP layer maps core w = sqrt(knob) (heavy region spread across the knob), the knob value keeps ONE documented shell-side slot (sqrt/square round-trips are not bit-exact and state_check demands exactness), and both state-load paths route through applyParam so the taper applies identically. Core DSP and all goldens untouched. Also fixed: the XY pad was one-way (pad→knobs); setControl and the slider handlers now update the pad crosshair, and the GUI polls params ~2×/s (drag-guarded) so host automation and device-panel moves reflect back into the GUI.

## ADR-025 · Super-width: width param extends to 1.5 with mid/side boost — ACCEPTED
Human request (2026-07-18): wider. The pan law tops out at hard L/R, so beyond-reference width is a master-bus mid/side side-boost (up to 2× at 1.5) applied before the soft clip, reachable only at width > 1 — the reference range is bit-untouched (pan-law width contribution clamps at 1 to prevent anti-phase pan angles). Verified: side energy ~4× at 1.5 vs 1.0, bounded output. Range extension carries the usual pre-release normalized-automation caveat (ADR-020).

## ADR-026 · Voice mode: mono/legato/glide + octave transpose — ACCEPTED
Human request (2026-07-18). Core superset (parity 51/51 unchanged): retargetNote(slot, midi, f, legatoKeepPhase) reuses a sounding voice (legato keeps phases/envelope; non-legato re-strikes in place but glides from the current pitch), glide is an exponential f0 approach (tau = glide seconds, per-tick coefficient per ADR-009) that moves f0cur multiplicatively so gravity offsets survive; the poly path never arms it. Shell owns the mono note-routing: held-note stack (16 deep), last-note priority with return-to-held on release, mode-switch panic-clears. Octave (−2..+2) is a pure shell transpose at note-on (frequency only; host note identity untouched). Params 32–35 (24–30 stay reserved for the dynamics surface); param 15 renamed "Mono Fold" to disambiguate. NOTE_END in mono legato carries the latest note identity — earlier notes in a legato chain get no individual END (known edge, recorded; pluginval clean).

## ADR-027 · Live transposition suite; glide gated on held keys — ACCEPTED
Human clarifications (2026-07-18). (a) Octave/semitone/fine-cents/pitch-knob (params 35–38) combine into ONE multiplicative core tune factor applied at law evaluation — so the pitch knob bends SOUNDING notes (the note-on-time octave transpose from ADR-026 is removed in favor of this; one mechanism). tune = 1.0 is bit-inert (parity 51/51 unchanged); gravity ratios are tune-invariant (common factor cancels pairwise). (b) The "octave toggle" the human actually meant is a UNISON octave layer — parked as a design-session question (PARKED #14: coupling-graph membership and σ-normalization under a 2:1 split are real physics questions; reference-first like clustered-pairs). (c) Mono glide/legato now engage ONLY when another key is physically held; a ringing release tail alone gets a fresh strike on a new slot, overlapping the tail naturally (the previous voice-live test conflated tail-ringing with held).

## ADR-028 · Dynamics surface exposed (params 24–31, meters, readouts); absolute-K semantics — ACCEPTED
Phase 3 increment 2. Params: Topology/Ring Reach/Cluster Link μ/Phase Lag α/Poles q/Gravity/Basin/Absolute-K (ids 24–31; labels for enums; α in degrees). GUI Dynamics cluster; meters gain the dyn readouts (R_q under poles, A/B cluster orders under two-cluster); gravity readout shows captured pairs as ratio-name + octave fold + live cents (the ADR-008 requirement); ADR-016/017 grid-status line implemented verbatim (unit, occupied rungs, cause-AND-state lock warning — KsmS > 0.05 AND coherence > 0.8 — on a fixed-height line). Absolute-K (ADR-004) semantics chosen: σ replaced by 1.0 Hz in the coupling targets (taper preserved) — the minimal bypass; guarded, default-inert, verified (identical oscillators lock from scatter, R = 1.0). En route, state_check went RED and caught a real bug: readParam's hand-maintained key chain lacked the new params (get_value → 0; state saved lies; the value-round-trip check passed because both sides lied identically — only the restored-audio-bit-identity requirement caught it). Fixed by construction: core.getParam reads through the SAME paramSlot map setParam uses; the shell's parallel chain is deleted (L0005).


*(ADR-029..032 arrived 2026-07-18 in design-session packet UPDATE-001, numbered 013-016 there; renumbered per the standing collision policy (ADR-012/L0001). The packet's 010-012 matched local 015-017 verbatim and were skipped. Internal references remapped.)*

## ADR-029 · Filter family: parameter-domain coupling; home spring exposed; allpass topology rejected for the notch swarm — ACCEPTED
The swarm force system applies to filter center frequencies at control rate (SPEC-FILTER.md): two engines (resonator bank, notch swarm) on one shared core, accepting external audio in the VST (effect line). Three bound decisions:
(a) **Home spring is a musical parameter.** Competing springs settle at err_final = h/(h+g)·err_initial — verified predictive in both engines (bank gravity 28.8¢→6.9¢ at h=1.5/g=4.8; notch capture residuals 7–10¢). The prototype's h=1.5/s is a compromise, not a constant; expose it, and park a capture-tightening mode (adaptive h release inside basins) for sub-cent snap.
(b) **Allpass-cascade phaser topology REJECTED by measurement** for the notch swarm: cascade notches form at cumulative-phase odd-π points, not stage centers — the swarm moved stage frequencies while actual nulls sat elsewhere (pseudo-depths scattering ±10 dB, some negative). True SVF notch sections (y = x − k·v1) null exactly at fc (~150 dB measured), which the engine's premise requires. Feedback retained (tanh-bounded); classic-phaser phase character traded for exactness.
(c) **State-domain filter coupling stays parked** behind the Stuart–Landau bridge ADR; this family is deliberately control-rate-only.

## ADR-030 · Time-domain family (tap swarm + FDN room swarm); FDN eigenstructure finding — ACCEPTED
The shared force core applies to delay times (SPEC-FILTER family, log2-seconds coordinate). Reference: swarmtime.html (TimeLab, echo/room modes). Verified: tap gather σ 0.328→0.066 and grid CV 0.720→0.055; rhythmic gravity onto {¼,⅓,½,⅔,¾,1,1½,2,3,4}×beat, in-basin 34.9→8.3¢; impulse echoes confirmed at all tap times; room→note collapse yields a 16–19 dB comb at k/L; sympathetic gravity (lengths → multiples of note period) 34.3→7.4¢ on captured lines; stable at regen 0.97 with drift+K.
(a) **FDN feedback matrix sign is load-bearing.** Householder I−(2/N)J gives the all-ones vector eigenvalue −1; injecting input along ones then resonates at odd/(2L) — the initial build probed 1/L and measured an anti-resonance (−1.4 dB). Negated Householder puts +1 on the ones channel → comb at k/L, which the room→note and sympathetic-tuning semantics require. Any future matrix/topology work must state which eigenchannel the input excites.
(b) **Equilibrium law err_final = h/(h+g)·err_0 now confirmed three-for-three to the decimal** (bank 6.9¢, echo 8.3¢, room 7.4¢). Promote from finding to design law; acceptance criteria may cite it directly.
(c) **Sympathetic-room capture is geometry-limited**: multiples of the note period are 100–390¢ apart for short lines, so fixed cents-basins capture few lines (3 of 8 at ±80¢). Park: adaptive basin (fraction of local inter-target spacing) and/or longer-line placement defaults for sympathetic presets; f0 reinforcement scales with capture count (measured +3.3 dB relative at 3/8).
(d) Time-domain drift = wow/flutter; moving taps chirp (Doppler) — continuous-interp "tape mode" is the prototype default; discrete between-repeat motion parked as a mode switch.

## ADR-031 · Effects consolidation; two stability laws from measured failures — ACCEPTED
SPEC-EFFECTS.md supersedes SPEC-FILTER.md, adding the time-domain family (tap swarm delay, FDN room swarm; ADR-030 findings incorporated). Two stability laws promoted from bug post-mortems to design law:
(a) **Feedback normalization assumes worst-case correlation.** N delay taps reading shared low-frequency content sum with gain N; a √N feedback norm gave the echo loop LF gain regen·√N — unstable above regen ≈ 0.35, i.e., at defaults. Measured runaway: buffer mean 0.04→0.42 over 10 s of a held note, followed by tanh-saturation squashing the audio to near-silence with slow recovery — the field-reported "plays unreliably / too quiet / buffer takes long to clear" symptom, diagnosed correctly by the user from the phenomenology. Fix: feedback paths /N, output paths /√N, verified flat over 12 s at regen up to 0.97.
(b) **DC-block inside every feedback loop; corner ≤ lowest-musical-comb/6.** The room's +1 eigenchannel passes DC at loop gain ≈ regen. A 15 Hz in-loop blocker fixed DC but compounded per pass and gutted a 26 Hz comb (16.3→2.6 dB); at 4 Hz the comb holds 13.5 dB and line DC stays ≤ 0.006. In-loop filters compound: their passband edges must clear musical content by a wide margin.
Both laws are Layer-0-guarded (L0-19/L0-21) so neither failure mode can silently return in the C++ port.

## ADR-032 · Focus-trap silence; interaction-layer failure modes are real failure modes — ACCEPTED
Field report: "suddenly no sound." DSP exonerated by 60 s randomized parameter fuzz (no NaN, RMS floor 0.04). Root cause was the interaction layer: sliders retain keyboard focus after use, and the note-key handler correctly ignores keys targeted at INPUT elements (so typing in seed/bpm fields doesn't trigger notes) — therefore adjusting any knob silently killed the entire musical keyboard until an incidental click elsewhere. Present in all six labs; never caught because on-screen key clicks incidentally restore focus.
Fixes (all labs): range inputs blur on pointerup; selects blur on change; Escape = panic (blur + all notes off). Belt-and-suspenders: NaN watchdogs added to both feedback engines (TimeLab, PhaserLab) — non-finite feedback state self-heals within one block instead of rendering permanent silence.
Laws for the VST: (a) any "keyboard input guarded by focus" design must actively release focus from momentary controls, and the guard's scope must be per-control-type, not blanket; (b) every feedback engine ships a finiteness watchdog — a NaN must never be able to silence the instrument for longer than one processing block; (c) diagnostic order is preserved as method: exonerate the DSP by fuzz before touching the UI, so fixes land in the layer that actually failed.

## ADR-033 · Absolute-K unit raised to 2.5 Hz; phase-scatter percentage — ACCEPTED
Human requests (2026-07-18). (a) Absolute-K's unit was my ADR-028 "minimal bypass" placeholder (1 Hz → max pull 4 Hz — measurably feeble next to σ-normalized patches at ~13 Hz). No reference constrains this mode, so the unit rises to 2.5 Hz (max pull 10 Hz at knob 1); existing absK patches get proportionally stronger (pre-release, acceptable). (b) Phase scatter (param 39): partial-random phase init generalizing the retrigger toggle — phase_i = rng·scatter when scatter > 0; at 0 the LEGACY retrig path runs bit-exactly (rng untouched → all goldens stand); at 1.0 the retrig-off stream is reproduced by value (rng·1.0). The retrigger checkbox dims when overridden. Purpose: everything between "tight aligned attack" and "full cloud" becomes a knob.

## ADR-034 · E0 unification ruling: one force core for the effects line; SwarmCore shares only what is genuinely shared — ACCEPTED
The Track E packet's sequencing note assumed the dynamics engine would carry its own position-force implementation to unify with. Inspection says otherwise: SwarmCore's dynamics are PHASE-domain Kuramoto (sin coupling on phases; frequency offsets come from static placement laws), while the effect labs run a POSITION-domain spring system (home/sync/splay/gravity/drift/inertia on log2 coordinates). These are mathematically distinct systems, both reference-frozen — rewriting either side's constants to "unify" them would be a parity-breaking spec change wearing a refactor's clothes.
What unification honestly means here, and what shipped as src/force_core.h:
(a) ONE shared module implements the labs' force system verbatim (they embed one identical force expression; per-engine deltas are frozen constants captured as a Profile — clamps, span floor, inertia ω₀ — plus the attractor set: harmonics n·f0, rhythmic RSET×beat, period multiples). All four E1–E3 engines will consume this module; the four-copies future the packet warned about is dead.
(b) SwarmCore adopts from it the ONE piece of arithmetic the two families genuinely share — mulberry32 — by delegation (parity 51/51 proves bit-neutrality). The drift walks remain separate on purpose: the labs bake depth into the walk unclamped, SwarmCore clamps to ±1 and scales at application — same family, different frozen constants, both reference-exact.
(c) Correctness contract: force_check reproduces the labs' population trajectories seed-for-seed (Node-extracted cores → checkpoint goldens; observed divergence ≤ 1.3e-14 log2 units, tolerance 1e-9) and enforces the population halves of L0-14/15/17/19/20. Protocol finding (L0002 class): the recorded acceptance numbers reproduce ONLY with drift off — TimeLab defaults drift to 30¢, but σ 0.755 / CV 0.009·0.055 / equilibrium ratios 0.238·0.217 all land exactly at driftDepth 0, so the reference measurements encoded a drift-off protocol nobody wrote down. Goldens pin it.

## ADR-035 · Output stage: bass mono (M/S elliptic) + pan scatter — ACCEPTED
Human requests (2026-07-18). (a) **Bass mono** (params 40/41): one 2nd-order Butterworth TPT-SVF high-pass on the SIDE channel — L = M + HP(S), R = M − HP(S). Lows collapse to mid with no crossover phase mismatch (the classic vinyl-elliptic routing; "responsible" = phase-coherent and mono-compatible by construction). Shell post-processing, runs before the spectrum feed so the visualizer shows what leaves the plugin. Measured: side −24 dB @ 30 Hz, −3 dB at corner, 0 dB @ 1 kHz (fc 120). (b) **Pan scatter** (param 42, core): the human's diagnosis was right — pan position is monotone in the detune offset (pan = x[i]·width), so spatial order == frequency order and detune sweeps march across the field in series. The param blends each voice's pan toward a SEEDED PERMUTATION of the legacy positions (Fisher–Yates on an independent stream derived from the seed — the phase/drift stream never moves differently). 0 = bit-exact legacy (parity 51/51 is the proof); 1 = fully scrambled order; between = partial decorrelation. Slider preserved per the request so series panning stays available as a choice.

## ADR-036 · MPE per-note pitch via CLAP note expressions — ACCEPTED
Human request (2026-07-18, "loop in MPE pitch bend sooner rather than later"). CLAP's NOTE_EXPRESSION/TUNING stream (relative semitones, wildcard note matching) maps to a per-swarm noteTune factor applied at the ADR-027 live-tune seam: f0c = f0cur · tune · noteTune. At 1.0 the multiply is bit-inert (IEEE x·1.0 == x — the same guarantee ADR-027 leans on; parity 51/51 green). Fresh strikes reset the factor; mono/legato retargets keep it (MPE bend streams continue across retargets). The note-ports extension already preferred CLAP_NOTE_DIALECT_CLAP, so hosts with MPE (Live via the wrapper) deliver expressions with no further surface. Full per-note MPE (pressure→K, slide→detune) stays Phase 5 with the mod matrix — this ships the pitch dimension early because it needs zero new routing concepts.

## ADR-037 · SPECTRA ships as its own parity-frozen core; the SPEC §2 "one engine at P=1" gate needs a ruling — ACCEPTED (a), 2026-07-18
Phase 4 reality check: SPEC §2 declares one engine ("per-partial swarm over a kernel"; SAW = P=1 + saw kernel), and the Phase 4 gate reads "SAW provably = SPECTRA at P=1". But the two REFERENCES are separate frozen programs whose arithmetic genuinely differs — sigma floor 0.05 vs 0.08, K-slew 0.08/(1+cascade·120) vs 0.08, fixed AR (4 ms/180 ms) vs ADSR, phase update max(0,f)/sr vs the SAW expression, sine-table kernel vs saw, gain norm /(ampSum·M^0.75), tanh on both channels. A literally shared voice path cannot be parity-exact with both references at once. Shipped now (unambiguous under any ruling): src/spectra_core.h as a verbatim port — parity RMS 0.0 on 9/9 scenarios, L0-6 zipper (monotone front; 7.21 s at cascade 1, 1.81 s at 0.5) and L0-7 interference gate (−15.06 dB, wmix engaged) green, wired into ./verify full via spectra_check. OPEN QUESTION for the human (gate interpretation): (a) reinterpret the P=1 gate as a MEASURED-equivalence check (e.g. locked-state spectra match within a stated tolerance) honoring both frozen references, or (b) amend SPEC to bless one unified voice path and re-measure one side's acceptance numbers (a spec change on protected docs). Shell integration (engine-select param, GUI mode gates, spectra viz) proceeds after the ruling — the core and its oracle don't depend on it.
RULING (human, 2026-07-18): option (a) now — the P=1 gate is a MEASURED equivalence check, implemented in spectra_check as a tick-for-tick R-trajectory comparison (at P=1 the two references' dynamics expressions coincide: same coupling/slew/Kenv forms, same rng stream formula, aligned initial phases — the kernel is the only difference, which is exactly SPEC §2's claim, proven at the dynamics level). FOLLOW-UP: attempt a shared voice path behind a build/runtime switch so the human can A/B the sound against the frozen cores; adoption only on that listening test.

## ADR-038 · MPE reaches the wrapped formats: VST3 tuning advertisement + MIDI-dialect member-channel bend — ACCEPTED
Field report (2026-07-18, post-ADR-036 merge): "the MPE bend isn't working yet" in Live (VST3). ADR-036's closing claim — "hosts with MPE (Live via the wrapper) deliver expressions with no further surface" — was wrong; this ADR corrects it. Investigation of the pinned clap-wrapper (v0.15.1):
(a) **The VST3 wrapper never advertised TUNING.** Its INoteExpressionController exposes only the expressions in `_expressionmap`, which defaults to PRESSURE-only unless the `CLAP_SUPPORTS_ALL_NOTE_EXPRESSIONS` compile flag is set (wrapasvst3.h:474) — a flag `make_clapfirst_plugins` never forwards (make_clapfirst.cmake:171 passes no `SUPPORTS_ALL_NOTE_EXPRESSIONS`), so it is unreachable from our build. A host that speaks VST3 note expressions therefore never sends tuning. The supported fix is the wrapper's own extension seam: the plugin now answers `CLAP_PLUGIN_AS_VST3` (clapwrapper/vst3.h) with `supportedNoteExpressions = TUNING|PRESSURE` (PRESSURE kept = wrapper default preserved) and `getNumMIDIChannels = 16`; the wrapper picks this up in `setupWrapperSpecifics` before building the advertisement and the process adapter. Incoming VST3 kTuningTypeID events were already converted correctly ((v−0.5)·240 semis, note-id matched — process.cpp:660) — only the advertisement was missing.
(b) **MPE can also arrive as rotating member channels + per-channel 0xE0, not note expressions.** The VST3 wrapper maps per-channel pitch-bend parameters (IMidiMapping, 16 channels) to `CLAP_EVENT_MIDI` 0xE0 events (vst3/process.cpp:321), and the AUv2 wrapper forwards raw 0xE0 the same way (auv2/process.cpp:551) — Logic's MPE path. The shell now handles `CLAP_EVENT_MIDI`: bend on member channels 2–16 (indices 1–15) at the MPE default ±48 st is latched per channel and applied through the same ADR-036 `setNoteExpr` seam to swarms whose VoiceTag channel matches; the latch is re-applied on note-on because MPE hosts send bend BEFORE the note it modifies and a fresh strike resets noteTune. Channel 1 (index 0 — MPE manager / plain single-channel MIDI) is deliberately excluded: ±48 applied to an ordinary ±2 bend wheel would be wildly wrong, and global bend already has param 38. Non-MPE hosts (everything on channel 1) therefore see zero behavior change.
Residuals, explicitly parked: MPE zone configuration (upper zone, RPN 0 bend-range negotiation) is not parsed — ±48 member default only; an "MPE bend range" param can join the Phase 5 MPE surface; manager-channel bend → param 38 mapping is untouched (bend wheel in non-MPE hosts still a GUI-only affair). Parity/state contract: noteTune stays 1.0 unless events arrive (latches init 0, `plug_reset` clears), so parity-frozen defaults are bit-inert — ./verify full green post-change (parity 51/51, state_check GREEN). CLAP-native hosts (Bitwig) keep the ADR-036 NOTE_EXPRESSION path unchanged. Human verification in Live with an MPE controller is the closing gate.

## ADR-039 · MIDI 1.0 vel-0 note-on IS a note-off; note-lifecycle fuzz oracle — ACCEPTED
*(Authored concurrently with ADR-038 in a parallel session; both claimed 038 — renumbered here at merge triage per the ADR-015 renumbering policy.)*
Field report (2026-07-18): "envelope doesn't always stop when you let go even when release is all the way down." Diagnosis by layered exoneration (the ADR-032 method): (1) primary hypothesis — that the ADR-021 sustain>=1 reference path ignored p.releaseS — REFUTED by reading (render() builds atk/rel from p.attackS/p.releaseS every call; the ADR-021 comment means defaults *coincide* with the reference constants, not that they are hardcoded) and by measurement (headless tail at release=0.005 s is 40 ms and scales with the knob; the 9.21τ truncation is the render loop's 1e-4 skip floor, by design). (2) Shell note routing (poly, mono held-stack, legato) exonerated by 120 seeded causal random note-stream scenarios through the real CLAP process() — all silent after all-keys-up. (3) The defect: the AUv2 clap-wrapper forwards a controller's 0x90-velocity-0 note-off verbatim as CLAP_EVENT_NOTE_ON velocity 0 (no MIDI 1.0 vel-0 remap upstream), and handleEvent ignored velocity — so a vel-0 release struck a FRESH full-gain voice that no note-off ever ends. Reproduced headlessly: 50/50 scenarios hang at sustain level (poly stacks hung voices, peak > 1.0). Controller-dependent, hence "doesn't always."
Decision: the shell remaps NOTE_ON velocity <= 0 to the shared note-off path (handleNoteOff — same mono/held-stack routing). Standard MIDI 1.0 semantics; this synth is velocity-insensitive, so no legitimate host intent is lost. Core untouched; parity 51/51 green (worst 4.262e-09). New oracle: tools/notefuzz_check (poly/mono/legato x normal/vel-0 releases, release at knob min, silence required 0.5 s after all-keys-up; timestamps drawn sorted per block — an unsorted generator emits acausal OFF-before-its-ON orders no host can produce and fakes hangs). NOT yet wired into ./verify (protected file — human gate): proposed addition to the full() chain. Residual noted for a follow-up: voice stealing and mono retarget overwrite tags[] without emitting NOTE_END for the displaced note (host-side bookkeeping leak; the wrapper's _activeNotes grows).


## ADR-040 · SPECTRA partial ceiling raised 24 → 32 — ACCEPTED
Human request (2026-07-18). The SpectraSynth reference caps partials at P_MAX=24; this raises kPMax to 32 — a RANGE EXTENSION in the same family as the ADR-020 drift-depth widening, not a change to the reference arithmetic. The port's amp tilt (1/k^t), gain normalization (/(ampSum·M^0.75)) and stretch law all consume P unchanged; the spectra goldens render at partials ≤ 12 so parity RMS 0.0 stands as the regression proof. VizSnapshot's per-partial arrays and the strip visualizer grew to 32 to match. Worst-case CPU 8 voices × 32 × 7 cloud = 1792 sine oscillators — inside the ADR-006 spike headroom. Param 44 max is now 32 (append-only id unchanged).

## ADR-041 · Post-Track-E1, the parity-to-prototype contract graduates to forward performance standards — ACCEPTED (scheduled)
Human direction (2026-07-18): "the prototypes were never the final intended form, just gesturing toward it. Once we're past the Track E1 integrations we can start making modifications to the reference path… come up with a new set of performance standards and run against those instead." Recorded as a scheduled transition, not immediate. UNTIL the E1 gate closes, the standing discipline holds unchanged: reference paths stay bit-frozen, additions use the superset-with-inert-defaults pattern (parity is the regression proof), and any prototype divergence needs an ADR. AT the E1 gate, a dedicated phase (ROADMAP "Phase F — reference-path liberation") opens: (a) author the successor acceptance standard — behavioral/perceptual targets and new golden references generated from the LIBERATED implementation, versioned, replacing the "== prototype" definition of correct; (b) migrate the L0 suites from parity-to-prototype onto the new references, one engine at a time, each migration its own ADR + gate; (c) retire or re-scope the protected prototype HTMLs (they become historical provenance, no longer the oracle). The bit-parity harness is NOT deleted — it is repointed. Nothing about this ADR changes today's gates; it fixes the sequencing so the graduation is deliberate, measured, and reversible, never a silent drift off the contract that has protected correctness so far.

## ADR-042 · SPECTRA per-voice uncoupled sub-oscillator — ACCEPTED (first beyond-prototype feature)
Human request (2026-07-18), and the first DELIBERATELY-ORIGINAL addition — no prototype implements it. It ships now (not deferred to Phase F) because it is a pure ADDITION under the superset-with-inert-defaults discipline, not a reference-path modification: at subOn=0 OR subVol=0 the render's inner loop is byte-identical to the ported reference (the partial sum l·g is held in a double, then added, exactly as before), so the spectra goldens hold at rms 0.0 — parity is the regression proof, same guarantee ADR-021/025/033/035 lean on. Design (human-specified): PER-VOICE (one sub per SPECTRA swarm), one octave down (f0/2), UNCOUPLED (its own phase accumulator, outside the Kuramoto graph), following the note envelope so it starts/stops with the voice, mono (centered — a sub belongs in the middle). Three params (SPECTRA-only, ids 52-54): subOn toggle, subVol (0..1, default 0), subWave — a smooth morph sine → triangle → smooth-square (tanh-rounded, gentler than a hard clip). Level constant subGain = subVol·0.6 (taste, sits under the output tanh). Verified: with A3 held, sub-off has 1.2 units at 110 Hz (negligible), sine-sub 6066, square-sub 7183 (plus harmonics) — the intended sub-octave. state_check GREEN proves the params round-trip and restored audio is bit-identical. This ADR also OPERATIONALIZES the ADR-041 framing: "beyond the prototype" additions are already routine and safe; only reference-path MODIFICATIONS wait for Phase F. The octave-locked monophonic sub the human mentioned as a later idea is parked (#17). **Addendum 2026-07-19:** a sub-octave selector (param 55, stepped -1/-2/-3; default -1 = f0/2 bit-exact) was added — subOct feeds subFreqMul = 2^subOct; measured -1→110 Hz, -2→55 Hz at A3.

## ADR-043 · Track E1 increment 1: resonator bank on the shared force core — ACCEPTED
First audio-processing effect engine (SPEC-EFFECTS §3). src/filter_core.h ports swarmfilter.html's FilterLab VERBATIM, consuming forcecore (ADR-034) for the population dynamics (filter frequencies under home/sync/splay/gravity) and adding only the filter-specific audio: polyBLEP saw exciter (EX_DET unison), N-band TPT SVF bandpass bank, collapse→Q routing (Qeff widens with population collapse), wet/dry, tanh. Correctness contract: L0-1-style audio parity vs the JS FilterLab — filter_check reproduces 11 scenarios at RMS 0.0 (bit-exact, first build; the E0 force-core parity was the load-bearing foundation), plus the audio half of L0-14 (collapse→Q lifts effective Q from base to 30.4 under K=+1). Wired into ./verify full (verify edit rides the PR — human gate). PROTOCOL: every parity scenario pins noise=0 — FilterLab's exciter mixes Math.random() dither when noise>0, which is non-deterministic; the noise mix is ported (runtime feature intact, seeded reproducibly) but the ORACLE avoids it. SCOPE (increment 1 of E1): this is the resonator bank engine + oracle only. Remaining E1: notch swarm (swarmphaser, L0-16/17), external-audio input path in the shell, and the effect-plugin shell target (E3 territory) — each its own increment. The population halves of L0-14/15/17 stay covered by force_check; filter_check owns the audio path.

## ADR-044 · Prior-art posture: Lem/Kuroscillator lineage acknowledged; novelty claims corrected — ACCEPTED
User-sourced (2026-07-19; triaged from PRIOR-ART-DELTA_1.md, now deleted). Lem & Orlarey (CMMR 2019) plus Lem's program (SMC 2019, SMC 2022, CCRMA PhD 2022) — with Collins (ICMC 2008) and Lambert (ICMC 2012) antecedents — establish RESEARCH prior art for Kuramoto audio synthesis with the mean-field O(N) reduction, coupled sine banks, and coupled trigger clocks for rhythm. Correction recorded: any "emergent trigger synchronization is unoccupied" framing is wrong at the research level — unoccupied as a PRODUCT only. PRIOR-ART.md §6 now carries the citation set, the precise novelty posture (mechanisms + shipping instrument architecture, not the base idea), and a defensive-publication recommendation to cite the lineage rather than claim an unqualified first.
TRIAGE DISCERNMENT (per explicit human instruction to be discerning): the delta's speculative items — an Arnold-tongue / Arnold-layer modulation surface, a sample-onset gravity attractor, and a built grain engine — were EXCLUDED from PRIOR-ART as not-in-project (zero references in SPEC/ROADMAP/DECISIONS; the Arnold surface is an undecided exploration from a parallel agent's experiments). PRIOR-ART must never claim features that do not ship. Mirollo–Strogatz pulse coupling parked (PARKED #18) as the candidate grain-trigger substrate if that engine is ever built.
NUMBERING: the packet proposed this as "ADR-018," which collides with the existing bank-renderer ADR-018 (2026-07-17). Renumbered to 044 per the external-packet renumbering policy (cf. ADR-015, renumbered from packet 010).

## ADR-045 · Coupling function (Γ) and topology (W) as first-class force-core objects — PROPOSED
*(Triaged 2026-07-19 from RECOMMENDATION-coupling-topology-axes.md, archived at docs/design/2026-07-19-coupling-topology-axes.md, source deleted. Status PROPOSED — captured in the log; IMPLEMENTATION is gated on human ratification + critic review, since the lift touches the force-core seam. Two ADR cross-references in the source packet were WRONG and are corrected here per the discernment discipline: the FDN mixing matrix is ADR-030 (not "ADR-014", which is the private-sibling alias rule), and the Daido validation is ADR-015 (not "ADR-010", which is the spin-up ratifications).)*

**Context.** Sakaguchi α, Daido poles (ADR-015), and the three topologies are scattered coordinates in one 2-D design space (Γ × W) but are currently hardcoded as inline branches in the dynamics path of the unified core (ADR-023). The FDN mixing matrix (ADR-030) and the parked drawn-distribution idea (PARKED #1, a *distribution*-object precedent) show the same "pluggable pure object" pattern. The parked list is growing along both axes with no shared seam to attach to.

**Decision (proposed).** Introduce two pure, seed-deterministic, parity-tested objects consumed by the force-core, peer to the existing distribution/law objects:
- **`CouplingFunction` (Γ)** as a Fourier coefficient vector {(K_n, φ_n)}, n=1..H (H=4 covers everything shipped). Mean-field evaluation via generalized order parameters Z_n = ⟨e^{inθ}⟩ (the core already computes Z_1 = R and, on the dynamics path, Z_q = R_q). Sakaguchi = {(K_1, α)}; Daido-q = {(K_q, 0)}; sync+cluster mixes {(K_1,0),(K_2,0)} are expressible and yield bistable landscapes — new behavior falling out of an object built anyway.
- **`Topology` (W)** selecting neighbor/weight structure; present set mean_field | ring(reach) | two_cluster(μ), with the parked zoo (small-world, scale-free, modular, spatial, adaptive) as future enum additions consuming one interface. Unifies with the FDN mixing-matrix constructor family (ADR-030's "which eigenchannel does the input excite" is a `Topology` property).

**Scope guard.** This is a *representation* change, not a behavior change. The lift must preserve bit-parity against the current dynamics reference — {mean_field, ring, two_cluster} × {sin, Sakaguchi-α, Daido-q} through the new objects must reproduce their existing L0 numbers exactly (ε=1e-6), added as an explicit Layer-0 guard so the abstraction can never silently alter shipped behavior. No new Γ or W behavior ships under this ADR. New points on either axis are separate ADRs, prototype-first per ADR-003, each with its own L0 rows.

**Consequences.** Converts the coupling/topology parked entries from per-feature builds into single-object additions. Blocks nothing in flight. Earned-next payoffs it enables (NOT committed here): drawn-Γ (a user-drawn curve whose Fourier transform IS the CouplingFunction vector — the phase carpet already visualizes the result) and spatial/distance-dependent W (topology as audible stereo space). Both prototype-first when proposed.

**Assessment (triage).** Technically sound and idiomatic to the repo — the Γ×W factorization is the same move that made distribution × detune-law orthogonal, applied to coupling; the parity-guard-on-a-representation-lift is exactly the right discipline. Recommended for ratification. Sequencing note: the source pitched this "as Phase 3 finishes"; Phase 3 is now CLOSED and the code is stable, so the lift is a clean standalone refactor whenever prioritized — it does not gate E1 or Phase F.

## ADR-046 · Track E1 increment 2: notch swarm on the shared force core — ACCEPTED
Second frequency effect engine (SPEC-EFFECTS §4). src/notch_core.h ports swarmphaser.html's PhaserLab VERBATIM, consuming forcecore (ADR-034) for the population (notch frequencies) and the same polyBLEP exciter as filter_core; the audio path is a feedback allpass→TRUE-NOTCH cascade (per-section SVF y = x − k·v1, exact null at fc — the allpass-cumulative-phase topology was rejected by measurement, ADR-029b) with a tanh'd feedback loop and a NaN watchdog (ADR-032). Constant Q (1/stageQ), not collapse→Q. Correctness: L0-1 audio parity vs the JS PhaserLab — notch_check reproduces 11 scenarios at RMS 0.0 (bit-exact, first build), plus the audio half of L0-16 (on-notch attenuation 158 dB deeper than a quarter-octave-off probe — matching the reference's ~150 dB nulls, the regression guard against the allpass defect). Wired into ./verify full (now SEVEN oracle chains). noise=0 parity protocol (Math.random dither). Parity setup note: PhaserLab runs a construction-time controlTick that notch_core omits — harmless because the generator applies 'seed' last, whose rebuild wipes that step in the reference too, so pre-render state matches. SCOPE (increment 2 of E1): the two frequency engines (resonator bank ADR-043, notch swarm) now exist as parity-proven cores. Remaining E1: external-audio input + an effect-plugin shell so they are auditionable (the "testing them out" step); then L0-18 family-stability long-runs.

## ADR-047 · SWARM-FX: the effects line as a standalone audio-effect plugin — ACCEPTED
Track E1 increment 3 (the "testing them out" step, human-approved 2026-07-19). The swarm effect engines need external audio to be auditioned; the instrument can't take audio input. Decision: a SEPARATE plugin target, SWARM-FX (CLAP/VST3/AUv2, AUv2 type aufx), with its own factory/entry (src/swarmfx_clap.cpp, swarmfx_entry.*) so the instrument shell is byte-untouched. It shares the parity-proven cores headers-only (filter_core.h, notch_core.h) via a new processExternal() path added to each — same controlTick + SVF bank/cascade as the reference render(), but the dry signal is the plugin input (mono-summed) instead of the exciter; render() stays the parity-frozen path (all 7 oracle chains still green). One unified 16-param surface (engine select Bank/Notch + shared force params + per-engine Q/feedback/mix) maps onto both cores; a MIDI note-in moves the gravity center (setNoteFreq, no exciter voice). RATIONALE for engine-select-in-one-plugin (vs one plugin per effect): the human framed these as experimental FX that may not survive — one shell hosts all of them, so adding an engine is a core + a dropdown entry and discarding one is a deletion. Deliberately minimal first cut: generic host-drawn params (no webview yet), block-start param application (sample-accurate scheduling deferred). Validated: pluginval strictness 10 SUCCESS, auval SUCCEEDED (aufx), installed; headless check confirms both engines process external audio (finite, non-silent, spectrally shaped). SCOPE: this is the effect-plugin SHELL + external-audio path. Remaining E1: L0-17 audio (tuned harmonic rejection) + L0-18 family-stability long-runs as oracle rows; a webview GUI for SWARM-FX; then E2 time engines slot into the same shell. E3 ("effects as sections inside the instrument") is a later, separate integration.

## ADR-048 · Swarmalator engine ingested + ported (experimental) — ACCEPTED
Human drop (2026-07-19): SPEC-SWARMALATOR.md + swarmalator.html, "a new engine to test out." The swarmalator couples audio phase θ and spatial angle ξ TO EACH OTHER (ring swarmalator, O'Keeffe-Hong-Strogatz 2017), so pan is a state variable of the same system that makes the tone — the stereo/spectral unification, on-thesis. Mean-field-reducible: two compound order parameters W± = <e^{i(ξ±θ)}> (two extra complex accumulators per tick, O(N)); K = ordinary Kuramoto sync (phase axis == SAW at J=0), J = the cross coupling. Ingested as spec-in-code per ADR-003; swarmalator.html + SPEC-SWARMALATOR.md added to protected paths (7th prototype). Ported src/swarmalator_core.h verbatim (stereo out — pan from ξ — unlike the mono effect cores; shares mulberry32 via forcecore per ADR-034; K-norm reuses ADR-004). Oracle: gen_swarmalator_goldens.mjs + swarmalator_check — stereo L0-1 parity RMS 0.0 on 9/9 scenarios (first build), plus the SPEC §5 acceptance anchors (sync R=0.966 vs spec 0.97; splay R=0.056; rainbow max(R±)=0.515 with R incoherent; sync+rainbow both raised; stability peak 0.927 NaN-clean). Wired into ./verify full (eight oracle chains). TRIAGE (L0009): the packet's ADR cross-refs were wrong and corrected on ingest — Daido tick-holding is ADR-015 (packet said "ADR-010"); the ingest/design ADR is this one, 048 (packet said "ADR-046", the notch swarm here); prior-art posture is ADR-044 (packet said "ADR-018"). STATUS: EXPERIMENTAL — the human is testing the effects line and this in parallel; may not survive, may be joined by other new engines. Under ADR-045 (PROPOSED) it is a point in the (Γ, W) space (spatial ring topology × two-term coupling), and it is the parked grain-swarm's spatial dynamics as a special case (J on position). NEXT: shell integration for auditioning (instrument engine-select, or its own target) after the human's first read of the effects + this. No shell wiring yet — core + oracle only, the parity foundation.

## ADR-049 · Track E2: time engines (tap-swarm delay + FDN room) + SWARM-FX integration — ACCEPTED
Human direction after the frequency effects underwhelmed ("Let's try the time engines"): delays/reverbs are intrinsically effect-shaped, so the time engines are the effects line's better bet. src/time_core.h ports swarmtime.html's TimeLab VERBATIM on forcecore (the population being DELAY TIMES, log2 seconds): ECHO mode = tap-swarm delay (N taps on one buffer, feedback of the tap SUM DC-blocked+damped; /N feedback norm per the worst-case-correlation stability law, not √N), ROOM mode = FDN room swarm (N lines, NEGATED Householder h·s−outs[i] so the input-excited ones-eigenchannel carries +1 → comb at k/L, ADR-030a; per-line damping LP + DC blocker). Gravity attractors: rhythmic ratios×beat (echo), period-multiples (room). NaN watchdog self-heals feedback state (ADR-032). Oracle: gen_time_goldens.mjs + time_check — L0-1 parity vs the JS TimeLab (both modes), worst RMS 5.6e-12 (NOT bit-exact like the non-feedback engines: transcendental-ulp of tanh accrues in the feedback loop, but 5.6e-12 << the 1e-6 eps — same class as parity_check's dyn-ring at 4.3e-9), plus L0-19 echo LF stability (12 s at regen 0.5/0.97, tail mean|amp| 0.073/0.075 <= 0.15 — the √N-runaway ref was 0.42+), L0-20 room resonance, L0-21 room 12 s at regen 0.95 bounded + DC-controlled. verify full = NINE oracle chains. TESTABLE NOW: both wired into SWARM-FX (ADR-047) as engines 2 (Tap Delay) / 3 (FDN Room) — the unified param surface reused (Center→size, Resonance→regen, Feedback→damp), MIDI note moves the rhythmic/period gravity center; pluginval 10 + auval SUCCEEDED, installed. Residual (noted): mono core (stereo delay/reverb is the obvious refinement if they survive); host-tempo sync for the rhythmic gravity (bpm fixed 120 for now); block-start params. This closes the E1/E2 effect-engine PORTS; remaining before an effects gate: human listening verdict on which survive, then stereo + GUI + host-tempo for the survivors.

## ADR-050 · Stereo effect path for the time engines; Foxfire prior-art correction — ACCEPTED
Human direction (2026-07-19) after preferring the time engines: "Let's definitely expand into stereo." Added time_core.h processExternalStereo() — mono-summed into the delay/room network (feedback logic mirrors processSample exactly), but each tap/line panned across the field by index × a new `stereo` param (constant power), so the swarm of delays/room-modes spreads L↔R; dry keeps the input's own stereo, wet is the decorrelated field. DELIBERATE DIVERGENCE from the mono reference (swarmtime.html outputs mono) — legitimate because the effects are EXPERIMENTAL (the human explicitly wants license to experiment) and the mono reference path (render()/processExternal()) stays byte-frozen so time_check still guards it (parity worst 5.6e-12 unchanged). SWARM-FX param 17 "Stereo Width" (append-only) → time.stereo; the time engines route to processExternalStereo. Measured: L/R correlation 1.0 at width 0 (mono) → 0.69 echo / 0.79 room at 0.8. pluginval 10 SUCCESS, installed. Stereo is currently time-engines-only (the survivors); frequency-engine stereo is deferred to the "reconsider them" work. Residuals: host-tempo sync (bpm fixed 120); block-start params; per-tap pan is by INDEX (rank), not by the swarm's live state — a spatial-by-delay-value pan is a candidate refinement.
PRIOR-ART CORRECTION (same session): the human surfaced Chiral Audio's Foxfire — a SHIPPING 16-voice Kuramoto chorus (coupled LFOs modulating delay taps). This triggers the falsifier the "no shipping product" market claim carried; PRIOR-ART §1/§6 corrected to retract the flat claim and sharpen it to "no product uses coupled-oscillator dynamics as the timbre/synthesis engine" (Foxfire is a modulation effect, not a phase-transition synth). ROADMAP gains a Kuramoto-chorus engine idea (coupled modulation LFOs on short delays — distinct from the force-herded tap-swarm delay), prototype-first, citing Foxfire.

## ADR-052 · Entangled-mods proposal: oracle model for net-new dynamical cores; Phase A accepted — ACCEPTED (direction), phases gate individually
Human design proposal (2026-07-19, `docs/proposals/2026-07-19-kuramoto-entangled-mods.md`, archived from root per ADR-011's no-loose-design-files rule). Four phases: A observable extraction (R/ψ/drift/R₂/lock-ratio/slip-events as a smoothed, unwrapped mod-source bus), B coherence-budget coupling (a conserved [0,2] budget makes G₂'s *effective K* — not its gain — anticorrelate with G₁'s R), C membership spinors (per-voice (a,b)∈ℂ² with equal-power two-path render, phase carried over so a migrated voice beats against its new ensemble; C.1 stochastic tunneling, C.2 Pareto blinking, C.3 coherent Rabi), D measurement bus (slip/note/transient events collapse spinors by Born rule, then slerp-relax back). Honest framing kept: classical coupled oscillators cannot literally entangle — these are *structural analogues* (shared-state correlation, conserved budgets, measurement-as-conditioning, population transfer), and that honesty stays in README/PRIOR-ART per living-README doctrine.

GROUNDING (L0009 — every repo-state claim checked before it shaped ROADMAP): §2's mean-field assumption HOLDS (swarm_core.h:706 is `KsmS·R·sin(ψ−θᵢ−α)`, R/ψ at 664-665 — no O(N²) migration needed). Phase A's sources are ~70% already computed each control tick (R, ψ, RN, RQ, RA, RB) but only as VIZ readouts, not a published bus with unwrap/smoothing/slip-events — the bus is the real new work. §8.3's open "control tick rate 1-4 kHz" is self-answered: `controlTick` already runs every 16 samples = 2756 Hz, in-band — reuse it, don't invent a rate. §8.5 "budget acts on K not gain" already matches the core's coupling/output separation. MAPPING: Phase A = the source layer of the Phase-5 mod matrix (ROADMAP:68-69) and the Kuramoto LFO (under active human prototyping); Phase B = the CROSS-COUPLED OSC2/OSC3 variant (ROADMAP:53, "swarm-of-swarms," PARKED #5); Phases C/D = genuinely net-new, no precedent.

DECISION — oracle model (extends, does not weaken, ADR-003). Net-new dynamical cores with no external prototype follow PROTOTYPE-FIRST: build an in-repo HTML lab for the audible mechanisms (esp. C's membership rendering / phase-carry-over beating — §8.2, a by-ear choice) so the human can A/B before porting; the proposal's analytic + statistical criteria (norm conservation <1e-9 over 10⁶ ticks, Born-rule frequency within binomial bounds, deterministic replay, slip-rate-peaks-near-K_c) become the C++ PORT oracle, added as Layer-0/Layer-E rows. Parity stays the primary contract wherever a prototype exists; analytic oracles SUPPLEMENT it for inherently-statistical mechanisms — they never replace parity to make a red gate pass. Human ratified prototype-first (design Q, 2026-07-19).

FLAG (unresolved, human's call — NOT decided here): §8.1 G₂ identity is ambiguous — "modal bank from the spectral resonator (gravity lens)" could mean HYPERSAW's own filter_core (E1.1) / consonance gravity (ADR-008), OR the terrain sibling (private). The proposal itself flags it cross-project ("writes stay home" → brief→response, not direct import); if it is the sibling, it is INTEGRATIONS.md territory and the sibling's real name never enters a tracked file. §8.2 (power vs amplitude render), §8.6 (drift gating by R) are design-session items. ACCEPTED NOW: Phase A into ROADMAP as the mod-matrix source layer (rides existing prototypes, gated prototype-first per this ADR); B/C/D remain forward/parked design items pointing at the archived proposal, each ratified and sized individually when reached — realistically ~6-7 gated phases, sequenced behind the Kuramoto LFO since A is its source layer.

## ADR-053 · Kuramoto LFO design accepted (routable mod primitive, rotor-first); prototypes are NON-GOLDEN — ACCEPTED (design), port gated on a golden rotor
Human design brief (2026-07-20, `docs/proposals/2026-07-20-kuramoto-lfo.md`, end-of-prototyping report for the modulation thread; the Kuramoto LFO the ROADMAP mod-matrix item has awaited). Four concept-test prototypes attached (archived non-golden at `docs/design/kuramoto-lfo-prototypes/`, see its NON-GOLDEN.md). DESIGN DECISION accepted: build the Kuramoto LFO as a **routable modulation primitive** (per-voice swarm-coordinate LFO population published to the mod bus), NOT a hardwired chorus — the chorus is one demo destination. Ship the **rotor** first (4 phase-coupled LFOs → morph/cutoff/chorus/saturation, bipolar K, shape selector, rotor viz), then add rate → depth → destination axes behind it, one at a time, each a routable extension with its own Layer-0 rows. This is the generator side of the ADR-052 mod matrix; ADR-052 Phase A (audio-swarm observables) is the emergent-source side — complementary, same bus.

GROUNDING (L0009 — prototype code read, packet claims corrected): the packet's §4 "consuming the existing force-core" is IMPRECISE and the imprecision is load-bearing. Two coupling domains, axis-dependent: the **rotor** (shipping face) is PHASE-domain Kuramoto (`R=√(cx²+cy²)/NV`, `psi=atan2`, bipolar `±K²·3`) — reuses SwarmCore's law, NOT force_core; the rate/depth/dest **axes** are POSITION-domain springs on log2(rate)/depth/destination — force_core's domain. This ANSWERS the ROADMAP's open "reuse force_core or a dedicated phase population?" question: BOTH, axis-dependent, and the audible spine is always phase (recommendation lesson (d): rate coupling was inaudible until phase coupling rode alongside it — the ear tracks phase coherence, not rate coherence). Bipolar-K → splay differentiation from Foxfire's unipolar macro confirmed in the rotor code; both prior-art finds (Lem/Kuroscillator, Foxfire) already in PRIOR-ART.md (posture ADR-044 here — the packet's "ADR-018/048" is wrong-for-repo).

NOT GOLDEN (human override): the packet self-describes the prototypes as "each a parity oracle / headless-verified / reproduce those numbers per ADR-003"; the human directly stated they are "not golden, merely tests of the concepts." The human's word governs — these are NOT ingest-and-port references. Port gate (ADR-003 / ADR-052 prototype-first): the chosen concept (rotor) must first be hardened into a golden reference (headless-extractable core + measured anchors + the multi-LFO-cycle mod-test rule) then ingested like every engine. No port, no code this ADR — design + archive only. ADR-number collision noted: the packet's 048–052 are the authoring kit's numbers and all collide here (this repo's 052 = entangled-mods, ADR-052); ignore the packet's numbers entirely.

CARRIED LESSONS (recommendation §5 — banked for WHEN the mod subsystem is built, not written to LIBRARY now: inherited-but-unverified-here per the write gate): (a) constructor-initialize every viz-read field (render thread outruns audio thread at startup); (b) place a modulated parameter's resting point inside the signal's active range, sweep symmetric in the perceptual domain (log for freq); (c) **measure modulation over ≥3–4 full LFO cycles, never a single block** — per-block RMS/correlation/centroid is invalid below audio rate (this will DICTATE the shape of the mod acceptance oracle); (d) couple the audible quantity alongside a one-step-removed swarm coordinate — the modulator-domain analogue of ADR-016. CORRECTED FINDING (§6, self-falsified): "thin voices to graduate comb-resonance PITCH" was a spectral-centroid/brightness shift misread as pitch by a zero-crossing proxy (2821→2522 Hz was timbre, not pitch — same class as L0002 and the squareness even/odd confound). Do NOT build that control; if graduating resonance pitch is wanted, modulate base tap spacing directly.

## ADR-051 · Two-cluster A/B balance knob; swarmdynamics reference updated — ACCEPTED
Human proposal (2026-07-19, PROPOSAL-cluster-balance.md + swarmdynamics_AB.html clone). Adds one control — A/B balance in [0,1] — to the two-cluster topology: cluster A keeps intra-coupling gain 1, cluster B is scaled by kB = 1 − 2·balance, so one knob sweeps symmetric (balance 0, kB=1) → B uncoupled (0.5, kB=0) → B splayed (1.0, kB=−1), reaching the broken-symmetry state (A coherent, B dissolved) that previously required hunting the (alpha, mu, detune) corner. INGEST (ADR-011/012 reference-update pattern): verified the clone is ADDITIVE before ingesting — extracted both DynSynth cores headless, balance=0 reproduces the current reference BIT-IDENTICALLY (worst |delta|=0), change confined to the two-cluster branch. The clone replaced swarmdynamics.html (old recoverable in git; protected-path edit is the human's proposal + ratify-by-merge). PORT: swarm_core.h two-cluster branch gains kB (default balance 0 = bit-inert); shell param 56 "A/B Balance" (append-only), GUI row gated two-cluster-only. ORACLE: new golden `dyn-cluster-balance` (balance 0.5) proves the port matches the reference on the NEW code path (parity now 54/54, worst 4.262e-09 unchanged); L0-23 (ACCEPTANCE) enforces the behavioral anchors (R_A pinned, R_B dissolves, split>=0.4/0.5, mu sets floor). PROPOSAL NOTES honored: K is UNIPOLAR here (4·K²·σ) so balance is the only cluster-splay axis (K-sign×balance corners collapse to two) — verified; the per-cluster-normalization refinement is DECLINED (the reference uses global norm; parity to the reference takes precedence — flagged for Phase F if the effect drifts). The optional bipolar-balance extension (kB>1, B over-coupled) is left parked. Addresses PARKED #16 (per-cluster two-cluster controls).

## ADR-054 · Internal FX rack with a routing grid; one rack, two shells; placeholder-first — ACCEPTED (architecture)
Human direction (2026-07-20): the instrument makes an extraordinary morph across one XY sweep (orchestral swell → organic texture → alien metallic) *when placed behind the right saturation + wet reverb*; bring that inside. GROUNDING: most of the DSP already exists — delay (time_core ECHO), reverb (time_core FDN ROOM, both human-approved, ADR-049/050), filter (filter_core), notch (notch_core), all on force_core, all hosted by the standalone SWARM-FX shell (ADR-047). This direction is therefore mostly the E3 INTEGRATION (effects as post-oscillator sections) plus ONE genuinely new engine (saturation — today only the master tanh, swarm_core.h:419) plus the mod-aware/experimental layer.

DECISIONS (ratified by the human's three answers):
1. **Routing GRID, not a fixed chain — prioritized ASAP.** Order of FX is expected to be particularly significant to this instrument, so the rack ships with reorderable/routable slots from the start, not a hardcoded sat→delay→reverb order. This is the near-term build target.
2. **One rack, two shells.** Everything packages inside the main synth (internal FX sections); the standalone SWARM-FX plugin then offers *all of it* — the same rack over shared cores (E3's "same cores, two shells," now the same *rack*), possibly with a **MIDI sidechain input** for anything that needs note context (e.g. consonance-gravity-centered engines whose attractor tracks incoming MIDI, as the time engines already do via setNoteFreq).
3. **Placeholder-first.** Build the rack + routing grid + the mod-destination wiring against TRIVIAL placeholder FX to get the architecture and feel right, THEN refine each slot / drop in the real cores (filter/notch/time) and the new saturation engine. Reduce-first: the routing/UX/param-plumbing is the risk, not the DSP (which is done); prove it with placeholders before investing in each engine.
4. **Saturation engine** is the one new DSP: `drive → shaper → tone → level`, where the pre-shaper DRIVE is the lever the squareness experiment (2026-07-20, scratchpad) already identified — the master tanh is barely engaged at normal volume, so a driven saturation section is what unlocks the aggressive/metallic end of the morph (and makes the K→square transition musically reachable). Not a generic waveshaper bolt-on; the drive is the point.
5. **Mod-aware / experimental FX deferred behind the mod matrix (human accepted the recommendation).** First: FX params as ordinary mod destinations (XY / Kuramoto LFO / velocity sweep drive/size/feedback — exactly the by-hand discovery, made internal). Later (needs the mod bus to exist, ADR-052 Phase A + ADR-053 rotor): FX driven by the synth's OWN emergent observables (reverb size ← R, delay feedback gated by slip, drive ← σ) and/or FX cores CROSS-COUPLED to the carrier swarm (the effect as part of the same dynamical system — the swarmalator-unification thesis one level out, and where the truly novel engines live). E3's "collapse/comb-regularity/in-basin-error as mod sources" is this layer.

SEQUENCING: the routing grid + placeholder rack is the near-term target (per (1)); the mod-aware layer is gated on the mod-matrix work, so the rotor-to-golden step (ADR-053, PR #55 spec) remains the highest-leverage unblock. No code this ADR — architecture + roadmap only; E3 expanded accordingly. STILL OPEN (not decided): global-bus vs per-voice FX placement (per-voice saturation-before-sum is a distinct, thicker sound — flagged for the build), and which experimental engines to attempt first.

## ADR-055 · SPECTRA ADSR as a parity-safe superset defaulting to SPECTRA's OWN reference AR — ACCEPTED
Human request (2026-07-20): the shell's Attack/Decay/Sustain/Release knobs (ids 19-22) route to the SAW core but were DEAD in SPECTRA mode — SpectraCore had no ADSR, running a hardcoded AR (spectra_core.h: `atk = 1-exp(-1/(0.004·sr))`, `rel = 1-exp(-1/(0.18·sr))`). This adds ADSR to SpectraCore, mirroring ADR-021's superset-with-inert-default exactly.

THE PARITY WRINKLE AND THE RULING. SPECTRA's reference (swarmspectra.html) has NO envelope — just a fixed AR whose constants (4 ms attack / 180 ms release) DIFFER from the SAW reference's (3 ms / 160 ms; ADR-021). So SPECTRA's ADSR must default to ITS OWN constants (attackS=0.004, releaseS=0.18, sustainL=1.0) for the current AR to reproduce bit-exactly. At sustainL >= 1 the render loop takes the reference's exact expressions ((1-env)·atk gated, (0-env)·rel released) with the SAME operands/doubles as the former hardcoded AR — so the spectra goldens are the regression proof, unchanged. sustainL < 1 enters the attack→decay machine (new, deliberately divergent), identical in shape to ADR-021. Prototype-first (ADR-003): CHECKED — swarmspectra.html has no envelope to mirror, and the ADSR SHAPE added is byte-identical to the already-ratified ADR-021 superset (not a new divergent envelope), so no new prototype is required; the reference AR is the base, the ADSR the accepted extension. This is a plugin-surface superset (ADR-020), recorded here because it touches the DSP core.

OWN PARAMS, NOT THE SHARED 19-22 (the load-bearing decision). SPECTRA gets its own ADSR param ids 65-68 (sAttack/sDecay/sSustain/sRelease, SPECTRA-only, append-only) — it does NOT read the shared shell ADSR (19-22, which stay SAW-only). Sharing 19-22 was REJECTED on a parity-integrity argument: the shell's `attack` param defaults to 0.003 (SAW's constant), and if it routed into SPECTRA it would push 0.003 into the core at plugin load, silently shifting SPECTRA's default AR off its 4 ms reference — and NO oracle would catch it, because spectra_check constructs SpectraCore DIRECTLY (bypassing the shell) and renders at the core's own SParams defaults, so the goldens would stay green while the shipped plugin diverged. Under the epistemic-discipline doctrine (a comfortable green that doesn't test the real path is the dangerous case), the fix must make BOTH the goldens and the shipped plugin reference-exact by construction. Own params with own reference defaults (0.004/0.18) do exactly that. This also follows the ADR-042 precedent (SPECTRA's sub-osc took its own ids 52-55) and honors ADR-037's framing: the two references are separate frozen programs with genuinely different envelope constants, so each engine carries its own envelope.

WIRING. SSwarm gains `inAttack` (re-armed per strike, unread on the reference-exact path); SParams gains attackS/decayS/sustainL/releaseS; paramSlot maps sAttack/sDecay/sSustain/sRelease (distinct names so shell id-19 never leaks into SPECTRA via the shared-name mirror). Shell: ParamDefs 65-68 with SPECTRA reference defaults, set/read routed to the spectra core alongside 44-55. State: unique coreKeys round-trip via the existing kParams loop; old saved states load with SPECTRA's reference defaults (additive schema). GUI: rows 65-68 added to the Envelope cluster, gated SPECTRA_ONLY (19-22 already SAW_ONLY) — the one cluster shows the engine-appropriate ADSR, same pattern as the sub-osc. Regression proof: spectra_check parity (goldens) + P=1 gate (env-independent R-trajectory) stay green; state_check proves the new params round-trip.

## ADR-056 · Bipolar onset lock (signed Kenv; onset<0 = splay burst) — ACCEPTED (superset, human-approved direct implementation)
Human request (2026-07-20): make onset lock bipolar, asking whether dissolve would still return it to neutral. Onset lock is a note-start coupling burst (Kenv, initVoice) that ADDS to sync and DECAYS over `dissolve` (controlTick) back to the steady K — but it was unipolar because Kenv = 8·onset² is always ≥0. Change: `Kenv = 8·onset·fabs(onset)` (signed quadratic), and route the sign — `syncT += max(0, Kenv)`, `splayT += max(0, −Kenv)·3` (the ×3 matches the steady splay gain). So onset>0 is the existing sync burst; onset<0 is a SPLAY burst (voices start spread, then settle). Param range widened 0..1 → −1..1 (shell id 7 + GUI).

PARITY (superset, bit-inert default): for onset ≥ 0 (the entire former range) fabs(onset)==onset so Kenv is byte-identical to 8·onset², and max(0,Kenv)==Kenv / max(0,−Kenv)==0 reduce the routing to the reference's exact `(max(0,km)+Kenv)` / `max(0,−km)·3` — verify full stays green (parity 54/54, all 9 chains). onset<0 is NEW C++-only behavior with no JS reference — same posture as ADR-025 super-width (reference range bit-untouched, the extension validated behaviorally). Divergence acknowledged per ADR-003; human approved direct implementation ("throw it in, we can revert" — the change is a signed fabs + one sign-routed term, trivially revertible).

ORACLE: L0-24 behavioral anchor in trajectory_check (ADR-056 block): splay-onset early R < sync-onset early R by a clear margin (got 0.92 — the burst direction is unmistakable); dissolve neutralizes (late |ΔR| < 0.06 at K=0.9). FINDING surfaced by the anchor (answers the human's dissolve question honestly): dissolve returns the COUPLING boost to neutral (Kenv→0), but near-critical K the splay-vs-sync initial condition can leave the swarm in different basins even after Kenv→0 — genuine path-dependence/multistability, NOT a failure. So "dissolve returns to neutral" holds for the coupling; the resulting R is path-dependent near criticality and guaranteed-convergent only at strong K. The anchor checks convergence only in the guaranteed regime. TUNING OPEN: the ×3 splay-onset gain is a first guess (perceptual symmetry with steady splay); adjustable by ear. ACCEPTANCE L0-24 added.

## ADR-057 · Transposition (octave/semi/fine/pitch) extended to SPECTRA — ACCEPTED (parity-safe superset)
Human sweep request (2026-07-20): "more routings from SAW can extend to SPECTRA, like the pitch controls." AUDIT of SAW_ONLY vs what spectra_core supports: the shared coupling knobs (K, onset, dissolve, seed, vol, retrig, width) already route to both; the clear gap is TRANSPOSITION — the shell's octave/semi/fine/pitch (ids 35-38) fold into one `tune` factor (updateTune) that was applied to `core` ONLY, and spectra_core had no `tune` param, so SPECTRA was pitch-locked to the played note. Fix: spectra_core gains `double tune = 1.0` (paramSlot key "tune"); partial freqs and the sub-osc multiply `s.f0 * p.tune`; updateTune now sets tune on BOTH cores; ids 35-38 leave SAW_ONLY (shown in both engines). PARITY: tune 1.0 is bit-inert (f0*1.0 == f0 in IEEE), so spectra_check stays GREEN (worst rms 0) and every golden is untouched — verify full 9/9 green. NOT a reference divergence in the dynamics sense (it's a frequency scale, always 1.0 in the swarmspectra.html reference). SWEEP VERDICT for the rest (recorded for the follow-ups): voice mode mono/glide/legato (32-34) → needs glide/retarget in spectra_core (bigger, deferred); MPE per-note bend → spectra_core needs per-voice noteTune (deferred); drift/rtone/scatter/panScatter → core additions, lower value; dynamics (topo/poles/α/μ/grav/absK), n, dist, law, digital, inertia, detune-law → structurally SAW-only (SPECTRA has no phase-swarm topology). Also this change ships a GUI fix: the phase-circle viz box's helper-text lines (gravline/beatinfo) were min-height and grew the box when text arrived, bumping the vizcol; now fixed heights (18/34px) lock the box.

## ADR-058 · SAW waveshape morph (saw ↔ band-limited square) — ACCEPTED (parity-safe superset)
Human request (2026-07-20): a parameter that reshapes the saw waves in real time — "other waves but not sine" — even short of a full wavetable, to hear how non-saw waves behave in the swarm as they transform. Implementation: one `shape` knob (0..1, id 69, SAW-only). The osc already computes a polyBLEP saw `v`; the morph subtracts a HALF-CYCLE-OFFSET polyBLEP saw: `v = saw(ph) − shape·saw(ph+½)`. At shape 0 → saw; shape 1 → `saw(ph) − saw(ph+½)` which is a SQUARE by construction; in between, a continuum of asymmetric pulse-like hybrids. Both saws carry the SAME polyBLEP correction, so the morph stays anti-aliased (a real quality win over waveshaping a band-limited saw, which would re-alias). PARITY: guarded `if (shape > 0)`, so shape 0 is the reference path untouched — parity 54/54, all 9 chains green, build clean. shape>0 is C++-only (no swarmsaw.html reference — same posture as ADR-025 super-width / ADR-056 bipolar onset); its correctness is the provable identity `saw−saw(ph+½)=square` plus a trajectory_check bounded/NaN-clean guard (L0/ADR-058 block) at shape 0.5 and 1.0. The ×16-voice cost of the second polyBLEP is paid only when shape>0. SCOPE (first pass, deliberately minimal): one saw↔square axis. FOLLOW-UPS (noted): a pulse-WIDTH knob (offset ≠ ½ → PWM/pulse family) and triangle (integrate the square) are cheap extensions of the same two-saw machinery. Revertible: delete the guarded block + the param.

## ADR-059 · Inertia knob taper: tune-then-lock dev slider — ACCEPTED (dev tuning aid, temporary)
Human report (2026-07-21, correcting the Phase-2 E-1 caveat): the inertia knob's response is steep just after 0 — a small move off 0 has an outsized effect — pronounced at low detune + retrigger-on (voices start synced + near-identical, so the underdamped momentum system rings coherently even at small w; the ADR-024 sqrt taper compressed the audible-hunt onset to the bottom of the knob). The response is a CONTINUOUS taper problem, not a bifurcation, so reshaping the knob→w map fixes it — but the perceptually-right curve is an ear call, not a derivable one. So: a TUNE-THEN-LOCK dev control. The inertia knob taper becomes `w = knob^curve` (id 70 "Inertia Curve (dev)", 0.3..5, default 0.5). At 0.5 it uses `sqrt` EXACTLY (special-cased — pow(x,0.5) is not bit-identical to sqrt(x)), so the default feel is unchanged; higher curve → smaller w at low knob → gentler onset. The human dials it by ear, reports the value where the response feels even, and that value gets HARDCODED (taper replaced, id 70 + slider removed). PARITY/SAFETY: the taper is SHELL-side (applyParam), not the core — parity_check (which sets core inertia directly) is unaffected regardless; verify full 9/9 green, state_check confirms the param round-trips, pluginval 10 SUCCESS. Pre-1.0 so the temporary id (70) is acceptable churn. Same tune-then-lock mechanism is offered for the K cloud→order edge (~0.6-0.8) if wanted. Trace: 2026-07-21-inertia-curve.md.

## ADR-060 · Tone tilt folded from the detune lab into the SAW reference + core — ACCEPTED (parity-safe superset)
First fold from the detune-lab audition (docs/design/detune-lab.html) into the real thing, chosen as the pipeline-primer (human: "safe superset first", 2026-07-23). Tone tilt is a bipolar per-voice one-pole filter: `tilt>0` darkens (LP), `tilt<0` thins/removes low end (HP), cutoff rising as √(f/f0) so higher voices lose more top; `tilt=0` is inert. REFERENCE-FIRST per ADR-003: transcribed into swarmsaw.html (the oracle) FIRST — new `tilt:0` param, per-voice `vlp`/`vlpc` state, the coeff loop after σ, the one-pole in the render voice loop, `vlp` reset on note-on — then mirrored bit-for-bit into swarm_core.h (param, Swarm arrays, ctor init vlpc=1, initVoice reset, controlTick coeff, render application after the ADR-058 shape block, paramSlot "tilt", `tiltHP` member). PARITY: two new golden scenarios (tilt-dark +0.6/K, tilt-thin −0.6) × 3 seeds; `parity_check` 60/60 within ε (the 6 new at **rms 0.000e+00** — C++ matches JS exactly), the pre-edit 54 unchanged (verified bit-identical vs origin/main before the C++ side), all 9 chains green, notefuzz green. The taper is `H = tilt>0 ? 2·200^(1−|t|) : 0.1·24^|t|` (smooth onset from bypass — no jump). NOT YET a CLAP param: this ADR folds the DSP + parity only; exposing `tilt` on the frozen CLAP surface (a public-interface change) is the next gated step. Trace: 2026-07-23-fold-tone-tilt.md.

## ADR-061 · Hi-tame equal-loudness roll-off folded into the SAW reference + core — ACCEPTED (parity-safe superset)
Second lab→core fold (after ADR-060 tone tilt), continuing the map's superset-first sequence. Hi-tame is a per-voice equal-loudness gain: each voice is scaled by `(f0/f)^hiTame`, turning the higher voices down so a tall detuned/harmonic stack isn't harsh up top; `hiTame=0` is inert (guarded — no multiply). REFERENCE-FIRST per ADR-003: swarmsaw.html edited first (new `hiTame:0` param, per-voice `hg` gain array init 1, the gain computed after the tilt coeff loop, applied `v *= hg[i]` after the tilt in the render voice loop), then mirrored bit-for-bit into swarm_core.h (param, `Swarm.hg`, ctor init 1, controlTick compute, render application after the tilt, paramSlot "hiTame"). No note-on reset — hg is recomputed each control tick, not stateful. PARITY: one new golden scenario (hi-tame, hiTame 1.0 / detune 0.6 / n 12) × 3 seeds; `parity_check` 60/60 → **63/63** within ε (the 3 new at **rms 0.000e+00**, C++ == JS exactly), the pre-edit 60 verified bit-identical vs origin/main (incl. with tilt active), all 9 chains + notefuzz green. NOT YET a CLAP param — DSP + parity only; the CLAP surface is one batched pass at the end of the fold (human cadence choice, 2026-07-23). Trace: 2026-07-23-fold-hi-tame.md.

## ADR-062 · Drift modes + keep-phase folded into the SAW reference + core — ACCEPTED (grouped parity-safe supersets)
Third fold (human cadence: group low-risk supersets, 2026-07-23). TWO inert-default features in one PR, each proven individually. (a) **Drift modes**: `driftMode` 0 walk (the original 1/f random walk) / 1 sine (per-voice decorrelated LFO) / 2 sample&hold (stepped random); mode 0 is bit-identical to the prior drift. (b) **Keep-phase**: `keepPhase=1` makes note-on continue from the last-sounding phases (a per-block `lastPhase` snapshot of the focus swarm) instead of retrig/random; 0 inert. REFERENCE-FIRST (ADR-003): swarmsaw.html edited (params, `driftPh`/`driftHoldT` state, `lastPhase`, the drift-mode branches, the note-on phase precedence `keepPhase ? lastPhase : (retrig ? 0 : rand)`, the render-tail snapshot), then mirrored into swarm_core.h — with keep-phase placed ABOVE the core-only `scatter` branch (ADR-033) to match swarmsaw's precedence, so both engines skip rngNext identically when keep-phase is on (the rngState must stay in lockstep for subsequent drift). PARITY: 3 new goldens (drift-sine, drift-sh, keep-phase-with-drift) × 3 seeds; `parity_check` 63/63 → **72/72** within ε — drift modes **rms 0.000e+00**, keep-phase ≤ 6e-18 (sub-ULP, one seed; the rngNext-skip stays exact). Inertness proven first vs origin/main (defaults / drift-walk / gauss+tilt bit-identical at the defaults). All 9 chains + notefuzz green. Harness note: the golden harness fires chords, not note SEQUENCES, so the keep-phase golden starts from an empty `lastPhase` (phase 0) — it exercises the branch + the rngState-skip, not the non-zero continuation (that is lab-verified). NOT YET CLAP params (batched at fold end). Trace: 2026-07-23-fold-drift-modes-keep-phase.md.

## ADR-063 · Opt-in frequency glide (de-zipper) folded into the SAW reference + core — ACCEPTED (parity-safe superset)
Fourth fold. The lab's frequency smoothing is ALWAYS-ON, which would have been a behavioural change to the reference (it shifts drift and slider-sweep transients and could move the trajectory anchors) rather than an inert superset — so it was excluded from the ADR-062 group and folded here as **OPT-IN** instead (human decision, 2026-07-23: "opt-in approved, for now"). `freqGlide` is a time constant in **SECONDS** per ADR-009 (seconds in, coefficients out), default **0 = off = bit-identical**. Two legs, mirroring the lab: (a) a control-rate one-pole on each voice's target frequency, coefficient `1 − exp(−dt/freqGlide)` with `dt = kTick/sr`; (b) a per-sample slew on the oscillator frequency at a quarter of that time constant, `1 − exp(−1/(freqGlide·0.25·sr))`, which removes the residual control-rate stair the coarse leg leaves. Both SNAP on note-on (`vfInit`), so a new note starts dead on pitch instead of gliding in from the previous one. REFERENCE-FIRST (ADR-003): swarmsaw.html edited (param, `vfSm`/`fRun`/`vfInit` state, the conditional smoothing in the frequency loop, the note-on snap, the per-sample leg in render), then mirrored bit-for-bit into swarm_core.h (+ paramSlot "freqGlide"). EFFICACY measured on the steppiest source (sample&hold drift at full rate): per-tick voice-frequency jump **9.26 Hz → 0.54 Hz (94 % smaller)**. PARITY: one new golden (freq-glide, freqGlide 0.006 against S&H drift) × 3 seeds; `parity_check` 72/72 → **75/75** within ε, the 3 new at **rms 0.000e+00** (C++ == JS exactly); inertness proven first vs origin/main (defaults / drift-walk / drift-sine / keep-phase+tilt all bit-identical at freqGlide=0); all 9 chains + notefuzz green. "For now" is recorded: if the always-on behaviour is later preferred, that is a separate ADR with re-measured trajectory anchors. NOT YET a CLAP param (batched at fold end). Trace: 2026-07-23-fold-freq-glide.md.

## ADR-064 · Pan motion + centre pin folded into the SAW reference + core — ACCEPTED (grouped parity-safe supersets)
Fifth fold; the two are grouped because they are coupled — the centre pin scales pan motion as well as drift. (a) **Pan motion**: `panMotion` depth + `panMode` (0 = independent per-voice LFO drift, 1 = one shared sweep of the whole image); slow LFOs displace each voice's signed base pan, recomputed once per block, then re-encoded to the constant-power L/R gains. (b) **Centre pin**: `motionCenter` scales BOTH drift and pan motion by each voice's normalised distance from the fundamental (`cdist = |vf − f0| / max`), so at 1 the fundamental is held still and the outer voices keep full motion — the human's "stable core, moving periphery". All three default 0 = inert. REFERENCE-FIRST (ADR-003): swarmsaw.html edited (params; signed base pan now stored in rebuild — bit-inert, panL/panR unchanged; per-voice `cdist`; the drift weight `mw = 1 − motionCenter·(1 − cdist)`, exactly 1.0 when off; the per-block pan-motion block selecting modulated gains), then mirrored into swarm_core.h. NAMING NOTE: the core's member is `panBase` (not `pan`) because `rebuild()` already has a local `double pan`; the core's base pan is captured POST-scatter (ADR-033), so pan motion rides on top of scatter. cdist uses the PREVIOUS tick's value (one-tick lag, as in the lab) — negligible since cdist is near-static. MEASURED: with the pin at 1 the centre voice's drift swing goes 8.06 Hz → **0.00 Hz** while the edge voice stays 9.98 → 9.98 Hz. PARITY: 3 new goldens (pan-drift, pan-sweep, centre-pin driving both legs) × 3 seeds; `parity_check` 75/75 → **84/84**, all 9 new at **rms 0.000e+00**; inertness proven first vs origin/main (defaults / drift-walk / wide+tilt / glide+S&H bit-identical). All 9 chains + notefuzz green. NOT YET CLAP params (batched at fold end). Trace: 2026-07-23-fold-pan-motion-centre-pin.md.

## ADR-065 · Harmonic detune law (law 4) + `harmReach` folded into the SAW reference + core — ACCEPTED (parity-safe superset), with a recorded DOMAIN LIMIT on the bit-parity contract
Sixth fold, and the first of the map's **new detune laws** (the supersets are done). The law is the one the human reverse-engineered from the NI demo (2026-07-23): pulling voices out one at a time at full spread walked them **down the harmonic series** — the spread pattern IS the harmonic series. So law 4 places voice `i` at `f = f0·(1 + detune·harmReach·i)`: the voice INDEX is the harmonic rung. At `detune·harmReach = 1` the stack is exactly f0, 2f0, 3f0 … (H1..Hn); below that it is a continuous unison→harmonic-series morph. It deliberately ignores `dist` and `anchor` — the geometry is the integer series, not a distribution — and `harmReach` decouples the top harmonic from the voice count (`harmReach` 1.833 with n=7 → top voice at H12), which is what makes the character adjustable without changing polyphony cost.

NAMING (a real collision, not a style choice): the core ALREADY had `reach = 5` — the RING-topology neighbourhood radius (ADR-030 lineage). The lab's knob is called "reach", so a literal transcription would have been a duplicate member with a clashing meaning in the same struct. Folded as **`harmReach`** in BOTH files; the lab keeps its label.

REFERENCE-FIRST per ADR-003: swarmsaw.html edited first (`harmReach:1` param, the `else if (p.law === 4)` branch ahead of the ERB branch), then mirrored bit-for-bit into swarm_core.h (`double harmReach = 1`, the same branch, paramSlot). Laws 0/1/2/3 take no new code path — inert by construction.

PARITY: 3 new golden scenarios (harm-series = full series; harm-partial = mid-morph; harm-reach = harmReach beyond n) × 3 seeds. `parity_check` 84/84 → **93/93** within ε, the 9 new at **rms 0.000e+00** (C++ == JS exactly); the pre-edit 84 unchanged; all 8 chains + notefuzz green.

**THE DOMAIN LIMIT (the load-bearing finding).** A fourth scenario — law 4 at full spread WITH strong coupling (K 0.6) — FAILED parity at **rms 9.736e-02**, ~5 orders of magnitude outside ε. Diagnosis, not assumption: divergence was bracketed and scales with coupling × spread (3.7e-19 → 4.4e-13 → 3.5e-07 → 9.7e-02), and then falsified directly — perturbing the **JS reference alone by 1 ULP** and re-rendering gave **rms 9.383e-02**, i.e. JS-vs-itself diverges as far as C++-vs-JS, growing over time (5.62e-2 in 0–0.5 s → 1.147e-1 in 1–2 s), while stable regimes under the same probe stayed at 1.869e-19 and exactly 0. So this is **chaotic amplification of the last bit, not a porting bug**: a harmonic stack under strong coupling has a positive Lyapunov exponent, and `Math.sin` and `std::sin` are not identically rounded, so sample-exact parity in that regime is impossible **in principle** — no implementation could pass it.

RULING: the golden was REMOVED (with an explanatory comment in `gen_goldens.mjs` pointing here) rather than kept red or given a loosened ε — never weaken a gate (CLAUDE.md oracle discipline). The regime is covered instead by a **behavioural anchor** in `trajectory_check` (ADR-065 block): bounded + NaN-clean uncoupled and coupled, and coupling measurably entrains the harmonic stack. The anchor uses **K 0.9, not 0.6** — probed, K 0.6 lifts late-window R by only 0.006 on an octave-plus stack (indistinguishable from chaotic jitter, an anchor that could flip sign on any legitimate refactor), while K 0.9 is the first value that actually entrains this geometry (R 0.24 → 0.70), so the claim is checked where it is true, with margin (gate: ΔR > 0.2).

CONSEQUENCE FOR THE CONTRACT: bit-parity (ACCEPTANCE L0-1, ε=1e-6 RMS) is a valid oracle **only in non-chaotic regimes**; where the dynamics amplify ULP differences, correctness must be defined behaviorally. ACCEPTANCE.md is a protected path, so this ADR records the limit and **flags the amendment for human approval** rather than making it. NOT YET a CLAP param (batched at fold end). Trace: 2026-07-23-fold-harmonic-law.md.

## ADR-066 · Stretch (inharmonic) detune law — law 5, NOT the lab's law 3 — folded into the SAW reference + core — ACCEPTED (parity-safe superset)
Seventh fold, second of the new detune laws. The stretch law places voices by cents as law 0 does, then stretches the OFFSET ITSELF by `(1 + stretchB·x²)`, so outer voices spread disproportionately: `rat = 2^(x·detune·100/1200) − 1; f = f0·(1 + rat·(1 + stretchB·x²))`. That is piano/bell inharmonicity — the metallic end of the law set, and the complement to ADR-065's harmonic law (which lands ON the integer series; this one deliberately misses it).

NUMBERING (the decision this ADR exists to record). The lab menus stretch as **law 3**, but law 3 in the core is the TEMPO-GRID law (ADR-005/ADR-022, ported from the DYNAMICS reference) and law 4 is now harmonic (ADR-065) — so a literal transcription would have silently overwritten the tempo grid, and `dyn-grid` (which selects law 3 via `coreToDyn`) would have started rendering an inharmonic stack while still called a grid. Stretch takes **law 5**. The lab keeps its own numbering: it is an audition instrument with its own menu, not a numbering authority, and forcing it to renumber would edit a file whose only job is to be played. The divergence is recorded here and in both law tables so the next transcription does not re-derive it wrong.

REFERENCE-FIRST per ADR-003: swarmsaw.html edited first (`stretchB:0` param, the `else if (p.law === 5)` branch ahead of the ERB else), then mirrored bit-for-bit into swarm_core.h (`double stretchB = 0`, same branch, paramSlot). Laws 0/1/2/3/4 take no new code path — inert by construction.

PARITY: 3 new golden scenarios (stretch-flat B=0; stretch-mild B=2; stretch-bell B=6 across 12 voices) × 3 seeds. `parity_check` 93/93 → **102/102** within ε, the 9 new at **rms 0.000e+00** (C++ == JS exactly); all 8 chains + notefuzz green.

THE B=0 IDENTITY, MEASURED NOT ASSERTED. `stretchB = 0` reduces the expression to `f0·(1 + (2^(…) − 1))`, which is algebraically law 0 — but `1 + (v − 1) == v` is NOT a general floating-point identity, so the claim was checked rather than reasoned: a temporary `ident-law0` scenario (law 0, same detune/n) was generated alongside `stretch-flat` and the renders byte-compared — **IDENTICAL across all three seeds** (sha256 ac88657551bd… for both). So `stretch-flat` doubles as a regression pin on that identity: if a future edit to the stretch branch perturbs the B=0 path, its golden moves. The temporary scenario was removed; the check is recorded here as the evidence for the comment in both law tables.

NOT YET a CLAP param — `stretchB` is unexposed and the `law` param still ranges 0..3 (`kLawLabels` has four entries), so neither harmonic nor stretch is reachable from the host yet. Exposing them is part of the one batched CLAP pass at fold end (human cadence, 2026-07-23), which is a public-interface change and therefore its own gate. Trace: 2026-07-23-fold-stretch-law.md.

## ADR-067 · Golden (irrational) distribution — dist 4 — folded into the SAW reference + core — ACCEPTED (parity-safe superset)
Eighth fold, last of the *cheap* detune-geometry items (octave spread + root-anchor remains, and it rewrites the placement block — sequenced separately). The golden distribution places voice i at `x = 2·(((i+1)·φ⁻¹) mod 1) − 1` with φ⁻¹ = 0.6180339887498949: low-discrepancy irrational spacing — voices never repeat a position and never cluster, "even-but-inharmonic". It fills the gap between the deterministic-but-regular dists (even/JP) and the seeded-random ones (gaussian/cauchy): structured like the former, non-repeating like the latter.

MECHANICS: the reference's cauchy branch was the trailing `else` (the menu caps at 3), so golden required converting it to an explicit `else if (dist === 3)` with golden as the new `else`. Bit-inert for dists 0–3 — no branch body changed. Golden consumes NO rngG draws; safe because `grng` is re-seeded at the top of every rebuild, so a zero-draw distribution cannot desync anything downstream. C++ mirror uses `std::fmod`, which equals JS `%` for positive operands (both are here). Per convention (unchanged since the tempo-grid law), the reference's browser menu is NOT extended — the engine code is the spec; the menu is incidental GUI.

PARITY, with the inertness proof made explicit this time: golden manifests were generated from BOTH the edited reference and origin/main's reference and diffed per-scenario — all 102 pre-existing renders **sha256-identical**, only the 6 new files added. Then `parity_check` 102/102 → **108/108** within ε, the 6 new (golden-dist wide/11-voice; golden-sync coupled K=0.8) at **rms 0.000e+00**; all chains + notefuzz green. The coupled scenario is safe under the ADR-065 domain limit — K 0.8 over a ±semitone-class spread is the same regime as the long-standing sync goldens, and it renders exactly.

NOT YET a CLAP param — `dist` id 4 currently ranges 0..3 (kDistLabels); widening to 0..4 joins the batched CLAP pass (task #18) alongside the law 0..5 widening. Trace: 2026-07-24-fold-golden-dist.md.

## ADR-068 · Octave spread + root anchor — the placement-block rewrite — folded into the SAW reference + core — ACCEPTED (parity-safe superset)
Ninth fold, and the one the fold map warned about: unlike every previous fold (a new guarded branch), `spread` and `anchor` thread through EVERY detune law — the placement block is rewritten, not extended. `dep = detune·spread` replaces `detune` in all six laws, and each voice's position becomes `x − anchor·xmin` (xmin = the lowest raw x, recomputed per rebuild). At spread 24 law 0 goes from ±1 st to ±2 oct; at anchor 1 the LOWEST voice lands exactly on the root and the ensemble fans upward — the NI-style unipolar image the whole detune-lab campaign started from.

SEMANTIC RULINGS (recorded because the lab could not decide them):
- **Harmonic law (4)** ignores x and therefore the anchor (it is inherently root-anchored), but spread DOES scale it — `1 + dep·harmReach·i` — matching the lab.
- **Stretch law (5)** uses the ANCHORED x in its `x²` term, matching the lab: the inharmonicity follows the shifted geometry, not the raw one.
- **Tempo-grid law (3)** exists only in the core (the lab and the SAW reference have no law 3), so the lab is silent on it. Ruling: dep and the anchored x apply uniformly there too — placement semantics stay singular across the law table. Bit-inert at defaults; a non-default spread/anchor on the grid is C++-reachable-only behavior (same posture as ADR-025 super-width) until a golden covers it.
- **rootWeight is OUT of scope**: the lab entangles it with anchor (`aw = anchor` gates it), but it is a GAIN feature, not placement; it stays lab-only pending its own fold.

INERTNESS — the load-bearing proof, done the ADR-067 way: goldens generated from the edited reference AND origin/main's reference, manifests diffed per scenario — all **108 pre-existing renders sha256-identical**. The IEEE identities doing the work (`detune·1.0 == detune`, `x − 0.0 == x`) hold exactly, including for negative and ±0 x.

PARITY: 3 new golden scenarios × 3 seeds — spread-octave (±1 oct, even), anchor-root (anchor 1 over SEEDED cauchy, so xmin varies per seed and the anchor path is exercised for real — its three seed hashes all differ, as designed), spread-anchor-ni (JP, spread 8, anchor 1 — the NI image). `parity_check` 108/108 → **117/117**, the 9 new at **rms 0.000e+00**; all chains + notefuzz green. Wide-spread scenarios stay UNCOUPLED (K 0): wide + strong K is the chaotic regime the ACCEPTANCE §L0-1 domain limit excludes (ADR-065); coupled-wide behaviour is anchor territory, not golden territory.

GEOMETRY, measured on the reference before the goldens: spread 24·detune 0.5 puts the bottom voice at exactly 0.5000·f0 (−1200 c); anchor 1 over cauchy pins the lowest voice at exactly 1.0000·f0 with every other voice above it. Interior voices sit off naive positions because density-comp (normExp 0.75) is active at defaults — the established engine layer, not this change.

NOT YET CLAP params — spread/anchor join the batched pass (task #18). Trace: 2026-07-24-fold-spread-anchor.md.

## ADR-069 · Root-pinned pacemaker (`pivotMode`) — the first approved coupling-law divergence — ACCEPTED (parity-safe superset)
Tenth fold; first of the human-approved divergences (2026-07-24), and unlike ADR-060..068 it touches the COUPLING LAW, not placement or tone. `pivotMode` 1 replaces the mean-field sync attractor with a pacemaker: every voice entrains to the FUNDAMENTAL — the voice nearest f0 (min |vf−f0|, recomputed each control tick: the centre voice in symmetric detune, the lowest in anchored/harmonic modes). `sin(root − self)` is zero for the root itself, so it never moves: the swarm folds ONTO the played pitch (pitch-stable collapse) instead of onto the drifting swarm mean. The splay anchor (c0) also re-anchors on the root, so K<0 splays the fan around the fundamental. Human verdict from the lab (2026-07-22, ROADMAP): "totally different, but a more musical sound in general at a lot of settings" — BOTH modes kept as a toggle.

KNOWN TRAIT, KEPT: the pacemaker term drops the R scaler (`KsmS·sin(...)` vs `KsmS·R·sin(...)`), so its onset just off K=0 is a touch stronger than mean-field's. The lab flagged it for possible re-taper at port; folding verbatim was chosen — re-tapering would diverge from the lab reference the human actually auditioned, and the taper can be revisited with a knob-curve later (ADR-059 precedent) without touching the DSP.

SCOPE RULING: in the core the pivot applies on the topo-0 / poles-1 path ONLY — the lab has neither ring/two-cluster topologies nor Daido poles, so pivot × those combinations is meaningless-by-construction and gated off (pivot is simply not read there). Sakaguchi alphaR rides along uniformly inside the pacemaker term (0 in the SAW reference → bit-equal, ADR-023 pattern).

REFERENCE-FIRST (ADR-003): swarmsaw.html edited first (`pivotMode:0` param; rootIdx scan; the branch in the sync term; c0 re-anchor), then mirrored into swarm_core.h (+ paramSlot). rootIdx is computed only when pivoting, identically in both.

PARITY: inertness by manifest diff — all **117 pre-existing renders sha256-identical** vs origin/main. 3 new goldens × 3 seeds, ALL COUPLED (pivot only acts through coupling): pivot-lock (K 0.8), pivot-splay (K −0.8), pivot-anchor (the musical combo with ADR-068 anchor 1 + spread 4, K 0.7). `parity_check` 117/117 → **126/126**, the 9 new at **rms 0.000e+00** — notable: the pacemaker under coupling renders EXACTLY, where mean-field at comparable drive can go chaotic (ADR-065); a pinned root leaves no drifting mean to amplify ULPs. Spreads kept modest by design to stay inside the L0-1 parity domain. Behavioural anchor (trajectory_check ADR-069 block): bounded/NaN-clean on both K signs; entrainment ΔR **0.672** vs a 0.2 gate. All chains + notefuzz green.

NOT YET a CLAP param — joins the batched pass (#18). Trace: 2026-07-24-fold-root-pivot.md.

## ADR-070 · Alternating pitch-ranked pan fan as the DEFAULT image — the one true default-changing divergence — ACCEPTED
Eleventh fold; second approved divergence (#17), and the only one in the campaign that CHANGES DEFAULT OUTPUT: the stereo image of every width-bearing patch. The new default (`panLayout` 0): voices ranked by PITCH (raw x; by index when harmonic — law 4's rung IS the pitch order), rank r stepping out from dead centre on alternating sides at distance (r/(n−1))^γ, γ = 6^(0.5−panCurve) (0.5 linear · <0.5 centre-focused · >0.5 side-focused), `panInvert` flipping the triangle. Rank 0 — the fundamental — sits EXACTLY centre: the root holds the middle and the ensemble widens as it climbs (the human's requested default image, 2026-07-21). The legacy x-proportional image is KEPT as `panLayout` 1 — same posture as ADR-069's toggle — because gaussian/cauchy under it scramble stereo order by design, which is its own character.

WHY pitch-ranked: the legacy image maps DISTRIBUTION position to FIELD position, so seeded dists (gaussian/cauchy/golden) place voices in scrambled stereo order — the complaint that started this (human, 2026-07-22: "the distributions don't order the pan voices correctly"). Ranking by pitch makes the image follow the EAR's order regardless of layout.

PARITY MECHANICS (three findings recorded):
1. **std::stable_sort, not std::sort** — JS Array.sort is stable (ES2019), and gaussian/cauchy CLAMP at ±1, so ties are reachable; an unstable sort would order ties differently and break parity silently on some seed eventually.
2. **rebuild triggers grew**: law/panLayout/panCurve/panInvert now rebuild (the fan ranks by pitch, so the image depends on them) — safe because rebuild is idempotent (x[] deterministic from seed).
3. **The DYN reference needed pinning** — caught by a RED, not by inspection: dyn scenarios replay core-side on SwarmCore, whose default image moved, while DynSynth (a separate frozen program) keeps the legacy image — all 8 dyn scenario groups failed at rms 2.5e-2..5.6e-2 until DYN_BASE pinned `panLayout: 1` (coreToDyn drops the key; dyn has no layout selector). The failure is the proof the pin is honest, not cosmetic.

DIVERGENCE SCOPE, measured not asserted (manifest diff vs origin/main): exactly the predicted set changed — all **34 SAW scenario groups** (width defaults 0.8; every one width-bearing) re-measured; all **8 dyn groups sha256-identical**; spectra untouched. THE LEGACY PIN: new scenario `pan-legacy` (= old tilt-thin params + panLayout 1) **hash-MATCHES the pre-ADR-070 tilt-thin render on all three seeds** — the old image is preserved bit-for-bit behind the toggle, so nothing auditioned before this fold is lost.

PARITY: `parity_check` 126/126 → **141/141** (5 new scenario groups: fan over gauss (per-seed hashes differ — ranking does real work), curve 0.15, invert, fan×harmonic, pan-legacy), all at **rms 0.000e+00**; trajectory anchors unaffected (R-based — pan never touches phase dynamics); all chains + notefuzz green.

MONO NOTE: constant-power panning means the mono fold (L+R) of a voice depends on its pan position, so the new default DOES alter mono content slightly — this was the lab's documented exception ("all default-inert except the deliberate new pan default") and is accepted with the divergence.

NOT YET CLAP params (panLayout/panCurve/panInvert join #18). Pre-1.0, no shipped sessions — the default flip costs nothing now; post-1.0 it would have been a breaking change (see the consolidation-review roadmap item for the general principle). Trace: 2026-07-24-fold-pan-fan.md.

## ADR-071 · FX rack increment 2: Comp and Comb — the rack's first REAL cores (E3; human-ruled destinations) — ACCEPTED
Implements the 2026-07-24 rulings: comp/limiter and the polyphonic KS comb land as RACK SLOTS (types 4/5 on the existing fx params 57–64), NOT core folds — the HPF precedent applied twice. First increment past ADR-054's placeholders.

**Comp (type 4).** The lab's comp + limiter as ONE dynamics slot: amount = comp strength; the 0.98 brickwall is always engaged while the slot is active ("optional" = the slot being Off — one amount knob per slot is the rack's grammar). Peak follower with fast-attack/slow-release; soft knee above 0.4; ratio 1 + 4·amount. **ADR-009 applied at transcription**: the lab's follower coefficients (0.3 / 0.0015) were PER-SAMPLE at 44.1 kHz — banned form. Converted to seconds (atk 63.58 µs, rel 15.11 ms) and re-derived per-rate in setSampleRate, so the follower is sr-independent; ctor defaults preserve the 44.1 kHz behavior if setSampleRate is never called.

**Comb (type 5).** The lab's per-note Karplus-Strong comb re-hosted BUS-side: the rack keeps kCombLines=8 tuned feedback lines; the SHELL feeds note events (`rack.noteOn` at the common note-on point, both engines — ADR-054's anticipated note-context sidechain, now real). Each line is fed the whole bus and resonates its own pitch out of the mix (sympathetic-string posture) — a deliberate ARCHITECTURAL divergence from the lab, where each swarm's comb ate only that swarm's own output; the polyphonic per-note-tuned character is preserved, the per-voice isolation is not (recorded honestly; if A/B against the lab says the isolation matters, the comb moves core-side as its own decision). amount = wet mix; resonance fixed at the lab default (fb 0.79, damp 0.5) until the rack grows per-slot param pages. Note-off leaves the line ringing (natural KS decay); reuse is steal-oldest. Allocation only in ctor/setSampleRate (main thread); note-on clearing is a bounded memset — RT-clean per the charter.

**Interface note.** fx type params 57/59/61/63 widened 0..3 → 0..5 + labels "Comp"/"Comb" — the direct implementation of the human's slot approvals; pre-1.0, ids unchanged, values append-only.

**EVIDENCE.** (1) All-Off passthrough intact: parity 141/141 untouched, all chains + notefuzz green (notefuzz drives the shell factory, so the new note feed is fuzzed). (2) Engaged paths probed headlessly through the REAL CLAP plugin (scratch harness, 4-note chord A3/C#4/E4/A4): Comp takes chord peak 0.840 → **0.523** with mild rms reduction (0.225 → 0.209) — compression engages above the knee and the brickwall holds; Comb lifts rms 0.105 → 0.401 (single note, mix 0.9) with tail decaying to **0.000000** after note-off — resonates, rings, dies; both NaN-clean and bounded. The first probe run caught its own blind spot: a single A3 peaks at 0.34 < the 0.4 knee, so Comp measured identical to Off — the chord drive is what makes the evidence real. (3) pluginval strictness 10 **SUCCESS** including its param-fuzz sweep over the widened type range. Trace: 2026-07-24-fx-comp-comb.md.

## ADR-072 · Batched CLAP param pass — fold-campaign features made host-reachable — ACCEPTED (public interface, human-ratified roster)
Task #18, the gate the fold campaign (ADR-060..070) parked its interface work behind. Human rulings (2026-07-29): expose everything EXCEPT `lpOut` (core-only oracle/structural switch); `toneTilt` rename approved ("whatever works in the medium term" — display names are free to change later, ids/keys are not); state-compat approach delegated. Widened: `law` 0..3→0..5 (harmonic ADR-065, stretch ADR-066), `dist` 0..3→0..4 (golden ADR-067). New ids **71..86** (toneTilt · hiTame · driftMode · keepPhase · freqGlide · panMotion · panMode · motionCenter · harmReach · stretchB · spread · anchor · pivotMode · panLayout · panCurve · panInvert), each at its audition-lab range, each defaulting to the core's bit-inert default.

GHOST ID 70: the ADR-059 dev inertia-taper exponent is intercepted by NUMBER in applyParam/readParam with no row in kParams, so the id space has a hole the table cannot show — "table max + 1" is NOT the next free id. toneTilt landed on 70 first and its writes were silently swallowed (the functional smoke caught it: 15/16 ACT, toneTilt DEAD). The block starts at 71 and the ghost is documented at the table. Successor rule: check applyParam/readParam for numeric intercepts before allocating ids.

KEY COLLISION RETIRED, NOT GUARDED: SwarmCore's tone tilt is exposed as `toneTilt` (core alias to `p.tilt`), never as `tilt` — id 45 owns that key for SPECTRA and unguarded ids mirror into both cores by name (PR #125's probe evidence).

STATE: state_save writes raw values by key, so saved sessions are immune to the enum widening BY CONSTRUCTION — no remap machinery added, none needed. Recorded residual: VST3 normalized automation lanes on law/dist recorded pre-widening re-scale (a lane at law 3 = normalized 1.0 replays as law 5); accepted — sessions are the protected object, enum automation lanes are rare, and the alternative (parallel params) permanently forks the surface.

EVIDENCE: paramfunc_smoke (new diagnostic) — all 16 ids ACT at extremes against a width/detune-hot baseline, keepPhase through a real retrigger, plus law 4 / dist 4 alone; paramleak_probe extended to both directions with firing positive controls — all 16 inert on the SPECTRA engine, id 45 still inert on SAW. `./verify full` green, all nine chains; the only core edit is the alias (parity-neutral, worst rms unchanged). Neither diagnostic is wired into ./verify (gate changes need a human; propose separately if wanted).

## ADR-073 · Even-voice symmetric pan fan — no centre seat at even n — ACCEPTED (default-output change, human-directed)
ADR-070 seats fan rank 0 at dead centre and steps out at d = r/(n−1). At EVEN voice counts that degenerates: n=2 gives one voice dead centre and one voice HARD side — a lopsided image at any width ("2 with any width is unlistenable", human 2026-07-30, which is also the protected-path gate approval). FORK ON PARITY: odd n unchanged (ADR-070's image is explicitly the odd case, root holding the middle); even n uses d = (floor(r/2) + 0.5)/(n/2), so pairs sit symmetrically about centre and no voice occupies it. Alternating sides, pitch ranking, panCurve and panInvert are untouched — only the distance law forks.

EVIDENCE. Inertness by sha256 over renders (stashed reference vs edited): 144 bit-identical, exactly 4 changed — cauchy-cloud/gauss-cloud (n=16), hi-tame/stretch-bell (n=12) — i.e. every changed render is genuinely even-n and no odd-n render moved. Parity 147/147 within ε after adding even-fan-2 / even-fan-4 goldens (n=2 rms 0.000e+00, n=4 rms 2.393e-17); goldens carry width EXPLICITLY because the fan is inert at width 0. Balance: pan-seat sum is exactly 0 for n = 2,4,6,8,12,16 with no centre-seat occupant. ./verify full green.

SCATTER'S ROLE in the new mode (human sketch: "scatter can offset them instead of what it does now") is NOT part of this ADR — panScatter still blends toward the legacy image as before. Recorded as the open follow-up: offset pairs together to preserve balance, or per-voice with a re-centre.

MEASUREMENT NOTE worth keeping: three separate false readings preceded the real proof — a manifest.tsv diff (that file has no hash column, so it reported 0 changed), and two scenario parsers that misidentified which scenarios were even-n. The valid detector is sha256 over the .f32 renders. L0016/L0017 again: calibrate on a case the detector MUST fire on.

## ADR-074 · Super-width becomes a 3-mode system — F (clean) default, A/D as documented character — ACCEPTED (revises ADR-025)
The width-lab characterization (docs/reports/2026-08-02-width-characterization.html) and the human's ratified ship list. Width > 1 now selects via `superMode` (id 87): **0 wide (F)** — seat steepening (fan exponent 1/(1+2(w−1))) + per-voice ITD (far channel delayed up to 0.6 ms·(w−1)·2·|pan|, ring 256 samples = 1.2 ms at 192 kHz) — measured the WIDEST of all six candidates (S/M −0.1 dB, corr 0.01) at zero cliffs; **1 pulse (A)** — the original ADR-025 M/S boost verbatim, negative cross-feed and all; **2 smear (D)** — allpass-side variant. A and D are DOCUMENTED CHARACTER: their polarity cross-feed (the up-cliff mechanism, PR #158) is the sound the human kept. C and E retired as subsumed by F. Default 0 is a deliberate default-output change at width > 1 versus the old always-A behavior.

L0021 DISCHARGED IN THE SAME CHANGE: waveshape_check gains three gates — F at width 1.5 MUST be clean (0 cliffs; got 0), A and D MUST cliff (pinned: 1,867 and 14,300 — if a future change silently linearizes a character mode, the pin fires and the change owns up). Width ≤ 1 bit-untouched: parity 147/147 after the fold, worst rms unchanged (4.262e-09, pre-existing dyn-ring).

Known limits, recorded: mode F costs ~1.3 dB more mono-fold loss than gains-only widening (Haas combing — the standard cost of every delay widener); ITD max is a fixed 0.6 ms audition value (a trim knob can come later without ids churn); mode D's allpass corner is fixed at 700 Hz likewise.

## ADR-075 · Opt-in 2x oscillator oversampling — ACCEPTED (superset, bounded claim)
Closes the HF-droop thread (measured 2026-07-31/08-03). The voice sum runs twice per output sample at half the phase step and feeds a 63-tap windowed-sinc halfband decimator (cutoff 0.235 of the 2x rate); everything after the sum — output pole, envelope, ITD head — stays at 1x. Param `oversample` (id 88), **default OFF and bit-exact**: the ADR-063 precedent, so all 147 goldens and every saved session are untouched.

MEASURED IN-CORE (single saw, bin-commensurate f0, droop vs the ideal 1/k law): 5 k −0.60 → −0.52 · 10 k −2.17 → **−1.23** · 15 k −4.50 → **−2.13** · 20 k −7.56 → −5.65. CPU at 8 notes x 16 voices: 2.5% → 6.3% of one core — comfortably inside the E-6 envelope (<50%).

BOUNDED CLAIM, and an honest gap: the JS spike predicted −0.83 dB at 15 kHz; the real core reaches −2.13. The residual is NOT the decimator — the likely source is additional in-core filtering the spike did not model (the R→tone output pole runs on the summed signal at 1x). An isolation measurement was attempted and abandoned when the harness kept mangling; recorded as OPEN. So the shipped claim is "roughly halves the droop through 15 kHz", NOT "flat to 15 kHz" as the spike suggested. 20 kHz stays down: at 0.91x Nyquist it lives inside any decimator's transition band, and 4x OS is the only way to clear it (~2x again the CPU).

L0021 DISCHARGED IN THE SAME CHANGE: waveshape_check gains (a) an OS-off determinism gate (worst |diff| = 0 across independent constructions — catches state bleed from the new decimator members) and (b) a recovery gate asserting 2x lifts the 15 kHz harmonic by >= 1.5 dB (measured +2.37). Harmonic LEVEL by Goertzel at bin-commensurate f0 is valid here; L0017's trap was sparse probes for ALIASING, where folded products miss the grid.

## ADR-076 · Poly glide — every new voice bends in from the last-played pitch — ACCEPTED (superset, opt-in)
Human request 2026-07-31 ("poly glide ... as long as it's trivial to implement", plus "an always-on glide that remembers the position of the last played note(s) and always begins with a bend"). It was in fact small: the core already had per-swarm glide machinery from the ADR-026 mono retarget, so poly glide is a core+shell change — seed the new voice's f0/f0cur from a remembered `lastNoteF`, set glideTarget, reuse the existing per-tick glide. The `glide` TIME knob (id 33) is shared and consequently stops being mono-gated in the GUI.

DELIBERATELY MEMORY-BASED, NOT VOICE-BASED: `lastNoteF` persists across silence, so the first note after a rest still bends in from wherever you last played — the human's "always begins with a bend", not merely legato-when-overlapping. Mono retargets refresh the memory too, so switching modes stays coherent.

`polyGlide` id 89, default 0 and bit-exact (ADR-063 precedent). MEASURED (A2 -> A3, glide 0.30 s): off -> the new note starts at 220.0 Hz; on with glide 0 -> 220.0 (the time knob still gates it); on with glide 0.30 -> starts 110.0, 50 ms 125.8, 300 ms 179.8, 1.5 s 219.2. parity 147/147, verify full green.

GLIDE SOURCE IS A MODE, not a behaviour (human, 2026-08-03: "memory-based glide is a function I want access to but I don't always want it"). `glideMode` id 90: **0 = held note (legato)** — bend only while another key is still HELD, which is exactly the rule the mono path has used since 2026-07-18, and the default; **1 = last note (memory)** — always bend from the last-played pitch, even after a rest. MEASURED (A2 then A3, glide 0.3 s; 220 = no bend, 110 = bends in): legato 110.0 overlapping / 220.0 after a rest; memory 110.0 / 110.0. The held check runs BEFORE alloc(), since alloc can steal a still-gated voice and asking afterwards would misreport.

DEFERRED, honestly: the chord case. `lastNoteF` is ONE frequency, so a chord's voices all bend from the same pitch rather than each from its own nearest predecessor — which the human's "last played note(s)" anticipated. That is a bigger design (voice-to-predecessor assignment), left as a follow-up rather than smuggled in half-done.

## ADR-077 · Ensemble onset timing folded into the core — Vorberg/Wing correction, not jitter — ACCEPTED (superset, increment 1)
The 2026-07-28 research headline (LIBRARY L0019) reaches the instrument. Voices enter at different times, and those times come from MUTUAL ERROR CORRECTION — `off_i <- off_i − alpha·(off_i − mean_off) + motorNoise_i` — because listeners judge ensemble togetherness from the SERIAL STRUCTURE of asynchrony, not its variance. Params: `onsetScatter` id 91 (motor-noise sigma in ms; **0 = off = bit-exact**, the master switch), `onsetAlpha` id 92 (correction gain, default 0.25 — the measured near-optimal for real quartets), `attackScatter` id 93.

MEASURED FROM THE FOLDED CORE, and it reproduces the research's regimes: alpha 0 → onset SD 202 ms, lag-1 **+0.985** (random walk, drifts without bound); alpha 0.25 → SD 35.8 ms, lag-1 **+0.679** (bounded WITH structure); alpha 1.0 → SD 26.4 ms, lag-1 **−0.072** (i.i.d. — precisely what conventional humanize produces); alpha 1.5 → lag-1 **−0.550** (over-correction, alternating early/late).

ARCHITECTURE NOTE, honestly: the lab gives every voice its own full ADSR; the core's envelope is PER-SWARM. Rather than rewrite the envelope architecture (large, parity-risky), this increment adds a per-voice ENTRY: a voice holds silent — and does not advance its phase, since it has not started playing — until its offset elapses, then fades in on its own attack coefficient so a late entry cannot click in at whatever level the shared envelope has reached. Per-voice release/decay shaping is NOT folded and remains lab-only; that is increment 2 and needs the envelope rework.

L0021 DISCHARGED: waveshape_check gains a STRUCTURE gate — lag-1 must fall monotonically with alpha (a0 > 0.9, a0 > a.25 > 0.4, a1 < 0.2). Deliberately not a variance test: a regression that silently turned this into per-note jitter would still "scatter onsets" and pass any variance check; only the ordering catches it. Parity 147/147 unchanged, `./verify full` green.

## ADR-078 · Per-voice envelopes — ACCEPTED (superset; ADR-077 increment 2)
Human, 2026-08-03: the ensemble timing sounded right across the board, and per-voice envelopes "would allow for some very interesting randomization and voice-state-based modulation down the line" — the second reason is the stronger one, since per-voice state is what a mod matrix wants as a SOURCE. `voiceEnv` (id 94) gives every voice its own ADSR; `relScatter` (id 95) spreads release times the way attackScatter spreads attacks, so a chord decays as players rather than as one gate. Default off = the shared-envelope reference path, bit-exact.

DESIGN: the shared `s.env` is NOT removed — it becomes pure BOOKKEEPING, tracking the loudest voice (`s.env = max(onsE)`). Every liveness test, voice-steal decision and NOTE_END emission keys off s.env, so all of that machinery keeps working unchanged while the AUDIO is enveloped per voice inside the loop. That is why this fold did not have to touch the note-lifecycle code at all — a rewrite there would have re-opened the stuck-note surface we spent three rounds closing.

BUG CAUGHT BY TEST, NOT INSPECTION: the first build computed the per-voice attack/release coefficients only inside the ADR-077 `onsetScatter > 0` branch, so enabling voiceEnv alone left every coefficient at 0 and the swarm rendered SILENCE. The probe found it immediately; the coefficient setup now runs whenever either feature is on, with only the Vorberg/Wing timing correction still gated on scatter.

MEASURED: voiceEnv on with scatter 0 — audible (peak 0.211) and per-voice spread exactly 0 (uniform, as it must be when nothing is scattered); scatter 0.8 — spread 0.237 at 150 ms into the release, i.e. voices genuinely releasing at different rates; both cases reach true silence (106 / 174 blocks). Gates pin all four properties. Parity 147/147, `./verify full` green.

FORWARD: per-voice envelope level is now a natural mod SOURCE (the human's stated motivation). Wiring it into the matrix belongs with that work, not here.

## ADR-079 · NOTE_END must survive a rejected push — ACCEPTED (the real stuck-note bug)
Four rounds of stuck-note work fixed real bugs and never reached this one. The human's clue closed it: "voices stall ... most when I've recently changed the K value", with release/attack/decay at minimum so the envelope was excluded by construction. Measured first: the CORE is innocent — tail after note-off is 46.4 ms at K 0.0 steady, K 0.9 steady, K 0.9→0.0, K 0.0→0.9, and at rtone ±1. Identical everywhere, exactly what a 5 ms release predicts.

THE BUG: `emitNoteEnds` called `out->try_push(...)` and IGNORED THE RETURN VALUE, then retired the tag (or cleared the pending queue) regardless. `try_push` can legitimately fail — the host's output-event buffer is finite, and `drainQueue` floods that same buffer with outgoing param events every time a knob moves. So sweeping K could crowd out a NOTE_END, which was then destroyed forever: Live never learned the note ended and withheld retriggering that pitch until something else cleared its table. That is precisely the reported signature, including why it correlated with knob movement and why an arpeggiator "fixed" it.

FIX: both emission sites now retire only on ACCEPTANCE. Rejected ends stay queued (the pending array is compacted in place) or leave their tag active, and the next block retries — the note resolves late rather than never.

ORACLE: endprobe gains a REJECTING HOST — try_push refuses the next 40 pushes, as a full buffer does. Proven discriminating by running it against the pre-fix code: **old 0 NOTE_END delivered, new 1**. A test that does not fail on the old code proves nothing, so that check was run deliberately.

LESSON (candidate for LIBRARY): an API that returns a success flag is telling you it can fail. Ignoring it converts a transient, recoverable condition into permanent silent data loss — and the loss is invisible precisely because the failing path is rare and load-dependent, so it presents as intermittent flakiness rather than a bug.

## ADR-080 · FX rack: a per-slot SECOND axis; comb resonance folded (2026-08-03) — ACCEPTED
Human, on the comb: *"Wasn't there a second slider on the comb in the lab?"* — correct. `detune-lab.html` has **comb mix** and **resonance**; only mix was folded. ADR-071 hardcoded `fb = 0.79` and said so explicitly: *"resonance fixed at the lab default until the rack grows per-slot param pages."* This is that page.

**One generic axis, not a comb param.** New ids **96–99** are `fx1tone..fx4tone` — a second knob on *every* slot, so the next slot type wanting a second control costs no new ids and no new concept. Only Comb reads it today; the others ignore it. The alternative — a comb-specific `combRes` id — would have to be repeated per type forever, and would sit dead in the param list whenever the slot is anything else.

**Bit-inert by construction.** Comb resonance uses the lab's own law, `fb = 0.6 + 0.38·tone`. Default `tone = 0.5` gives exactly `0.79`, the value ADR-071 hardcoded — so parity stays 147/147 and **every saved state loads unchanged** (a missing key reads the default, which is the old behaviour).

**Gated, because an inert default is exactly how a dead control hides.** Parity staying green proves only that 0.5 changes nothing; it says nothing about whether the knob is connected. The FX dropdowns shipped Comb *unreachable* for precisely that reason and no oracle saw it. `waveshape_check` T7 therefore measures ring time to −40 dB across the range: **99 / 190 / 772 ms at tone 0.1 / 0.5 / 0.9**, required strictly increasing.

**Interface note.** Ids 96–99 are additive and append-only, pre-1.0 — the same shape as ADR-071's widening of 57/59/61/63 and ADR-078's 94/95.

## ADR-081 · Envelope cluster owns per-voice envelopes + scatter; envelope display added (2026-08-03) — ACCEPTED
Human: *"the per voice envelope should go in the envelope tab. Maybe also the scatter controls, and we can add an envelope visualizer which also shows the variation from scatter?"*

**Moved** ids 91/92/93/94/95 (onset scatter, timing correction, attack scatter, per-voice envelopes, release scatter) from **Drift** to **Envelope**. The resulting split is cleaner than the one it replaces: **Drift = pitch variation, Envelope = amplitude/time variation**. Onset scatter was the debatable one — it is *timing*, not envelope shape — but it moves the moment each voice's envelope begins, so it belongs with the display that shows exactly that, and separating it from attack/release scatter would scatter one idea across two clusters.

**The display is fed by the ENGINE, not re-derived.** New viz fields `envOnsetMs/envAtkMs/envRelMs` publish, per voice, the times the core actually assigned — recovered from the one-pole coefficients it is using (`t = −1/(sr·ln(1−c))`). The scatter draws from the core's seeded RNG stream, so a JS reconstruction would be a **second implementation free to drift from the one you hear** — the display would eventually lie, and lie plausibly. Requires one new core array, `onsD0` (the initial onset delay, kept because `onsD` decrements to zero); viz-only bookkeeping, never read in the audio path, so parity is untouched.

**Why a visual at all:** the standing convention recorded the same day — lab visuals ship with the feature they explain. Horizontal offset between traces *is* onset scatter; differing slopes *are* attack/release scatter. A numeric readout accompanies it, so the effect is legible without playing a note.

## ADR-082 · Multi-oscillator architecture: param-id namespace, CPU budget, per-osc state — ACCEPTED (ratified 2026-08-06: **2 oscillator slots**)

**Why this blocks the interface renovation.** The layout lab's own Decision 2 says it: 2-3 full
oscillators is not a GUI change, it is N core instances with per-osc params, per-osc state,
multiplied CPU, and a param-id namespace. **CLAP ids are append-only**, so the namespace is
designed once or lived with forever. Every other renovation item (osc page, morph page,
per-parameter corner colouring) sits downstream of it.

**Measured context.** 99 params today, ids **1..99, densely packed with no gaps**
(`src/hypersaw_clap.cpp` `kParams[]`). Classifying them: roughly **70 are per-oscillator** —
the entire swarm surface (n, dist, seed, detune, law, K, onset, dissolve, drift, topo, reach,
mu, alpha, poles, grav, basin, the SPECTRA block 44-51, shape, tone, pan/spread/anchor 76-87,
the scatter/env block 91-95) — and roughly **29 are global**: output/image (width, mono, vol,
bassMono, bassMonoHz), voice & glide behaviour (voiceMono, glide, legato, polyGlide, glideMode,
freqGlide, pitchBend, inertia, inertiaCurve), the FX rack (57-64, 96-99), tempo grid
(beatMult), and oversample.

### Decision 1 - id scheme: a fixed +100 stride per oscillator

    id(P, osc k) = id(P, osc 0) + 100k

Oscillator 0 **keeps every id it has today**, so all existing sessions, automation lanes and
patches survive untouched. Oscillator 1 occupies 100-199, oscillator 2 occupies 200-299. Where
a global param's id would fall inside a block, the slot is simply never allocated - the gaps
are self-documenting evidence of which ids are global.

*Rejected - re-homing all per-osc params into clean blocks.* Correct-looking and impossible:
ids are append-only, so moving osc 0 breaks every saved session.

*Rejected - one param set plus an "edit target" selector.* Cheap in the GUI and wrong for a
plugin: automating oscillator 2's detune becomes impossible, because the lane's meaning depends
on a selector position. Per-osc automation is table stakes.

### Decision 2 - CPU budget, and a constraint that falls out of it

Current measured cost is **2.5% of one core at 1x, 6.3% at 2x oversampling** (ADR-075, on this
M3), against the **E-6 budget of 50% of one core on min-spec**. The voice loop scales linearly
with oscillator count, so:

| config | this M3 | x4 min-spec derate (the ADR-018 precedent) |
|---|---|---|
| 3 osc, 1x | ~7.5% | ~30% - inside |
| 2 osc, 2x OS | ~12.6% | ~50% - at the line |
| 3 osc, 2x OS | ~18.9% | **~75% - over budget** |

**So "3 oscillators" and "2x oversampling" cannot both be unconditional.** Proposed: ship 3
oscillator slots, keep oversampling opt-in and global, and **measure on real min-spec hardware
before increment 2** rather than trusting a derate factor borrowed from a different workload.
The derate is an estimate and this table is arithmetic on top of it - it is a reason to
measure, not a verdict.

### Decision 3 - per-osc state keys

State is `hypersaw-state 1` followed by `coreKey=value` lines. Proposal: per-osc keys for
k >= 1 gain an `o<k>.` prefix (`o1.detune=...`); **oscillator 0's keys are unchanged**, so every
existing patch loads bit-identically and `state_check` stays green as the regression proof.
Format bumps to `hypersaw-state 2` while still accepting `1` (absent `o1.`/`o2.` keys => those
oscillators at defaults, i.e. silent). "Copy osc 1 -> osc 2" becomes a key-prefix rewrite.

### Increments (the walking-skeleton order the layout lab recommends)

1. Id scheme + state format land with **N = 1 still**, so nothing audible changes and the
   parity/state oracles prove the refactor inert.
2. A second oscillator behind the existing `balance` param - thin, no GUI.
3. GUI: osc page with per-parameter corner colouring and glyphs (the standing convention).

### Ratification (human, 2026-08-06)

**Two oscillator slots**, on the reasoning that SAW alone does not need three. Ids therefore
occupy 1-99 (osc 0) and 100-199 (osc 1); the 200-299 block stays unallocated and available if a
third slot is ever wanted.

**CORRECTION — the sub/balance question was withdrawn, because it rested on a false premise.**
The ADR asked whether the sub-oscillator block (52-55) and `balance` (56) are "superseded by
real oscillators". Checking the code rather than the layout lab's prose:
- **`balance` (56) is not an oscillator mixer.** It is the two-cluster A/B *coupling* balance
  inside the SAW swarm — `kB = 1 - 2*balance` (ADR-051) — an intra-swarm parameter.
- **The sub (52-55) is SPECTRA-only**; it exists in `spectra_core.h` and `swarm_core.h` has no
  sub at all.

Neither is superseded by anything. Both are ordinary **per-oscillator** parameters and get
blocks like every other per-osc param (osc 1: 152-156). No deprecation, no deletion, no
decision required. The question came from the layout lab's shorthand ("ONE engine select + A/B
balance + sub") being read as an architecture description when it was a sketch.

**Oversampling stays GLOBAL** (param 88). With two slots the derated estimate is ~20% at 1x and
~50% at 2x — the latter sits exactly on the E-6 budget line, so making oversampling per-osc
would multiply the one number already at its limit for no expressive gain. The min-spec
measurement required before increment 2 stands.

### Amendment 1 (2026-08-06) — stride 100 → 1000, and `vol` is per-oscillator

Found while starting increment 2, before anything was built on the scheme. Both defects are
in the ratified text above; both were free to fix at that moment and would have been permanent
a week later.

**(a) The stride is also the CAPACITY of oscillator 0's block — and it was full.** With stride
100, osc 0 owns ids 1..99. The instrument has **99 params with zero free slots**. A new param
would need id 100, and `findParam` computes `osc = id / kOscStride`, so id 100 resolves to
oscillator 1 / base 0 and is **never found** — the param would not be cramped, it would be
silently unreachable. Stride 100 therefore capped the instrument at 99 parameters *forever*,
and it was already at the cap on the day it was ratified.

**Stride is now 1000**: osc 0 = 1..999 (900 free slots), osc 1 = 1000..1999, osc 2 =
2000..2999. Every existing id is unchanged. This costs nothing **only because increment 1
shipped at `kNumOsc = 1`** — no id ≥ 100 has ever been exposed to a host, so no session,
automation lane or patch can reference one. After the first 2-oscillator build ships, this
amendment becomes impossible.

**(b) `vol` (17) was misclassified as global.** It is the swarm's *own* output gain, computed
inside `SwarmCore::render` (`gain = p.vol * 0.9 / n^normExp`), so it belongs to the oscillator.
Leaving it global would give two oscillators one shared gain and **no way to balance them** —
which is precisely the control increment 2 exists to provide. It is now per-oscillator. A
patch-level master volume, if wanted, is a separate new param; the stride amendment leaves
room for one.

**How this was caught, since the process point generalises:** not by review of the ADR, but by
starting to build the increment it authorised and finding there was no mixer. The
`balance` (56) param that the original increment-2 sketch named as the place to hide a second
oscillator is the two-cluster *coupling* balance (ADR-051), not a mixer — a correction already
recorded in the ratification note. Following that thread is what exposed both defects. An ADR
reads as complete right up until you try to execute it.

**Verified:** `./verify full` GREEN at `kNumOsc = 1` (parity 147/147, worst 4.262e-09 —
unchanged), and calibrated at `kNumOsc = 2` where `state_check` passes in full, including the
version assertion and the `o1.` key round-trip.

### Per-osc / global split (the part that is baked in forever)

Everything not listed here is **per-oscillator**. GLOBAL params, by group:

| group | ids |
|---|---|
| output & image | 14 width, 15 mono, 17 vol, 40 bassMono, 41 bassMonoHz |
| amp envelope | 19 attack, 20 decay, 21 sustain, 22 release |
| voice & glide | 32 voiceMono, 33 glide, 34 voiceLegato, 38 pitchBend, 75 freqGlide, 89 polyGlide, 90 glideMode, 11 inertia, 70 inertiaCurve |
| FX rack | 57-64, 96-99 |
| global misc | 23 beatMult, 88 oversample |

**Three entries are judgement calls, flagged rather than buried** — if any is wrong it is wrong
permanently: (a) **amp envelope 19-22 global** — conventional (the *voice* has an amp envelope,
not each oscillator), and SPECTRA already has its own env at 65-68; (b) **transpose 35 octave /
36 semi / 37 fineCents per-osc** — this is much of the point of a second oscillator (an octave
down replaces what a sub would do), so they move into the per-osc class; (c) **18 retrig and 74
keepPhase per-osc** — phase behaviour is a property of the oscillator, not the note.

## ADR-083 · Voice steal: three-tier policy — releasing tails before held notes — ACCEPTED (deliberate divergence)

Human report (2026-08-08): a sustained note under a running arpeggio is eventually stolen.
Measured mechanism: a released tail occupies its slot until `env < 1e-3` (~1.1 s at the 0.16 s
release), so a 9-note/s arp keeps ~10 tails alive; when the 16-slot pool fills, the reference's
steal-oldest policy sacrifices the OLDEST voice — which is precisely the note being deliberately
held. Reproduced with a Goertzel probe at the held note's f0 (long windows + a no-arp control,
after a short-window first attempt measured the sustain's own 65 Hz beat nulls): stolen ~11 arp
notes in, power to 7% of the beating floor.

**Decision:** `alloc()` becomes three tiers — (1) free slot, oldest first (as the reference);
(2) **releasing tail, quietest first** (least audible loss; age as tiebreak) — an arp recycles
its own tails and never touches holds; (3) only when every slot is gated, oldest held note
(unavoidable, as the reference). The JS reference keeps its naive policy (protected prototype);
this is a recorded divergence in the overflow regime only. Goldens never overflow the pool, so
parity is the regression proof: **147/147, worst 4.262e-09, unchanged**.

**Verified:** the pre-fix probe run is the calibration (sustain stolen at ~1.2 s); post-fix the
sustain survives the entire 160-note arp with its f0 power never dropping below the no-arp
beating floor. Follow-up queued: fold the arp-sustain scenario into `notefuzz_check` as a
permanent gate.

## ADR-084 · Velocity and MPE pressure → per-voice volume, on by default — ACCEPTED (superset)

Human request (2026-08-08). The synth ignored velocity entirely (the AU-wrapper comment said as
much) and note expressions handled only TUNING. Now: **note-on velocity scales the voice's
gain** (linear, `vel` ∈ [0,1]) and **CLAP `PRESSURE` expressions drive a per-voice gain
target** smoothed over ~20 ms (ADR-009 seconds→coefficient; expression streams arrive at UI
rate and a raw multiply zippers). Pressure re-arms to 1.0 per note, so hosts that never send it
are untouched; MPE surfaces get strike-then-swell behaviour by default.

Superset discipline: `vel`/`press` default 1.0 exactly, the multiply is bit-inert, and the core
`noteOn` signature is unchanged (velocity applied via `setNoteVelocity` after allocation) — so
every golden and every pre-existing caller is byte-identical. **Parity 147/147, worst
4.262e-09, unchanged** is the regression proof. Calibrated through the real CLAP path: vel 0.5
→ rms ratio 0.503; pressure 0.3 mid-note → ratio 0.302 after the smoother settles.

Recorded residuals: SPECTRA ignores velocity still (its own increment); the mono/legato
retarget keeps the original strike's velocity (a retargeted note is the same phrase — revisit
if the ear disagrees); raw MIDI channel aftertouch (0xD0) relies on the wrapper translating to
PRESSURE expressions — verify in a DAW pass. Velocity CURVE (soft/hard) is a future param; the
stride amendment leaves room.

## NOTICE · Library standby (plugin-skeleton FOUNDATIONS) — RECORDED 2026-08-08

Relayed by the human from the library thread. A shared infrastructure library
("plugin-skeleton") is being founded; its FOUNDATIONS document defines contracts this project
will eventually consume (parameter registry, mod routing, scoped presets, event pipeline,
payload interfaces). A mediator agent will open a brief→response dialogue when this repo
becomes the active correspondent; until then **passive standby**: no refactoring toward the
library, no imagined interfaces, no speculative extraction — writes stay home, roadmap
continues as-is. Passive disciplines adopted: stable hierarchical param addresses (record any
rename old→new), no new singletons/global state in components plausibly per-voice or per-module
elsewhere, engine internals behind clean boundaries. Standby artifact:
`INTEGRATION-STANDBY.md` (friction list · component inventory · param+mod architecture
sketch), kept cheap and current — it becomes the first brief when the mediator calls. The
pre-existing donor manifest (`docs/integrations/corelib-insights.md`) predates this notice and
is documentation, not refactoring; the standby artifact references it rather than duplicating.

## ADR-085 — `mpe_check` is a blocking gate in `./verify full`

**Date:** 2026-08-09 · **Status:** accepted (human: *"Gate ratified."*)

**Context.** Eight sites in the CLAP event loop applied per-voice and lifecycle
operations to `core` (the oscillator-0 alias) where the intent was every
oscillator. Every existing gate passed: parity renders a SINGLE core, so a bend
that split the oscillator pair and an all-notes-off that left half the
instrument gated — a stuck note — were invisible to all fourteen other chains.

**Decision.** `tools/mpe_check.cpp` runs in `./verify full` as a blocking gate.
`./verify` is a protected path; this ADR records the human approval required to
edit it.

**Why this gate and not a state assertion.** It drives the real CLAP path
(factory → activate → events → process) and detects via emitted audio with a
single-bin Goertzel. There is no per-voice tuning getter, and adding one to
test with would test the accessor — the `state_check` trap, where a round-trip
through one broken accessor agreed with itself.

**Calibration is part of the decision.** Planted `allOff` → a frozen plateau
(rms 0.142 → 0.142 → 0.142, decay ×1.0); planted tuning and retarget → 49.8%
and 50.0% of the energy stranded at the old pitch. Verified at the DISPATCHER,
not only the probe: `./verify full` exits 1 with the bug planted and 0 restored.

**Consequence.** The fan-out fix is a helper per operation family, so every
future consumer (third oscillator, sub-oscillator, per-voice FX send) re-opens
the whole class until the L0029 routing layer lands. This gate is what makes
that debt visible rather than silent.

## ADR-086 — Consonance gravity integrates on a fixed grid (ACCEPTED)

**Date:** 2026-08-09 · **Status:** ACCEPTED 2026-08-10, ratified after an ear
check ("the two gravity buffers sound nearly identical"). Implemented same day.

### Implementation note — the accumulator alone was not the fix

The first implementation did exactly what this ADR described: accumulate
samples, then `while (accum >= grid) gravityStep(grid/sr)` at the top of
`render()`. The subdivision probe rejected it immediately — still 1.04.
Fixing the step SIZE is insufficient because it leaves the step PLACEMENT
wrong: in one whole call every step fires *before any audio is written*, while
chunked calls spread the same steps through the buffer. Same steps, different
placement, same divergence.

The working fix **segments the render**: `render()` is now a loop that calls a
private `renderSeg()` in pieces bounded by the grid, advancing gravity between
them. Invariance verified at 0.00 for chunk sizes 64…44100 including 333 and
127, which are not multiples of the grid.

### And it moved something it should not have

Segmenting also changed **pan motion** (ADR-064), which is a per-render-call
integrator too — nine SAW parity scenarios went red, against goldens whose
reference (`swarmsaw.html`) this ADR never touched. Pan motion is now hoisted
into `advancePanMotion()`, called once per outer call, so it keeps its
per-call rate exactly. **Gravity was ratified for a fixed grid; pan motion was
not**, and confining the change to what was approved is the whole reason the
hoist exists.

Pan motion therefore remains subdivision-dependent — measured 0.191 at chunk
333 — and that is now a declared, visible exclusion in `subdiv_check` rather
than an unknown. See ROADMAP § pan motion.

**Verified:** `./verify full` GREEN, 15 gates, parity **147/147 worst
4.262e-09**. Golden footprint exactly as predicted: **248 unchanged, 3 moved**
(`dyn-gravity` × 3 seeds).

**Original proposal follows.**

**Date:** 2026-08-09 · Touches a protected path (`swarmdynamics.html`) and
moves one golden.

### Context — a measured defect

`SwarmCore::render()` opens with `gravityStep((double)frames / sr)`. Gravity is
integrated **once per render call, with dt = the block length** — explicit Euler
on a nonlinear ODE (`move = err · rate · dt`, then `f0cur *= 2^(-move/1200)`,
with `err` recomputed from the current `f0cur` each call). One step of dt and
two of dt/2 therefore disagree, so output depends on how a buffer is
**subdivided**, not merely on its total length.

The JS reference has the identical shape: `swarmdynamics.html:405`,
`this.gravityStep(outL.length / sr)`.

**Why no gate caught it.** The golden generator renders a fixed-size buffer and
`parity_check` renders `kBlock` — both sides use the *same* subdivision, so
parity agrees with itself. The defect is invisible to the oracle by
construction, which is the same blind spot the multi-oscillator fan-out bug
exploited.

### Evidence

Bare `SwarmCore`, same seed and notes, 1 s:

| gravity | one whole call vs 256-frame chunks | vs 333-frame blocks |
|---|---|---|
| 0.00 | **0** | **0** |
| 0.50 | **1.028** | **1.029** |

**Correction to the first report of this bug.** That 1.03 was described as "a
different sound". It is not — it is **phase**, not tuning. Re-measured on
`dyn-gravity`'s own settings, the musically meaningful quantity is invariant:

| integration step | interval settles at | waveform rmsDiff / rms |
|---|---|---|
| 2048 samples | 701.931 ¢ | 0.128 |
| 512 (reference) | 701.927 ¢ | — |
| 256 | 701.926 ¢ | 0.022 |
| 128 | 701.926 ¢ | 0.033 |
| 16 | 701.926 ¢ | 0.043 |

Gravity settles to within **0.005 cents** of the same place at every step size
(just 3/2 is 701.955 ¢). What varies is the phase trajectory getting there.
So the bug is a **reproducibility** defect, not a tuning defect — real, but
smaller than first stated, and that distinction should survive into whatever
gets fixed.

### Options

1. **Do nothing; document it.** Cheapest. Leaves renders non-reproducible across
   host buffer sizes whenever `grav > 0.005`, and leaves oscillator 0 (rendered
   in one `n`-frame call) integrating differently from oscillators 1..N (256-frame
   chunks). Rejected: "the sound changes when you change your buffer size" is a
   defect users report and cannot work around.
2. **Fixed accumulator grid at 256 samples** — accumulate elapsed samples, step
   gravity by a constant `dt = 256/sr` whenever a full interval has elapsed.
   **Recommended.**
3. **Fine grid at the existing 16-sample control tick.** Most accurate, aligns
   with `controlTick`. Rejected on measured cost: gravity is O(gated²) per step,
   and with 10 held notes a 16-sample grid costs **+66% CPU** (2.09% → 3.48% of
   one core) to buy a settling difference of 0.001 cents.
4. **Align the shell instead** — render oscillator 0 in `kMixChunk` chunks like
   the others. Makes the oscillators agree with each other but leaves the whole
   instrument dependent on the residual partial chunk, so host buffer size still
   changes the sound. Fixes the symptom we noticed, not the defect.

### Measured cost of the recommendation

10 held notes, 2 s of audio:

| grid | CPU | vs today |
|---|---|---|
| 512 (today, typical) | 41.9 ms (2.09% of a core) | — |
| **256 (proposed)** | **42.7 ms (2.13%)** | **+2%** |
| 128 | 44.9 ms (2.25%) | +7% |
| 16 | 69.7 ms (3.48%) | +66% |

256 samples is 172 Hz — roughly 57 updates across gravity's ~1/3 s settle time,
which is ample for a slow restoring force.

### Consequences

- **One golden moves**: `dyn-gravity` is the *only* scenario in the 147 that
  engages gravity (`grav` defaults to 0 and `gravityStep` early-returns below
  0.005). The other 146 are bit-identical. An earlier note in ROADMAP implied a
  broad re-measurement; that was wrong.
- **Both references change together** — `swarmdynamics.html` (PROTECTED, the
  spec) and `swarm_core.h`. Parity stays green because both sides move; the
  audio for gravity patches changes in phase, not in tuning.
- Oscillator 0 and oscillators 1..N integrate identically afterwards, by
  construction rather than by matching their chunk sizes.
- Unblocks the Tonality gap-24 slice-1 ask: with a stable integrator, swapping
  the ratio table becomes one attributable change instead of two entangled ones.
- `grav = 0` patches are untouched at every step of this.

### What would falsify the recommendation

If a listening pass on `dyn-gravity` finds the phase-trajectory change audible
and *worse* — the settling numbers say the destination is identical, but they
say nothing about the character of the journey, and gravity's whole point is
that settling is an audible physical event (decelerating beating). That is an
ear question, not a numbers question, and it should be answered before this is
ratified.

## ADR-086 Amendment 1 — the gravity grid is a fixed TIME (ACCEPTED)

**Date:** 2026-08-10 · **Status:** ACCEPTED, ratified same day.

ADR-086 shipped `kGravGrid = 256` **samples**. A sample-rate invariance probe,
written within the hour, found that this is a duration that shrinks as the rate
rises — 5.81 ms at 44.1 kHz, 2.67 ms at 96 kHz — so Euler truncation error and
therefore gravity's trajectory tracked the sample rate. Settle time drifted
**+0.42% at 96 kHz, monotonically**. The original fix removed a dependence on
buffer size and left one on sample rate: relocated, not closed.

**No golden could ever have seen this** — goldens are generated at 44.1 kHz
only, so parity is silent about every other rate. This is L0031's thesis
demonstrated on the fix that produced L0031.

**Decision.** `kGravGridSeconds = 256.0 / 44100.0` (5.805 ms), with
`gravGridSamples() = lround(sr * kGravGridSeconds)`. The value is chosen so the
result is **exactly 256 at 44.1 kHz**: verified, **252 goldens unchanged, 0
moved**. After the amendment the settle time is 0.00% / +0.16% / 0.00% /
−0.03% across 44.1–96 kHz — no longer monotonic, the residual being integer
rounding of the grid (279 samples at 48 kHz against an ideal 278.6), which is
inherent and musically nil.

Brings gravity under ADR-009 like every other time constant in the engine.

## ADR-087 — `subdiv_check` is a blocking gate; pan motion is a declared exclusion

**Date:** 2026-08-10 · **Status:** accepted (human ratified).

**Decision 1.** `tools/subdiv_check.cpp` runs in `./verify full`. It asserts
that rendering does not depend on how a buffer is subdivided — **the property
no golden can test**, because the generator and `parity_check` both render at a
fixed block, so both sides share any subdivision bug and parity agrees with
itself. All 147 scenarios passed for the entire life of the gravity bug and
could not have failed. Calibrated: reverting the segmenting gives FAIL at 1.093.

**Decision 2.** Pan motion (ADR-064) remains a **per-render-call** integrator
and therefore subdivision-dependent (measured 0.191 at chunk 333). Ruled: leave
it. The audible stake is near zero — worst-case step is 0.014 on a ±1 pan range,
about 0.06 dB at 21 Hz, below click threshold — and the reference
(`swarmsaw.html`) is a stage that will be graduated rather than patched
piecemeal. `subdiv_check` reports it as **KNOWN**, printed loudly and named in
its summary line, because an undeclared exclusion is how a gate rots into
decoration.

`./verify full` now runs **16 gates**. Parity 147/147 worst 4.262e-09.

## ADR-088 — B23 routing topology: dense crosspoint matrix (ACCEPTED)

**Date:** 2026-08-10 · **Status:** topology ACCEPTED (human-ratified); the id
allocation in §4 **ACCEPTED 2026-08-10** on the specification — see there.
(Header said PROPOSED until 2026-08-18 while §4's own body already read
"Therefore §4 is ACCEPTED": the summary line was not updated when the section
under it was resolved. Caught by triaging the FOUNDATIONS thread that resolved
it. A header that contradicts its own body is the same failure as an exchange
whose frontmatter contradicts its tree — LIBRARY L0037.)

### Decision

HYPERSAW's audio routing is a **dense crosspoint matrix**: every rack slot may
be fed from any source and from any **earlier** slot, each crosspoint carrying a
continuous coefficient, each slot additionally carrying an **initial value**
(`out_i = in_i + Σ g_ki · m_k`, the canonical crosspoint form). A slot that no
later slot reads is an output.

### Why, on our own evidence

`docs/design/routing-lab.html` benched six topologies over identical slots and
sources, with audio, a cost model, and a morph analysis. Two things decided it:

1. **Topology morph.** Quantum morph is a headline feature and a corner
   interpolates *values*. In a dense table a crosspoint at 0 **is** "not
   connected", so connecting and disconnecting are one continuous motion. Every
   sparse scheme stores topology as discrete structure, so adding an edge,
   reordering a chain, or repatching a bus is a **hard cut**. C is the only
   scheme that is simultaneously serial-capable, single-instance, morphable and
   modulatable.
2. **The cost objection was measuring the wrong thing.** Round 1 rejected C at
   "120 params" by conflating patch state with automation ids. Split properly it
   is **88 patch-state values and 8 automation ids** at 4 oscillators × 8 slots.

**Acyclicity by construction, enforced on the read side.** A slot may only read
earlier slots, so one forward pass is always correct and no runtime cycle check
is needed — an audio thread cannot afford one. The legality test lives where
edges are *consumed*, never in the editor: preset load, morph corners and
automation are all writers, so a guard on the write path is bypassable by
construction. (The lab's scheme-D implementation proved this the hard way, and
the principle generalizes past routing to every structure a preset can carry.)

### What this decision is NOT

It is not a bet on the shared library. FOUNDATIONS was asked one doorframe
question and answered that §3.5's chain is "a default shape, not a
constitutional commitment", explicitly declining to choose a topology and asking
not to be cited as design input. Its only role here is **removing the retrofit
risk**: divergence between HYPERSAW's topology and any future library shape is
information, not debt.

### §4 — Id allocation (ACCEPTED 2026-08-10, on the specification)

**Resolved 2026-08-10 by reading the specification we vendor.** This section
originally opened *"CLAP ids are append-only, so this cannot be unmade."* That
sentence is wrong in **both** directions at once, and FOUNDATIONS caught it by
citing `libs/clap/include/clap/ext/params.h` — our own tree — so it could be
checked without taking their word. Verified verbatim:

- **`params.h:212`** — *"Stable parameter identifier, it must never change."*
  The id-stability half is **stronger** than claimed: append-only is **mandated
  by specification**, not a convention we adopted for safety. The block
  allocation needs no further permission from anyone.
- **`params.h:70-77` ("VI. Adding or removing parameters")** — the parameter
  **set** is explicitly revisable: `restart()` → deactivate → apply → `clear(host,
  param_id, CLAP_PARAM_CLEAR_ALL)` for any id gone or reused → `rescan(ALL)`.
  The "cannot be unmade" half is simply **false**.
- **`params.h:328`** — *"It can only be used while the plugin is deactivated."*
  There is no mid-session rescan to survive; the question we were both asking
  contained a false premise.

**The accurate claim, which is weaker and truer:** *ids are append-only by
specification; the parameter SET is revisable through a documented
deactivate/rescan cycle whose host-side automation behaviour is unmeasured.*

**What this opens.** "Params that exist only when their rack does" is a flow
CLAP documents, not a door we assumed shut. It is not adopted here — a static
block is simpler and the host behaviour is unmeasured — but it is a **live
option** for a future rack rather than a foreclosed one, and that is worth
knowing before a second rack exists.

The original framing cited FOUNDATIONS **ROADMAP open question #15** — *"can shipping
hosts survive `rescan(CLAP_PARAM_RESCAN_ALL)` mid-session with automation lanes
intact? Decides whether the shell needs a bounded macro/proxy surface (five
vendors' answer) or can expose dynamic params (CLAP's promise, unverified). Runs
before any shell phase relies on either answer."*

HYPERSAW holds a `clap_host_params_t *` and **has never called rescan**. Our
param list is static for an instance's life by assumption, never by measurement.
So the "permanent" framing is the CONSERVATIVE reading, not a verified one.

**What survives the uncertainty, and what does not:**

- **The block allocation is safe under BOTH answers** and is therefore still
  proposed: a clean block costs nothing if ids turn out to be revisable, and
  saves everything if they do not. Hygiene either way.
- **The justification does not survive**, and matters beyond this ADR: if hosts
  do survive a rescan, HYPERSAW gains an option it has been treating as
  foreclosed — params that exist only when their rack does, rather than a
  statically allocated block sized for the worst case. That is a different
  design, not a tidier version of this one.

**Therefore §4 is ACCEPTED**, on the specification rather than on the
measurement. The residual empirical question — what a host does to an automation
lane across the documented restart cycle — is narrowed, no longer blocking, and
carried separately (`offer-param-rescan-spike.md`, accepted by FOUNDATIONS with
two amendments: cases must run the legal cycle rather than an out-of-spec live
rescan, and the clap-wrapper VST3 row is to be protected ahead of extra hosts,
since most hosts meet us through the wrapper rather than through CLAP).

**Process note worth keeping.** We held §4 on a library open question that was
filed "pre-shell" — i.e. scoped against *their* phases, not against the fact
that it gated a consumer's current work. Their words: *"a phase gate that
ignores its correspondent's blocker is not a gate, it is a schedule."* The
lesson cuts both ways: we also sat on it rather than asking, and it surfaced
only because the human noticed the two threads were the same question.

### The proposal itself, unchanged

- **Topology is patch state, not CLAP params.** Crosspoint on/off is discrete
  and nothing needs to automate it; morph corners are internal snapshots and
  reach it without host ids. Only the continuous quantities get ids.
- **Routing gets its own stride block: `10000 + rack*1000 + local`.** Rack 0
  occupies 10000–10999. Rationale: ADR-082's amendment had to move once already
  because a flat space ran out on the day it was needed, and stride IS capacity.
  Oscillators own 0–2999 (`kOscStride` 1000, third slot reserved); 3000–9999
  stays unallocated so a future block does not have to squeeze.
- **Today's id cost is 8 per rack** — per-slot amount ×4, per-slot initial
  value ×4.

**Not implemented.** This ADR records the ruling; the increment that builds it
is scoped in ROADMAP.

## ADR-089 — Gate changes are asymmetric: adding is delegated, weakening is not

**Date:** 2026-08-11 · **Status:** accepted. Human: *"I'm just going to have to
trust your judgment on those gates because I don't really have any insight into
them."*

### The problem with taking that at face value

The charter requires a human decision before "editing `./verify` or the gates it
runs". Its purpose is that **a weakened gate is invisible** — nothing goes red,
the suite still prints GREEN, and the loss is silent. A human who cannot inspect
the gate cannot supply that check, so accepting the delegation unchanged would
remove the safeguard rather than relocate it.

### The asymmetry the charter does not draw

Two operations are covered by one rule and they carry opposite risk:

| operation | effect on the suite | who decides |
|---|---|---|
| **adding** a gate | strictly stricter — can only turn green→red, never red→green | **agent may proceed**, with calibration recorded |
| **weakening** a threshold, **skipping**, **xfail**, **removing**, or **narrowing** a gate | strictly looser — can turn red→green silently | **always the human**, no exceptions, no delegation |

Adding a gate cannot hide a regression; it can only expose one or cost time.
Every failure mode the original rule protects against lives on the second row.

### Conditions on the delegated half

A gate may be added without asking only if all hold, and each is recorded in the
gate's own source so a later reader can audit without re-deriving:

1. **Calibrated** — proven RED on the defect it exists for and GREEN on restore.
   A gate never shown to fail is decoration.
2. **Threshold from measurement**, with the regression's magnitude on record.
   `samplerate_check`'s 0.3% sits between a measured 0.163% and 0.419%; a 1%
   tolerance would have passed its own defect.
3. **Vacuity control** — a case that must read exactly zero, or a distinctness
   check, so it cannot pass with the feature absent.
4. **Deterministic**, and cheap enough to state (`routing_check` 0.00 s,
   `samplerate_check` 0.02 s against a 3–4 minute suite).
5. **Exclusions declared loudly** in the gate's own output — `subdiv_check`
   prints pan motion as KNOWN and names it in its summary line.

### Applied

`samplerate_check` and `routing_check` are gated. `./verify full` runs **18**
gates.

### Not amended: the charter itself

`CLAUDE.md` above §Domain is the invariant harness layer and is explicitly not
edited per-project. This ADR records a project-level working agreement about how
that rule is applied here; it does not rewrite the rule. If the distinction is
worth having everywhere, it belongs upstream in the doctrine repo, not in this
file.

## ADR-090 — Per-oscillator pan is an IMAGE SHIFT, not a balance (ACCEPTED)

**Date:** 2026-08-11 · **Status:** ACCEPTED (human-ratified). Not yet
implemented; it is parity-affecting and scoped below.

### Decision

A per-oscillator pan control **offsets every voice's seat** in that
oscillator's stereo field. It is not a mix-stage balance.

### Why, and why the cheap option was rejected

The cheap option is a balance at the mix stage: `gL = min(1, 1-pan)`,
`gR = min(1, 1+pan)`. Exactly unity at centre, never boosts, zero parity risk,
shell-only. It is what a mixer does to a stereo source.

**But HYPERSAW's stereo image is GENERATED, not recorded.** Voices are *seated*
across the field by `panLayout` / `panScatter` / `panCurve`. Attenuating one
channel does not move that image — it **deletes the far side of the ensemble**.
Hard-panning a swarm under a balance law silences the voices seated opposite,
which is a different instrument, not a different position. Balance is the right
law for material that arrived stereo; this material did not.

### Consequences, honestly

- **Parity-affecting.** Seats live in `swarm_core.h`, and the reference
  (`swarmsaw.html`, PROTECTED) must move with it. Goldens that engage pan will
  re-measure.
- Composes with the existing seat laws rather than fighting them: a shift is
  applied to `panBase` before `panScatter`/`panCurve`, so the image's *shape*
  is preserved and only its centre moves.
- Edge clamping is the open sub-question: a shifted seat can exceed ±1. Clamp
  (voices pile at the edge, image compresses) or wrap (voices reappear on the
  far side, image tears). **Recommend clamp** — compression is a musical result,
  tearing is not.

### Scope

Its own increment, not folded into B23's. New per-oscillator param, seat offset
in the core, reference edit, goldens re-measured, and a probe proving the far
side survives a hard pan — which is exactly what a balance law would fail.


## ADR-091 — Engine family expansion: SAW renamed HYPERSAW; a hyperpop-oriented experimental engine track opens with the formant engine (ACCEPTED)

**Date:** 2026-08-17 · **Status:** ACCEPTED (human direction). Documentation,
ingestion and label rename done in this change; engine implementation is
roadmapped, not started.

### Decision

1. **The engine formerly named SAW is named HYPERSAW.** Label only: `kEngineLabels[0]`,
   the GUI's visible string, and the engine-name mentions in SPEC/ACCEPTANCE. The
   enum value (0) and the state key are untouched, so no saved patch changes.
2. **The instrument's sound-design space widens** from "the supersaw taken
   seriously as physics" to a *family* of experimental, responsive engines with
   dynamical characteristics, oriented toward the sounds of hyperpop. HYPERSAW and
   SPECTRA are the first two members; each new engine follows the same discipline —
   a browser prototype that is the oracle, a `SPEC-*.md`, seedable determinism,
   parity as correctness.
3. **The first new engine is the formant engine** (working name CANTO): FOF /
   pulsar grain synthesis where the fundamental is a firing rate, formants are
   grains, every continuous control is a mass on a spring, and one hidden register
   state R reshapes the whole engine as pitch descends. Ingested today as
   `horde_formant_pulsar_fof.html` (prototype, `FormantCore`) and `SPEC-FORMANT.md`
   (spec v0.1, moved from `HORDE_formant_engine_spec.md` into the `SPEC-*.md`
   convention so it is found where the other engine specs are found).

### Triage of the ingested prototype — what it is and is not yet

- **It has a separable core** (`class FormantCore` with a headless `render(out)`),
  which is what makes it a candidate oracle at all (ADR-003 shape).
- **Its register formulas match the spec exactly** (`bwk`, `reg.shift`, `tilt`,
  `tex`, `sub` — checked line for line).
- **It is NOT yet a valid oracle**, for one reason: masking uses `Math.random()`.
  The spec (§9) already requires a seedable RNG for parity; the prototype does not
  have it. Under this project's determinism invariant (mulberry32 streams only,
  SPEC §5.7) this is a blocker for golden generation, not a nit. **Fixing it is a
  spec-preserving edit** — a seeded stream substituted for `Math.random()` — and is
  the first roadmapped item.
- Both `performance.now()` reads are UI-side (event-train display, a 1.5 s meter
  window), not in the DSP path. Fine.
- It links a Google font (`<link href="https://fonts.googleapis.com/...">`). Not
  blocking for a root prototype; must be inlined or dropped before any lab copy
  (self-contained rule) or webview embedding (CSP).
- It does not load in `lab_load_check` (bare `devicePixelRatio` in a canvas `fit()`
  helper). Root prototypes are not swept today, so this gates nothing now; it is
  recorded so the lab copy fixes it on arrival.

### Consequences

- The prototype and spec are **protected paths** from this ADR on, with the same
  meaning as the other seven: an edit is a spec change. The one edit already
  sanctioned is the seeded-RNG substitution above.
- Corner colour, morph, mod-matrix and presentation-table work all assumed one or
  two engines; a family means the presentation table's `scope` prefixes and the
  engine selector's real-surface gating both grow. That is the design work, and it
  is on the ROADMAP rather than implied.

## ADR-095 — The FX rack owns dry/wet (ACCEPTED)

**Date:** 2026-08-19 · **Status:** ACCEPTED (human approved the contract 2026-08-15;
this is its rack-side half). The gate half landed first, deliberately.

### The rule

```
out = (mix == 0) ? in : lerp(in, wet, mix)
```

Applied by the RACK to every slot type. `mix == 0` is an early-out, so passthrough
is bit-identical **by construction** rather than by each slot's implementation
remembering to honour it. That is the property the old design lacked, and the
reason Notch shipped collapsing stereo to mono at a setting a patch author reads
as "off": there was no rule a new slot type could not break.

`mix == 1` runs the wet path untouched — no copy, no lerp — which is why every
patch predating the contract is bit-identical and parity did not move (156/156).

`amount` stops carrying bypass duty and becomes purely per-slot character. Gain's
0.5-is-unity and Comp's always-on brickwall stop being anomalies: they are simply
what those slots DO at `mix = 1`.

### Two pins retired, which is what paying a debt looks like

`slotcontract_check` pinned three violations when it landed. Two are now gone:
**Comp** and **Notch** had *no identity point at any amount* — they are bypassable
now, universally. **Comb's +8.8 dB at amount 0.5 remains pinned**: that is a fact
about the AMOUNT axis and bypass does not touch it.

The gate's assertion changed with the rule. It no longer hunts a per-slot
`identity_at`; it asserts the universal guarantee — **`mix = 0` is bit-exact for
every slot type** — which is the assertion a new slot cannot escape.

### The plant that did not fire, and why that mattered

The first plant removed the `mix <= 0` early-out for one slot type. The gate stayed
green — **correctly**. With `mix = 0` the blend path computes
`dry + (wet − dry)·0 = dry`, so the early-out is an OPTIMISATION and the lerp is
the guarantee. The plant tested the wrong thing.

Replanting against the actual guarantee — inverting the blend so `mix = 0` means
fully wet — turns the gate **RED on four slot types at once**: Drive 0.151,
Filter 0.192, Comb 0.438, Notch 0.229. Restored: green.

That distinction is worth keeping: a check can look untested when the plant, not
the check, is the thing that is broken.

### Surface

Four global params, ids 133-136, appended. Default **1** so nothing existing
changes. Placed in the FX rack group.

## ADR-094 — Saw shape (glass) folded from the detune lab (ACCEPTED)

**Date:** 2026-08-19 · **Status:** ACCEPTED (human: *"I approve of that plan"*).
**Spec change:** yes — `swarmsaw.html` is a protected reference and it changed;
the port changed identically in the same commit, so this is a fold, not a
divergence.

### What was missing

All four controls — `sawBase`, `sawProfile`, `round`, `roundHi` — existed **only**
in `docs/design/detune-lab.html`. Zero occurrences in the SAW reference, the core,
the shell or the presentation table. This was never a GUI omission: **the fold had
never happened.** `shape` (id 69, ADR-058) is a different axis entirely — saw to
band-limited square, not roundness.

The fifth fold in an established sequence: tone tilt (ADR-060), hi-tame (061),
drift modes + keep-phase (062), glide (063), and now this. Same terms as all four:
**parity-safe superset, inert at its defaults.**

### The feature

Two independent axes, applied in the reference's order — BLEP, then base, then
roundness:

- **BASE** (`sawBase`) crossfades the top-level saw through four
  sawtooth-flavoured variants: curved, fat, driven, aggressive. **Anchor 0 is the
  BLEP saw**, so the crossfade begins at exactly what the engine already produced.
- **ROUNDNESS** (`round`, `sawProfile`, `roundHi`) morphs the result toward one of
  five anchors — glass (parabola) · soft · hollow (triangle) · pure (sine) · reedy
  — per voice, optionally scaled by the voice's position in the spread so higher
  voices round more.

Both banks are carried over **verbatim**, placeholders and all: the lab's own
comment says the real profiles are still to be measured from synths, and a fold
moves code rather than improving it, or parity stops meaning anything.

### Placement, which parity alone would not have caught

The port carries a stage the reference does not — ADR-058's `shape`, a C++-only
superset. The new stages go **before** it, exactly where the reference puts them.
ADR-058 is the stage that yields position because it has no reference to be
faithful to. Ordering them the other way would have been invisible to parity until
someone set both non-zero, which is the kind of latency this project keeps paying
for.

### Inert first, then actually exercised

The fold moved **147/147 to 147/147** — bit-identical, by construction, because
`sawBase <= 0.001` skips the crossfade and `round <= 0.001` leaves every voice at
zero roundness.

**That green says nothing about the feature**, which is exactly the hole ADR-093
found in the glide goldens. Three scenarios added — `saw-base`, `saw-glass`,
`saw-round-hi` — taking parity to **156/156**, and both proved able to fail:

| plant | result |
|---|---|
| drop the `roundHi` pitch scaling | **FAIL** `saw-round-hi`, rms 3.591e-02 |
| swap the hollow anchor for glass | **FAIL** `saw-round-hi`, rms 8.329e-03 |
| restored | 156/156, worst 4.262e-09 |

### Surface

Four **per-oscillator** params, ids 129-132 appended (`params.h:212` — ids never
change). Per-oscillator because giving each oscillator its own saw character is
the point of having two. Presented as a *Saw shape* group on the OSC page.

## ADR-093 — Quantiser ties resolve toward the previous emitted step (ACCEPTED)

**Date:** 2026-08-19 · **Status:** ACCEPTED (human ruling in session, "go for it").
**Spec change:** yes — `docs/design/bend-lab.html` is a protected path and the
reference changed. The port changed identically in the same commit, so this is a
spec change, NOT a divergence.

### Two defects, one code path

**1. Ties resolved by loop order.** `if (d < bestD)` with a scan ascending from
`floor(semis) − 12` means the lower candidate arrives first, claims `bestD`, and
the equal-distance higher candidate fails `d < bestD`. Every tie resolved
**downward**, decided by iteration order rather than by any musical choice.

Tonality found this first, in their own quantiser, independently
(`HYPERSAW-002` §2). Their domain made it severe: `conform_to_scale` quantises
**integer MIDI notes**, so every out-of-scale pitch class is exactly equidistant
and all five accidentals in a major scale tie deterministically — a fixed
direction did not resolve a corner, it decided every accidental and sagged
chromatic lines flat. **Ours is narrower and we said so** after first overstating
it: our input is the glide's continuous output, so a tie is a knife edge one ULP
wide, reachable when the law is off (`x = target` bit-exactly) and rare otherwise.

**2. Chromatic mode disagreed between reference and port.** Chromatic never ran
the candidate loop at all — the reference used `Math.round`, the port used
`std::lround`, and those **differ on negatives**: `Math.round(-1.5) = -1` (half
toward +∞) versus `lround(-1.5) = -2` (half away from zero). A full semitone of
divergence in the shipped plugin, at every exact negative half-step, in a project
whose definition of correctness is 1e-6 parity with that reference.

### Decision

- **One candidate search for both modes.** Chromatic is "every pitch class
  admitted" running the same loop. There is no rounding function left to
  disagree about, which closes defect 2 by construction rather than by matching
  two library behaviours.
- **On an exact tie, prefer the candidate nearer the previous EMITTED step.** The
  output line is the melody a listener hears, so continuity is owed to the output,
  not the input (Tonality's ruling, adopted). Deterministic and replayable, where
  the existing hysteresis is continuity-in-time and depends on wobble history —
  both are kept because they do different jobs.
- **With nothing emitted yet, keep the lower candidate.** Stated as a choice
  rather than left to loop order; reachable only on the first quantised sample.

### Why the goldens never saw either defect

The standing gesture settles at **0.5**, which in C major is equidistant from
nothing. The whole tie path was unrendered, so 147/147 and `glide_check` were
silent about it — `L0031` exactly: a reference oracle certifies agreement over
the surface the reference RENDERS, and this surface was never rendered.

Two scenarios added, `glide-tie-scale` and `glide-tie-chrom`, landing on exact
ties of both signs. **`model = kOff` is load-bearing in both**: under any moving
law the output approaches asymptotically and never lands exactly on a midpoint,
so a tie scenario using one is a test that cannot fail. The first version used
`kConstTime` and a planted regression sailed straight through it.

### Proof the gate can fire

- Remove the tie-break → **RED**, `glide-tie-scale` rms 1.1547, `glide-tie-chrom`
  rms 0.4564.
- Put chromatic back on `lround` → **RED**, same two scenarios.
- Restored → **GREEN**, both at rms **0**.

`./verify full` green; parity 147/147 unchanged (worst 4.262e-09), the pre-existing
five glide goldens byte-identical.

### One thing this exposed and did not fix

The scenario list exists **twice** — `tools/golden/gen_glide_goldens.mjs` and
`tools/glide_check.cpp` — including the gesture. Adding a scenario to one produced
goldens the other never read, and the check stayed green while covering nothing.
Recorded as a known duplication; the fix is for the C++ to read the generator's
manifest rather than mirror its table.


## ADR-092 — WARP (distortion engine) ingested as a CANDIDATE; it is FX-C's prototype, not a fourth voice engine

**Date:** 2026-08-18 · **Status:** accepted · **Human:** *"just finished another
prototype engine. This one will require more tweaking before it's ready to become
the gold standard (just like the other one)."*

**Context.** `horde_distortion_engine.html` + its spec arrive one day after the FX
roadmap queued **FX-C, a morphing waveshaper with experimental hysteresis**, and
noted that SPEC-FORMANT §10's promised "distortion engine spec" did not exist yet.
It exists now, and it is that: a simplex-blended shaper (§4), three independent
memory mechanisms (§5), an all-pass phase network pre and dispersion post (§3, §6).

**Decision.**
1. **WARP is FX-C's prototype, not a fourth engine in the ADR-091 family.** The
   family (HYPERSAW · SPECTRA · CANTO) are *sources*; WARP is a *post-stage*. The
   spec says so itself (§10: "WARP is the shared post-stage; parameter surface
   should be identical regardless of source"), and that sentence is the strongest
   argument against filing it as a sibling: a stage every source hands off to is a
   slot type under the FX slot contract, which is exactly FX-C's slot.
2. **Ingested as a CANDIDATE, not a reference** — the same status CANTO holds, for
   the same reason. `SPEC-DISTORTION.md` becomes a protected path; the prototype
   does NOT yet join the seven spec-in-code references.
3. **The blocker is identical to CANTO's, at `horde_distortion_engine.html:161`:**
   `Math.random()` inside `WarpCore.render()`, driving the `walk` parameter's
   coefficient drift. Same seed + same note order must give identical output
   (CLAUDE.md invariant, SPEC §5.7). A prototype that cannot reproduce itself
   cannot be a parity reference, because there is nothing stable to be at parity
   WITH. One sanctioned edit outstanding: replace with a mulberry32 stream.

**Consequences.** WARP inherits FX-C's prerequisite — the FX slot contract must
exist first (rack owns dry/wet; per-slot `{identity_at, blends_dry, changes_image,
changes_level, latency_samples}`), and its `changes_image` is certainly true given
the all-pass network. Its hysteresis must declare its own settling and face the
feedback lab's edge-width scan before it is allowed on any feedback path. The
spec's own §10 already lists the work honestly — clicks from block-rate all-pass
coefficient recompute, no oversampling, LUT resolution at high fold order — and
that list is the queue, not a disclaimer.

**Rejected:** filing WARP as engine #4. It would give the family a member that
generates nothing, and would duplicate FX-C rather than fulfil it.

## ADR-096 — The note lane travels by the same five laws as the bend lane (ACCEPTED)

**Date:** 2026-08-19 · **Status:** ACCEPTED (human proposed the merge: "could we
increase the bendTau range and then merge them?").

### The problem

Two controls held the same physical quantity. `glide` (id 33, ADR-026/076) was a
lag time in SECONDS driving a hard-wired one-pole in HERTZ at `swarm_core.h:1238`;
`bendTau` (id 109) was a lag time in MILLIseconds feeding `GlideCore::kLag`. The
laws were identical — `glide_core.h:110` and the old site are the same
`x += (target - x) * (1 - exp(-dt/tau))` — so the note lane had one law where the
bend lane had five, and the human wanted all five on both.

### The ruling

The note lane runs `GlideCore`, the same struct the bend lane runs. Ids 137-145
mirror the bend block field-for-field minus `retMul` (a note has no home pitch to
spring back to — the same rule the bend block already states). `noteLawLink`
selects own-settings or follow-the-bend-law; the shell resolves the link and
pushes a finished struct, so no shell-only concept enters the DSP.

### Why id 33 survived instead of being superseded

The tempting move — mint `noteTau` in ms and retire id 33 — puts a silent 1000x
between a shipped preset and its meaning: `docs/presets/serum-parity-reference.json`
already stores `"glide":0.89`, and 0.89 read as milliseconds is not slow
portamento, it is no portamento. Id 33 therefore keeps its id, its key, its
SECONDS and its range, and the core converts at the use site. `bendTau` widened
1-400 -> 1-2000 ms instead, which is free because nothing has ever stored it —
it shipped hours earlier (bbdfd0c) and appears in no preset.

**A merge between two units is a migration, not a rename.** The cheap direction
is the one where the stored value never moves.

### Why the defaults are own-settings + lag

`noteLawLink` could not default to FOLLOW: `bendLaw` ships off (human ruling
2026-08-19), so a following note lane would travel instantly and portamento would
vanish from every patch that ever set glide. Own-settings + `noteLaw = lag` +
tau-from-id-33 reproduces the pre-ADR-096 behaviour exactly. Sync is the option,
not the base state.

### The divergence, stated plainly

Travel moved from the HERTZ domain to SEMITONES. The law is the same one-pole;
the domain is not, and Hz-linear travel audibly accelerates at the bottom of a
wide interval. **No gate can see this.** `trajectory_check`'s glide criterion is
"within 1c in 12 tau" — tau-relative, so both domains pass it identically; a
plant (tau x10) was run and the gate went RED, confirming it covers the lane's
TIMING and confirming that timing is all it covers. Parity is silent by
construction: the JS reference has no glide at all (156/156 unmoved, worst
4.262e-09). Curve SHAPE is therefore a human ear ruling — filed as NTR-3.

### Also landed

`shown_when` gained AND across comma-separated clauses. One key could not express
`noteLawLink=0,noteLaw=3`: gating on the link alone showed all seven law controls
at once, gating on the law alone showed them while the lane was following. This
is the same missing capability the chord layer will need for OR-across-keys.

## ADR-097 — Per-note (MPE) bend obeys the bend law (ACCEPTED)

**Date:** 2026-08-20 · **Status:** ACCEPTED (human: "I remember the lab having a
toggle to apply the same inertia settings to MPE pitch bend").

### The gap

`docs/design/bend-lab.html` gives every sounding note its own inertia state and
steps it with the SAME params as the wheel — `nt.bend.step(nt.bendTgt, P,
nt.midi)`, line 539. The port applied per-note bend **instantly** at all three of
its entry points: the ADR-038 note-on latch, `CLAP_NOTE_EXPRESSION_TUNING`, and
member-channel `0xE0`. So a patch with a bend law shaped the wheel and left MPE
snapping — in the one context where character matters most, because on an MPE
controller the bend IS the performance.

This is the third unported reference behaviour found in two days (the wheel,
`qTime`, this). They share a shape: the reference had it, a gate existed for the
neighbouring case, and nothing asked whether THIS transport was covered.

### The rule

One traveller per note SLOT, not per channel — two notes on one channel can be at
different bends mid-flight, and a channel-keyed lane would drag them together.
`bendLane = true`, because return-toward-rest is as meaningful per note as on the
wheel. Both lanes step on one clock, at the existing bend-grid boundary.

A fresh strike **arrives** at its latched bend instead of travelling to it: the
note did not exist while the controller moved, so gliding in from zero would
invent a gesture the player never made. That mirrors the reference's `reset()`.

### `bendMpeLaw` (id 149) ships FOLLOW

Because the reference has no way to give per-note bend a different character from
the wheel — it steps both with the same P. The toggle exists anyway for the one
real case: a player wanting the raw controller under their finger while the wheel
keeps its character. Inert at defaults regardless: `bendLaw` ships off, so
following it is the instant write either way.

### Two stranding paths, both closed

`stepNoteBends()` only runs inside the `bendActive()` branch of `process()`. So a
note left travelling when the law is switched off — or when `bendMpeLaw` is
turned off — would never be stepped again and would hang at a partial bend
forever. Both transitions now land every travelling note immediately.

### Evidence

`mpe_check`: early 659 Hz bin 0.00597 against settled 0.20031 — it travels AND
arrives. The late read is the must-arrive control: without it, a lane that simply
DROPPED the bend would pass the early check for the wrong reason. Plant (instant
application restored) reads early 0.32062 — already at the destination — and the
gate goes RED.

## ADR-098 — Control columns FLOW; they do not tile (ACCEPTED)

**Date:** 2026-08-20 · **Status:** ACCEPTED (human: "come up with a formatting
rule that doesn't allow so much empty space").

### The rule

> A control column is a **flow**, not a grid. Panels are laid out with CSS
> multi-column, so a panel's height is exactly its content's height and the next
> panel begins immediately beneath it. No panel's size may be a function of any
> other panel's size.

`.ctlcol { display:block; columns:240px; column-gap:10px }` with
`break-inside: avoid` on every direct child.

### Why the old layout could not avoid the waste

`.ctlcol` was `display:grid`, and a grid lays items out in **rows**. Every box in
a row is stretched to the height of the tallest box in that row. With Bend at ~25
control rows sitting beside Pitch at 3, Pitch was painted **498px tall around
136px of content**. Measured across the whole OSC control column: **37.2% empty**,
and none of it between panels where it would read as breathing room — all of it
inside them, which reads as a bug.

After: **1.9%**, and the column got SHORTER (952px → 764px) holding the same
controls, so it is denser and needs less scrolling at once.

### Why this is not a threshold to police

The defect class is removed **by construction**, not by vigilance: multi-column
flow has no row concept, so there is no mechanism left that can stretch a panel
to a neighbour's height. Nothing needs to check a percentage because nothing can
produce the percentage. (Doctrine: safety by construction, not by vigilance.)

That is deliberate, because a pixel-level layout gate is **not CI-able here** —
it needs a real layout engine, the same reason pluginval/auval live in the
human-paced Layer-E set rather than `./verify fast`. A rule enforced by structure
outranks one enforced by a check that cannot run.

### Amendment 1 (2026-08-20) — the rule is about TILING, not about `.ctlcol`

First application only converted `.ctlcol`. FX and MIX tile their clusters
straight into the page grid, and both carried `align-items:start` — which stops a
box being STRETCHED but does nothing about the ROW still being as tall as its
tallest member. On FX the ~560px "About the rack" note set row 1's height and
dropped "FX rack" into row 2, far below four short slot boxes. **Half the rule
fixes half the defect**, and the half it fixes is the one that is easy to see.

Stated properly, the rule is: *wherever clusters are tiled*, they flow. Both
pages now do.

Known consequence, not a defect: an unbreakable tall panel sets a floor on the
column height, so FX's four slots wrap 1-2-3 / 4 rather than sitting in a row.
Reading order is preserved (top-to-bottom, then right).

### The tradeoff, taken knowingly

Reading order becomes top-to-bottom per column rather than left-to-right. That is
the newspaper convention and what every masonry layout accepts. `break-inside:
avoid` applies to EVERY direct child, not just `.cluster` — the OSC column also
carries a bare `.note`, and flowed prose with no break rule splits across a
column boundary and reads as orphaned text.

## ADR-099 — A silent oscillator is skipped, not rendered-and-zeroed (ACCEPTED)

**Date:** 2026-08-20 · **Status:** ACCEPTED (human: "I am starting to notice a bit
of a processor choke... is there anywhere we can work on efficiency before we make
it even heavier?").

### Measure first, and what the measurements said

- `cpu_bench` (core alone): 1.81% of a core at 7 voices x 8 notes — the swarm
  math is NOT the choke.
- New `shell_bench` (the real plugin through the CLAP factory): heavy patch tops
  out at 5.9%. The DAW's ~40% is this figure times sample rate, buffer size,
  held tails and instance count — so every shell percent is a multiplier.
- The decisive row: **oscillator 2 at its shipped default vol 0 cost exactly the
  same as oscillator 2 audible** (5.68% vs 5.69%). Every single-oscillator patch
  paid double.
- `sample` profile: renderSeg 67%, libm transcendentals ~23% (sincos/atan2/exp2
  from controlTick's order-parameter and coupling loops).

### The rule

An oscillator k >= 1 whose output is provably exact zeros — its own `p.vol` is 0
(the gain multiplies INSIDE render), or its mixer gain has SETTLED at 0 — is
skipped entirely: no render, no chunked sum, meter forced to 0. Skipping an
exact-zero contribution is output-identical by construction.

Not gain-smoothed-out mid-ramp: the skip waits for the smoother to settle, so a
mute's fade-out completes before the core stops. Oscillator 0 is not skipped (it
renders straight into the output buffer, and silencing the primary is not a
pattern worth complexity).

### The traded behaviour, stated

While skipped, the core's envelopes and phases FREEZE. Raising the volume
mid-held-note resumes voices from where they paused rather than where they would
have decayed to. That is the "muted layer costs nothing" contract every DAW
mixer teaches, and it is the entire source of the win.

### Numbers

default patch 8 notes: 3.51 -> 1.82% (−48%) · 16 voices: 5.64 -> 3.78% (−33%) ·
osc2 audible: unchanged (5.67%) — the cost returns exactly when bought.

### Oracles

mixer_check gained the vol-path case (vol 0 -> 0.4 mid-note un-skips; the shipped
default IS the skip state, and the pre-existing solo-restore case cannot see this
path). Plant (sticky skip `< 0.5`) takes it and four solo/mute cases RED.
parity 156/156 unmoved; state, routing, mpe, rtsafety, notefuzz all GREEN.

### Left on the table, recorded not taken

controlTick's order-parameter sincos runs even at K = 0, where the coupling force
it feeds is multiplied by exactly zero (~15-20% of the remaining bill). Skipping
it is output-identical for AUDIO but stales R/psi, which the VIZ reads — so it
needs a design ruling on viz staleness, not a stealth optimisation. ROADMAP has
the entry.

## ADR-099 Amendment 1 — Skip only when OFF; volume is just volume (2026-08-21)

The vol-0/settled-mute skip bought CPU by freezing the core, and a frozen core
freezes its VISUALS: a silent osc 2 half-rendered (viz stopped, meter zeroed)
while a silent osc 1 — never skipped — kept moving. The human read it exactly:
"currently it just looks like a mistake." Human ruling: the skip only kicks in
when the oscillator is switched OFF (ADR-100's enable), never when it is merely
silent.

The CPU is kept a different way: **osc 2 now SHIPS off** (enable default 0 for
osc > 0), so the default patch takes the cheap path through the honest switch
instead of through a silently frozen core, and "Osc On" unchecked SAYS why the
oscillator is dark. Pre-ADR-100 patches (no "enable" key) migrate to
enabled-everywhere — they were saved when everything always rendered.

paramscope_check's setup now switches osc 2 on before raising its volume — the
check predates the switch; its assertions are unchanged.

Fresh-default cost after: 1.97% (was 1.82% under the vol-skip) — the difference
is the honest price of visuals that never lie.

## ADR-100 — Oscillators have an on/off switch, and OFF means gone (ACCEPTED)

**Date:** 2026-08-20 · **Status:** ACCEPTED (human: "add the ability to turn
oscillators off and on instead of just volume", with the morph design: "they can
just toggle on and gradually increase the volume as they move between corners").

### The rule

`enable` (base id 150, PER-OSC so each morph corner can hold its own answer per
oscillator). OFF hard-kills the core's voices — gate AND envelope to zero — and
joins the ADR-099 render skip, so a disabled oscillator costs nothing at all.
Osc 0 disabled zeroes its span by hand (it renders straight into the output
buffer; every other path relies on render() doing the zeroing).

OFF differs from vol 0 on purpose: vol 0 freezes envelopes for resume (a muted
layer); OFF kills them (the oscillator is not part of the patch right now). Both
transitions kill — OFF because the switch means silence now, ON because voices
frozen since the disable would resume as zombies at whatever loudness they froze.

### Measured (enable_probe, through the CLAP factory)

both on 4 held: rms 0.284 / 3.71% · osc2 off: 1.96% · both off: rms 0.00000 /
0.03%, tails KILLED · re-enable + new note: SOUNDS, old notes stay dead.

### The choke, diagnosed while here (user_patch_bench, the human's own patch)

Both oscillators, 16 voices, release 5 s, spring bend, comb + drive, 48 kHz /
64-sample blocks: idle 0.14%, ~1% per held note — and EIGHT SECONDS after
note-off the CPU had not moved (8.7% flat). The release envelope is a one-pole
with the knob as its TIME CONSTANT and voices die at -60 dB, which a 5 s release
reaches after ~7 tau ≈ 35 s. Every note burns full price for half a minute,
inaudible for most of it. "Each takes a long time to let up" is that arithmetic.
Changing the kill threshold or the curve is a SOUND change and a reference
change — parked on the ROADMAP as an open question, not smuggled in as an
optimisation. The debug-mode hypothesis was checked and cleared: Release, -O3,
-DNDEBUG, zero assert strings, installed binary byte-identical to the artifact.

### Also in this change

- `shown_when` gained its OR tier (`;` between groups) with its first real
  consumer: the Scale section serves BOTH quantisers, so it shows on
  `bendQuant=2;noteQuant=2`. The tonic selector was never missing — its gate and
  the dynamically-built picker's disagreed, so Scale showed while Root hid.
- COPY PATCH / PASTE in the tab bar over hzGetState/hzApplyState — the "send me
  the patch" loop the human asked for, and the seed of the preset system.

## ADR-100 Amendment 1 — Enable-ON re-strikes what is held (2026-08-21)

"Sometimes osc 2 doesn't work": switching an oscillator ON mid-chord produced
nothing until the next fresh note (the ON transition killed voices, correctly,
and nothing re-created them). The human enabled it, played nothing new, heard
nothing — "broken" was fair. Enable-ON now re-strikes every active tag on that
core at its held velocity (velocity joined NoteTag for exactly this). A fresh
attack rather than a resumed envelope is intentional: the note is NEW on this
oscillator. Side effect that is really the point: MORPH-driven enable flips
now pop the oscillator in WITH the held chord — the corner design working.
Probe: osc1 vol 0 + osc2 off = silence 0.00000; osc2 ON mid-hold = 0.069, no
fresh note. The second reported cause is the documented v1 seam: with morph ON,
corners captured with osc2 off will force it off every grid tick — the
corner-editing phase (and exempt) is the designed fix.

## ADR-101 — The saw-shape anchors are tabled; the functions stay the truth (ACCEPTED)

**Date:** 2026-08-20 · **Status:** ACCEPTED (human diagnosis, confirmed by bench:
"I think I figured out what's eating the most CPU — saw shape").

### The measurement that confirmed the human

16-voice baseline 3.71% of a core. sawBase 0.5 alone: **8.07%**. Roundness +
profile: **8.62%**. All four stages: **12.38%** — the section cost 2.3x the rest
of the synth combined. Cause: `std::pow(|r|, 1.4)` (and sin/tanh) per sample per
voice, two anchors per stage for the blend — ~11M pow/s at 128 oscillators.

### The rule

The reference anchor FUNCTIONS remain the single source of truth — the tables
are built FROM them at load (never on the audio thread) and the render loop
reads the tables. 16384 cells + linear interpolation; N+1 points so the last
cell interpolates toward f(1-) instead of wrapping +1 down to -1. Anchor
selection (index + blend fraction) hoisted to the segment head — it was
recomputed per sample per voice, clamp, floor and all. A zero-weight blend side
is skipped entirely.

### The divergence, measured not argued

Linear interpolation is an approximation, so this is an INTENTIONAL divergence
from the reference path and is recorded as such. Budget: smooth anchors ~2e-8;
the pow cusp at r=0 ~5e-7 for one sample-neighbourhood per cycle; the triangle's
corner lands exactly on a grid point at this power-of-two size. Measured against
the reference goldens: saw-base 1.007e-09, saw-glass 2.485e-09, saw-round-hi
9.707e-10 RMS — three orders of magnitude under the 1e-6 bar, and the fleet
worst is still the unrelated dyn-ring at 4.262e-09.

### After

sawBase 4.30% · roundness 4.23% · ALL FOUR **5.02%** (was 12.38) — the section
now costs +35% instead of +234%.

### Also in this change

- id 69 relabelled **Squareness** and moved into the Saw shape section (human:
  two unrelated things were both named "Saw Shape"). The key stays `shape` —
  state compatibility; only the face moved.
- The Saw shape section has an ENGINE-DRAWN one-cycle waveform (`hzGetShapeWave`
  → the same anchor functions + the ADR-058 morph; the bend graphs' "drawn by
  the engine, never a JS twin" rule). Display honesty: the ideal saw stands in
  for the BLEPped one, and roundness shows at its knob value — roundHi's
  per-note pitch scaling cannot appear in a static cycle.

### Process note

PR #365 was stacked on render-efficiency and merged into it AFTER that branch's
own PR had merged — the PR #328 trap, second occurrence. Recovered by PR #366
(the branch diff against main IS the lost content). Standing rule from here:
**no more stacked PRs**; a dependent change waits for its base to land on main.

## ADR-102 — The bend lane is active when ANYTHING processes it; poly glide is automatic; Bend lives on MAIN (ACCEPTED)

**Date:** 2026-08-21 · **Status:** ACCEPTED (four human reports/requests in one).

### The regression: quantise with the law off did nothing

`bendActive()` asked only about the LAW, so with the law off — the shipped
default — `applyParam(38)` instant-wrote the wheel past `GlideCore::step()`, and
the quantiser lives inside step(). The note lane quantised (it steps its own
GlideCore); the wheel's lane skipped its own. Fix: active = law engaged OR
quantiser engaged; kOff + quant on the grid is the published kOff contract
(pass-through that still quantises). Turning the quantiser off settles the lane,
same rule as leaving a law. Gate in mpe_check: law off, chromatic, wheel to
+1.37 st → must sound +1 st; both bins measured (the step bin is the must-arrive
control); plant reads the raw bin and goes RED.
The human's follow-up suggestion (gate bend-quantise on a global quantise) was
motivated by the bug reading as arbitrary; deferred — it works standalone now.

### Poly glide is not a choice

"On as long as mono isn't toggled and a bend law is set." The arming condition
drops `p.polyGlide` (declared for state compat, labelled (dev), permanently
gated off in the GUI via an impossible shown_when — voiceMono=2). Mono never
reaches the poly path, so the law is the only question left.

**The flag was load-bearing in a way nobody knew.** Removing it turned parity
RED (153/156, worst 8.4e-02 at dyn-gravity): the CORE's default noteLaw lambda
set kLag but kept GlideCore's own tau = 60 ms, so every bare core glided out of
the box — invisible for two days because the flag gated the arming. The shell's
twin lambda fixed this exact default on 2026-08-19; the core's did not. A
default that only a removed flag was hiding is a bug with a delay fuse.

### "Always glide" already existed

The requested toggle is glideMode = 1 (ADR-076, human-ruled 2026-07-31:
"remembers the last played note(s) and always begins with a bend"). It was
undiscoverable behind "last note (memory)"; relabelled "always (from last
note)". No new parameter.

### Bend returns to MAIN

The whole merged section (29 rows), per the human. The cluster is fully
generated, so the move is a page-column edit and a regen.

## ADR-103 — Glide From has three sources, and "always" is its own thing (ACCEPTED)

**Date:** 2026-08-21 · **Status:** ACCEPTED (human: "I want 'always' to be an
option available to mono... I don't want always to replace the former option of
'glide from last note', I want it to be its own thing").

### The split

0 **held note (legato)** — glide only while another key is down.
1 **last note (ringing)** — glide while the previous note still SOUNDS: its
  release tail counts, and true silence resets the phrase. Ringing is measured
  as the render measures audibility (env > 1e-3), so the glide source dies at
  the moment the ear loses the note. **This option never existed before.**
2 **always** — glide from the last played note regardless, silence included.

The selector is un-gated (visible in mono). Mono's fresh strikes flow through
the same core noteOn arming, so all three sources work in mono without a
separate mono path.

### The history correction the split forced

The pre-split option 1 ("last note (memory)") BEHAVED as always — lastNoteF
persisted across silence, human-ruled 2026-07-31. So the new value 2 is the old
value 1's sound, and stored patches migrate: schema<2 + glideMode=1 → 2 (state
schema bumped to 2 for exactly this). A patch keeps its sound; the number moves.

### Gate

trajectory_check, four criteria, each mid-flight (arrival cannot fail — the
arming-bug family's lesson): held/ringing snap after true silence, always
glides from silence, ringing glides while a tail still sounds. Plant (mode 1
forced to old always-behaviour) takes "after true silence, snaps" RED.

## ADR-104 — The quantum morph engine: flips, not crossfades (ACCEPTED, v1)

**Date:** 2026-08-21 · **Status:** ACCEPTED (human: "let's go ahead with the morph
port"). Reference: `docs/design/quantum-morph-lab.html` — bilinear corner weights,
per-parameter Gumbel-max corner assignment, temperature + coupling + (later) bias
in ONE score, mulberry32 streams. The Gumbel-max trick makes one seeded draw per
(param, corner) a committed, repeatable patchwork instead of a dice roll per
visit — argmax(log w/T + gumbel) IS a softmax sample.

### Division of labour

`morph_core.h` owns the math (weights, draws, the single scoring law — what you
hear and what a map paints must be one function or the map lies). The SHELL owns
which parameters morph, what a corner snapshot is, capture, persistence — the
glide_core/scale-table division.

### v1 scope, stated

- Morphable set: every PER-OSC parameter (the twin-having set), both
  oscillators, enables included (the human's corner design needs them). Globals
  stay patch-level; the set widens with the corner-editing phase, not by v1
  guessing.
- Corner snapshots are STATE, not parameters (4 x ~100 automation lanes would be
  noise); they ride the state chunk in morphIds order (id-ascending, stable).
- morphStep runs on the 256-sample gravity grid with the ADR-086 accumulator —
  buffer-subdivision independent, and a parameter field does not need 2.7 kHz.
- Stepped params take the winning corner outright; continuous params flip with a
  one-pole slew (`morphGlide`, seconds — ADR-009) or crossfade in blend mode.
- morphOn ships OFF: parity-safe superset, all 156 goldens untouched.
- A fresh instance's corners all hold the DEFAULT patch, so morph-on before any
  capture is silence-safe: every corner agrees.
- While morphOn, live edits to morphed params are overwritten at the next grid —
  the corner-editing model (recorded 2026-08-20) is the designed answer and the
  explicit NEXT PHASE, along with per-param bias/pin from the lab.

### RT discipline

morphInit allocates → it runs at activate (main thread). The audio-thread paths
(morphOn flip, seed reshuffle) are fill/array-writes only. Caught by reading the
code, not by rtsafety_probe — the probe never flips morph mid-run; extending it
to sweep stepped params is noted for the fine-tune pass.

### Amendment 1 (2026-08-21) — the FX rack joins the field

Slot type, amount, tone, mix (57-64, 96-99, 133-136) are appended to the
morphable set: "off in this corner, driven in that one" becomes a rack story,
and module-level EXEMPT (roadmapped the same day) has something to exempt.
**morphIds is append-only from here, like param ids** — a v1 corner chunk fills
the per-osc prefix in its original order and the FX tail takes defaults, so an
old patch loads with corners intact and rack unmorphed: exactly what it said
when saved.

### Gates

trajectory_check: same seed + position -> identical assignment on two cores
(128/128); a different seed changes the patchwork (83/128 moved); cold +
cornered -> the dominant corner owns everything (128/128). Shell inertness rides
state_check/parity (morphOn off).

## ADR-105 — Presets: the quick win is storage that already exists (ACCEPTED)

**Date:** 2026-08-21 · **Status:** ACCEPTED (human: "a save/load preset section
for a quick win... maybe also for individual corner presets").

Named presets over the EXISTING state JSON, stored in the webview's
localStorage (per-app-container in WKWebView — survives reloads and sessions).
COPY PATCH / PASTE stays the export/import path; the disk browser over
docs/presets is the roadmapped step this deliberately does not attempt.
Corner presets: the same pattern over one corner's snapshot
(hzMorphCornerJson/Apply, morphIds order — the append-only contract), so a
corner sound is a nameable, reusable thing independent of the patch it was
born in. SAVE…/load… live on each corner row next to CAPTURE.

Also in this change (ADR-100 A2): oscillator power toggles live ON the osc tab
bar (raw id 150+stride, never effId — each button addresses its own oscillator
regardless of which is being edited), and every per-swarm visual (phase circle,
carpet, voice map) shows an explicit OFF state when the viewed oscillator is
switched off — the "phase dots with the visuals off" were real voices fanned
into the disabled core for the enable-ON re-strike, frozen because render is
skipped. The visual follows the SWITCH, per the human's ruling.

The saw-shape globality report was investigated and NOT reproduced: the engine
routes 129/1129 independently (probe: osc1=0.25, osc2=0.75 simultaneously) and
the GUI sends 129 vs 1129 correctly per the selector (instrumented bridge).
Likeliest experiential cause: morph ON with corners captured when both
oscillators shared shape values — the field forces them back in lockstep.

## ADR-106 — The quantiser is anchored: base + offset, the reference's own words (ACCEPTED)

**Date:** 2026-08-21 · **Status:** ACCEPTED (human report: "notes are resolving
to a slightly different pitch after a bend than when they're initially played").

### The dropped argument

bend-lab's quantiser has carried a `base` parameter since it existed —
`quantise(p, base = 0)`, first line `semis = base + this.x` with the comment
"ABSOLUTE pitch, not the offset". The port dropped it. Consequences:
- The BEND lane quantised its OFFSET as if the offset were a pitch: any note
  whose class is off the offset-space grid resolved to a shifted pitch after a
  bend. The report, verbatim.
- The NOTE lane quantised in log2(f)*12 space, whose classes sit 0.376 st off
  MIDI's — every scale-quantised note landed a constant 37.6 cents off the grid
  (latent: noteQuant ships off).
- Every golden ran base = 0, where offset and pitch space coincide — L0031's
  blind spot, hit for the third time (the wheel, qTime, now base).

### The fix

`step(target, p, base = 0)` / `quantise(p, base)`: candidates, the qStep latch
and hysteresis all live in ABSOLUTE pitch (as the reference's do); only the
return converts back to lane units. Anchors: per-note MPE lanes pass their own
key; the GLOBAL wheel lane passes the last-struck key (one lane, many notes —
the global compromise; the per-note lanes are exact); the note lane passes
69 − 12·log2(440) to align its log-frequency space with MIDI classes.

### Proof

Two new goldens sliced live from the lab, anchored at F#3 (54) and 54.5 —
classes chosen to be maximally off the default major grid, so a port ignoring
base CANNOT pass. First run: RED at rms 0.5 exactly (the port emitting integer
offsets against the lab's half-offsets) — the scenarios' own must-fail control.
After threading: **rms = 0 on both** — bit-exact. All 21 pre-existing goldens
unmoved (base = 0 is byte-identical). A process note: the runner edit was first
applied with an assert-free regex replace that silently missed `, s.p);` — the
un-asserted-edit trap, again, caught by the golden staying red.

### Also in this change

The GUI1 gravity pitch-correction readout (ratio name + octave fold + live
cents) is ported to gui2 — the snapshot and marshal existed end-to-end; only
the display line was missing. Both swarm panels carry it, class-addressed.

## ADR-105 Amendment 1 — The JSON state path carries the twins (2026-08-21)

"Saving a patch doesn't seem to do anything, or at least loading doesn't."
Measured: the JSON path (the preset system's path, and the dev panel's before
it) saved and scanned BASE ids only — oscillator 2's params never round-tripped
(detune2 stayed at 0.900 against a saved 0.222 while detune1 restored exactly).
For the two-oscillator morph patches being built, a load restored half a patch.

Fix: `o<k>.`-prefixed twins in both directions — the SAME convention
`state_save`/`state_load` (ADR-082) have always used; the JSON path simply
never got it. Gate: state_check gained a JSON-path case driven through new
headless debug exports (`hypersaw_debug_state/apply` — the same two calls the
GUI preset buttons make, previously reachable only through webview lambdas,
which is WHY this path had no oracle and the bug shipped unobserved). Plant
(twin write removed): base case OK, twin case RED — fires on exactly the
missing half.

Process note, third occurrence: a bare `git checkout -- <file>` to clear a
plant destroyed the turn's uncommitted work again. New personal rule enforced
this turn: plants are removed by REVERSE REPLACE, never by checkout, and a
checkpoint commit lands before any plant cycle.

## ADR-105 A2 + ADR-100 A3 — Presets on disk; one gate on osc 2's silence (ACCEPTED)

**Date:** 2026-08-21 · **Status:** ACCEPTED (human: "the save/load functions
still aren't working in either interface. Also the power buttons on the
oscillators don't work").

### Presets: localStorage was quicksand

The plugin webview loads via setHTML — an OPAQUE ORIGIN, where localStorage
THROWS SecurityError — and the store's own try/catch swallowed it: presets
no-opped in the plugin while passing every lab test on localhost. The store is
now DISK: `~/Library/Application Support/LiftedTruck/HYPERSAW/{presets,corners}`
via hzPresetList/Save/Load/Delete (GUI-thread binds — filesystem there, never
in process()), names sanitised to filename-safe characters. The GUI prefers the
host store and falls back to localStorage only in the lab. This is also the
roadmapped "disk browser" landing earlier than planned, because the shortcut
proved not to exist.

### The power buttons: two safeties on one door

Osc 2 shipped enable=0 (ADR-099 A1) AND vol=0 (the ADR-082 era
SilenceHigherOscillators + the id-17 twin default). Clicking power ON worked —
and produced nothing, because the volume was still down. A switch that "works"
inaudibly is indistinguishable from a broken switch. The vol zeroing is retired
at BOTH sites (defaultFor and the constructor silencer); the enable switch is
the ONE silent-by-default gate, and switching ON is audible immediately.
enable_probe now proves the switch ALONE suffices (no explicit vol in setup);
mixer_check and paramscope_check setups flip the switch explicitly.

### Also

The morph-vs-switch interplay got its rule early (the exempt-lean design):
while morph is ON, a manual enable toggle writes itself into all four corners,
so the switch cannot be reverted by the field. Probe-proven; the full per-edit
routing remains the corner-editing phase.

### Process

The recurring python-heredoc corruption (unclosed-paren SyntaxErrors on
multi-line replaces) is now avoided by writing edit scripts through the Write
tool; heredocs carry only trivial one-liners.
