# 2026-07-21 — Inertia taper tune-then-lock dev slider (ADR-059)

Human (correcting the Phase-2 caveat): the inertia knob is steep just after 0,
worst at low detune + retrigger-on. It's a taper problem (ADR-024 sqrt compressed
the audible onset to the knob bottom), not a bifurcation — so a curve reshape
fixes it, but the right curve is an ear call. Hence a tune-then-lock dev control.

## Change (shell + GUI only; core untouched)
- `src/hypersaw_clap.cpp`: inertia taper `w = knob^curve` (id 70 "Inertia Curve
  (dev)", 0.3..5, default 0.5). curve==0.5 uses `sqrt` exactly (bit-identical to
  ADR-024). id 70 re-derives inertia from the stored knob on change.
- `src/gui/gui.html`: "· curve (dev)" slider under inertia; DEFAULTS 70:0.5; SAW_ONLY.

## Evidence
- Taper is shell-side → parity unaffected. `./verify full` EXIT 0, parity 54/54,
  state_check GREEN (id 70 round-trips), all 9 chains green. pluginval 10 SUCCESS.
- git: 04a19ed on branch feat-inertia-curve

## The lock step (next)
Human dials `Inertia Curve (dev)` by ear until the response off 0 is even,
reports the value → hardcode `w = knob^<value>`, delete id 70 + the slider.
Same mechanism available for the K cloud→order edge (~0.6-0.8) on request.
