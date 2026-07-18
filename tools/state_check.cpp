/*
 * state_check — session-persistence oracle at the CLAP layer.
 *
 * Links the plugin impl statically (clap-first idiom makes the factory a
 * plain symbol) and drives it with a stub host: set every parameter to a
 * non-default value through the params extension, save state, build a FRESH
 * instance, load, and require (1) every get_value round-trips exactly and
 * (2) the restored instance renders bit-identical audio for the same note —
 * i.e. sessions restore *sound*, not just numbers. Also checks forward
 * compatibility: a state blob missing keys (old session) loads with
 * defaults intact, and unknown keys are ignored.
 *
 * The wrapper layers (VST3/AU setState paths) are covered by pluginval and
 * auval; this pins OUR side of the contract in ./verify full.
 */

#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <clap/clap.h>

#include "../src/hypersaw_clap_entry.h"

namespace
{

int g_failures = 0;
void check(bool ok, const char *what)
{
  std::printf("%s %s\n", ok ? "OK  " : "FAIL", what);
  if (!ok) g_failures++;
}

/* ---- stub host ---- */
const void *host_get_extension(const clap_host_t *, const char *) { return nullptr; }
void host_noop(const clap_host_t *) {}
const clap_host_t kHost = {CLAP_VERSION, nullptr, "state_check", "", "", "1.0",
                           host_get_extension, host_noop, host_noop, host_noop};

/* ---- string-backed streams ---- */
struct OStr
{
  clap_ostream_t s;
  std::string data;
};
int64_t ostr_write(const clap_ostream_t *s, const void *buf, uint64_t n)
{
  auto *o = (OStr *)s;
  o->data.append((const char *)buf, (size_t)n);
  return (int64_t)n;
}
struct IStr
{
  clap_istream_t s;
  std::string data;
  size_t pos = 0;
};
int64_t istr_read(const clap_istream_t *s, void *buf, uint64_t n)
{
  auto *i = (IStr *)s;
  const size_t take = std::min((size_t)n, i->data.size() - i->pos);
  std::memcpy(buf, i->data.data() + i->pos, take);
  i->pos += take;
  return (int64_t)take;
}

/* ---- single-event input list for params.flush ---- */
struct EvList
{
  clap_input_events_t list;
  std::vector<clap_event_param_value_t> evs;
};
uint32_t ev_size(const clap_input_events_t *l) { return (uint32_t)((EvList *)l)->evs.size(); }
const clap_event_header_t *ev_get(const clap_input_events_t *l, uint32_t i)
{
  return &((EvList *)l)->evs[i].header;
}
uint32_t oev_try_push_count = 0;
bool oev_try_push(const clap_output_events_t *, const clap_event_header_t *)
{
  oev_try_push_count++;
  return true;
}
const clap_output_events_t kOut = {nullptr, oev_try_push};

const clap_plugin_t *makePlugin()
{
  auto *factory =
      (const clap_plugin_factory_t *)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  const clap_plugin_t *p =
      factory->create_plugin(factory, &kHost, "com.lifted-truck.hypersaw");
  p->init(p);
  return p;
}

uint64_t renderHash(const clap_plugin_t * /*unused*/, const clap_plugin_t *p)
{
  // 1 s of a held A3 through the CLAP process path, FNV over the f32 stream.
  p->activate(p, 44100.0, 32, 1024);
  p->start_processing(p);
  std::vector<float> L(256), R(256);
  float *chans[2] = {L.data(), R.data()};
  clap_audio_buffer_t out{};
  out.data32 = chans;
  out.channel_count = 2;

  EvList in;
  clap_event_note_t on{};
  on.header.size = sizeof(on);
  on.header.type = CLAP_EVENT_NOTE_ON;
  on.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
  on.note_id = -1;
  on.port_index = 0;
  on.channel = 0;
  on.key = 57;
  on.velocity = 1.0;
  // reuse EvList storage: param_value sized >= note event? No — use separate list.
  struct NoteList
  {
    clap_input_events_t list;
    clap_event_note_t ev;
    bool sent = false;
  } notes;
  notes.ev = on;
  notes.list.ctx = &notes;
  notes.list.size = [](const clap_input_events_t *l) -> uint32_t {
    return ((NoteList *)l->ctx)->sent ? 0u : 1u;
  };
  notes.list.get = [](const clap_input_events_t *l, uint32_t) -> const clap_event_header_t * {
    return &((NoteList *)l->ctx)->ev.header;
  };
  EvList empty;
  empty.list.ctx = &empty;
  empty.list.size = ev_size;
  empty.list.get = ev_get;

  clap_process_t proc{};
  proc.frames_count = 256;
  proc.audio_outputs = &out;
  proc.audio_outputs_count = 1;
  proc.out_events = &kOut;

  uint64_t h = 1469598103934665603ull;
  for (int block = 0; block < 172; block++)  // ~1 s
  {
    proc.in_events = block == 0 ? &notes.list : &empty.list;
    p->process(p, &proc);
    if (block == 0) notes.sent = true;
    for (int i = 0; i < 256; i++)
    {
      uint32_t a, b;
      std::memcpy(&a, &L[i], 4);
      std::memcpy(&b, &R[i], 4);
      h = (h ^ a) * 1099511628211ull;
      h = (h ^ b) * 1099511628211ull;
    }
  }
  p->stop_processing(p);
  p->deactivate(p);
  return h;
}

}  // namespace

