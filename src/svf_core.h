/*
 * svf_core.h — B81 increment 2: the per-voice filter's core.
 *
 * A TPT (topology-preserving transform) state-variable filter, Zavalishin's
 * form: one g/k pair, two integrator states, LP/BP/HP taken from the same
 * pass. Chosen over biquads because the trapezoidal integrators stay stable
 * under audio-rate cutoff motion — this filter's whole job is to be swept by
 * a per-voice envelope (B81's dedicated filter env), so modulation behaviour
 * is the design center, not an afterthought.
 *
 * Contract (the svf_check oracle is the spec, ADR-142 precedent):
 * - Deterministic: no RNG, no allocation, no wall-clock. Same input, same
 *   params -> identical output, any sample rate, any chunking.
 * - Coefficients update at the shell's 16-sample tick via setParams(); the
 *   per-sample path reads them, never computes tan(). Cutoff slew lives in
 *   the CALLER (the shell's seconds-based tick coefficients, ADR-009) — this
 *   core is memoryless in its parameters on purpose, so the oracle can pin
 *   input/param pairs to exact outputs.
 * - kBypass passes the input through BIT-EXACTLY (the voicetap T1/T2
 *   discipline: off must be provably absent, not merely quiet).
 *
 * Not this core's business: routing (A/B/both, serial/parallel — shell),
 * the filter envelope (shell), per-note matrix fan-out (B82).
 */
#pragma once

#include <algorithm>
#include <cmath>

namespace hypersaw
{

struct SvfCore
{
  enum Mode : int { kBypass = 0, kLowpass = 1, kBandpass = 2, kHighpass = 3 };

  // fcHz clamped to [10, 0.49*fs]: tan() blows up at Nyquist and the audible
  // difference between 0.49fs and "off" is nil — the clamp is the stability
  // guarantee the oracle asserts, not a tuning choice.
  void setSampleRate(double fs)
  {
    sr = fs > 1.0 ? fs : 44100.0;
    setParams(fc, res);
  }

  void setParams(double fcHz, double resonance)
  {
    fc = std::max(10.0, std::min(0.49 * sr, fcHz));
    res = std::max(0.0, std::min(1.0, resonance));
    const double g = std::tan(3.14159265358979323846 * fc / sr);
    // k = sqrt(2)*(1-res): res 0 -> k=1.414 (Q=0.707, Butterworth: -3 dB at
    // fc — svf_check T3 measured the first draft's k=2 at -6.02 dB, which is
    // critical damping, not the classic synth lowpass at rest). res 1 ->
    // the 0.05 floor, a tall ring that stays bounded: k=0 is an undamped
    // resonator and this filter must decay (T6 asserts exactly that).
    const double k = std::max(0.05, 1.4142135623730951 * (1.0 - res));
    kDamp = k;
    a1 = 1.0 / (1.0 + g * (g + k));
    a2 = g * a1;
    a3 = g * a2;
  }

  void reset() { ic1 = 0.0; ic2 = 0.0; }

  // One sample. Branchless in the filter path; the mode switch is the only
  // branch and predicts perfectly (mode changes at tick rate, not per sample).
  inline double process(double in, int mode)
  {
    if (mode == kBypass) return in;   // bit-exact: no state touched
    const double v3 = in - ic2;
    const double v1 = a1 * ic1 + a2 * v3;              // bandpass
    const double v2 = ic2 + a2 * ic1 + a3 * v3;        // lowpass
    ic1 = 2.0 * v1 - ic1;
    ic2 = 2.0 * v2 - ic2;
    if (mode == kLowpass) return v2;
    if (mode == kBandpass) return v1;
    return in - kDamp * v1 - v2;                       // highpass
  }

  double sr = 44100.0, fc = 1000.0, res = 0.0;

 private:
  double a1 = 0.0, a2 = 0.0, a3 = 0.0, kDamp = 2.0;
  double ic1 = 0.0, ic2 = 0.0;
};

}  // namespace hypersaw
