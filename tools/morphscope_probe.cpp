/*
 * morphscope_probe — WHERE does an edit land while the morph is on?
 *
 * The human (2026-09-03): "some global params act like corner params: they
 * get the corner colour, but if I edit them in one corner they change in all
 * relevant corners. Bend params in particular." No hypothesis survived code
 * reading (morphIds has no duplicates; the tables are clean), so this probe
 * MEASURES the edit-scope truth table instead: author four distinct corners,
 * make one edit under each (mode x arm/position) context, then read every
 * corner back by pinning the puck on it. A corner whose readback moved was
 * written by the edit. The table is the diagnosis.
 *
 * Params under test span the classes the report names:
 *   107 bendTime   — continuous, GLOBAL, in the field by ADR-104 A2
 *   106 bendLaw    — stepped,    GLOBAL, in the field by ADR-104 A2
 *     4 detune     — continuous, per-osc, in the field by construction
 * Diagnostic, not a gate: it prints, it does not judge.
 */
#include <algorithm>   // notefuzz_scaffold.inc uses std::stable_sort and expects the
                       // includer to provide it — MSVC's <vector> does not (build-windows red)
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <clap/clap.h>
#include "../src/hypersaw_clap_entry.h"

namespace
{
#include "notefuzz_scaffold.inc"

struct Probe
{
  const clap_plugin_t *p = nullptr;
  std::vector<float> L, R;
  clap_audio_buffer_t out{};
  clap_process_t proc{};
  float *ch[2];
  void boot()
  {
    auto *f = (const clap_plugin_factory_t *)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
    p = f->create_plugin(f, &kHost, "com.lifted-truck.hypersaw");
    p->init(p); p->activate(p, kSR, 32, kBlock); p->start_processing(p);
    L.assign(kBlock, 0); R.assign(kBlock, 0);
    ch[0] = L.data(); ch[1] = R.data();
    out.data32 = ch; out.channel_count = 2;
    proc.frames_count = kBlock; proc.audio_outputs = &out;
    proc.audio_outputs_count = 1; proc.out_events = &kOut;
  }
  void step(EvList &e) { e.finalize(); proc.in_events = &e.list; p->process(p, &proc); }
  void set(const std::vector<std::pair<clap_id, double>> &kv)
  { EvList e; for (auto &x : kv) e.params.push_back(mkParam(x.first, x.second)); step(e); }
  void pump(double s) { for (int i = 0; i < (int)(s * kSR) / kBlock; i++) { EvList q; step(q); } }
  double read(clap_id id)
  {
    auto *ext = (const clap_plugin_params_t *)p->get_extension(p, CLAP_EXT_PARAMS);
    double v = -1; if (ext) ext->get_value(p, id, &v); return v;
  }
  void kill() { p->stop_processing(p); p->deactivate(p); p->destroy(p); }
};

struct Case { clap_id id; const char *name; double corner[4]; double edit; double depLaw; };
// depLaw: bendLaw (106) authored into EVERY corner first, or -1 to leave it at
// the default 0 (off) — which is the ADR-108 hold condition for bendTime.
const Case kCases[] = {
  {107, "bendTime, bendLaw=0 all corners (HELD)", {100, 200, 300, 400}, 900, -1},
  {107, "bendTime, bendLaw=1 all corners (its law)", {100, 200, 300, 400}, 900, 1},
  {108, "bendRate, bendLaw=2 all corners (its law)", {10, 20, 30, 40}, 90, 2},
  {108, "bendRate, bendLaw=1 all corners (NOT its law)", {10, 20, 30, 40}, 90, 1},
  {106, "bendLaw  (stepped, global-in-field)", {0, 1, 2, 3}, 4, -1},
  {4,   "detune   (cont, per-osc)", {0.1, 0.2, 0.3, 0.4}, 0.9, -1},
};
const double kCornerXY[4][2] = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};

// Read corner k's stored value the only way a host can: pin the puck on it
// unarmed and let the field land (glide 0.005 s, one second of grid ticks).
double cornerRead(Probe &pr, int k, clap_id id)
{
  pr.set({{159, 0}, {152, kCornerXY[k][0]}, {153, kCornerXY[k][1]}});
  pr.pump(1.0);
  return pr.read(id);
}

void runCase(const Case &c, int mode, const char *ctxName, int arm, double px, double py)
{
  Probe pr; pr.boot();
  // morph ON first (morphRouteEdit is a no-op while off), temp 1, glide short
  pr.set({{151, 1}, {154, 1.0}, {157, (double)mode}, {158, 0.005}});
  for (int k = 0; k < 4; k++)
  {
    pr.set({{159, (double)(k + 1)}});
    if (c.depLaw >= 0) pr.set({{106, c.depLaw}});
    pr.set({{c.id, c.corner[k]}});
  }
  pr.set({{159, 0}});
  // the edit context
  pr.set({{152, px}, {153, py}}); pr.pump(1.0);
  pr.set({{159, (double)arm}});
  pr.set({{c.id, c.edit}});
  pr.pump(0.2);
  // readback
  std::printf("  %-8s %-30s arm=%d @(%.1f,%.1f):", mode ? "BLEND" : "QUANTUM", ctxName, arm, px, py);
  int moved = 0;
  for (int k = 0; k < 4; k++)
  {
    const double v = cornerRead(pr, k, c.id);
    const bool ch = std::fabs(v - c.corner[k]) > 1e-6;
    moved += ch;
    std::printf("  %c=%.3g%s", 'A' + k, v, ch ? "*" : " ");
  }
  std::printf("   -> %d corner(s) written\n", moved);
  pr.kill();
}
}  // namespace

int main()
{
  std::printf("morphscope_probe — edit scope truth table (* = corner changed by the edit)\n");
  for (const Case &c : kCases)
  {
    std::printf("\n== %u %s  authored A/B/C/D = %g/%g/%g/%g, edit -> %g ==\n",
                (unsigned)c.id, c.name, c.corner[0], c.corner[1], c.corner[2], c.corner[3], c.edit);
    for (int mode = 0; mode < 2; mode++)
    {
      runCase(c, mode, "armed 1, puck mid",   1, 0.5, 0.5);
      runCase(c, mode, "unarmed AT corner A", 0, 0.0, 0.0);
      runCase(c, mode, "unarmed NEAR A",      0, 0.1, 0.1);
      runCase(c, mode, "unarmed mid-field",   0, 0.5, 0.5);
    }
  }
  hypersaw_entry_deinit();
  return 0;
}
