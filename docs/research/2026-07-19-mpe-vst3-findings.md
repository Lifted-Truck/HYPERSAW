# MPE → VST3 not working in Live — consolidated research (2026-07-19)

Three parallel Sonnet research agents, dispatched after ADR-036/038's fixes
still failed in Ableton Live. Fix deliberately TABLED; this is the root-cause
map so the eventual fix is one shot. Nothing here changes code.

## Convergent conclusion

The plugin/wrapper side is **not** the bug. The untested variable is Live's
own per-device MPE gate. Check these two things — in order — before any code:

1. **Enable Live's per-device MPE toggle (no build needed).** Live gates all
   MPE-to-plugin behind an explicit per-device setting: right-click the device
   title bar → **"MPE Settings"** (Live 12) / "Enable MPE mode" (Live 11),
   configure zone + channel count. Without it, Live sends ordinary non-MPE MIDI
   and NO mechanism in the plugin can help. This is the single most likely
   cause of "both ADR-038 mechanisms failed" and costs nothing to rule out.
   Refs: ableton.com/en/live-manual/12/editing-mpe/ ;
   help.ableton.com/hc/en-us/articles/360019144999
2. **Objectively confirm the advertisement with vst3_validator.** The VST3 SDK
   validator (CMake target `vst3_validator`, vendored under
   build-release/cpm/vst3sdk/.../validator/) prints each advertised
   NoteExpressionTypeID by name — a direct readout of whether TUNING (typeId 2)
   is actually exposed. BLOCKER: its build needs full Xcode (fails CLT-only at
   SMTG_DetectPlatform "XCode 9 or newer required"). Run on a full-Xcode
   machine, or use Steinberg's prebuilt VST3PluginTestHost (needs a download OK).

## What each agent proved

- **Advertisement path (agent A):** correct line-for-line in clap-wrapper
  v0.15.1. Extension ABI matches (clapwrapper/vst3.h:157-163 vs
  hypersaw_clap.cpp:1312-1317); setupWrapperSpecifics reads it before bus/param
  setup (clap_proxy.cpp:307-324); INoteExpressionController is exposed
  unconditionally; kTuningTypeID conversion (v-0.5)*240 is ungated. Both routes
  (native VST3 note-expression AND per-channel MIDI via IMidiMapping) are wired.
  Verdict: the wrapper does NOT silently no-op the advertisement.
- **Inbound + voice matching (agent B):** SAW-mode shell plumbing is clean —
  14-bit bend decode, ±48 scaling, per-channel latch, note-on re-apply, and
  channel matching all correct, no off-by-one. BUT **SPECTRA mode is a silent
  dead-end**: SpectraCore has no noteTune/setNoteExpr, and the mode-blind
  handlers write to the idle SwarmCore whose render() isn't even called in
  SPECTRA. Also flagged: Voice-Mono collapses rotating MPE channels onto one
  slot (bend for an earlier-held note can't match).
- **Live behavior + observability (agent C):** the per-device toggle above;
  VST3 note-expression is macOS-only in Live (Windows falls back to per-channel
  MIDI); ±48 default matches ours; no open clap-wrapper issue for this case;
  v0.15.1 is the latest tag (nothing fixed upstream we're missing).

## Implications for the eventual fix (when untabled)

- If the Live toggle was off: no code fix needed — re-test.
- If on and still dead in SAW: instrument actual host traffic; likely a
  Live-specific delivery quirk, not our code.
- Independently worth doing regardless: give SpectraCore a noteTune seam so
  SPECTRA isn't a silent bend sink (small, and it's the one confirmed code gap).