int main()
{
  hypersaw_entry_init("");

  const clap_plugin_t *a = makePlugin();
  auto *params = (const clap_plugin_params_t *)a->get_extension(a, CLAP_EXT_PARAMS);
  auto *state = (const clap_plugin_state_t *)a->get_extension(a, CLAP_EXT_STATE);
  check(params && state, "params + state extensions present");

  // Set EVERY param to a distinct non-default value inside its range via
  // the params extension (the host path), stepped params to valid steps.
  const uint32_t n = params->count(a);
  std::vector<double> want(n);
  EvList evs;
  evs.list.ctx = &evs;
  evs.list.size = ev_size;
  evs.list.get = ev_get;
  for (uint32_t i = 0; i < n; i++)
  {
    clap_param_info_t info{};
    params->get_info(a, i, &info);
    double v;
    if (info.flags & CLAP_PARAM_IS_STEPPED)
      v = info.min_value + std::floor((info.max_value - info.min_value) * 0.5);
    else
      v = info.min_value + (info.max_value - info.min_value) * (0.3 + 0.02 * i);
    want[i] = v;
    clap_event_param_value_t ev{};
    ev.header.size = sizeof(ev);
    ev.header.type = CLAP_EVENT_PARAM_VALUE;
    ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
    ev.param_id = info.id;
    ev.note_id = -1;
    ev.port_index = -1;
    ev.channel = -1;
    ev.key = -1;
    ev.value = v;
    evs.evs.push_back(ev);
  }
  params->flush(a, &evs.list, &kOut);

  // The plugin may legally quantize (grid steps); capture what it SAYS it
  // holds — that is what must persist.
  std::vector<double> held(n);
  for (uint32_t i = 0; i < n; i++)
  {
    clap_param_info_t info{};
    params->get_info(a, i, &info);
    params->get_value(a, info.id, &held[i]);
  }

  OStr saved;
  saved.s.ctx = &saved;
  saved.s.write = ostr_write;
  check(state->save(a, &saved.s), "state save succeeds");
  check(saved.data.rfind("hypersaw-state 1\n", 0) == 0, "state blob is versioned");

  // Fresh instance, load, compare every value exactly.
  const clap_plugin_t *b = makePlugin();
  auto *paramsB = (const clap_plugin_params_t *)b->get_extension(b, CLAP_EXT_PARAMS);
  auto *stateB = (const clap_plugin_state_t *)b->get_extension(b, CLAP_EXT_STATE);
  IStr in;
  in.s.ctx = &in;
  in.s.read = istr_read;
  in.data = saved.data;
  check(stateB->load(b, &in.s), "state load succeeds on fresh instance");
  bool allEqual = true;
  for (uint32_t i = 0; i < n; i++)
  {
    clap_param_info_t info{};
    paramsB->get_info(b, i, &info);
    double got = 0;
    paramsB->get_value(b, info.id, &got);
    if (got != held[i])
    {
      std::printf("     mismatch id %u: held %.17g restored %.17g\n", info.id, held[i], got);
      allEqual = false;
    }
  }
  check(allEqual, "every param value round-trips exactly");

  // Restored instance must SOUND identical: same note, bit-identical render.
  const uint64_t ha = renderHash(nullptr, a);
  const uint64_t hb = renderHash(nullptr, b);
  check(ha == hb, "restored instance renders bit-identical audio");

  // Old-session compatibility: a blob missing keys keeps defaults; unknown
  // keys are ignored.
  const clap_plugin_t *c = makePlugin();
  auto *stateC = (const clap_plugin_state_t *)c->get_extension(c, CLAP_EXT_STATE);
  auto *paramsC = (const clap_plugin_params_t *)c->get_extension(c, CLAP_EXT_PARAMS);
  IStr partial;
  partial.s.ctx = &partial;
  partial.s.read = istr_read;
  partial.data = "hypersaw-state 1\nK=0.5\nfutureParam=42\n";
  check(stateC->load(c, &partial.s), "partial/forward blob loads");
  double kv = 0, det = 0;
  paramsC->get_value(c, 6, &kv);
  paramsC->get_value(c, 4, &det);
  check(kv == 0.5 && det == 0.28, "missing keys keep defaults; unknown keys ignored");

  a->destroy(a);
  b->destroy(b);
  c->destroy(c);
  hypersaw_entry_deinit();

  std::printf("state_check: %s (%d failure%s)\n", g_failures ? "RED" : "GREEN", g_failures,
              g_failures == 1 ? "" : "s");
  return g_failures ? 1 : 0;
}
