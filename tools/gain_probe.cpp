/*
 * gain_probe — SCRATCH DIAGNOSTIC, not a gate.
 *
 * Trigger: human 2026-08-25, "I feel like the synth is in general a little too
 * quiet on many settings." Renders a held note through the REAL plugin (same
 * static-lib link as notefuzz_check, so what is measured is the shipped signal
 * path and not a model of it) and reports peak / RMS dBFS across the axes that
 * touch the summing gain in swarm_core.h:796,
 *
 *     gain = vol * 0.9 / n^normExp
 *
 * The hypothesis under test is that normExp = 0.75 sits BETWEEN the coherent
 * (n^1) and incoherent (n^0.5) summing cases, so it cannot be right at both
 * ends of the K range -- and the shipped default is K = 0, the splayed end.
 * MUST-READ-ZERO CONTROL included: a vol=0 render must come back at -inf, or
 * the meter is measuring something other than the instrument.
 */
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <clap/clap.h>

#include "../src/hypersaw_clap_entry.h"

namespace
{
#include "notefuzz_scaffold.inc"

struct Meter { double peak, rms; };

double db(double x) { return x <= 1e-12 ? -240.0 : 20.0 * std::log10(x); }

/* One held note at A3, measured over the second of the two seconds so the
   attack and the swarm's own settling are behind us. */
Meter render(const std::vector<std::pair<clap_id, double>> &set)
{
  auto *factory = (const clap_plugin_factory_t *)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  const clap_plugin_t *p = factory->create_plugin(factory, &kHost, "com.lifted-truck.hypersaw");
  p->init(p);
  p->activate(p, kSR, 32, kBlock);
  p->start_processing(p);

  std::vector<float> L(kBlock), R(kBlock);
  float *ch[2] = {L.data(), R.data()};
  clap_audio_buffer_t out{};
  out.data32 = ch;
  out.channel_count = 2;
  clap_process_t proc{};
  proc.frames_count = kBlock;
  proc.audio_outputs = &out;
  proc.audio_outputs_count = 1;
  proc.out_events = &kOut;

  auto step = [&](EvList &e) {
    e.finalize();
    proc.in_events = &e.list;
    p->process(p, &proc);
  };

  { EvList e; for (auto &kv : set) e.params.push_back(mkParam(kv.first, kv.second)); step(e); }
  { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON, 0, 57, 1)); step(e); }

  for (int b = 0; b < Math_blocks(1.5); b++) { EvList e; step(e); }

  double peak = 0, energy = 0; long nsamp = 0;
  for (int b = 0; b < Math_blocks(1.0); b++)
  {
    EvList e; step(e);
    for (int i = 0; i < kBlock; i++)
      for (float v : {L[i], R[i]})
      { double a = std::fabs(v); if (a > peak) peak = a; energy += (double)v * v; nsamp++; }
  }

  p->stop_processing(p); p->deactivate(p); p->destroy(p);
  return {peak, nsamp ? std::sqrt(energy / nsamp) : 0.0};
}

/* Four held notes. Per-voice normalisation is inside the core; the shell sums
   voices without further scaling, so a chord should be the one place level is
   recovered -- worth confirming rather than assuming. */
Meter renderChord(const std::vector<std::pair<clap_id, double>> &set)
{
  auto *factory = (const clap_plugin_factory_t *)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  const clap_plugin_t *p = factory->create_plugin(factory, &kHost, "com.lifted-truck.hypersaw");
  p->init(p); p->activate(p, kSR, 32, kBlock); p->start_processing(p);
  std::vector<float> L(kBlock), R(kBlock);
  float *ch[2] = {L.data(), R.data()};
  clap_audio_buffer_t out{}; out.data32 = ch; out.channel_count = 2;
  clap_process_t proc{}; proc.frames_count = kBlock; proc.audio_outputs = &out;
  proc.audio_outputs_count = 1; proc.out_events = &kOut;
  auto step = [&](EvList &e) { e.finalize(); proc.in_events = &e.list; p->process(p, &proc); };
  { EvList e; for (auto &kv : set) e.params.push_back(mkParam(kv.first, kv.second)); step(e); }
  { EvList e;
    int k = 0;
    for (int key : {45, 52, 57, 61}) e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON, 0, (int16_t)key, ++k));
    step(e); }
  for (int b = 0; b < Math_blocks(1.5); b++) { EvList e; step(e); }
  double peak = 0, energy = 0; long nsamp = 0;
  for (int b = 0; b < Math_blocks(1.0); b++)
  { EvList e; step(e);
    for (int i = 0; i < kBlock; i++)
      for (float v : {L[i], R[i]})
      { double a = std::fabs(v); if (a > peak) peak = a; energy += (double)v * v; nsamp++; } }
  p->stop_processing(p); p->deactivate(p); p->destroy(p);
  return {peak, nsamp ? std::sqrt(energy / nsamp) : 0.0};
}

