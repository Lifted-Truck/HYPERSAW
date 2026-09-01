# 2026-09-01 — morph corner integrity + QM-4 ingestion (ADR-152)

**What changed.** The human reported corners flattened by the global
XY/macros (K/detune identity overwritten by macro offsets) and delivered the
QM-4 intent-bus spec + prototype as the real fix. Interim landed now:
macro-family sources (slots 2-13) suspend to 0 while morph is ON (dests
release to base; base follows the morph field via the ADR-136 intercept —
verified no-fight at hypersaw_clap.cpp:3150); morphCapture flattens the
macro contribution into stored corners (QM-4 §7 brought forward). GUI: route
rows grey under morph with a title, pad route notes append "(suspended:
morph on)". Logo free-roam hue drift desaturated 18% toward luma. QM-4 spec
+ prototype ingested (protected paths, ADR-152), B89 filed with review
verdict and phasing; one sanctioned prototype edit outstanding (seed
reshuffle()).

**Evidence.** verify full exit 0 (suspension inert with morph off — the
shipped default — so parity untouched by construction); lab_load 26/0.
