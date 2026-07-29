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


// DIAGNOSTIC (not a gate): does sending a param its OWN DEFAULT value change the
// sound? It must not. If it does, that id is writing somewhere it should not.
//
// Suspicion under test: applyParam mirrors EVERY id into both cores by key name
// ("shared-name knobs mirror; unknown keys no-op"), and ADR-060 added a `tilt`
// key to SwarmCore. Id 45 is documented SPECTRA-only ("ids 44-51 are
// SPECTRA-only") and its key is also "tilt" — so a write that used to no-op on
// the SAW core now lands on ADR-060's tone tilt. The defaults disagree: SPECTRA
// tilt = 1 (flat), SwarmCore tilt = 0 (inert), CLAP id 45 default = 1.
// A direct-core parity oracle cannot see this by construction (LIBRARY L0011).
double render(bool sendParam, clap_id id, double value)
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
  out.data32 = chans; out.channel_count = 2;
  clap_process_t proc{};
  proc.frames_count = kBlock; proc.audio_outputs = &out;
  proc.audio_outputs_count = 1; proc.out_events = &kOut;
  auto process = [&](EvList &evs) { evs.finalize(); proc.in_events = &evs.list; p->process(p, &proc); };
  if (sendParam) { EvList e; e.params.push_back(mkParam(id, value)); process(e); }
  { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON, 0, 57, 1)); process(e); }
  double acc = 0;
  for (int b = 0; b < 120; b++)
  { EvList e; process(e);
    for (int i = 0; i < kBlock; i++) acc += (double)L[i] * (double)L[i]; }
  p->stop_processing(p); p->deactivate(p); p->destroy(p);
  return std::sqrt(acc / (120.0 * kBlock));
}
}  // namespace

int main()
{
  hypersaw_entry_init("");
  const double base = render(false, 0, 0);
  struct C { clap_id id; const char *key; double def; };
  // every documented SPECTRA-only id, sent at its OWN default
  // POSITIVE CONTROLS FIRST. A probe that cannot detect a change it SHOULD see is
  // not evidence of absence (LIBRARY L0016). id 4 is detune and id 45's suspected
  // victim is SwarmCore's tone tilt, so drive both at NON-default values: the
  // first must move the rms, and if id 45 is properly isolated the second must not.
  const C cases[] = {{4,"detune (CONTROL, must change)",0.6},
                     {45,"tilt @2.0 (max)",2.0},
                     {45,"tilt @0.5 (min)",0.5},
                     {46,"stretch @1.0",1.0},
                     {49,"wtilt @1.0",1.0},
                     {51,"cascade @1.0",1.0},
                     {44,"partials @32",32}};
  int bad = 0;
  std::printf("SAW engine rms with no param writes: %.9f\n\n", base);
  std::printf("id  what                            value   rms                verdict\n");
  for (const auto &c : cases)
  {
    const double r = render(true, c.id, c.def);
    const bool leak = std::fabs(r - base) > 1e-12;
    if (leak) bad++;
    std::printf("%-3u %-30s %7.4g   %.9f   %s\n", c.id, c.key, c.def, r,
                leak ? "CHANGES the SAW output" : "no effect on SAW");
  }
  std::printf("\n%s\n", "see per-row verdicts above");
  return 0;
}