bool header = false;
void row(const char *label, const std::vector<std::pair<clap_id, double>> &set)
{
  if (!header) { std::printf("  %-24s %8s %8s\n", "case", "peak dB", "rms dB"); header = true; }
  Meter m = render(set);
  std::printf("  %-24s %8.2f %8.2f\n", label, db(m.peak), db(m.rms));
}
void chord(const char *label, const std::vector<std::pair<clap_id, double>> &set)
{
  Meter m = renderChord(set);
  std::printf("  %-24s %8.2f %8.2f\n", label, db(m.peak), db(m.rms));
}
}  // namespace

int main()
{
  hypersaw_entry_init("");

  std::printf("\n== CONTROL (must read silence) ==\n");
  row("vol=0", {{17, 0.0}});

  std::printf("\n== SHIPPED DEFAULT PATCH ==\n");
  row("as shipped", {});

  std::printf("\n== VOICES at K=0 (shipped: splayed = incoherent sum) ==\n");
  for (double n : {1.0, 2.0, 4.0, 7.0, 12.0, 16.0, 24.0, 32.0})
  { char b[64]; std::snprintf(b, sizeof b, "n=%2.0f  K=0", n); row(b, {{1, n}, {6, 0.0}}); }

  std::printf("\n== VOICES at K=1 (locked = coherent sum) ==\n");
  for (double n : {1.0, 2.0, 4.0, 7.0, 12.0, 16.0, 24.0, 32.0})
  { char b[64]; std::snprintf(b, sizeof b, "n=%2.0f  K=1", n); row(b, {{1, n}, {6, 1.0}}); }

  std::printf("\n== K SWEEP at shipped voices (n=7) ==\n");
  for (double k : {0.0, 0.15, 0.3, 0.5, 0.75, 1.0})
  { char b[64]; std::snprintf(b, sizeof b, "K=%.2f", k); row(b, {{6, k}}); }

  std::printf("\n== normExp SWEEP at n=16, K=0 (0.5 = incoherent-correct) ==\n");
  for (double e : {0.5, 0.6, 0.75, 0.9, 1.0})
  { char b[64]; std::snprintf(b, sizeof b, "normExp=%.2f", e); row(b, {{1, 16.0}, {6, 0.0}, {13, e}}); }

  /* Does the shell's own stages recover any of it? Two oscillators and a
     four-note chord are what a player actually does when a patch is quiet. */
  /* THE LOAD-BEARING CLAIM. If 0.5 is genuinely the incoherent-correct
     exponent, level at K=0 must be FLAT across n at normExp=0.5 and must sag
     at 0.75. If instead 0.5 also sags, the diagnosis is wrong and the deficit
     is somewhere other than the exponent. */
  std::printf("\n== IS 0.5 THE INCOHERENT-CORRECT EXPONENT? (K=0) ==\n");
  for (double n : {1.0, 4.0, 7.0, 16.0, 32.0})
  { char b[64]; std::snprintf(b, sizeof b, "n=%2.0f normExp=0.50", n); row(b, {{1, n}, {6, 0.0}, {13, 0.5}}); }
  std::printf("  -- and the coherent end, where 1.0 should be the flat one --\n");
  for (double n : {1.0, 4.0, 7.0, 16.0, 32.0})
  { char b[64]; std::snprintf(b, sizeof b, "n=%2.0f normExp=1.00 K=1", n); row(b, {{1, n}, {6, 1.0}, {13, 1.0}}); }

  std::printf("\n== WHAT THE PLAYER DOES WHEN IT IS QUIET ==\n");
  row("1 osc (shipped)", {});
  row("2 osc, both at vol=.4", {{1150, 1.0}});
  row("2 osc, both at vol=1", {{1150, 1.0}, {17, 1.0}, {1017, 1.0}});
  chord("4-note chord, default", {});
  chord("4-note chord, vol=1", {{17, 1.0}});

  std::printf("\n== HEADROOM: everything the player can turn up ==\n");
  row("vol=1", {{17, 1.0}});
  row("vol=1 master=1.5", {{17, 1.0}, {100, 1.5}});
  row("vol=1 master=1.5 K=1", {{17, 1.0}, {100, 1.5}, {6, 1.0}});
  row("vol=1 mst=1.5 n=32 K=1", {{17, 1.0}, {100, 1.5}, {1, 32.0}, {6, 1.0}});

  hypersaw_entry_deinit();
  return 0;
}
