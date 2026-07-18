# 2026-07-18 — "Weak K" report resolved: absolute K was on; toggle made loud

**Report + diagnosis.** "Pull K suddenly much weaker, especially mean-field."
Engine exonerated by oracle first (parity 51/51, L0-2 exact on current
main), then suspects characterized by measurement: absK ON → R 0.44 vs
0.92 baseline; α=80° → 0.38; inertia knob 0.5 → 0.53 (ADR-024 taper).
Human confirmed: absolute K was enabled unnoticed.

**Fix (ADR-016/017 legibility principle).** The absolute-K toggle
redefines the pull-K knob's units while looking like any checkbox — its
ACTIVE state now renders loud: amber, bold, with an inline "— K in
absolute Hz" suffix, driven from both local edits and host state sync.

**Also noted for the future.** The diagnostic order (oracle-first
exoneration → measured suspect table → one-look state dump) resolved this
in minutes; the COPY STATE dev button and the parity oracle are earning
their keep as diagnostic instruments, exactly as designed.
