/*
 * spectra_core.h — SPECTRA engine: per-partial spectral swarm, transcribed
 * VERBATIM from swarmspectra.html's SpectraSynth (ADR-003: the prototype is
 * the spec-in-code). P partials × M cloud voices; each partial carries its
 * own Kuramoto cloud with cascade gating (zipper) and splay-narrowing
 * (interference gate, wmix). Correctness = L0-1-style parity with the JS
 * reference (spectra goldens) + L0-6/7 — never plausible-sounding audio.
 *
 * Parity-exactness rules (same discipline as swarm_core.h):
 *  - doubles for all state math; the SINE table is Float32Array in the
 *    reference, so it is float here (f32-rounded values, double interp).
 *  - output buffers are f32 read-modify-write per store, matching JS typed
 *    array semantics (accumulate across swarms, then tanh in a second pass).
 *  - mulberry32 shared via forcecore (ADR-034); JS int seed semantics.
 *  - reference literals kept: kTau 6.283185307, kPiRef 3.14159265.
 *
 * Relationship to SwarmCore (Phase 4 kernel question, ADR-037): the two
 * references are separate frozen programs (different sigma floors, slews,
 * envelopes, phase-update expressions). This port keeps SPECTRA its own
 * core behind the same shell seam; the SPEC §2 "one engine at P=1" gate
 * needs a human ruling on interpretation before any voice-path merger.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>

#include "force_core.h"

namespace hypersaw
{

class SpectraCore
{
 public:
  static constexpr int kPMax = 24, kMMax = 7, kSPoly = 4, kSTick = 16;
  static constexpr double kSTau = 6.283185307;
  static constexpr double kSPiRef = 3.14159265;

  struct SParams
  {
    double partials = 12, tilt = 1, stretch = 0, cloud = 5, cwidth = 0.25, wtilt = 0, wlaw = 0,
           seed = 1234, K = 0, cascade = 0, onset = 0, dissolve = 0.63, swidth = 0.8, vol = 0.4,
           retrig = 1;
  };

  struct SSwarm
  {
    double phase[kPMax * kMMax], vf[kPMax * kMMax], couple[kPMax * kMMax];
    double R[kPMax], psi[kPMax], sigma[kPMax], KsmS[kPMax], KsmP[kPMax], wmix[kPMax];
    double f0 = 220;
    int midi = -1, gate = 0;
    double env = 0, Kenv = 0;
    long age = -1;
    uint32_t rngState = 1;
  };

  explicit SpectraCore(double sampleRate) : sr(sampleRate)
  {
    for (int i = 0; i <= 4096; i++) SINE[i] = (float)std::sin((double)i / 4096 * 2 * kPiFullS);
    for (auto &s : swarms)
    {
      std::memset(s.phase, 0, sizeof(s.phase));
      std::memset(s.vf, 0, sizeof(s.vf));
      std::memset(s.couple, 0, sizeof(s.couple));
      std::memset(s.R, 0, sizeof(s.R));
      std::memset(s.psi, 0, sizeof(s.psi));
      std::memset(s.sigma, 0, sizeof(s.sigma));
      std::memset(s.KsmS, 0, sizeof(s.KsmS));
      std::memset(s.KsmP, 0, sizeof(s.KsmP));
      std::memset(s.wmix, 0, sizeof(s.wmix));
    }
    rebuild();
  }

  SParams p;

  bool setParam(const char *k, double v)
  {
    double *slot = paramSlot(k);
    if (!slot) return false;
    *slot = v;
    if (!std::strcmp(k, "cloud") || !std::strcmp(k, "swidth") || !std::strcmp(k, "partials") ||
        !std::strcmp(k, "tilt"))
      rebuild();
    return true;
  }
  double getParam(const char *k)
  {
    double *slot = paramSlot(k);
    return slot ? *slot : 0.0;
  }

  void rebuild()
  {
    const int M = (int)p.cloud, P = (int)p.partials;
    for (int m = 0; m < M; m++) x[m] = (M == 1) ? 0 : (2.0 * m / (M - 1) - 1);
    centerIdx = 0;
    for (int m = 1; m < M; m++)
      if (std::fabs(x[m]) < std::fabs(x[centerIdx])) centerIdx = m;
    for (int k = 0; k < P; k++)
    {
      amp[k] = 1 / std::pow(k + 1, p.tilt);
      const double sgn = (k & 1) ? -1 : 1;  // alternate pan orientation per partial
      for (int m = 0; m < M; m++)
      {
        const double pan = std::max(-1.0, std::min(1.0, x[m] * sgn)) * p.swidth;
        const double th = (pan + 1) * 0.25 * kSPiRef;
        const int idx = k * kMMax + m;
        panL[idx] = std::cos(th);
        panR[idx] = std::sin(th);
      }
    }
  }

  SSwarm &alloc()
  {
    SSwarm *best = nullptr;
    for (auto &s : swarms)
      if (!s.gate && s.env < 1e-3)
        if (!best || s.age < best->age) best = &s;
    if (best) return *best;
    for (auto &s : swarms)
      if (!best || s.age < best->age) best = &s;
    return *best;
  }

  int noteOn(int midi, double f)
  {
    SSwarm &s = alloc();
    s.midi = midi;
    s.f0 = f;
    s.gate = 1;
    s.age = noteCounter++;
    s.Kenv = 8 * p.onset * p.onset;
    // (seed|0) + age*7919 + 1 — same wrap semantics as SwarmCore initVoice
    s.rngState = (uint32_t)((int64_t)(int32_t)(int64_t)p.seed + (int64_t)s.age * 7919 + 1);
    const int P = (int)p.partials, M = (int)p.cloud;
    (void)P;
    for (int k = 0; k < kPMax; k++)
    {
      s.KsmS[k] = 0;
      s.KsmP[k] = 0;
      for (int m = 0; m < kMMax; m++)
        s.phase[k * kMMax + m] = (p.retrig != 0) ? 0.0 : forcecore::rngNext(s.rngState);
    }
    (void)M;
    return (int)(&s - &swarms[0]);
  }
  void noteOff(int midi)
  {
    for (auto &s : swarms)
      if (s.gate && s.midi == midi) s.gate = 0;
  }
  void allOff()
  {
    for (auto &s : swarms)
      s.gate = 0;
  }
  const SSwarm &swarmAt(int i) const { return swarms[i]; }
  const SSwarm *focus() const
  {
    const SSwarm *best = nullptr;
    for (const auto &s : swarms)
      if ((s.gate || s.env > 1e-3) && (!best || s.age > best->age)) best = &s;
    return best;
  }

  void controlTick(SSwarm &s)
  {
    const int P = (int)p.partials, M = (int)p.cloud;
    const double dt = kSTick / sr;
    s.Kenv *= std::exp(-dt / std::max(0.01, p.dissolve));
    const double km = 4 * p.K * std::fabs(p.K);
    const double B = p.stretch * p.stretch * 0.002;  // inharmonicity, squared taper
    const int c0 = centerIdx < M ? centerIdx : 0;
    for (int k = 0; k < P; k++)
    {
      const double h = k + 1;
      const double fk = s.f0 * h * std::sqrt(1 + B * h * h);
      const double tiltMul = std::pow(h, p.wtilt);
      double mean = 0;
      for (int m = 0; m < M; m++)
      {
        const int idx = k * kMMax + m;
        double f;
        if (p.wlaw == 0)
        {
          const double cents = x[m] * p.cwidth * 40 * tiltMul;
          f = fk * std::pow(2, std::max(-300.0, std::min(300.0, cents)) / 1200);
        }
        else
        {
          f = fk + x[m] * p.cwidth * 12 * tiltMul;
        }
        s.vf[idx] = std::max(1.0, f);
        mean += s.vf[idx];
      }
      mean /= M;
      double varsum = 0;
      for (int m = 0; m < M; m++)
      {
        const double d = s.vf[k * kMMax + m] - mean;
        varsum += d * d;
      }
      s.sigma[k] = std::max(0.05, std::sqrt(varsum / M));
      double sx = 0, sy = 0;
      for (int m = 0; m < M; m++)
      {
        const double a = s.phase[k * kMMax + m] * kSTau;
        sx += std::cos(a);
        sy += std::sin(a);
      }
      sx /= M;
      sy /= M;
      s.R[k] = std::sqrt(sx * sx + sy * sy);
      s.psi[k] = std::atan2(sy, sx);
      // cascade gate: partial k couples fully once partial k-1 has locked
      double gate = 1;
      if (k > 0 && p.cascade >= 0.005)
      {
        const double r = s.R[k - 1];
        const double t = std::max(0.0, std::min(1.0, (r - 0.4) / 0.45));
        gate = 1 - p.cascade + p.cascade * t * t;
      }
      const double syncT = (std::max(0.0, km) + s.Kenv) * gate * s.sigma[k];
      const double splayT = std::max(0.0, -km) * 3 * gate * s.sigma[k];
      const double slew = 0.08 / (1 + p.cascade * 120);
      s.KsmS[k] += (syncT - s.KsmS[k]) * slew;
      s.KsmP[k] += (splayT - s.KsmP[k]) * slew;
      // interference gate needs per-channel coherence: narrow stereo as splay engages
      s.wmix[k] = s.KsmP[k] / (s.KsmP[k] + 2 * s.sigma[k]);
      const double anchor = s.phase[k * kMMax + c0];
      for (int m = 0; m < M; m++)
      {
        const int idx = k * kMMax + m;
        double c = s.KsmS[k] * s.R[k] * std::sin(s.psi[k] - s.phase[idx] * kSTau);
        if (s.KsmP[k] > 0.001)
          c += s.KsmP[k] * std::sin(kSTau * (anchor + (double)(m - c0) / M - s.phase[idx]));
        s.couple[idx] = c;
      }
    }
  }

  void render(float *outL, float *outR, int nSamples)
  {
    const int P = (int)p.partials, M = (int)p.cloud;
    double ampSum = 0;
    for (int k = 0; k < P; k++) ampSum += amp[k];
    const double gain = p.vol * 1.4 / (ampSum * std::pow((double)M, 0.75));
    const double atk = 1 - std::exp(-1 / (0.004 * sr));
    const double rel = 1 - std::exp(-1 / (0.18 * sr));
    for (int i = 0; i < nSamples; i++)
    {
      outL[i] = 0;
      outR[i] = 0;
    }
    for (auto &s : swarms)
    {
      if (!s.gate && s.env < 1e-4) continue;
      int tick = globalTick;
      for (int smp = 0; smp < nSamples; smp++)
      {
        if (tick == 0) controlTick(s);
        tick = (tick + 1) & (kSTick - 1);
        double l = 0, r = 0;
        for (int k = 0; k < P; k++)
        {
          const double a = amp[k];
          const int base = k * kMMax;
          const double wm = s.wmix[k], iwm = 1 - wm;
          for (int m = 0; m < M; m++)
          {
            const int idx = base + m;
            const double f = s.vf[idx] + s.couple[idx];
            double ph = s.phase[idx] + std::max(0.0, f) / sr;
            ph -= std::floor(ph);
            s.phase[idx] = ph;
            const double ti = ph * 4096;
            const int t0 = (int)ti;
            const double v = (SINE[t0] + (SINE[t0 + 1] - SINE[t0]) * (ti - t0)) * a;
            l += v * (panL[idx] * iwm + 0.7071 * wm);
            r += v * (panR[idx] * iwm + 0.7071 * wm);
          }
        }
        const double coef = s.gate ? atk : rel;
        s.env += ((s.gate ? 1 : 0) - s.env) * coef;
        const double g = gain * s.env;
        outL[smp] = (float)(outL[smp] + l * g);  // f32 RMW, JS typed-array exact
        outR[smp] = (float)(outR[smp] + r * g);
      }
    }
    globalTick = (globalTick + nSamples) & (kSTick - 1);
    for (int smp = 0; smp < nSamples; smp++)
    {
      outL[smp] = (float)std::tanh((double)outL[smp]);
      outR[smp] = (float)std::tanh((double)outR[smp]);
    }
  }

 private:
  static constexpr double kPiFullS = 3.141592653589793;  // Math.PI (SINE table build)
  double sr;
  float SINE[4097];
  double x[kMMax] = {0};
  double panL[kPMax * kMMax] = {0}, panR[kPMax * kMMax] = {0};
  double amp[kPMax] = {0};
  int centerIdx = 0;
  long noteCounter = 0;
  int globalTick = 0;
  SSwarm swarms[kSPoly];

  double *paramSlot(const char *k)
  {
    if (!std::strcmp(k, "partials")) return &p.partials;
    if (!std::strcmp(k, "tilt")) return &p.tilt;
    if (!std::strcmp(k, "stretch")) return &p.stretch;
    if (!std::strcmp(k, "cloud")) return &p.cloud;
    if (!std::strcmp(k, "cwidth")) return &p.cwidth;
    if (!std::strcmp(k, "wtilt")) return &p.wtilt;
    if (!std::strcmp(k, "wlaw")) return &p.wlaw;
    if (!std::strcmp(k, "seed")) return &p.seed;
    if (!std::strcmp(k, "K")) return &p.K;
    if (!std::strcmp(k, "cascade")) return &p.cascade;
    if (!std::strcmp(k, "onset")) return &p.onset;
    if (!std::strcmp(k, "dissolve")) return &p.dissolve;
    if (!std::strcmp(k, "swidth")) return &p.swidth;
    if (!std::strcmp(k, "vol")) return &p.vol;
    if (!std::strcmp(k, "retrig")) return &p.retrig;
    return nullptr;
  }
};

}  // namespace hypersaw
