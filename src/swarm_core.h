/*
 * swarm_core.h — C++ port of the SAW reference core (SwarmSynth in
 * swarmsaw.html, ADR-003). Correctness = L0-1 parity (ε = 1e-6 RMS vs the JS
 * renders), NOT plausible audio — so this is a statement-by-statement
 * transcription, and every deviation from the obvious C++ idiom below is
 * load-bearing:
 *
 * - All state is double (JS numbers are f64); output buffers are float with
 *   read-modify-write rounding per store, because the reference accumulates
 *   into a Float32Array (each += rounds to f32 — skipping that rounding is
 *   an audible-in-the-oracle divergence).
 * - The RNG reproduces mulberry32 under JS int semantics: |0 (ToInt32),
 *   Math.imul (low-32 multiply), >>> (uint32 shift). uint32_t arithmetic is
 *   bit-identical to all of these.
 * - Expression shapes match the reference (e.g. sigma floor before use,
 *   couple computed from the PREVIOUS tick's phases, per-swarm local `tick`
 *   counter with the shared counter advanced once per render call).
 * - Header-only, pure, allocation-free after construction, no wall-clock
 *   (SPEC §5.7); tools and the plugin both include it.
 *
 * Known parity caveat (measured, not assumed): V8's transcendentals are
 * fdlibm-based and may differ from libm by ULPs; chaotic regimes can amplify
 * that. The parity harness reports per-scenario RMS so any such divergence
 * is visible and attributable.
 */
#pragma once

#include <cstdint>
#include <cmath>
#include <cstring>
#include <string>

namespace hypersaw
{

constexpr int kMaxV = 32;
constexpr int kPoly = 8;
constexpr int kTick = 16;
constexpr double kTau = 6.283185307;   // matches the reference's literal
constexpr double kPiRef = 3.14159265;  // ditto — NOT M_PI, parity over precision

struct Params
{
  double n = 7, dist = 1, seed = 1234, detune = 0.28, law = 0, K = 0, onset = 0,
         dissolve = 0.63, driftDepth = 0, driftRate = 0.4, inertia = 0, rtone = 0,
         normExp = 0.75, width = 0.8, mono = 0, digital = 1, vol = 0.4, retrig = 1;
  // ADSR (ADR-021): defaults reproduce the reference AR bit-exactly — at
  // sustainL >= 1 the render loop takes the reference's exact expressions,
  // and attackS/releaseS defaults are the reference's own constants. L0-1
  // goldens are the regression proof; change defaults only with an ADR.
  double attackS = 0.003, decayS = 0.16, sustainL = 1.0, releaseS = 0.16;
  // Tempo-grid law (law == 3; ADR-005/ADR-022): ported expression-for-
  // expression from the DYNAMICS reference (swarmdynamics.html beatQ path).
  // bpm is host-owned (CLAP transport), beatMult is cycles-per-beat.
  double bpm = 120, beatMult = 1;
  // Dynamics layer (Phase 3, ADR-023): topology / Sakaguchi lag / Daido
  // poles / consonance gravity, ported expression-for-expression from the
  // DYNAMICS reference. Defaults reproduce the SAW reference bit-exactly
  // (topo 0, alpha 0, poles 1, grav 0 leave every SAW expression untouched);
  // the DYN golden set proves the non-default paths. lpOut=0 bypasses the
  // R->tone output pole — the one structural difference between the two
  // references (DynSynth has no output filter).
  double topo = 0, reach = 5, mu = 0.6, alpha = 0, poles = 1, grav = 0, basin = 35, lpOut = 1;
};

// Consonance gravity ratio set (SPEC Layer 3, ADR-008) — the DYNAMICS
// reference's exact values; octave-folded snap targets.
constexpr double kRatios[13] = {1.0,       16.0 / 15, 9.0 / 8, 6.0 / 5, 5.0 / 4,
                                4.0 / 3,   7.0 / 5,   3.0 / 2, 8.0 / 5, 5.0 / 3,
                                16.0 / 9,  15.0 / 8,  2.0};
// Math.PI (full precision) — the DYNAMICS reference uses it for the alpha
// degrees->radians conversion, unlike the truncated kPiRef literals.
constexpr double kPiFull = 3.141592653589793;

class SwarmCore
{
 public:
  struct Swarm
  {
    double phase[kMaxV], driftS[kMaxV], couple[kMaxV], vf[kMaxV], eff[kMaxV], mom[kMaxV];
    double f0 = 220, f0cur = 220;
    int midi = -1, gate = 0;
    double env = 0, Kenv = 0, KsmS = 0, KsmP = 0;
    double R = 0, RN = 0, psi = 0, sigma = 0, RA = 0, RB = 0, RQ = 0;
    long age = -1;
    uint32_t rngState = 1;
    int fresh = 1;
    int inAttack = 1;  // ADSR phase flag; unread on the reference-exact path
    double lpL = 0, lpR = 0, lpc = 1;
  };

