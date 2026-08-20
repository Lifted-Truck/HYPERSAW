// The HUMAN'S patch, reproduced: both oscillators audible at 16 voices, long
// release, spring bend, comb + drive. Play 8 notes, RELEASE them, and time the
// tail in one-second windows — the report is "every note adds 3-5% and each
// takes a long time to let up".
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
int main()
{
  hypersaw_entry_init("");
  auto *factory=(const clap_plugin_factory_t*)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  const double sr=48000.0; const uint32_t BLK=64;   // the DAW's likely settings, not the bench's
  const clap_plugin_t *p=factory->create_plugin(factory,&kHost,"com.lifted-truck.hypersaw");
  p->init(p); p->activate(p,sr,32,BLK); p->start_processing(p);
  std::vector<float> L(BLK),R(BLK);
  float*chans[2]={L.data(),R.data()};
  clap_audio_buffer_t out{}; out.data32=chans; out.channel_count=2;
  clap_process_t proc{}; proc.frames_count=BLK; proc.audio_outputs=&out;
  proc.audio_outputs_count=1; proc.out_events=&kOut;
  auto once=[&](EvList&e){e.finalize();proc.in_events=&e.list;p->process(p,&proc);};
  { EvList e;
    for (auto pv : std::vector<std::pair<clap_id,double>>{
      {1,16},{1001,16},{1017,0.4},          // both oscs, 16 voices, osc2 audible
      {22,5.0},{1022,5.0},                  // long releases
      {106,4},                              // bend law: spring
      {57,5},{59,1},                        // comb + drive
      {9,30},{14,1.5}})                     // drift + width, as a real patch has
      e.params.push_back(mkParam(pv.first,pv.second));
    once(e); }
  auto timeSec=[&](double secs)->double{
    const int NB=(int)(secs*sr/BLK);
    auto t0=std::chrono::steady_clock::now();
    for(int b=0;b<NB;b++){ EvList e; once(e); }
    return std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count()/secs*100.0;
  };
  // strike notes one at a time and report the increment
  double prev=timeSec(1.0);
  std::printf("idle                 %5.2f%%\n", prev);
  for(int k=0;k<8;k++){
    { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON,0,(int16_t)(48+k*3),k+1)); once(e); }
    const double c=timeSec(1.0);
    std::printf("+note %d              %5.2f%%   (+%.2f)\n", k+1, c, c-prev);
    prev=c;
  }
  // release everything and watch the tail decay
  { EvList e; for(int k=0;k<8;k++) e.notes.push_back(mkNote(CLAP_EVENT_NOTE_OFF,0,(int16_t)(48+k*3),k+1)); once(e); }
  for(int t=0;t<8;t++){
    const double c=timeSec(1.0);
    std::printf("tail t+%ds            %5.2f%%\n", t+1, c);
  }
  p->stop_processing(p); p->deactivate(p); p->destroy(p);
  return 0;
}
