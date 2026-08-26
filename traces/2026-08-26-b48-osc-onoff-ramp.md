# Trace — B48: osc on/off morphs as a level ramp (ADR-123)

**Trigger** human 2026-08-26: *"on blend, it should jump to on and gradually
bring the volume of the osc up to max instead of picking an on/off value from
one patch and a volume value from another."*

**What changed.** `src/hypersaw_clap.cpp` only: `oscOnW[]` (morph-derived
per-osc on-weight) as a new factor in `oscGainTarget()`, riding the existing
~8 ms smoother and its skip-if-1.0 guard; a `morphStep` special case for
150-ids (bilinear on-weight from plain `w[]`, stepped flip deferred to the
1e-3 floor); release-to-1.0 on morph-off and on exempt; and the
**`!morphFromField` guard on ADR-100 A3's write-into-all-corners**.
DECISIONS ADR-123; tests B48-1 (parity), B48-2 (human, click-free sweep);
ROADMAP B48 filed this session.

**The latent bug.** A3's corner write ran for the FIELD's own flips: the first
time a morph crossed an on/off boundary it overwrote all four corners' enable
— the stored boundary silently ceased to exist. Proven by A/B control: guard
removed → back at corner A the enable reads 1 (destroyed); guard present → 0.

**Evidence.**
- Osc2-solo sweep across the boundary (scratch probe, real plugin):
  −240 dB at the off corner, −41 dB at first blend step, monotone to −14 dB
  at the on corner. Enable jumps to ON at first blend, exactly as asked.
- Parity 156/156 within ε=1e-6 (worst 4.262e-09) after the change — goldens
  never engage the morph and the settled gain path multiplies by exactly 1.0.
- `./verify fast` exit 0.

**Install.** Both formats copied, hash-verified equal to the build BEFORE
signing (post-sign hashes legitimately differ — the signature rewrites the
binary; earlier attempt compared post-sign and misread), then signed, seals
verified, AU cache reset. Note for future L0042 use: C++ identifiers are NOT
valid binary markers (stripped by compilation) — only embedded-data strings
are; hash-compare pre-sign instead.
