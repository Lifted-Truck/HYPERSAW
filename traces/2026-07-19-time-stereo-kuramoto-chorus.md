# 2026-07-19 — Time engines → stereo; Kuramoto chorus roadmapped; Foxfire correction

**Stereo (ADR-050).** time_core.h processExternalStereo(): mono-summed into
the delay/room network, each tap/line panned by index × new `stereo` param;
dry keeps input stereo, wet is the decorrelated field. Deliberate divergence
from the mono reference (experimental license) — mono path (render/
processExternal) stays byte-frozen so time_check still guards it (5.6e-12
unchanged). SWARM-FX param 17 "Stereo Width" → time.stereo; time engines
route to the stereo path. Measured L/R corr: 1.0 @ width 0 → 0.69 echo /
0.79 room @ 0.8. pluginval 10 SUCCESS, installed.

**Kuramoto chorus (roadmap).** Human found Chiral Audio's Foxfire (16-voice
Kuramoto chorus: coupled LFOs modulating delay taps, K-as-gesture). Added a
Kuramoto-chorus engine idea to ROADMAP — coupled modulation LFOs on short
delays, DISTINCT from the force-herded tap-swarm delay; prototype-first,
citing Foxfire.

**Foxfire prior-art correction.** Foxfire is a SHIPPING Kuramoto product —
triggers the falsifier the "no shipping product" claim carried. PRIOR-ART
§1/§6 corrected: retract the flat claim, sharpen to "no product uses
coupled-oscillator dynamics as the timbre/synthesis engine" (Foxfire is a
modulation effect, not a phase-transition synth). Honest per doctrine.

**Effects-viz direction** noted in ROADMAP: the survivors need a webview GUI +
live visualizer and license to iterate the DSP (not parity-frozen).

Branch created BEFORE editing. verify full green (9 chains; time parity
unchanged). Evidence: scratchpad/st.cpp decorrelation PASS.
