/*
 * notefuzz_check — note-lifecycle oracle at the CLAP layer (ADR-038).
 *
 * Links the plugin impl statically (like state_check) and drives process()
 * with seeded random note on/off streams — poly, mono, mono+legato — plus
 * the MIDI 1.0 vel-0 convention (releases delivered as NOTE_ON velocity 0,
 * which is what the AU wrapper forwards for a controller's 0x90-vel-0
 * note-off). Release sits at its knob minimum; after every held key is
 * released the output must decay to silence. A voice still audible 0.5 s
 * after all-keys-up is a hang — the 2026-07-18 "doesn't stop when you let
 * go" report.
 *
 * Harness trap (learned the hard way): events are delivered time-sorted
 * within a block, so per-block timestamps must be drawn sorted BEFORE
 * assigning on/off actions — otherwise the generator emits an OFF that
 * precedes its own ON and fakes a hang no host can produce.
 */
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>
#include <clap/clap.h>

#include "../src/hypersaw_clap_entry.h"

namespace
{
const void *host_get_extension(const clap_host_t *, const char *) { return nullptr; }
void host_noop(const clap_host_t *) {}
const clap_host_t kHost = {CLAP_VERSION, nullptr, "notefuzz_check", "", "", "1.0",
                           host_get_extension, host_noop, host_noop, host_noop};

bool oev_try_push(const clap_output_events_t *, const clap_event_header_t *) { return true; }
const clap_output_events_t kOut = {nullptr, oev_try_push};

struct EvList
{
  clap_input_events_t list;
  std::vector<clap_event_note_t> notes;
  std::vector<clap_event_param_value_t> params;
  std::vector<const clap_event_header_t *> order;  // time-sorted view
  void finalize()
  {
    order.clear();
    for (auto &n : notes) order.push_back(&n.header);
    for (auto &p : params) order.push_back(&p.header);
    std::stable_sort(order.begin(), order.end(),
                     [](const clap_event_header_t *a, const clap_event_header_t *b) {
                       return a->time < b->time;
                     });
    list.ctx = this;
    list.size = [](const clap_input_events_t *l) -> uint32_t {
      return (uint32_t)((EvList *)l->ctx)->order.size();
    };
    list.get = [](const clap_input_events_t *l, uint32_t i) -> const clap_event_header_t * {
      return ((EvList *)l->ctx)->order[i];
    };
  }
};

clap_event_note_t mkNote(uint16_t type, uint32_t time, int16_t key, int32_t noteId,
                         double velocity = 1.0)
{
  clap_event_note_t ev{};
  ev.header.size = sizeof(ev);
  ev.header.time = time;
  ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
  ev.header.type = type;
  ev.note_id = noteId;
  ev.port_index = 0;
  ev.channel = 0;
  ev.key = key;
  ev.velocity = velocity;
  return ev;
}

clap_event_param_value_t mkParam(clap_id id, double value)
{
  clap_event_param_value_t ev{};
  ev.header.size = sizeof(ev);
  ev.header.time = 0;
  ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
  ev.header.type = CLAP_EVENT_PARAM_VALUE;
  ev.param_id = id;
  ev.cookie = nullptr;
  ev.note_id = -1;
  ev.port_index = -1;
  ev.channel = -1;
  ev.key = -1;
  ev.value = value;
  return ev;
}

// mulberry32 — deterministic, same family the core uses
uint32_t mrand(uint32_t &s)
{
  s += 0x6D2B79F5u;
  uint32_t t = s;
  t = (t ^ (t >> 15)) * (t | 1u);
  t ^= t + (t ^ (t >> 7)) * (t | 61u);
  return t ^ (t >> 14);
}

constexpr int kBlock = 256;
constexpr double kSR = 44100.0;

// Peak |output| in [allOff + 0.5 s, allOff + 1.5 s]. With release at the
// 5 ms knob minimum the audible tail is ~46 ms, so anything above the floor
// in that window is a hung voice.
double runScenario(uint32_t seed, bool mono, bool legato, bool vel0off)
{
  auto *factory =
      (const clap_plugin_factory_t *)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  const clap_plugin_t *p =
      factory->create_plugin(factory, &kHost, "com.lifted-truck.hypersaw");
  p->init(p);
  p->activate(p, kSR, 32, kBlock);
  p->start_processing(p);

  std::vector<float> L(kBlock), R(kBlock);
  float *chans[2] = {L.data(), R.data()};
  clap_audio_buffer_t out{};
  out.data32 = chans;
  out.channel_count = 2;
  clap_process_t proc{};
  proc.frames_count = kBlock;
  proc.audio_outputs = &out;
  proc.audio_outputs_count = 1;
  proc.out_events = &kOut;

  uint32_t rng = seed;
  std::vector<int16_t> held;
  int32_t nextId = 1;

  auto process = [&](EvList &evs) {
    evs.finalize();
    proc.in_events = &evs.list;
    p->process(p, &proc);
  };
  auto pushOff = [&](EvList &evs, uint32_t t, int16_t key) {
    if (vel0off)
      evs.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON, t, key, -1, 0.0));
    else
      evs.notes.push_back(mkNote(CLAP_EVENT_NOTE_OFF, t, key, -1));
  };

  {  // configure: release at knob min, sustain 1 (defaults elsewhere)
    EvList evs;
    evs.params.push_back(mkParam(22, 0.005));
    evs.params.push_back(mkParam(21, 1.0));
    if (mono) evs.params.push_back(mkParam(32, 1.0));
    evs.params.push_back(mkParam(34, legato ? 1.0 : 0.0));
    process(evs);
  }

  const int fuzzBlocks = 400;  // ~2.3 s of dense playing
  for (int b = 0; b < fuzzBlocks; b++)
  {
    EvList evs;
    const int nEv = (int)(mrand(rng) % 4);  // 0..3 events per block
    uint32_t times[4];
    for (int e = 0; e < nEv; e++) times[e] = mrand(rng) % kBlock;
    std::sort(times, times + nEv);  // causal delivery — see header comment
    for (int e = 0; e < nEv; e++)
    {
      const uint32_t t = times[e];
      const bool doOff = !held.empty() && (mrand(rng) & 1u);
      if (doOff)
      {
        const size_t idx = mrand(rng) % held.size();
        const int16_t key = held[idx];
        held.erase(held.begin() + idx);
        pushOff(evs, t, key);
      }
      else
      {
        const int16_t key = (int16_t)(48 + (mrand(rng) % 25));
        if (std::find(held.begin(), held.end(), key) != held.end()) continue;
        held.push_back(key);
        evs.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON, t, key, nextId++));
      }
    }
    process(evs);
  }

  {  // release every held key
    EvList evs;
    uint32_t t = 0;
    for (int16_t key : held) pushOff(evs, t++, key);
    held.clear();
    process(evs);
  }

  const int skipBlocks = (int)(0.5 * kSR) / kBlock;
  const int measureBlocks = (int)(1.0 * kSR) / kBlock;
  double peak = 0;
  EvList empty;
  for (int b = 0; b < skipBlocks + measureBlocks; b++)
  {
    process(empty);
    if (b < skipBlocks) continue;
    for (int i = 0; i < kBlock; i++)
    {
      const double a = std::fabs((double)L[i]) + std::fabs((double)R[i]);
      if (a > peak) peak = a;
    }
  }

  p->stop_processing(p);
  p->deactivate(p);
  p->destroy(p);
  return peak;
}
}  // namespace

