/*
 * strata_core.h — the STRATA hierarchical value-shaping layer (B77 increment 1).
 *
 * Ported from the human's own bench (`strata-modulation-bench.html`) and spec
 * (`STRATA-integration-spec.md`, v0.1). Framework-free, stateless, allocation-
 * free; the shell owns every value it reads.
 *
 * WHAT IT IS, AND WHAT IT IS NOT. STRATA is NOT a modulator and must not be
 * filed as one: it computes the EFFECTIVE STATIC VALUE of a parameter from a
 * stack of tiered hand controls, each retaining agency over the result. It
 * runs at control rate and sits UPSTREAM of the mod matrix (spec §3): the
 * matrix's offsets ride on top of wherever the hand has put the value. That
 * placement is what makes it compose with ADR-136 rather than fight it —
 * STRATA produces the BASE, the matrix adds its offset to that base, and
 * readback still reports the stored T0 value, so host automation never sees
 * either layer's output and cannot form a loop with itself.
 *
 * THE COMBINATOR, and why it is not addition. `lift` interpolates toward the
 * ceiling for u >= 0 and toward the floor for u < 0, so a tier's reach is
 * always the WHOLE remaining range and the result cannot leave [0,1] without
 * a clamp. Addition clamps, and past the clamp point every lower tier's agency
 * is exactly zero — the knob under your finger stops doing anything. Under
 * `lift`, agency compresses linearly (d v'/d v = 1-|u|) and reaches zero only
 * at the rail itself. That property — the human's "full range of autonomy of
 * each" — is the reason this layer exists, and P4 in the oracle is its guard.
 *
 * ORDER IS THE HIERARCHY. Same-sign contributions commute; mixed-sign ones do
 * not, and later tiers dominate. Folding group-then-master therefore means the
 * master gesture wins at the extremes. That is a DECISION, not an accident
 * (spec §2.2 P6), and reversing it is a different instrument.
 */
#pragma once

#include <algorithm>

namespace hypersaw
{

struct StrataCore
{
  enum Mode
  {
    kCascade = 0,   // lift — bounded by construction, agency never truncates
    kAdditive = 1,  // clamped sum — offered because the human asked for it as
                    // an option beside cascade, not because it is equivalent
  };

  /* THE COMBINATOR. u is pre-scaled by its tier's depth before it arrives.
     u == 0 is an EXACT passthrough — not approximately, exactly: `v + 0*(1-v)`
     is v in IEEE arithmetic for every finite v, which is what lets a patch
     with centred pads render bit-identically to one with no STRATA at all
     (spec P1, and this repo's parity-safe-superset rule stated in someone
     else's words). */
  static double lift(double v, double u)
  {
    return u >= 0.0 ? v + u * (1.0 - v) : v + u * v;
  }

  static double additive(double v, double u)
  {
    const double s = v + u;
    return s < 0.0 ? 0.0 : (s > 1.0 ? 1.0 : s);
  }

  static double combine(double v, double u, int mode)
  {
    return mode == kAdditive ? additive(v, u) : lift(v, u);
  }

  /* One target, one block. O(tiers) multiply-adds, no branching beyond the
     sign test, no state, no allocation — safe to call on the audio thread,
     though the shell calls it at the control tick. Tier order is fixed and
     documented: group, then master (see the header note on P6). */
  static double evalTarget(double base, double uGroup, double uMaster,
                           double dGroup, double dMaster, int mode)
  {
    const double a = combine(base, uGroup * dGroup, mode);
    return combine(a, uMaster * dMaster, mode);
  }
};

}  // namespace hypersaw
