# 2026-07-17 — Phase 0 gate closed; Phase 1 golden generator

**Ratifications recorded (human, in-session).** ADR-018 (bank renderer)
ACCEPTED as proposed. ADR-019 (choc webview) ACCEPTED with the swappability
amendment: engine↔GUI via a transport-agnostic message/param layer, all
webview/platform-view specifics in one module, visualization feed as plain
structs so a native fallback consumes the same data. E-6 envelope ratified
and written into ACCEPTANCE.md. Live evidence: VST3 loads and plays sine on
MIDI input (no GUI, as designed at this phase). Recorded residual:
Reaper/Bitwig load checks deferred (hosts not installed here; CI pluginval
both platforms is the proxy; real check no later than Phase 2 gate).

**Phase 1 start — golden generator.** `tools/golden/extract_core.mjs` loads
SwarmSynth by evaluating the DSP section straight out of swarmsaw.html
(banner-marker slicing, loud failure if the structure changes) — no code
fork, the HTML stays the single source of truth per ADR-003/L0001.
`tools/golden/gen_goldens.mjs` renders the L0-1 matrix — 10 scenarios
(defaults, sync-lock, sync-shimmer, splay-jp7, inertia-hunt, rtone-full,
scatter-drift, onset-dissolve, gauss-cloud, cauchy-cloud) × seeds
{1234, 777, 42} — 4 s stereo renders with a 3 s note-off, Float32LE +
manifest (sha256/peak/rms) into build-golden/ (gitignored). `--selfcheck`
mode renders everything twice and byte-compares; wired into `./verify full`.

**Finding worth recording.** First matrix run showed identical hashes across
seeds for 7/8 scenarios — correct per SPEC (seed only governs seeded draws,
un-retriggered scatter, drift), but it exposed that the matrix wasn't
exercising the seed axis; added gauss-cloud/cauchy-cloud (seeded
distributions), which now hash distinctly per seed.

**Verify.** `./verify fast` green; `full` green including 30/30 scenarios
deterministic (output in session).

**Open.** SwarmCore C++ port + the compare harness (the other half of L0-1);
commits are LOCAL — pushes held since the user declined one this session.