  explicit SwarmCore(double sampleRate) : sr(sampleRate)
  {
    for (auto &s : swarms)
    {
      std::memset(s.phase, 0, sizeof(s.phase));
      std::memset(s.driftS, 0, sizeof(s.driftS));
      std::memset(s.couple, 0, sizeof(s.couple));
      std::memset(s.vf, 0, sizeof(s.vf));
      std::memset(s.eff, 0, sizeof(s.eff));
      std::memset(s.mom, 0, sizeof(s.mom));
    }
    rebuild();
  }

  // Mirrors the JS setParam(k, v) including its rebuild triggers.
  bool setParam(const std::string &k, double v)
  {
    double *slot = paramSlot(k);
    if (!slot) return false;
    *slot = v;
    if (k == "n" || k == "dist" || k == "seed" || k == "width" || k == "topo") rebuild();
    return true;
  }

  // Returns the swarm slot index so the shell can track host note identity
  // (CLAP NOTE_END bookkeeping). DSP behavior unchanged — parity-neutral.
  int noteOn(int midi, double f)
  {
    Swarm &s = alloc();
    const int slot = (int)(&s - &swarms[0]);
    s.midi = midi;
    s.f0 = f;
    s.f0cur = f;
    s.gate = 1;
    s.age = noteCounter++;
    s.Kenv = 8 * p.onset * p.onset;
    s.KsmS = 0;
    s.KsmP = 0;
    s.fresh = 1;
    s.inAttack = 1;
    s.lpL = 0;
    s.lpR = 0;
    for (int i = 0; i < kMaxV; i++) s.mom[i] = 0;
    // (seed|0) + age*7919 + 1 under ToInt32 — uint32 wrap is bit-identical
    s.rngState = (uint32_t)((int64_t)toInt32(p.seed) + (int64_t)s.age * 7919 + 1);
    for (int i = 0; i < kMaxV; i++)
    {
      s.driftS[i] = 0;
      s.phase[i] = (p.retrig != 0) ? 0.0 : rngNext(s.rngState);
    }
    return slot;
  }

  void noteOff(int midi)
  {
    for (auto &s : swarms)
      if (s.gate && s.midi == midi) s.gate = 0;
  }

  void allOff()
  {
    for (auto &s : swarms) s.gate = 0;
  }

  // Read-only accessors for the shell (viz feed; NOTE_END bookkeeping).
  // Not used by the DSP path; parity-neutral.
  int centerIndex() const { return centerIdx; }
  const Swarm &swarmAt(int i) const { return swarms[i]; }

  const Swarm *focus() const
  {
    const Swarm *best = nullptr;
    for (const auto &s : swarms)
      if ((s.gate || s.env > 1e-3) && (!best || s.age > best->age)) best = &s;
    return best;
  }

  // Consonance gravity (ADR-008; DYN reference exact): once per render call,
  // pull each gated pair's f0cur toward the nearest folded ratio inside the
  // basin. grav < 0.005 returns untouched — the SAW-parity path.
  void gravityStep(double dtB)
  {
    gravCount = 0;
    const double g = p.grav;
    if (g < 0.005) return;
    Swarm *act[kPoly];
    int na = 0;
    for (auto &s : swarms)
      if (s.gate) act[na++] = &s;
    if (na < 2) return;
    // insertion sort ascending by f0cur (stable; matches JS sort for the
    // distinct-frequency case)
    for (int i = 1; i < na; i++)
      for (int j = i; j > 0 && act[j]->f0cur < act[j - 1]->f0cur; j--) std::swap(act[j], act[j - 1]);
    const double rate = g * 3;
    for (int a = 0; a < na - 1; a++)
      for (int b = a + 1; b < na; b++)
      {
        Swarm *lo = act[a], *hi = act[b];
        const double r = hi->f0cur / lo->f0cur;
        const double oct = std::floor(std::log2(r));
        const double rf = r / std::pow(2, oct);
        int bi = 0;
        double be = 1e9;
        for (int i = 0; i < 13; i++)
        {
          const double e = std::fabs(1200 * std::log2(rf / kRatios[i]));
          if (e < be)
          {
            be = e;
            bi = i;
          }
        }
        const double err = 1200 * std::log2(rf / kRatios[bi]);
        if (std::fabs(err) > p.basin) continue;
        const double move = err * rate * dtB * 0.5;
        hi->f0cur *= std::pow(2, -move / 1200);
        lo->f0cur *= std::pow(2, move / 1200);
        if (gravCount < 32)
        {
          gravPairs[gravCount][0] = bi;
          gravPairs[gravCount][1] = (int)oct;
          gravErr[gravCount] = err;
          gravCount++;
        }
      }
  }

