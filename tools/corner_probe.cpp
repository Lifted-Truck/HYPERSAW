// ADR-109: does an edit land where the human's model says it lands?
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <vector>
#include <clap/clap.h>
#include "../src/hypersaw_clap_entry.h"
extern "C" void hypersaw_debug_state(const clap_plugin_t*, char*, uint32_t);
extern "C" bool hypersaw_debug_exempt(const clap_plugin_t*, uint32_t);
extern "C" const char *hypersaw_debug_exemptjson(const clap_plugin_t*);
#include <string>
namespace { 
#include "notefuzz_scaffold.inc"
}
int main()
{
  hypersaw_entry_init("");
  auto *factory=(const clap_plugin_factory_t*)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  const clap_plugin_t *p=factory->create_plugin(factory,&kHost,"com.lifted-truck.hypersaw");
  p->init(p); p->activate(p,44100.0,32,256); p->start_processing(p);
  std::vector<float> L(256),R(256);
  float*chans[2]={L.data(),R.data()};
  clap_audio_buffer_t out{}; out.data32=chans; out.channel_count=2;
  clap_process_t proc{}; proc.frames_count=256; proc.audio_outputs=&out;
  proc.audio_outputs_count=1; proc.out_events=&kOut;
  auto ev=[&](clap_id id,double v){ EvList e; e.params.push_back(mkParam(id,v));
    e.finalize(); proc.in_events=&e.list; p->process(p,&proc); };
  auto pump=[&](int n){ for(int b=0;b<n;b++){ EvList e; e.finalize(); proc.in_events=&e.list; p->process(p,&proc);} };
  auto *par=(const clap_plugin_params_t*)p->get_extension(p, CLAP_EXT_PARAMS);
  auto get=[&](clap_id id){ double v=-1; par->get_value(p,id,&v); return v; };
  int bad=0;
  const clap_id DET=4;                      // detune, morphable + continuous

  ev(151,1);                                 // morph ON, corners hold defaults
  ev(152,0); ev(153,0);                      // pad at corner A
  pump(120);
  // 1. UNARMED edit must STICK (the v1 seam this closes)
  ev(DET,0.61); pump(200);
  const double stuck=get(DET);
  std::printf("unarmed edit sticks:      %.3f (want 0.610)  %s\n", stuck,
              std::fabs(stuck-0.61)<0.02?"OK":"FAIL");
  if(std::fabs(stuck-0.61)>0.02) bad++;

  // 2. ARMED at corner D, edit, then move the pad to D: the value must appear
  ev(159,4); ev(DET,0.23); pump(60);
  ev(152,1); ev(153,1);                      // pad to corner D
  pump(400);
  const double atD=get(DET);
  std::printf("armed edit lands in D:    %.3f (want 0.230)  %s\n", atD,
              std::fabs(atD-0.23)<0.03?"OK":"FAIL");
  if(std::fabs(atD-0.23)>0.03) bad++;

  // 3. EXEMPT: the field must stop touching it
  ev(159,0);
  hypersaw_debug_exempt(p, DET);
  ev(DET,0.42); 
  ev(152,0); ev(153,0);                      // slam the pad to the far corner
  pump(500);
  const double held=get(DET);
  std::printf("exempt holds its value:   %.3f (want 0.420)  %s\n", held,
              std::fabs(held-0.42)<0.01?"OK":"FAIL");
  if(std::fabs(held-0.42)>0.01) bad++;
  // 4. ADR-109 A1: the globals the human's scan found must now toggle
  const clap_id SCAN[] = {75, 11, 70, 32, 34, 38, 90, 116};
  const char *NAMES[] = {"freqGlide","inertia","inertiaCurve","voiceMono",
                         "voiceLegato","pitchBend","glideMode","scaleRoot"};
  int notToggling = 0;
  for (int i = 0; i < 8; i++)
  {
    const bool on = hypersaw_debug_exempt(p, SCAN[i]);
    if (!on) { std::printf("  %s did NOT toggle\n", NAMES[i]); notToggling++; }
    else hypersaw_debug_exempt(p, SCAN[i]);   // put it back
  }
  std::printf("scanned globals toggle:   %d/8 %s\n", 8-notToggling, notToggling?"FAIL":"OK");
  bad += notToggling ? 1 : 0;

  // 5. the scale exempts COLLECTIVELY: one degree toggles all thirteen
  hypersaw_debug_exempt(p, 120);            // a middle degree
  const std::string ex = hypersaw_debug_exemptjson(p);
  int inScale = 0;
  for (int id = 116; id <= 128; id++)
    if (ex.find("\"" + std::to_string(id) + "\"") != std::string::npos) inScale++;
  std::printf("scale exempts as a unit:  %d/13 %s\n", inScale, inScale==13?"OK":"FAIL");
  if (inScale != 13) bad++;

  p->stop_processing(p); p->deactivate(p); p->destroy(p);
  return bad;
}
