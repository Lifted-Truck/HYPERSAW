// Functional smoke for ADR-072: every new id, driven at an extreme, must CHANGE
// the SAW engine's output (stereo-aware: pan-image params move L vs R, not the
// sum). keepPhase is exercised with a retrigger, since it only acts at note-on.
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>
#include <clap/clap.h>
#include "../src/hypersaw_clap_entry.h"
namespace {
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


struct Setup { clap_id id; double v; };
double renderLR(const std::vector<Setup> &pre, bool twoNotes, double *outR)
{
  auto *factory=(const clap_plugin_factory_t*)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  const clap_plugin_t *p=factory->create_plugin(factory,&kHost,"com.lifted-truck.hypersaw");
  p->init(p); p->activate(p,kSR,32,kBlock); p->start_processing(p);
  std::vector<float> L(kBlock),R(kBlock);
  float*chans[2]={L.data(),R.data()};
  clap_audio_buffer_t out{}; out.data32=chans; out.channel_count=2;
  clap_process_t proc{}; proc.frames_count=kBlock; proc.audio_outputs=&out;
  proc.audio_outputs_count=1; proc.out_events=&kOut;
  auto process=[&](EvList&e){e.finalize();proc.in_events=&e.list;p->process(p,&proc);};
  { EvList e; for(auto&s:pre) e.params.push_back(mkParam(s.id,s.v)); process(e); }
  { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON,0,57,1)); process(e); }
  if(twoNotes){ for(int b=0;b<20;b++){EvList e;process(e);}
    { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_OFF,0,57,-1)); process(e); }
    { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON,0,57,2)); process(e); } }
  double aL=0,aR=0;
  for(int b=0;b<120;b++){EvList e;process(e);
    for(int i=0;i<kBlock;i++){aL+=(double)L[i]*L[i];aR+=(double)R[i]*R[i];}}
  p->stop_processing(p);p->deactivate(p);p->destroy(p);
  *outR=std::sqrt(aR/(120.0*kBlock));
  return std::sqrt(aL/(120.0*kBlock));
}
}
int main(){
  hypersaw_entry_init("");
  struct T { const char*name; std::vector<Setup> pre; bool two; };
  // width up + detune up so image/placement params have something to act on
  const std::vector<Setup> base={{14,1.5},{4,0.6}};
  auto with=[&](std::initializer_list<Setup> xs){auto v=base;for(auto&x:xs)v.push_back(x);return v;};
  const T tests[]={
    {"baseline",base,false},
    {"71 toneTilt @1",with({{71,1}}),false},
    {"72 hiTame @1",with({{72,1}}),false},
    {"73 driftMode @2 (+drift depth)",with({{9,80},{73,2}}),false},
    {"74 keepPhase @1 (retrig)",with({{74,1}}),true},
    {"75 freqGlide @0.1 (+S&H drift)",with({{9,80},{73,2},{75,0.1}}),false},
    {"76 panMotion @1",with({{76,1}}),false},
    {"77 panMode @1 (+panMotion)",with({{76,1},{77,1}}),false},
    {"78 motionCenter @1 (+drift)",with({{9,80},{78,1}}),false},
    {"79 harmReach @4 (law 4)",with({{5,4},{79,4}}),false},
    {"80 stretchB @6 (law 5)",with({{5,5},{80,6}}),false},
    {"81 spread @24",with({{81,24}}),false},
    {"82 anchor @1",with({{82,1}}),false},
    {"83 pivotMode @1 (+K)",with({{6,0.8},{83,1}}),false},
    {"84 panLayout @1",with({{84,1}}),false},
    {"85 panCurve @0",with({{85,0}}),false},
    {"86 panInvert @1",with({{86,1}}),false},
    {"law 4 alone (widened enum)",with({{5,4}}),false},
    {"dist 4 alone (widened enum)",with({{2,4}}),false},
  };
  double bR; const double bL=renderLR(base,false,&bR);
  double b2R; const double b2L=renderLR(base,true,&b2R);
  std::printf("baseline L %.9f R %.9f (retrig L %.9f)\n\n",bL,bR,b2L);
  int dead=0;
  for(const auto&t:tests){
    if(!std::strcmp(t.name,"baseline"))continue;
    double r; const double l=renderLR(t.pre,t.two,&r);
    const double refL=t.two?b2L:bL, refR=t.two?b2R:bR;
    const bool moved=std::fabs(l-refL)>1e-9||std::fabs(r-refR)>1e-9;
    if(!moved)dead++;
    std::printf("%-32s L %.9f R %.9f   %s\n",t.name,l,r,moved?"ACTS":"DEAD <-- param does nothing");
  }
  std::printf("\n%s\n",dead?"SOME PARAMS DEAD":"ALL 16 NEW PARAMS ACT (+ both widened enum values)");
  return dead?1:0;
}