  void render(float *outL, float *outR, int frames)
  {
    gravityStep((double)frames / sr);
    const int n = (int)p.n;
    const double gain = p.vol * 0.9 / std::pow((double)n, p.normExp);
    // ADR-021: at the defaults these are the reference's exact expressions
    // (attackS = 0.003, releaseS = 0.16 — same operands, same doubles).
    const double atk = 1 - std::exp(-1 / (p.attackS * sr));
    const double rel = 1 - std::exp(-1 / (p.releaseS * sr));
    const double dec = 1 - std::exp(-1 / (p.decayS * sr));
    for (int i = 0; i < frames; i++) { outL[i] = 0.0f; outR[i] = 0.0f; }
    for (auto &s : swarms)
    {
      if (!s.gate && s.env < 1e-4) continue;
      int tick = this->tick;
      for (int smp = 0; smp < frames; smp++)
      {
        if (tick == 0) controlTick(s);
        tick = (tick + 1) & (kTick - 1);
        double l = 0, r = 0;
        for (int i = 0; i < n; i++)
        {
          const double f = s.eff[i];
          const double dph = std::max(0.0, f) / sr;
          double ph = s.phase[i] + dph;
          ph -= std::floor(ph);
          s.phase[i] = ph;
          const double naive = 2 * ph - 1;
          double v = naive;
          if (p.digital > 0)
          {
            const double d = std::max(dph, 1e-6);
            double bl = 0;
            if (ph < d) { const double t = ph / d; bl = t + t - t * t - 1; }
            else if (ph > 1 - d) { const double t = (ph - 1) / d; bl = t * t + t + t + 1; }
            v = naive - p.digital * bl;
          }
          if (p.mono != 0) { l += v * 0.7071; r += v * 0.7071; }
          else { l += v * panL[i]; r += v * panR[i]; }
        }
        if (p.lpOut != 0)
        {
          s.lpL += s.lpc * (l - s.lpL);
          s.lpR += s.lpc * (r - s.lpR);
        }
        else
        {
          // DYN-reference config: no output pole (the one structural
          // difference between the references; ADR-023)
          s.lpL = l;
          s.lpR = r;
        }
        // ADR-021 envelope. sustainL >= 1: the reference's exact arithmetic
        // ((1-env)*atk while gated, (0-env)*rel released) — decay never
        // engages, parity preserved. sustainL < 1: attack->decay machine,
        // deliberately divergent (superset behavior).
        if (s.gate)
        {
          if (p.sustainL >= 1.0)
          {
            s.env += (1 - s.env) * atk;
          }
          else if (s.inAttack)
          {
            s.env += (1 - s.env) * atk;
            if (s.env >= 0.995) s.inAttack = 0;
          }
          else
          {
            s.env += (p.sustainL - s.env) * dec;
          }
        }
        else
        {
          s.env += (0 - s.env) * rel;
        }
        const double g = gain * s.env;
        // Float32Array += semantics: round to f32 on every store
        outL[smp] = (float)((double)outL[smp] + s.lpL * g);
        outR[smp] = (float)((double)outR[smp] + s.lpR * g);
      }
    }
    this->tick = (this->tick + frames) & (kTick - 1);
    for (int smp = 0; smp < frames; smp++)
    {
      outL[smp] = (float)std::tanh((double)outL[smp]);
      outR[smp] = (float)std::tanh((double)outR[smp]);
    }
  }

  Params p;

 private:
  static int32_t toInt32(double v) { return (int32_t)(int64_t)v; }

