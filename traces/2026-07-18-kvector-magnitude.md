# 2026-07-18 — K-vector magnitude decoupled from direction smoothing

Follow-up on the Cartesian smoothing: averaging POSITIONS of a rotating
vector systematically shortens it toward the center, so the arrow read
as lower R than reality. Display now takes direction from the
Cartesian-smoothed vector (fluid, wrap-safe) and LENGTH from a
separately smoothed scalar R (0.35/frame — faster, magnitude-faithful).
Near-zero guard on normalization. Viz-only; readouts raw as before.
