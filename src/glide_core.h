/*
 * glide_core.h — the travel laws (A1 fold, ratified 2026-08-06), transcribed
 * VERBATIM from bend-lab.html's Inertia class. Pitch does not arrive where you
 * point it, it TRAVELS there, and "how long does a glide take" has more than
 * one honest answer:
 *
 *   1 constant time  — same duration near or far (velocity set by distance)
 *   2 constant rate  — fixed cents/second, so distance sets the duration  [DEFAULT]
 *   3 lag            — one-pole, asymptotic, never technically arrives
 *   4 spring         — true inertia: overshoots and rings
 *
 * Law 5 of the prototype (lag -> constant rate) is CUT by the fold ruling: it
 * measured as the closest pair to law 3 (every headline metric identical, peak
 * divergence 8.70 cents) and added a control without adding a behaviour. Its
 * `case 5` is deliberately absent rather than dead-coded.
 *
 * SCALE QUANTISE is a MODIFIER, not a fifth law: it snaps the EMITTED pitch
 * while the law's dynamics run untouched underneath, so it composes with all
 * four (spring + quantise = an overshooting autotune wobble).
 *
 * Parity-exactness rules (as swarm_core.h): doubles for state, control rate =
 * sr/16, no wall-clock. Correctness = trajectory parity vs the JS reference
 * (tools/glide_check.cpp) — never "plausible-feeling glide".
 *
 * NOT YET WIRED INTO THE AUDIO PATH. Core + oracle first, shell integration
 * behind its own increment — the same order the swarmalator port used.
 */
#pragma once

#include <algorithm>
#include <cmath>

namespace hypersaw
{

class GlideCore
{
 public:
  enum Law { kOff = 0, kConstTime = 1, kConstRate = 2, kLag = 3, kSpring = 4 };
  enum Quant { kQuantOff = 0, kQuantChromatic = 1, kQuantScale = 2 };

  struct Params
  {
    double model = kConstRate;   // ratified default: keeps 93% of wheel vibrato
    double gtime = 120;          // ms — constant-time duration
    double rate = 24;            // semitone-units/s for the rate-limited laws
    double tau = 60;             // ms — lag time constant
    double springF = 4;          // Hz
    double damp = 0.6;           // zeta
    double distOver = 1;         // overshoot ~ distance^distOver
    double retMul = 1;           // return-toward-rest time scale (BEND LANE ONLY)
    double quant = kQuantOff;
    double qhyst = 8;            // cents of stickiness at a step boundary
    double scaleRoot = 0;
    // major, as the reference literal
    int scaleMask[12] = {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1};
  };

  // lane matters: only the BEND lane has a home to spring back to, so retMul
  // is meaningless on note pitch and the reference disables it there.
  GlideCore(double controlRate, bool bendLane) : cr(controlRate), bend(bendLane) {}

  void reset(double v)
  {
    x = v; vel = 0; y = v; D = 0;
    q = v; qStep = 0; qArmed = false; qFlips = 0;
  }

  double step(double target, const Params &p)
  {
    const double dt = 1.0 / cr;
    // Move distance, peak-held since the last arrival: constant-time needs it
    // (velocity = distance/time, latched at the start) and so does
    // distance-steered overshoot.
    const double err = std::fabs(target - x);
    if (err > D) D = err;
    // Rearm only once the move has ARRIVED AND STOPPED. Arrival alone is wrong
    // for the spring: it crosses the target at full speed on the way to
    // overshooting, so err dips through zero mid-flight and D would rearm to
    // the overshoot height — re-deriving the damping DURING the overshoot it
    // was meant to set. (The reference records this as a calibration find.)
    if (err < 0.004 && std::fabs(vel) < 0.02) D = 0;
    const double Dc = std::max(D, 0.01);
    const double rm = (bend && std::fabs(target) < std::fabs(x))
                          ? std::max(0.05, p.retMul)
                          : 1.0;
    switch ((int)p.model)
    {
      case kOff:
        x = target; vel = 0; y = target;
        break;
      case kConstTime:
      {
        const double s = Dc / std::max(1e-3, p.gtime * 1e-3 * rm) * dt;
        const double d = target - x;
        x += std::max(-s, std::min(s, d));
        y = x;
        break;
      }
      case kConstRate:
      {
        const double s = (p.rate / rm) * dt, d = target - x;
        x += std::max(-s, std::min(s, d));
        y = x;
        break;
      }
      case kLag:
      {
        const double tau = std::max(1e-4, p.tau * 1e-3 * rm);
        x += (target - x) * (1 - std::exp(-dt / tau));
        y = x;
        break;
      }
      case kSpring:
      {
        // Forward Euler is safe: dt*omega <= 2*pi*20/2756 = 0.046.
        const double w = 6.283185307 * p.springF / rm;
        double z = p.damp;
        if (z < 1 && p.distOver != 1)
          z = zetaFromOs(osFromZeta(z) * std::pow(Dc / 2, p.distOver - 1));
        vel += (w * w * (target - x) - 2 * z * w * vel) * dt;
        x += vel * dt;
        y = x;
        break;
      }
      default:
        x = target; vel = 0; y = target;
        break;
    }
    return quantise(p);
  }

  double value() const { return q; }
  int flips() const { return qFlips; }

 private:
  // Closed-form overshoot of a second-order step response, and its inverse —
  // solving for the damping that PRODUCES a wanted overshoot rather than
  // scaling the output by a fudge factor.
  static double osFromZeta(double z)
  {
    return z >= 1 ? 0 : std::exp(-3.14159265358979323846 * z / std::sqrt(1 - z * z));
  }
  static double zetaFromOs(double os)
  {
    const double l = std::log(std::min(0.95, std::max(1e-4, os)));
    return -l / std::sqrt(3.14159265358979323846 * 3.14159265358979323846 + l * l);
  }

  double quantise(const Params &p)
  {
    const int qm = (int)p.quant;
    if (!qm) { q = x; return x; }
    const double semis = x;
    double bestD = 1e300;
    long best = (long)std::lround(semis);
    if (qm == kQuantScale)
    {
      const long root = (long)p.scaleRoot;
      for (long c = (long)std::floor(semis) - 12; c <= (long)std::ceil(semis) + 12; c++)
      {
        const int idx = (int)(((c - root) % 12 + 12) % 12);
        if (!p.scaleMask[idx]) continue;
        const double d = std::fabs((double)c - semis);
        if (d < bestD) { bestD = d; best = c; }
      }
    }
    // stick to the previous step until the challenger wins by qhyst cents
    const double hyst = std::max(0.0, p.qhyst) / 100.0;
    if (qArmed && best != qStep &&
        std::fabs(semis - (double)qStep) - std::fabs(semis - (double)best) < hyst)
      best = qStep;
    if (!qArmed || best != qStep) { qStep = best; qArmed = true; qFlips++; }
    q = (double)best;
    return q;
  }

  double cr;
  bool bend;
  double x = 0, vel = 0, y = 0, D = 0, q = 0;
  long qStep = 0;
  bool qArmed = false;
  int qFlips = 0;
};

}  // namespace hypersaw
