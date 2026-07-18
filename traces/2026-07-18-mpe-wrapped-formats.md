# 2026-07-18 — MPE bend reaches the wrapped formats (ADR-038)

**Field report.** Post-ADR-036 merge: "the MPE bend isn't working yet" in
Live (VST3 via clap-wrapper). ADR-036 assumed the wrapper delivers Live's
MPE as CLAP note expressions "with no further surface" — wrong.

**Root cause (two legs, both in the pinned clap-wrapper v0.15.1).**
(a) The VST3 wrapper advertises only PRESSURE through
INoteExpressionController: `_expressionmap` defaults PRESSURE-only unless
the `CLAP_SUPPORTS_ALL_NOTE_EXPRESSIONS` compile flag is set
(wrapasvst3.h:474), and `make_clapfirst_plugins` never forwards that flag
(make_clapfirst.cmake:171) — unreachable from our build. So Live never
sent tuning note expressions at all. The conversion side was always
correct ((v−0.5)·240 semis, note-id matched, vst3/process.cpp:660);
only the advertisement was missing.
(b) MPE also arrives as rotating member channels + per-channel 0xE0: the
VST3 wrapper emits these as `CLAP_EVENT_MIDI` from its per-channel
IMidiMapping bend params (vst3/process.cpp:321), the AUv2 wrapper forwards
raw 0xE0 the same way (auv2/process.cpp:551) — and our handler ignored
CLAP_EVENT_MIDI entirely.

**Fix (src/hypersaw_clap.cpp only; core untouched).**
(a) Answer the wrapper's `CLAP_PLUGIN_AS_VST3` extension:
`supportedNoteExpressions = TUNING|PRESSURE` (PRESSURE = wrapper default,
preserved), `getNumMIDIChannels = 16`. Picked up in the wrapper's
`setupWrapperSpecifics` before advertisement + process-adapter setup.
(b) Handle `CLAP_EVENT_MIDI` 0xE0: member channels 2–16 (indices 1–15) at
the MPE default ±48 st, latched per channel (`mpeBendSemis[16]`), applied
via the ADR-036 `setNoteExpr` seam to swarms with matching VoiceTag
channel; latch re-applied on note-on (MPE hosts send bend before the
note; fresh strikes reset noteTune). Channel index 0 (manager / plain
MIDI) deliberately excluded — non-MPE hosts see zero behavior change.
`plug_reset` clears the latches.

**Evidence consulted.** clap-wrapper sources as cited above; ADR-036/027
seams in src/swarm_core.h:234,609; existing NOTE_EXPRESSION handler
src/hypersaw_clap.cpp (kept unchanged for CLAP-native hosts).

**Verify.** ./verify full GREEN post-change (recorded exit 0): parity
51/51 within eps=1e-6 (worst 4.262e-09 @ dyn-ring.seed42) — defaults
bit-inert; state_check GREEN; trajectory/force/spectra suites all OK.
Human gate open: verification in Live with an MPE controller (VST3), and
optionally Logic (AU) for the 0xE0 path.

**Git.** Branch mpe-wrapped-formats; PR filed from this trace's commit.