  // mulberry32 under JS int semantics; uint32 ops are bit-identical
  static double rngNext(uint32_t &state)
  {
    state += 0x6D2B79F5u;
    uint32_t t = (uint32_t)(state ^ (state >> 15)) * (uint32_t)(1u | state);
    t = (uint32_t)(t + (uint32_t)((uint32_t)(t ^ (t >> 7)) * (uint32_t)(61u | t))) ^ t;
    return (double)(t ^ (t >> 14)) / 4294967296.0;
  }

  double *paramSlot(const std::string &k)
  {
    if (k == "n") return &p.n;
    if (k == "dist") return &p.dist;
    if (k == "seed") return &p.seed;
    if (k == "detune") return &p.detune;
    if (k == "law") return &p.law;
    if (k == "K") return &p.K;
    if (k == "onset") return &p.onset;
    if (k == "dissolve") return &p.dissolve;
    if (k == "driftDepth") return &p.driftDepth;
    if (k == "driftRate") return &p.driftRate;
    if (k == "inertia") return &p.inertia;
    if (k == "rtone") return &p.rtone;
    if (k == "normExp") return &p.normExp;
    if (k == "width") return &p.width;
    if (k == "mono") return &p.mono;
    if (k == "digital") return &p.digital;
    if (k == "vol") return &p.vol;
    if (k == "retrig") return &p.retrig;
    if (k == "attack") return &p.attackS;
    if (k == "decay") return &p.decayS;
    if (k == "sustain") return &p.sustainL;
    if (k == "release") return &p.releaseS;
    if (k == "bpm") return &p.bpm;
    if (k == "beatMult") return &p.beatMult;
    if (k == "topo") return &p.topo;
    if (k == "reach") return &p.reach;
    if (k == "mu") return &p.mu;
    if (k == "alpha") return &p.alpha;
    if (k == "poles") return &p.poles;
    if (k == "grav") return &p.grav;
    if (k == "basin") return &p.basin;
    if (k == "lpOut") return &p.lpOut;
    return nullptr;
  }

  void rebuild()
  {
    const int n = (int)p.n;
    grng = (uint32_t)(toInt32(p.seed) + 1);
    if ((int)p.topo == 2)
    {
      // bimodal placement tied to the two-cluster topology (DYN reference
      // exact): cluster A on [-0.85,-0.35], cluster B on [0.35,0.85].
      const int h = n >> 1;
      for (int i = 0; i < h; i++)
      {
        const double t = h == 1 ? 0.5 : (double)i / (h - 1);
        x[i] = -0.85 + 0.5 * t;
      }
      for (int i = h; i < n; i++)
      {
        const double t = (n - h) == 1 ? 0.5 : (double)(i - h) / (n - h - 1);
        x[i] = 0.35 + 0.5 * t;
      }
      finishRebuild(n);
      return;
    }
    static const double JP[7] = {-1, -0.5715, -0.1774, 0, 0.181, 0.565, 0.9766};
    for (int i = 0; i < n; i++)
    {
      const double u = (n == 1) ? 0.5 : (double)i / (n - 1);
      double xv;
      if (p.dist == 0) { xv = 2 * u - 1; }
      else if (p.dist == 1)
      {
        const double pos = u * 6;  // (JP.length - 1)
        const int a = (int)std::floor(pos);
        const int b = a + 1 < 6 ? a + 1 : 6;
        xv = JP[a] + (JP[b] - JP[a]) * (pos - a);
      }
      else if (p.dist == 2)
      {
        const double u1 = std::max(rngNext(grng), 1e-9), u2 = rngNext(grng);
        xv = std::sqrt(-2 * std::log(u1)) * std::cos(kTau * u2) / 2.5;
        if (xv > 1) xv = 1;
        if (xv < -1) xv = -1;
      }
      else
      {
        xv = std::tan(kPiRef * (rngNext(grng) - 0.5)) / 4;
        if (xv > 1) xv = 1;
        if (xv < -1) xv = -1;
      }
      x[i] = xv;
    }
    if (n == 1) x[0] = 0;
    finishRebuild(n);
  }

  void finishRebuild(int n)
  {
    centerIdx = 0;
    for (int i = 1; i < n; i++)
      if (std::fabs(x[i]) < std::fabs(x[centerIdx])) centerIdx = i;
    for (int i = 0; i < n; i++)
    {
      const double pan = std::max(-1.0, std::min(1.0, x[i])) * p.width;
      const double th = (pan + 1) * 0.25 * kPiRef;
      panL[i] = std::cos(th);
      panR[i] = std::sin(th);
    }
  }

