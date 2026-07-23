# 2026-07-22 — Roadmap: Quantum Morph triage + movement/arp + history-modulation

Human dropped three long-horizon ideas and two Quantum Morph spec sheets at
root; asked to triage QM and fold all three into the roadmap. Also launched a
read-only efficiency + file-structure audit sub-agent (recommendations only).

## Change (ROADMAP.md only — planning; no code)
- **Movement / arp layer** → Phase 5 bullet: random-walk scales/chords across
  parallel voices, two variants (scale-quantized · relative-to-played-note),
  feeds detune/harmonic law, seeded-deterministic, Tonality integration.
- **Quantum Morph** → new forward section. Triaged from `QM-0-core-engine-spec.md`
  (normative core: Gumbel-max corner selection, salience, module coupling,
  quantum flip of discrete params, pure-function determinism) and
  `QM-2-instrument-integration-spec.md` (integration contract: per-param manifest
  + morph_class SAFE/VOICE_BOUND/STRUCTURAL/FORBIDDEN, authored salience, poly
  superposition). Recorded: strong HYPERSAW fit (named best pilot, §8 poly-super
  maps onto the coupled-oscillator ensemble), invariant ALIGNMENT (determinism +
  "byte-identical with morph off" = our superset-inert discipline), mod-matrix
  interaction (macro layer vs continuous-mod layer; field position is a mod dest),
  QM-2 §10 open decisions, prototype-first sequencing (QM-0 supersedes the demo's
  blend coupling → harden a golden first).
- **Performance-history modulation** → forward bullet: history→bounded
  deterministic state→mod bus; Tonality key/scale inference; design-first.

## Alias hygiene (verified)
- PRIVATE-NOTES: **Tonality is public — named directly** (4× in ROADMAP, OK).
- Place = "terrain sibling" (aliased); TERRANE/ORRERY not named. grep of ROADMAP
  for {Place|PLACE|TERRANE|ORRERY|Auricle} → none. `./verify fast` leak gate EXIT 0.

## Flags to the human (not acted on)
- The QM specs + demo sit UNTRACKED at repo root and name a private sibling
  (Place). Leak risk if ever `git add`-ed. Recommend: relocate specs →
  `docs/proposals/`, demo → `docs/design/`, and alias private names OR gitignore
  before tracking. Left in place pending the human's call (they "dropped" them).
- Efficiency/file-structure audit sub-agent running (read-only); findings to be
  relayed on completion.

## Evidence
- `./verify fast` EXIT 0 (leak gate + structure). ROADMAP-only; no code, no oracle
  impact. git: on branch lab-detune-octave-spread (PR #70).
