# 2026-07-19 — Track E2: time engines (tap delay + FDN room), testable in SWARM-FX

Human pivot after the frequency effects underwhelmed. Ported src/time_core.h
(TimeLab verbatim on force_core): echo = tap-swarm delay (/N feedback norm),
room = FDN with negated Householder. Oracle time_check: parity vs JS TimeLab
both modes (worst RMS 5.6e-12 — feedback tanh transcendental-ulp, within
1e-6 eps), + L0-19 echo stability (0.073/0.075 @ regen 0.5/0.97), L0-20 room
resonance, L0-21 room 12s bounded+DC. verify full = 9 chains.

WIRED into SWARM-FX: engines 2 (Tap Delay) / 3 (FDN Room), unified surface
remapped (Center→size, Resonance→regen, Feedback→damp), note→gravity center.
pluginval 10 + auval SUCCEEDED, installed. Headless: both produce feedback
tails on external audio (echo rms 0.027, room 0.032).

Residual: mono core (stereo = obvious refinement); host-tempo sync (bpm 120);
block-start params. Awaiting the human's listen on which effects survive.

Branch created BEFORE editing (L0004).
