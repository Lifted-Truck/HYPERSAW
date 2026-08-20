// ADR-100 probe: osc enable kills, silences, cheapens, and returns cleanly.
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cmath>
#include <vector>
#include <clap/clap.h>
#include "../src/hypersaw_clap_entry.h"
namespace {
#include "notefuzz_scaffold.inc"
}
static double rms(const std::vector<float>&v){double s=0;for(float x:v)s+=x*x;return std::sqrt(s/v.size());}
int main()
{
  hypersaw_entry_init("");
  auto *factory=(const clap_plugin_factory_t*)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  const double sr=44100.0; const uint32_t BLK=128;
  const clap_plugin_t *p=factory->create_plugin(factory,&kHost,"com.lifted-truck.hypersaw");
  p->init(p); p->activate(p,sr,32,BLK); p->start_processing(p);
  std::vector<float> L(BLK),R(BLK);
  float*chans[2]={L.data(),R.data()};
  clap_audio_buffer_t out{}; out.data32=chans; out.channel_count=2;
  clap_process_t proc{}; proc.frames_count=BLK; proc.audio_outputs=&out;
  proc.audio_outputs_count=1; proc.out_events=&kOut;
  auto once=[&](EvList&e){e.finalize();proc.in_events=&e.list;p->process(p,&proc);};
  auto cap=[&](double secs){ std::vector<float> t; const int NB=(int)(secs*sr/BLK);
    auto t0=std::chrono::steady_clock::now();
    for(int b=0;b<NB;b++){ EvList e; once(e); t.insert(t.end(),L.begin(),L.end()); }
    double cpu=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count()/secs*100;
    std::printf("  rms %.5f  cpu %5.2f%%", rms(t), cpu); return rms(t); };
  { EvList e; e.params.push_back(mkParam(1,16)); e.params.push_back(mkParam(1001,16));
    e.params.push_back(mkParam(1017,0.4)); e.params.push_back(mkParam(22,5.0)); once(e); }
  for(int k=0;k<4;k++){ EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON,0,(int16_t)(48+k*5),k+1)); once(e); }
  std::printf("both on, 4 held:      "); const double base=cap(1.0); std::printf("\n");
  { EvList e; e.params.push_back(mkParam(1150,0)); once(e); }   // osc2 OFF
  std::printf("osc2 OFF:             "); cap(1.0); std::printf("\n");
  { EvList e; e.params.push_back(mkParam(150,0)); once(e); }    // osc1 OFF too
  std::printf("both OFF:             "); const double dead=cap(1.0); std::printf("   (tails? %s)\n", dead<1e-6?"KILLED":"STILL RINGING");
  { EvList e; e.params.push_back(mkParam(150,1)); e.params.push_back(mkParam(1150,1)); once(e); }
  { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON,0,72,99)); once(e); }
  std::printf("re-enabled + new note:"); const double back=cap(1.0); std::printf("   (%s)\n", back>0.01?"SOUNDS":"SILENT — BROKEN");
  return (base>0.01 && dead<1e-6 && back>0.01) ? 0 : 1;
}