  Swarm &alloc()
  {
    Swarm *best = nullptr;
    for (auto &s : swarms)
      if (!s.gate && s.env < 1e-3)
        if (!best || s.age < best->age) best = &s;
    if (best) return *best;
    for (auto &s : swarms)
      if (!best || s.age < best->age) best = &s;
    return *best;
  }

  static double erb(double f) { return 24.7 * (4.37 * f / 1000 + 1); }

  void controlTick(Swarm &s)
  {
    const int n = (int)p.n;
    const double dt = kTick / sr;
    s.Kenv *= std::exp(-dt / std::max(0.01, p.dissolve));
    if (p.driftDepth > 0)
    {
      const double rate = (0.2 + p.driftRate * 8);
      for (int i = 0; i < n; i++)
      {
        s.driftS[i] += (rngNext(s.rngState) - 0.5) * 2 * std::sqrt(rate * dt);
        s.driftS[i] -= s.driftS[i] * 0.4 * dt;
        if (s.driftS[i] > 1) s.driftS[i] = 1;
        if (s.driftS[i] < -1) s.driftS[i] = -1;
      }
    }
    double mean = 0;
    for (int i = 0; i < n; i++)
    {
      double f;
      const double xv = x[i];
      if (p.law == 0) { f = s.f0cur * std::pow(2, (xv * p.detune * 100) / 1200); }
      else if (p.law == 1) { f = s.f0cur + xv * p.detune * 20; }
      else if (p.law == 3)
      {
        // tempo-grid (ADR-022): cents placement, then snap the Hz offset to
        // the nearest multiple of u — every pairwise beat rate becomes an
        // exact grid multiple. Expression ported verbatim from the DYNAMICS
        // reference. Drift (below) deliberately loosens the grid when used.
        const double u = (p.bpm / 60.0) * p.beatMult;
        const double df = s.f0cur * (std::pow(2, (xv * p.detune * 100) / 1200) - 1);
        f = s.f0cur + std::round(df / u) * u;
      }
      else { f = s.f0cur + xv * p.detune * 0.35 * erb(s.f0cur); }
      if (p.driftDepth > 0) f *= std::pow(2, (s.driftS[i] * p.driftDepth) / 1200);
      s.vf[i] = std::max(1.0, f);
      mean += s.vf[i];
    }
    mean /= n;
    double varsum = 0;
    for (int i = 0; i < n; i++)
    {
      const double d = s.vf[i] - mean;
      varsum += d * d;
    }
    s.sigma = std::max(0.08, std::sqrt(varsum / n));
    const double km = 4 * p.K * std::fabs(p.K);
    const double syncT = (std::max(0.0, km) + s.Kenv) * s.sigma;
    const double splayT = std::max(0.0, -km) * 3 * s.sigma;
    s.KsmS += (syncT - s.KsmS) * 0.08;
    s.KsmP += (splayT - s.KsmP) * 0.08;
    double sx = 0, sy = 0;
    for (int i = 0; i < n; i++)
    {
      const double a = s.phase[i] * kTau;
      sx += std::cos(a);
      sy += std::sin(a);
    }
    sx /= n;
    sy /= n;
    s.R = std::sqrt(sx * sx + sy * sy);
    s.psi = std::atan2(sy, sx);
    double nx = 0, ny = 0;
    for (int i = 0; i < n; i++)
    {
      const double a = s.phase[i] * kTau * n;
      nx += std::cos(a);
      ny += std::sin(a);
    }
    s.RN = std::sqrt(nx * nx + ny * ny) / n;
    // Topology / Sakaguchi / Daido (ADR-023, DYN reference exact). SAW
    // defaults (topo 0, alpha 0, poles 1) reduce every expression to the SAW
    // reference's own: sin(psi - theta - 0.0) is bit-equal to sin(psi -
    // theta), and the splay term only exists on the mean-field path (the SAW
    // reference has no topologies; the DYN reference has no splay).
    const double alphaR = p.alpha * kPiFull / 180;
    const int topo = (int)p.topo;
    if (topo == 0)
    {
      const int q = (int)p.poles;
      if (q > 1)
      {
        double qx = 0, qy = 0;
        for (int i = 0; i < n; i++)
        {
          const double a = s.phase[i] * kTau * q;
          qx += std::cos(a);
          qy += std::sin(a);
        }
        qx /= n;
        qy /= n;
        s.RQ = std::sqrt(qx * qx + qy * qy);
        const double psiQ = std::atan2(qy, qx);
        for (int i = 0; i < n; i++)
          s.couple[i] = s.KsmS * s.RQ * std::sin(psiQ - s.phase[i] * kTau * q - alphaR);
      }
      else
      {
        s.RQ = 0;
        const int c0 = centerIdx < n ? centerIdx : 0;
        for (int i = 0; i < n; i++)
        {
          double c = s.KsmS * s.R * std::sin(s.psi - s.phase[i] * kTau - alphaR);
          if (s.KsmP > 0.001)
            c += s.KsmP * std::sin(kTau * (s.phase[c0] + (double)(i - c0) / n - s.phase[i]));
          s.couple[i] = c;
        }
      }
      s.RA = 0;
      s.RB = 0;
    }
    else if (topo == 1)
    {
      const int r = std::min((int)p.reach, (n - 1) >> 1);
      for (int i = 0; i < n; i++)
      {
        const double ti = s.phase[i] * kTau;
        double acc = 0;
        for (int d = 1; d <= r; d++)
        {
          const int jl = (i - d + n) % n, jr = (i + d) % n;
          acc += std::sin(s.phase[jl] * kTau - ti - alphaR);
          acc += std::sin(s.phase[jr] * kTau - ti - alphaR);
        }
        s.couple[i] = s.KsmS * acc / (2 * r);
      }
      s.RA = 0;
      s.RB = 0;
    }
    else
    {
      const int h = n >> 1;
      const double m = p.mu;
      double ax = 0, ay = 0, bx = 0, by = 0;
      for (int i = 0; i < h; i++)
      {
        const double a = s.phase[i] * kTau;
        ax += std::cos(a);
        ay += std::sin(a);
      }
      for (int i = h; i < n; i++)
      {
        const double a = s.phase[i] * kTau;
        bx += std::cos(a);
        by += std::sin(a);
      }
      ax /= h;
      ay /= h;
      bx /= (n - h);
      by /= (n - h);
      const double RA = std::hypot(ax, ay), psiA = std::atan2(ay, ax);
      const double RB = std::hypot(bx, by), psiB = std::atan2(by, bx);
      s.RA = RA;
      s.RB = RB;
      const double norm = 1 + m;
      for (int i = 0; i < n; i++)
      {
        const double ti = s.phase[i] * kTau;
        double c;
        if (i < h) c = (RA * std::sin(psiA - ti - alphaR) + m * RB * std::sin(psiB - ti - alphaR)) / norm;
        else c = (RB * std::sin(psiB - ti - alphaR) + m * RA * std::sin(psiA - ti - alphaR)) / norm;
        s.couple[i] = s.KsmS * c;
      }
    }
    const double w = p.inertia;
    if (w <= 0.001)
    {
      for (int i = 0; i < n; i++)
      {
        s.eff[i] = s.vf[i] + s.couple[i];
        s.mom[i] = 0;
      }
      s.fresh = 0;
    }
    else
    {
      if (s.fresh)
      {
        for (int i = 0; i < n; i++)
        {
          s.eff[i] = s.vf[i] + s.couple[i];
          s.mom[i] = 0;
        }
        s.fresh = 0;
      }
      const double w0 = kTau * (8 * (1 - w) + 0.6);
      const double S = w0 * w0, D = 0.9 * w0;  // zeta = 0.45
      for (int i = 0; i < n; i++)
      {
        const double target = s.vf[i] + s.couple[i];
        s.mom[i] += (target - s.eff[i]) * S * dt;
        s.mom[i] *= std::exp(-D * dt);
        s.eff[i] += s.mom[i] * dt;
      }
    }
    double fc = 18000;
    if (p.rtone > 0.001) fc = 16000 * std::pow(2, -6 * p.rtone * s.R);
    else if (p.rtone < -0.001) fc = 16000 * std::pow(2, -6 * (-p.rtone) * (1 - s.R));
    fc = std::max(120.0, std::min(18000.0, fc));
    s.lpc = 1 - std::exp(-kTau * fc / sr);
  }

  double sr;

 public:
  // Gravity readout (per render call): ratio index into kRatios, octave
  // fold, live cents error. Display feed only — never read by the DSP.
  int gravCount = 0;
  int gravPairs[32][2] = {{0}};
  double gravErr[32] = {0};

 private:
  double x[kMaxV] = {0}, panL[kMaxV] = {0}, panR[kMaxV] = {0};
  long noteCounter = 0;
  int tick = 0;
  int centerIdx = 0;
  uint32_t grng = 1;
  Swarm swarms[kPoly];
};

}  // namespace hypersaw
