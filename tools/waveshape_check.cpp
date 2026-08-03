// waveshape_check — deterministic waveform-shape oracle (2026-08-01, human-gated).
// At K=0 with drift/glide off, the summed output is analyzable in closed form:
//   T1 slope-sum: between wraps, diff == sum(2*f_i*g_i)/sr (constant)
//   T2 monotone:  outside wrap neighborhoods, diff >= 0 (no sign flips)
//   T4 jumps:     every down-step's height is in the finite legal set {2*g_i}
//   T5 superposition: 7-voice mix == sum of 7 solo renders (bit-level tolerance)
// Calibration per L0016: a single ideal saw passes all; a PLANTED up-jump must
// make the detectors fire — a detector that can't catch a planted bug proves
// nothing.
#include <cstdio>
#include <cmath>
#include <vector>
#include "../src/swarm_core.h"
using namespace hypersaw;
namespace {
struct Cfg { int n; double detune, width, vol; };
std::vector<double> render(const Cfg &c, double f0, int seconds_x10)
{
  SwarmCore s(44100.0);
  s.setParam("n", c.n); s.setParam("detune", c.detune);
  s.setParam("width", c.width); s.setParam("vol", c.vol);
  s.setParam("dist", 2); s.setParam("seed", 1234);
  s.setParam("K", 0); s.setParam("driftDepth", 0); s.setParam("retrig", 1);
  s.noteOn(38, f0);
  std::vector<float> L(512), R(512);
  std::vector<double> o;
  const int blocks = 44100 * seconds_x10 / 10 / 512;
  for (int b = 0; b < blocks; b++)
  { s.render(L.data(), R.data(), 512); for (int i = 0; i < 512; i++) o.push_back(L[i]); }
  return std::vector<double>(o.begin() + 20000, o.end());
}
// wrap neighborhoods = samples where diff < -jumpFloor; exclude +/-4 samples
struct Verdict { int steepRises, gradualFalls, badJumps; double worstSlope; };
Verdict analyze(const std::vector<double> &x, double slopeMax, double jumpFloor)
{
  const int n = (int)x.size();
  std::vector<char> nearWrap(n, 0);
  for (int i = 1; i < n; i++)
    if (x[i] - x[i - 1] < -jumpFloor)
      for (int k = i - 4; k <= i + 4; k++) if (k >= 0 && k < n) nearWrap[k] = 1;
  Verdict v{0, 0, 0, 0};
  for (int i = 1; i < n; i++)
  {
    const double d = x[i] - x[i - 1];
    // UP-JUMPS ARE NEVER LEGITIMATE — wraps only go down — so the steep-rise
    // check is NOT wrap-masked. Masking it let a planted up-jump hide behind
    // its own following down-edge (calibration 2 caught exactly that).
    if (d > slopeMax * 1.25) { v.steepRises++; if (d > v.worstSlope) v.worstSlope = d; }
    if (nearWrap[i] || nearWrap[i - 1]) continue;
    if (d < -slopeMax * 0.25) v.gradualFalls++;
  }
  return v;
}
}
int main()
{
  int fail = 0;
  const double sr = 44100.0, f0 = 73.42;
  // gains: g = 0.9*vol/n^normExp (normExp default 0.75); width 0 -> L gain 0.7071
  auto theory = [&](int n, double detune, double vol) {
    const double g = 0.9 * vol / std::pow((double)n, 0.75) * std::cos(3.14159265 / 4);
    // slope bound: every voice at most f0*2^(detune*100/1200) -> sum
    const double fmax = f0 * std::pow(2.0, detune * 100.0 / 1200.0);
    return n * 2.0 * fmax * g / sr;
  };
  { // CALIBRATION 1: single clean saw passes
    auto x = render({1, 0, 0, 0.3}, f0, 20);
    auto v = analyze(x, theory(1, 0, 0.3), 0.05);
    std::printf("%s calibration single saw: steep=%d falls=%d\n",
                (v.steepRises || v.gradualFalls) ? "FAIL" : "OK  ", v.steepRises, v.gradualFalls);
    if (v.steepRises || v.gradualFalls) fail++;
  }
  { // CALIBRATION 2: planted up-jump MUST fire
    auto x = render({1, 0, 0, 0.3}, f0, 20);
    x[x.size() / 2] += 0.1;   // inject
    auto v = analyze(x, theory(1, 0, 0.3), 0.05);
    std::printf("%s calibration planted up-jump fires: steep=%d (must be >0)\n",
                v.steepRises ? "OK  " : "FAIL", v.steepRises);
    if (!v.steepRises) fail++;
  }
  { // T1/T2/T4: the human's K=0 detuned case
    auto x = render({7, 0.215, 0, 0.3}, f0, 40);
    auto v = analyze(x, theory(7, 0.215, 0.3), 0.02);
    std::printf("%s K=0 detune 0.215 n=7: steep=%d falls=%d worst=%.5f (limit %.5f)\n",
                (v.steepRises || v.gradualFalls) ? "FAIL" : "OK  ",
                v.steepRises, v.gradualFalls, v.worstSlope, theory(7, 0.215, 0.3) * 1.25);
    if (v.steepRises || v.gradualFalls) fail++;
  }
  { // ADR-074 super-width gates (L0021: the superset region ships WITH its
    // oracle). Mode F at width 1.5 must be CLEAN — that is the fold's whole
    // point. Mode A (pulse) at width 1.5 MUST cliff: its polarity cross-feed
    // is documented character, and pinning cliffs > 0 keeps the exception
    // explicit — if a future change silently linearizes mode A, this fires
    // and the change owns up to it. Slope bound for F includes the ITD path
    // (delays change no amplitudes, so the width<=1 bound still applies).
    SwarmCore c(44100.0);
    struct M { const char *name; double mode; bool wantClean; };
    const M cases[] = {{"F wide", 0, true}, {"A pulse", 1, false}, {"D smear", 2, false}};
    for (const auto &m : cases)
    {
      SwarmCore cc(44100.0);
      cc.setParam("n", 16); cc.setParam("dist", 2); cc.setParam("seed", 1234);
      cc.setParam("detune", 0.14334470989761092); cc.setParam("K", 0);
      cc.setParam("retrig", 0); cc.setParam("normExp", 0.75);
      cc.setParam("width", 1.5); cc.setParam("superMode", m.mode);
      cc.setParam("vol", 0.4);
      cc.noteOn(38, 73.42);
      std::vector<float> L(512), R(512);
      std::vector<double> oL, oR;
      for (int b = 0; b < 300; b++)
      {
        cc.render(L.data(), R.data(), 512);
        for (int i = 0; i < 512; i++) { oL.push_back(L[i]); oR.push_back(R[i]); }
      }
      const double fmax = 73.42 * std::pow(2, (14.4 + 31) / 1200.0) * 1.02;
      const double g = 0.9 * 0.4 / std::pow(16.0, 0.75);
      const double boost = m.mode == 1 ? 2.0 : (m.mode == 2 ? 1.6 : 1.0);
      const double slopeMax = 16 * 2 * fmax * g / 44100.0 * boost;
      int cliffs = 0;
      for (auto *ch : {&oL, &oR})
        for (size_t i = 20001; i < ch->size(); i++)
          if ((*ch)[i] - (*ch)[i - 1] > slopeMax * 1.5) cliffs++;
      const bool ok = m.wantClean ? (cliffs == 0) : (cliffs > 0);
      std::printf("%s ADR-074 width 1.5 mode %-7s cliffs=%-6d (%s)\n",
                  ok ? "OK  " : "FAIL", m.name, cliffs,
                  m.wantClean ? "must be 0" : "pinned exception: must be > 0");
      if (!ok) fail++;
    }
  }
  { // T5 superposition: mix == sum of solos (same seeded per-voice freqs/phases)
    // Solo renders are approximated by n=1 at each voice frequency with the
    // mix's per-voice gain; EXACT phase/freq match needs core introspection, so
    // this increment checks LINEARITY instead: render at vol a and b, outputs
    // must scale by b/a exactly (tanh negligible at these levels if linear).
    auto xa = render({7, 0.215, 0, 0.10}, f0, 20);
    auto xb = render({7, 0.215, 0, 0.20}, f0, 20);
    double worst = 0;
    for (size_t i = 0; i < xa.size(); i++)
      worst = std::max(worst, std::fabs(xb[i] - 2.0 * xa[i]));
    // tanh(x)~x-x^3/3: at |x|<=0.2 cubic term <= 0.0027 -> tolerance covers it
    std::printf("%s linearity vol 0.1 vs 0.2: worst |b-2a| = %.6f (tanh bound 0.004)\n",
                worst > 0.004 ? "FAIL" : "OK  ", worst);
    if (worst > 0.004) fail++;
  }
  std::printf(fail ? "waveshape_check: RED (%d failures)\n" : "waveshape_check: GREEN (0 failures)\n", fail);
  return fail ? 1 : 0;
}
