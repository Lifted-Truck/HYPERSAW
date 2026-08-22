// SCRATCH tool for the B18(b) min-spec CPU measurement (docs/research/
// 2026-08-22-two-osc-cpu-measurement.md). Not wired into CMakeLists.txt —
// compiled by hand with the same flags shell_bench uses (see the report for
// the exact command). Deliberately NOT a permanent oracle.
//
// WHY THIS EXISTS RATHER THAN REUSING user_patch_bench AS-IS: that bench sets
// osc2's volume (id 1017) but never its `enable` (id 1150), and ADR-100
// shipped osc2 OFF by default (oscEnabled[] = {1, 0} in hypersaw_clap.cpp).
// So under the CURRENT default, user_patch_bench's "both oscillators
// audible" comment is stale — osc2 is silent AND skipped, and its numbers
// are actually a single-oscillator measurement. This tool sets id 1150
// explicitly so "2 osc active" means what it says, and reports the
// marginal cost against 1-osc and both-off baselines at the same voice
// counts, matching cpu_bench/shell_bench conventions (44.1 kHz, 128-sample
// block, % of one core against the E-6 50% budget).
#include <chrono>
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include <clap/clap.h>
#include "../src/hypersaw_clap_entry.h"
namespace {
#include "notefuzz_scaffold.inc"
}

namespace {

double runScenario(const clap_plugin_factory_t *factory, const char *name,
                    int voicesPerOsc, int notes, bool osc2on, double secs)
{
  const double sr = 44100.0;
  const uint32_t BLK = 128;
  const clap_plugin_t *p = factory->create_plugin(factory, &kHost, "com.lifted-truck.hypersaw");
  p->init(p);
  p->activate(p, sr, 32, BLK);
  p->start_processing(p);
  std::vector<float> L(BLK), R(BLK);
  float *chans[2] = {L.data(), R.data()};
  clap_audio_buffer_t out{};
  out.data32 = chans;
  out.channel_count = 2;
  clap_process_t proc{};
  proc.frames_count = BLK;
  proc.audio_outputs = &out;
  proc.audio_outputs_count = 1;
  proc.out_events = &kOut;
  auto once = [&](EvList &e) { e.finalize(); proc.in_events = &e.list; p->process(p, &proc); };

  // Set up: N voices/osc, osc1 always enabled (default), osc2 per osc2on.
  // Match the human patch's shape: long release, spring bend, comb+drive.
  { EvList e;
    e.params.push_back(mkParam(1, voicesPerOsc));      // osc0 voices/note
    e.params.push_back(mkParam(1001, voicesPerOsc));   // osc1 voices/note
    e.params.push_back(mkParam(1017, 0.4));             // osc1 vol (irrelevant if enable=0)
    e.params.push_back(mkParam(1150, osc2on ? 1 : 0));  // osc1 enable (ADR-100, id 150+100)
    e.params.push_back(mkParam(22, 5.0));
    e.params.push_back(mkParam(1022, 5.0));
    e.params.push_back(mkParam(106, 4));                // spring bend law
    e.params.push_back(mkParam(57, 5));                 // comb
    e.params.push_back(mkParam(59, 1));                 // drive
    once(e);
  }
  for (int k = 0; k < notes; k++)
  {
    EvList e;
    e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON, 0, (int16_t)(48 + k * 3), k + 1));
    once(e);
  }
  // warm-up, then timed window
  for (int b = 0; b < 200; b++) { EvList e; once(e); }
  const int NB = (int)(secs * sr / BLK);
  const auto t0 = std::chrono::steady_clock::now();
  for (int b = 0; b < NB; b++) { EvList e; once(e); }
  const auto t1 = std::chrono::steady_clock::now();
  const double cpu = std::chrono::duration<double>(t1 - t0).count();
  const double audio = NB * (double)BLK / sr;
  const double pct = 100.0 * cpu / audio;
  std::printf("%-42s %6.3f s cpu / %.1f s audio  = %5.2f%% of a core\n", name, cpu, audio, pct);
  p->stop_processing(p);
  p->deactivate(p);
  p->destroy(p);
  return pct;
}

}  // namespace

int main()
{
  hypersaw_entry_init("");
  auto *factory = (const clap_plugin_factory_t *)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);

  struct Row { int voices; int notes; };
  const Row rows[] = {{7, 8}, {16, 8}, {16, 16}, {8, 4}};

  for (const auto &r : rows)
  {
    char label[64];
    std::snprintf(label, sizeof(label), "--- voices/osc=%d notes=%d ---", r.voices, r.notes);
    std::printf("%s\n", label);
    const double off = runScenario(factory, "1 osc active (osc2 enable=0)", r.voices, r.notes, false, 3.0);
    const double on  = runScenario(factory, "2 osc active (osc2 enable=1)", r.voices, r.notes, true, 3.0);
    std::printf("  marginal cost of osc 2:                    +%.2f pct pts (%.1fx)\n\n",
                on - off, off > 0.0 ? on / off : 0.0);
  }
  return 0;
}
