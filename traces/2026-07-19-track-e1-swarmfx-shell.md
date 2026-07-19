# 2026-07-19 — Track E1.3: SWARM-FX effect plugin (external audio)

Human approved the effect-shell shape (one plugin, engine-select, hosts all
swarm FX). Built SWARM-FX as a SEPARATE target (CLAP/VST3/AUv2, aufx) with
its own factory/entry — instrument untouched — sharing filter_core.h/
notch_core.h via a new processExternal() (dry = plugin input instead of the
exciter; render() stays parity-frozen). 16-param unified surface mapped onto
both cores; MIDI note moves the gravity center.

Validated: pluginval 10 SUCCESS, auval SUCCEEDED (aufx), installed + sealed.
All 7 oracle chains still green (processExternal is additive). Headless
sanity: filter engine shapes noise (rms 0.17→0.017 resonant), notch engine
notches with feedback (0.17→0.18, diff 0.045) — both finite/non-silent/shaped.

E1 remaining: L0-17 audio + L0-18 stability oracle rows; SWARM-FX webview GUI.
Then E2 time engines drop into the same shell.

Evidence: verify full green (7 chains); pluginval 10; auval; scratchpad/fx_test PASS.