int main(int argc, char **argv)
{
  hypersaw_entry_init("");
  const int nSeeds = argc > 1 ? std::atoi(argv[1]) : 15;
  int failures = 0;
  struct Mode { const char *name; bool mono, legato, vel0off; };
  const Mode modes[] = {
      {"poly", false, false, false},
      {"mono", true, false, false},
      {"mono+legato", true, true, false},
      {"poly+vel0off", false, false, true},
      {"mono+vel0off", true, false, true},
  };
  for (const auto &m : modes)
  {
    int modeFail = 0;
    for (int s = 0; s < nSeeds; s++)
    {
      const double peak = runScenario(1000 + (uint32_t)s, m.mono, m.legato, m.vel0off);
      if (peak > 1e-5)
      {
        failures++;
        modeFail++;
        if (modeFail <= 3)
          std::printf("FAIL hang mode=%s seed=%d peak=%.6g\n", m.name, 1000 + s, peak);
      }
    }
    std::printf("%s %s: %d/%d silent after all-keys-up\n", modeFail ? "FAIL" : "OK  ",
                m.name, nSeeds - modeFail, nSeeds);
  }
  std::printf(failures ? "notefuzz_check: RED (%d hangs)\n" : "notefuzz_check: GREEN (0 hangs)\n",
              failures);
  return failures ? 1 : 0;
}
