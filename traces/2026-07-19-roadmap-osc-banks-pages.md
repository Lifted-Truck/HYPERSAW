# 2026-07-19 — Roadmap: parallel oscillator banks (OSC2/3) + multi-page device

Human standing by prototyping the Kuramoto LFO (updated its mod-matrix entry:
under active human prototyping; port on drop, don't build speculatively).

Roadmapped two coupled forward directions (human on separate threads):
- OSC2/OSC3 parallel Kuramoto oscillator banks — layered independent swarms;
  design questions captured (per-osc surface, independent-vs-cross-coupled
  [cross = swarm-of-swarms PARKED #5], mixing, CPU/E-6 re-check). Ingest-and-
  port on drop; cross-coupling wants its own reference clone.
- Multi-page device GUI with a high-level control page — home for osc-bank
  control, the mod matrix, and eventual FX routing; page structure sketched.
PARKED #5 cross-linked to the cross-coupled osc-bank vehicle.

Roadmap-only; no code. verify fast green.
