/*
 * mod_core.h — the modulation matrix, increment 1: routes and the combination
 * law, nothing else. Framework-free, preallocation-only, deterministic — the
 * same charter rules as every core (no wall-clock, no allocation after
 * construction, seeded streams only if randomness ever arrives).
 *
 * WHAT LIVES HERE: the route table (source → destination, depth, scope) and
 * `evaluate()`, which turns a vector of source values into per-destination
 * deltas by the combination law. The law is SUM — each route contributes
 * depth · source, contributions to one destination add. Chosen to match the
 * standard practitioner shape and FOUNDATIONS §3.1's framing (their register
 * asks *when* sources update, not *how* they combine; the sum answers the
 * settled half). Bounding is deliberately NOT here: per their OQ-30 ruling the
 * destination's own modMin/modMax bound the applied value, and application is
 * the SHELL's act — a core that clamped would hide the shell's failure to.
 *
 * WHAT DOES NOT LIVE HERE: sources (the shell feeds values in — ENV 1 is the
 * amp envelope the shell already owns, an LFO arrives with B16), parameter
 * application, per-note fan-out, and the GUI. Scope is carried per route
 * (kGlobal / kPerNote, the B34 vocabulary) so the shell can evaluate the two
 * populations at their own cadences, but WHAT per-note means is the shell's
 * tiering, not ours.
 *
 * NO CYCLES BY CONSTRUCTION in this increment: sources are primitive slots,
 * not destinations, so a route cannot read a route. When a source can be a
 * destination (macro-of-macros), FOUNDATIONS OQ-23's block-rate unit delay is
 * the ruled semantics for the CONTROL graph — recorded here so the future
 * increment inherits a decision instead of re-deriving one.
 *
 * Correctness = tools/mod_check.cpp (invariants, calibrated), never
 * "plausible modulation". Not yet wired into the audio path — core and oracle
 * first, shell integration behind its own increment (the glide_core order).
 */
#pragma once

#include <cstdint>

namespace hypersaw
{

struct ModCore
{
  static constexpr int kMaxSources = 24;   // ADR-149: MIDI/MPE sources joined at 14-17
  static constexpr int kMaxRoutes = 64;

  enum Scope : int { kGlobal = 0, kPerNote = 1 };

  struct Route
  {
    uint32_t src = 0;       // source slot [0, kMaxSources)
    uint32_t dest = 0;      // destination key (the shell's param id; opaque here)
    double depth = 0;       // signed; the route's whole authority
    int scope = kGlobal;    // B34 vocabulary: who fans this out, and when
    int active = 0;
  };

  Route routes[kMaxRoutes];
  int nRoutes = 0;
  double src[kMaxSources] = {0};   // written by the shell each control tick

  /* Add fails — rather than silently dropping — on a full table or an
     out-of-range source. The caller surfaces the refusal; a matrix that eats
     routes teaches the player the feature is broken. */
  bool addRoute(uint32_t source, uint32_t dest, double depth, int scope)
  {
    if (nRoutes >= kMaxRoutes || source >= (uint32_t)kMaxSources) return false;
    routes[nRoutes] = {source, dest, depth, scope, 1};
    nRoutes++;
    return true;
  }

  void removeRoute(int idx)
  {
    if (idx < 0 || idx >= nRoutes) return;
    for (int i = idx; i < nRoutes - 1; i++) routes[i] = routes[i + 1];
    nRoutes--;
  }

  void clear() { nRoutes = 0; }

  /* The combination law. Fills (dests[], deltas[]) with one entry per distinct
     destination among the ACTIVE routes of `scope`, deltas summed in route
     order. Returns the entry count. O(n²) compaction over ≤64 routes at
     control rate — a map would be an allocation for a problem this small.
     Caller owns the arrays (≥ kMaxRoutes entries); the audio thread allocates
     nothing here. */
  int evaluate(int scope, uint32_t *dests, double *deltas, int maxOut) const
  {
    int n = 0;
    for (int r = 0; r < nRoutes; r++)
    {
      const Route &q = routes[r];
      if (!q.active || q.scope != scope) continue;
      const double d = q.depth * src[q.src];
      int at = -1;
      for (int i = 0; i < n; i++)
        if (dests[i] == q.dest) { at = i; break; }
      if (at >= 0) deltas[at] += d;
      else if (n < maxOut)
      {
        dests[n] = q.dest;
        deltas[n] = d;
        n++;
      }
    }
    return n;
  }
};

}  // namespace hypersaw
