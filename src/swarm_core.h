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

#include "force_core.h"

namespace hypersaw
{

constexpr int kMaxV = 32;
constexpr int kPoly = 16;  // raised from 8 (2026-07-18: user hit the ceiling
                           // at ~6-7 held notes). 16 voices x 32 osc = trivial
                           // CPU; well inside the ADR-006 spike headroom.
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
  // Two-cluster A/B balance (ADR-051): 0 = both clusters full K (bit-inert
  // default); >0 scales cluster B's intra-coupling by kB = 1-2*balance, so one
  // knob sweeps symmetric → B-uncoupled (0.5) → B-splayed (1.0, kB=-1).
  double balance = 0;
  // Voice-mode support (ADR-026): glide time in seconds for retargetNote().
  // Pure superset — the poly/default path never sets a glide target, so all
  // reference expressions stay bit-untouched.
  double glide = 0;
  // Live tune factor (ADR-027): one multiplicative pitch for octave/semi/
  // cents/bend, applied to the fundamental at law evaluation so it bends
  // SOUNDING notes. Default 1.0 is bit-inert (x * 1.0 == x in IEEE754).
  double tune = 1.0;
  // Absolute-K mode (ADR-004): bypass sigma-normalization (strict-chimera /
  // identical-oscillator experiments; L0-10 note). Guarded — default 0 is
  // bit-inert.
  double absK = 0;
  // Phase scatter (ADR-033): partial-random phase init. 0 = follow the
  // legacy retrig toggle EXACTLY (bit-inert, rng untouched); > 0 overrides:
  // phase_i = rng * scatter (so 1.0 reproduces the retrig-off stream).
  double scatter = 0;
  // Pan scatter (ADR-035): blend pan positions toward a seeded permutation.
  // Legacy pan order is monotone in the detune offset (pan = x[i]*width), so
  // spatial order == frequency order and sweeps march across the field in
  // series — the 2026-07-18 report. 0 = bit-inert legacy; the permutation
  // draws from its OWN stream, never the phase/drift stream.
  double panScatter = 0;
  // Waveshape morph (ADR-058): 0 = saw (bit-inert), 1 = band-limited square.
  double shape = 0;
  // Tone tilt (ADR-060, folded from swarmsaw.html): bipolar per-voice one-pole.
  // 0 = inert; >0 darkens (LP), <0 thins (HP). cutoff rises as sqrt(f/f0).
  double tilt = 0;
  // Hi-tame (ADR-061): equal-loudness per-voice roll-off, gain (f0/f)^hiTame.
  // 0 = inert; >0 turns the higher voices down so a tall stack isn't harsh.
  double hiTame = 0;
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
    int glideActive = 0;
    double glideTarget = 0;
    double lpL = 0, lpR = 0, lpc = 1;
    double vlp[kMaxV] = {0}, vlpc[kMaxV];  // per-voice tone-tilt state (vlpc init 1 in ctor)
    double hg[kMaxV];                      // per-voice hi-tame gain (init 1 in ctor)
    // Per-note expression tuning factor (ADR-036, MPE): 1.0 is bit-inert
    // (x * 1.0 == x in IEEE), same guarantee ADR-027 leans on for p.tune.
    double noteTune = 1.0;
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
      std::memset(s.vlp, 0, sizeof(s.vlp));
      for (int i = 0; i < kMaxV; i++) { s.vlpc[i] = 1.0; s.hg[i] = 1.0; }  // tilt/hi-tame passthrough
    }
    rebuild();
  }

  // Read back any param by key — same map as setParam, so the two can never
  // drift apart (the 2026-07-18 readParam bug class).
  double getParam(const std::string &k) const
  {
    const double *slot = const_cast<SwarmCore *>(this)->paramSlot(k);
    return slot ? *slot : 0.0;
  }

  // Mirrors the JS setParam(k, v) including its rebuild triggers.
  bool setParam(const std::string &k, double v)
  {
    double *slot = paramSlot(k);
    if (!slot) return false;
    *slot = v;
    if (k == "n" || k == "dist" || k == "seed" || k == "width" || k == "topo" ||
        k == "panScatter")
      rebuild();
    return true;
  }

  // Returns the swarm slot index so the shell can track host note identity
  // (CLAP NOTE_END bookkeeping). DSP behavior unchanged — parity-neutral.
  int noteOn(int midi, double f)
  {
    Swarm &s = alloc();
    const int slot = (int)(&s - &swarms[0]);
    initVoice(s, midi, f);
    return slot;
  }

  // Mono/legato retarget (ADR-026): reuse a sounding voice for a new note.
  // legatoKeepPhase: keep phases/envelope running (classic legato); false
  // re-strikes the voice in place (phases/rng/attack) but still glides.
  // With p.glide <= 0 the pitch change is immediate.
  void retargetNote(int slot, int midi, double f, bool legatoKeepPhase)
  {
    Swarm &s = swarms[slot];
    if (!legatoKeepPhase)
    {
      const double keepF0 = s.f0, keepF0cur = s.f0cur;
      initVoice(s, midi, f);
      if (p.glide > 0)
      {
        s.f0 = keepF0;      // strike from the CURRENT pitch, glide to the new
        s.f0cur = keepF0cur;
      }
    }
    else
    {
      s.midi = midi;
      s.gate = 1;
    }
    if (p.glide > 0)
    {
      s.glideTarget = f;
      s.glideActive = 1;
    }
    else
    {
      const double ratio = f / s.f0;
      s.f0 = f;
      s.f0cur *= ratio;  // preserve gravity offsets multiplicatively
      s.glideActive = 0;
    }
  }

  void initVoice(Swarm &s, int midi, double f)
  {
    s.midi = midi;
    s.f0 = f;
    s.f0cur = f;
    s.gate = 1;
    s.age = noteCounter++;
    // ADR-056: signed quadratic → bipolar onset lock. onset >= 0 reproduces
    // the reference's 8*onset^2 bit-exactly (fabs(onset)==onset there); onset < 0
    // makes Kenv negative — an initial SPLAY burst instead of a sync burst.
    s.Kenv = 8 * p.onset * std::fabs(p.onset);
    s.KsmS = 0;
    s.KsmP = 0;
    s.fresh = 1;
    s.inAttack = 1;
    s.lpL = 0;
    s.lpR = 0;
    std::memset(s.vlp, 0, sizeof(s.vlp));  // tone-tilt one-pole state
    s.noteTune = 1.0;  // per-note expression resets with a fresh strike;
                       // legato retargets keep the incoming bend (MPE streams
                       // continue across mono retargets)
    for (int i = 0; i < kMaxV; i++) s.mom[i] = 0;
    // (seed|0) + age*7919 + 1 under ToInt32 — uint32 wrap is bit-identical
    s.rngState = (uint32_t)((int64_t)toInt32(p.seed) + (int64_t)s.age * 7919 + 1);
    for (int i = 0; i < kMaxV; i++)
    {
      s.driftS[i] = 0;
      if (p.scatter > 0)
        s.phase[i] = rngNext(s.rngState) * p.scatter;  // ADR-033 partial scatter
      else
        s.phase[i] = (p.retrig != 0) ? 0.0 : rngNext(s.rngState);
    }
    s.glideActive = 0;
  }

  // MPE per-note tuning (ADR-036): relative semitones from the host's
  // note-expression stream; 0 restores the exactly-inert 1.0 factor.
  void setNoteExpr(int slot, double semis)
  {
    if (slot < 0 || slot >= kPoly) return;
    swarms[slot].noteTune = semis == 0.0 ? 1.0 : std::pow(2.0, semis / 12.0);
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
    const double atk = forcecore::onePoleCoef(p.attackS, sr);
    const double rel = forcecore::onePoleCoef(p.releaseS, sr);
    const double dec = forcecore::onePoleCoef(p.decayS, sr);
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
          // ADR-058 waveshape morph: v = saw − shape·saw(ph+½). shape 0 = saw
          // (bit-exact, guarded); shape 1 = a band-limited square (the two
          // half-cycle-offset saws' difference). Both saws carry the SAME
          // polyBLEP correction, so the morph stays anti-aliased. C++-only
          // superset — no swarmsaw.html reference for shape>0 (like ADR-025).
          if (p.shape > 0)
          {
            double ph2 = ph + 0.5;
            if (ph2 >= 1) ph2 -= 1;
            double saw2 = 2 * ph2 - 1;
            if (p.digital > 0)
            {
              const double d = std::max(dph, 1e-6);
              double bl2 = 0;
              if (ph2 < d) { const double t = ph2 / d; bl2 = t + t - t * t - 1; }
              else if (ph2 > 1 - d) { const double t = (ph2 - 1) / d; bl2 = t * t + t + t + 1; }
              saw2 = (2 * ph2 - 1) - p.digital * bl2;
            }
            v = v - p.shape * saw2;
          }
          if (s.vlpc[i] < 1) { s.vlp[i] += s.vlpc[i] * (v - s.vlp[i]); v = tiltHP ? (v - s.vlp[i]) : s.vlp[i]; }
          if (p.hiTame > 0) v *= s.hg[i];
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
    if (p.width > 1.0)
    {
      // Super-width (ADR-025): mid/side side-boost ahead of the soft clip.
      // Only reachable at width > 1 — the reference range is bit-untouched.
      const double sideGain = 1 + (p.width - 1) * 2;  // up to 2x at 1.5
      for (int smp = 0; smp < frames; smp++)
      {
        const double mid = ((double)outL[smp] + (double)outR[smp]) * 0.5;
        const double side = ((double)outL[smp] - (double)outR[smp]) * 0.5 * sideGain;
        outL[smp] = (float)(mid + side);
        outR[smp] = (float)(mid - side);
      }
    }
    for (int smp = 0; smp < frames; smp++)
    {
      outL[smp] = (float)std::tanh((double)outL[smp]);
      outR[smp] = (float)std::tanh((double)outR[smp]);
    }
  }

  Params p;

 private:
  static int32_t toInt32(double v) { return (int32_t)(int64_t)v; }

  // mulberry32 shared with the Track E force system (ADR-034 unification —
  // the one piece of arithmetic the two dynamics families genuinely share).
  // Parity 51/51 proves the delegation is bit-neutral.
  static double rngNext(uint32_t &state) { return forcecore::rngNext(state); }

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
    if (k == "balance") return &p.balance;
    if (k == "alpha") return &p.alpha;
    if (k == "poles") return &p.poles;
    if (k == "grav") return &p.grav;
    if (k == "basin") return &p.basin;
    if (k == "lpOut") return &p.lpOut;
    if (k == "glide") return &p.glide;
    if (k == "tune") return &p.tune;
    if (k == "absK") return &p.absK;
    if (k == "scatter") return &p.scatter;
    if (k == "panScatter") return &p.panScatter;
    if (k == "shape") return &p.shape;  // ADR-058 waveshape morph
    if (k == "tilt") return &p.tilt;    // ADR-060 tone tilt
    if (k == "hiTame") return &p.hiTame;  // ADR-061 hi-tame equal-loudness
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
    // Pan scatter (ADR-035): at 0 the arithmetic below is the legacy path
    // exactly (goldens are the proof). At > 0, blend each voice's pan
    // position toward a seeded permutation of the legacy positions — the
    // permutation stream is derived from the seed but INDEPENDENT of the
    // phase/drift stream, so engaging it never shifts any other draw.
    int perm[kMaxV];
    const double ps = p.panScatter;
    if (ps > 0)
    {
      for (int i = 0; i < n; i++) perm[i] = i;
      uint32_t prng = (uint32_t)(int64_t)p.seed * 2654435761u ^ 0x9E3779B9u;
      for (int i = n - 1; i > 0; i--)
      {
        const int j = (int)(rngNext(prng) * (i + 1));
        const int t = perm[i];
        perm[i] = perm[j];
        perm[j] = t;
      }
    }
    double pos[kMaxV];
    for (int i = 0; i < n; i++)
      pos[i] = std::max(-1.0, std::min(1.0, x[i])) * std::min(1.0, p.width);
    for (int i = 0; i < n; i++)
    {
      double pan = pos[i];
      if (ps > 0) pan += (pos[perm[i]] - pos[i]) * ps;
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
    if (s.glideActive)
    {
      // seconds -> per-tick coefficient (ADR-009 discipline)
      const double coef = 1 - std::exp(-dt / std::max(1e-3, p.glide));
      const double newF0 = s.f0 + (s.glideTarget - s.f0) * coef;
      const double ratio = newF0 / s.f0;
      s.f0 = newF0;
      s.f0cur *= ratio;
      if (std::fabs(s.f0 - s.glideTarget) < 0.01)
      {
        const double snap = s.glideTarget / s.f0;
        s.f0 = s.glideTarget;
        s.f0cur *= snap;
        s.glideActive = 0;
      }
    }
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
    // ADR-027/036; tune and noteTune at 1.0 -> bit-identical
    const double f0c = s.f0cur * p.tune * s.noteTune;
    for (int i = 0; i < n; i++)
    {
      double f;
      const double xv = x[i];
      if (p.law == 0) { f = f0c * std::pow(2, (xv * p.detune * 100) / 1200); }
      else if (p.law == 1) { f = f0c + xv * p.detune * 20; }
      else if (p.law == 3)
      {
        // tempo-grid (ADR-022): cents placement, then snap the Hz offset to
        // the nearest multiple of u — every pairwise beat rate becomes an
        // exact grid multiple. Expression ported verbatim from the DYNAMICS
        // reference. Drift (below) deliberately loosens the grid when used.
        const double u = (p.bpm / 60.0) * p.beatMult;
        const double df = f0c * (std::pow(2, (xv * p.detune * 100) / 1200) - 1);
        f = f0c + std::round(df / u) * u;
      }
      else { f = f0c + xv * p.detune * 0.35 * erb(f0c); }
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
    // per-voice tone tilt (ADR-060, parity with swarmsaw.html): pitch-tracked
    // one-pole. >0 darken (LP), <0 thin (HP); cutoff rises as sqrt(f/f0). 0 = inert.
    const double tm = std::fabs(p.tilt);
    tiltHP = p.tilt < 0;
    const double Ht = tm <= 0.005 ? 0 : (p.tilt > 0 ? 2 * std::pow(200.0, 1 - tm) : 0.1 * std::pow(24.0, tm));
    const double nyqt = sr * 0.5;
    for (int i = 0; i < n; i++)
    {
      if (Ht <= 0) s.vlpc[i] = 1;
      else { const double fc = std::min(nyqt * 0.98, Ht * s.f0 * std::sqrt(std::max(1.0, s.vf[i] / s.f0))); s.vlpc[i] = 1 - std::exp(-kTau * fc / sr); }
    }
    // hi-tame (ADR-061, parity with swarmsaw.html): equal-loudness roll-off,
    // gain (f0/f)^hiTame turns the higher voices down. hiTame 0 → inert.
    if (p.hiTame > 0) for (int i = 0; i < n; i++) s.hg[i] = std::pow(s.f0 / std::max(s.f0, s.vf[i]), p.hiTame);
    const double km = 4 * p.K * std::fabs(p.K);
    // absK (ADR-004/ADR-033): coupling in absolute units of 2.5 Hz (max
    // pull 4*K^2*2.5 = 10 Hz at knob 1) so identical-oscillator states are
    // reachable with real authority; default path untouched.
    const double sigmaU = (p.absK != 0) ? 2.5 : s.sigma;
    // ADR-056 bipolar onset: route the signed Kenv by sign — positive adds to
    // sync (unchanged), negative adds to splay (x3, matching the steady splay
    // gain). For onset >= 0 (Kenv >= 0) this is bit-identical to the reference:
    // max(0,Kenv)==Kenv adds to syncT, max(0,-Kenv)==0 leaves splayT untouched.
    const double syncT = (std::max(0.0, km) + std::max(0.0, s.Kenv)) * sigmaU;
    const double splayT = (std::max(0.0, -km) * 3 + std::max(0.0, -s.Kenv) * 3) * sigmaU;
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
      // A/B balance (ADR-051, DYN reference exact): cluster A keeps gain 1, B is
      // scaled by kB. balance 0 -> kB 1 -> s.couple = s.KsmS*c (bit-identical).
      const double kB = 1 - 2 * p.balance;
      for (int i = 0; i < n; i++)
      {
        const double ti = s.phase[i] * kTau;
        double c;
        double kGain;
        if (i < h) { c = (RA * std::sin(psiA - ti - alphaR) + m * RB * std::sin(psiB - ti - alphaR)) / norm; kGain = 1; }
        else { c = (RB * std::sin(psiB - ti - alphaR) + m * RA * std::sin(psiA - ti - alphaR)) / norm; kGain = kB; }
        s.couple[i] = s.KsmS * kGain * c;
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
  bool tiltHP = false;  // tone-tilt sign (ADR-060), set each control tick
  uint32_t grng = 1;
  Swarm swarms[kPoly];
};

}  // namespace hypersaw
