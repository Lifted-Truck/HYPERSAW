/*
 * glide_check — trajectory parity for the travel laws (A1 fold): C++ GlideCore
 * vs the JS reference sliced live from bend-lab.html, plus the behavioural
 * anchors from the measured roundup.
 *
 * Parity bar is the project's L0-1 standard: RMS(diff) <= 1e-6. Correctness is
 * equivalence with the reference, never "feels like a glide".
 *
 * Usage: glide_check <golden-dir>
 */
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "../src/glide_core.h"

using hypersaw::GlideCore;

static int failures = 0;
static void check(bool ok, const char *what, const char *detail = "")
{
  std::printf("%s   %s%s%s\n", ok ? "OK  " : "FAIL", what, *detail ? "  " : "", detail);
  if (!ok) failures++;
}

static std::vector<float> readF32(const std::string &path, bool &ok)
{
  std::vector<float> v;
  FILE *f = std::fopen(path.c_str(), "rb");
  if (!f) { ok = false; return v; }
  std::fseek(f, 0, SEEK_END);
  const long n = std::ftell(f);
  std::fseek(f, 0, SEEK_SET);
  v.resize((size_t)(n / 4));
  ok = std::fread(v.data(), 4, v.size(), f) == v.size();
  std::fclose(f);
  return v;
}

static const double kCR = 44100.0 / 16.0;

static double targetAt(long i)
{
  const double t = i / kCR;
  if (t < 0.05) return 0;
  if (t < 0.65) return 2;
  if (t < 1.10) return 0;
  if (t < 1.70) return -1.5;
  return 0.5;
}

struct Scenario { const char *name; GlideCore::Params p; bool noteLane; };

int main(int argc, char **argv)
{
  const std::string dir = argc > 1 ? argv[1] : "build-golden/glide";
  GlideCore::Params base;   // constructor defaults mirror the reference literal

  std::vector<Scenario> scen;
  auto add = [&](const char *n, GlideCore::Params p, bool note = false) {
    scen.push_back({n, p, note});
  };
  { auto p = base; p.model = GlideCore::kConstTime;  add("glide-const-time", p); }
  { auto p = base; p.model = GlideCore::kConstRate;  add("glide-const-rate", p); }
  { auto p = base; p.model = GlideCore::kLag;        add("glide-lag", p); }
  { auto p = base; p.model = GlideCore::kSpring;     add("glide-spring", p); }
  { auto p = base; p.model = GlideCore::kSpring; p.damp = 0.2;     add("glide-spring-ring", p); }
  { auto p = base; p.model = GlideCore::kSpring; p.distOver = 2;   add("glide-spring-dist2", p); }
  { auto p = base; p.model = GlideCore::kConstTime; p.retMul = 0.35; add("glide-retmul", p); }
  { auto p = base; p.model = GlideCore::kConstRate; p.quant = GlideCore::kQuantChromatic;
    add("glide-quant-chrom", p); }
  { auto p = base; p.model = GlideCore::kLag; p.quant = GlideCore::kQuantScale;
    add("glide-quant-scale", p); }
  { auto p = base; p.model = GlideCore::kSpring; p.quant = GlideCore::kQuantChromatic;
    p.qhyst = 25; add("glide-quant-hyst", p); }
  { auto p = base; p.model = GlideCore::kSpring; add("glide-note-lane", p, true); }

  double worst = 0;
  for (const auto &s : scen)
  {
    bool ok = true;
    const std::vector<float> ref = readF32(dir + "/" + s.name + ".f32", ok);
    if (!ok || ref.empty())
    {
      check(false, s.name, "golden missing — run gen_glide_goldens.mjs");
      continue;
    }
    GlideCore g(kCR, !s.noteLane);
    g.reset(0);
    double acc = 0;
    for (size_t i = 0; i < ref.size(); i++)
    {
      const double got = g.step(targetAt((long)i), s.p);
      const double d = got - (double)ref[i];
      acc += d * d;
    }
    const double rms = std::sqrt(acc / (double)ref.size());
    if (rms > worst) worst = rms;
    char buf[96];
    std::snprintf(buf, sizeof(buf), "(rms=%g)", rms);
    check(rms <= 1e-6, s.name, buf);
  }

  // --- behavioural anchors, from the measured roundup ----------------------
  // These are not parity: they pin the CHARACTER each law was chosen for, so a
  // future refactor that keeps parity to a stale golden still trips.
  {
    // the spring must overshoot; the rate-limited laws must not
    auto peak = [](GlideCore::Params p) {
      GlideCore g(kCR, true); g.reset(0);
      double pk = 0;
      for (long i = 0; i < (long)(0.65 * kCR); i++)
      { const double v = g.step(targetAt(i), p); if (std::fabs(v) > std::fabs(pk)) pk = v; }
      return pk;
    };
    auto p4 = base; p4.model = GlideCore::kSpring;
    auto p2 = base; p2.model = GlideCore::kConstRate;
    const double over4 = (std::fabs(peak(p4)) - 2.0) * 100.0;
    const double over2 = (std::fabs(peak(p2)) - 2.0) * 100.0;
    char b[96];
    std::snprintf(b, sizeof(b), "(spring %+.1fc, const-rate %+.1fc)", over4, over2);
    check(over4 > 5.0 && over2 < 0.5, "spring overshoots, constant rate does not", b);
  }
  {
    // hysteresis must reduce boundary chatter (measured 15 -> 3 at zeta 0.5)
    auto flips = [](double hyst) {
      GlideCore::Params p;
      p.model = GlideCore::kSpring; p.damp = 0.5;
      p.quant = GlideCore::kQuantChromatic; p.qhyst = hyst;
      GlideCore g(kCR, true); g.reset(0);
      for (long i = 0; i < (long)(2.0 * kCR); i++) g.step(i > 100 ? 1.5 : 0.0, p);
      return g.flips();
    };
    const int f0 = flips(0), f8 = flips(8);
    char b[96];
    std::snprintf(b, sizeof(b), "(0c: %d flips, 8c: %d)", f0, f8);
    check(f8 < f0, "hysteresis reduces boundary chatter", b);
  }

  std::printf("glide_check: %s (%d failure%s; worst parity rms %g)\n",
              failures ? "RED" : "GREEN", failures, failures == 1 ? "" : "s", worst);
  return failures ? 1 : 0;
}
