# Kuramoto LFO prototypes — CONCEPT TESTS, NOT GOLDEN REFERENCES

**Status: non-golden.** These four HTMLs are exploratory concept tests of the
Kuramoto-LFO idea (design brief: `docs/proposals/2026-07-20-kuramoto-lfo.md`,
ADR-053). The human stated explicitly on drop (2026-07-20): *"these prototypes
are not golden, merely tests of the concepts."*

**Do NOT treat these as the reference implementation.** They are NOT in the
protected-prototype set (that set is the seven listed in `CLAUDE.md §Domain`:
swarmsaw / swarmspectra / swarmdynamics / swarmfilter / swarmphaser / swarmtime
/ swarmalator). They carry no verified acceptance numbers here, and their
in-file ADR references (048–052) are the authoring kit's numbering, which
collides with this repo's — ignore those numbers (see ADR-053).

**The ingest gate still applies (ADR-003 / ADR-052 prototype-first):** before
any of this is ported to C++, the chosen concept (the rotor first) must be
hardened into a *golden* reference — a headless-extractable DSP core with
measured acceptance anchors and the multi-LFO-cycle mod-test rule (recommendation
§5c) — then ingested like every other engine. Until then this folder is design
reference only.

Files:
- `swarmlfo-rotor.html` — the shipping face: 4 phase-coupled Kuramoto LFOs
  (phase-domain, SwarmCore-like) → morph / cutoff / chorus / saturation.
- `swarmlfo.html` — axis 1, rate (position-domain springs on log2 rate).
- `swarmlfo-depth.html` — axis 2, depth (R→depth, thin/attrition).
- `swarmlfo-dest.html` — axis 3, destination (per-voice routing target coupled by J).
