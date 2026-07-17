# LIBRARY — durable, evidence-backed lessons (HYPERSAW)

Entry format:
`[Lxxxx] <title> | tier | added: YYYY-MM-DD | tags: … | lesson: … | evidence: … | falsifier: … | supersedes: …`

Tags: swarm-dynamics · parity-oracle · plugin-platform · realtime-perf

---

[L0001] Prototype HTMLs are the spec — amendments arrive as new files, not edits | candidate | added: 2026-07-17 | tags: parity-oracle | lesson: The reference implementations arrive/update as externally-authored drops (files.zip, swarmsaw_v2.html + CHANGE-NOTE-*.md), so "current reference" is a pointer that moves: swarmsaw_v2.html superseded swarmsaw.html mid-spin-up, and the change note amended SPEC §5.6 and ACCEPTANCE L0-3. Before any parity work, confirm WHICH file is the pinned reference (DECISIONS ADR-010) and fold pending CHANGE-NOTE amendments into SPEC/ACCEPTANCE first — goldens generated from a stale reference are wrong with full confidence. | evidence: this spin-up session — v2 diff confirmed as v1 + change-note additions (R_N meter, seat rings, formation polygon); SPEC/ACCEPTANCE amended accordingly. | falsifier: reference updates start landing as in-repo edits under version control with the ADR trail, making the drop-pointer discipline unnecessary. | supersedes: —
