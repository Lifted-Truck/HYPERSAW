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
#include <initializer_list>
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

/* TIE-LANDING GESTURE — must mirror `tieTargetAt` in gen_glide_goldens.mjs.
   The standard gesture settles at 0.5, which is equidistant from nothing, so the
   tie path was entirely uncovered and two defects lived there unseen. */
static double tieTargetAt(long i)
{
  const double t = i / kCR;
  if (t < 0.10) return 0;
  if (t < 0.60) return 1;
  if (t < 1.10) return -1.5;
  if (t < 1.60) return 2.5;
  return 1;
}

struct Scenario { const char *name; GlideCore::Params p; bool noteLane; bool tieGesture; double base = 0; };

// Params::scaleMask is a C array, so it cannot be brace-assigned after
// construction; this keeps the scenario table reading like the JS one.
static void setMask(GlideCore::Params &p, std::initializer_list<int> m)
{
  int i = 0;
  for (int v : m) p.scaleMask[i++] = v;
}

int main(int argc, char **argv)
{
  const std::string dir = argc > 1 ? argv[1] : "build-golden/glide";
  GlideCore::Params base;   // constructor defaults mirror the reference literal

  std::vector<Scenario> scen;
  auto add = [&](const char *n, GlideCore::Params p, bool note = false) {
    scen.push_back({n, p, note, false});
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
  // Non-default scale masks (2026-08-09): the lab's scale picker makes the
  // whole 12-bit mask space reachable, so parity must cover a non-zero root,
  // a wide-gap set, and a sparse rooted set — not just the C major default.
  { auto p = base; p.model = GlideCore::kLag; p.quant = GlideCore::kQuantScale;
    p.scaleRoot = 3; setMask(p, {1,0,0,1,0,1,0,1,0,0,1,0}); add("glide-quant-root3", p); }
  { auto p = base; p.model = GlideCore::kConstRate; p.quant = GlideCore::kQuantScale;
    setMask(p, {1,0,1,0,1,0,1,0,1,0,1,0}); add("glide-quant-whole", p); }
  { auto p = base; p.model = GlideCore::kSpring; p.quant = GlideCore::kQuantScale;
    p.scaleRoot = 7; p.qhyst = 20; setMask(p, {1,0,1,1,0,0,0,1,1,0,0,0});
    add("glide-quant-sparse", p); }
  { auto p = base; p.model = GlideCore::kSpring; add("glide-note-lane", p, true); }
  /* TIME GATE — the mirror of the generator's three. This table duplicates the
     generator's by hand (recorded, not fixed), so a scenario added there and not
     here is a golden nothing reads. */
  { auto p = base; p.model = GlideCore::kConstRate; p.quant = GlideCore::kQuantChromatic;
    p.qTime = 400; p.rate = 12; add("glide-qtime-chrom", p); }
  { auto p = base; p.model = GlideCore::kConstRate; p.quant = GlideCore::kQuantScale;
    p.qTime = 200; p.rate = 9; add("glide-qtime-scale", p); }
  { auto p = base; p.model = GlideCore::kSpring; p.quant = GlideCore::kQuantChromatic;
    p.qTime = 90; p.damp = 0.25; add("glide-qtime-spring", p); }
  /* BASE-ANCHORED QUANTISE — the mirror of the generator's two. F#3 (54) is
     maximally off-grid for the default major mask; a port ignoring base snaps
     to the wrong absolute pitches everywhere, so these CANNOT pass by accident. */
  { auto p = base; p.model = GlideCore::kLag; p.quant = GlideCore::kQuantScale;
    p.tau = 80; scen.push_back({"glide-base-scale", p, false, false, 54.0}); }
  { auto p = base; p.model = GlideCore::kConstRate; p.quant = GlideCore::kQuantChromatic;
    p.rate = 18; scen.push_back({"glide-base-chrom", p, false, false, 54.5}); }
  /* TIES. model = kOff is load-bearing: under any moving law the output
     approaches asymptotically and never lands exactly on a midpoint, so the tie
     path is unreachable and the scenario cannot fail. With the law off
     `x = target` bit-exactly — which is also the only way a real patch reaches a
     tie. A first version used kConstTime and a planted regression sailed through. */
  { auto p = base; p.model = GlideCore::kOff; p.quant = GlideCore::kQuantScale;
    p.qhyst = 0; scen.push_back({"glide-tie-scale", p, false, true}); }
  { auto p = base; p.model = GlideCore::kOff; p.quant = GlideCore::kQuantChromatic;
    p.qhyst = 0; scen.push_back({"glide-tie-chrom", p, false, true}); }

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
      const double got = g.step(s.tieGesture ? tieTargetAt((long)i) : targetAt((long)i), s.p, s.base);
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
