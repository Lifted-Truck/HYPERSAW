/* mpe_check — per-note pitch expression must reach EVERY oscillator.
 *
 * The bug this exists for (2026-08-09): PRESSURE fanned out to all cores while
 * TUNING did not. With two oscillators a bend gesture moved oscillator 1 and
 * left oscillator 2 sitting at the unbent pitch — so the pair audibly SPLIT
 * mid-gesture instead of moving together. Nothing caught it: parity renders a
 * single core, and the plugin still made confident, musical-sounding sound.
 *
 * WHY GOERTZEL AND NOT A STATE READ. There is no per-voice tuning getter, and
 * adding one to test with would be testing the accessor rather than the audio
 * (the state_check trap: a round-trip through one broken accessor agrees with
 * itself). So this measures the only thing that actually matters — the emitted
 * signal — with a single-bin Goertzel at the unbent frequency. Both cores are
 * configured to one voice, zero detune, zero width, so each oscillator is one
 * clean partial and "did osc 2 come along?" becomes "is there still energy at
 * 440 Hz after a +7 semitone bend?".
 *
 * Calibrated (both directions, see the trace): with the fan-out removed the
 * residual is ~0.5 (half the energy stranded at the old pitch); with it in
 * place the residual is at the noise floor. The threshold sits between them by
 * more than an order of magnitude, so this cannot pass by being insensitive.
 */
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

#include "../src/hypersaw_clap_entry.h"

static const clap_host_t kHost = {CLAP_VERSION_INIT, nullptr, "mpe_check", "-", "-", "1.0",
                                  [](const clap_host_t *, const char *) -> const void * { return nullptr; },
                                  [](const clap_host_t *) {}, [](const clap_host_t *) {},
                                  [](const clap_host_t *) {}};

struct EvList
{
  clap_input_events_t in{};
  std::vector<const clap_event_header_t *> ev;
};
static uint32_t evSize(const clap_input_events_t *l) { return (uint32_t)((EvList *)l->ctx)->ev.size(); }
static const clap_event_header_t *evGet(const clap_input_events_t *l, uint32_t i)
{
  return ((EvList *)l->ctx)->ev[i];
}
static bool outPush(const clap_output_events_t *, const clap_event_header_t *) { return true; }

static int failures = 0;
static void check(bool ok, const char *what, const char *detail)
{
  std::printf("%-6s %s  (%s)\n", ok ? "OK" : "FAIL", what, detail);
  if (!ok) failures++;
}

// Single-bin DFT magnitude, normalised by block length.
// Own pi: M_PI is a POSIX extension, absent under MSVC without
// _USE_MATH_DEFINES, and libc++ defining it anyway is how this shipped
// mac-green and windows-red.
static constexpr double kPi = 3.14159265358979323846;
static double goertzel(const std::vector<float> &x, double freq, double sr)
{
  const double w = 2.0 * kPi * freq / sr;
  const double c = 2.0 * std::cos(w);
  double s1 = 0, s2 = 0;
  for (float v : x) { const double s0 = v + c * s1 - s2; s2 = s1; s1 = s0; }
  const double re = s1 - s2 * std::cos(w), im = s2 * std::sin(w);
  return 2.0 * std::sqrt(re * re + im * im) / (double)x.size();
}

int main()
{
  const double SR = 44100.0;
  const int BLK = 512;
  auto *factory = (const clap_plugin_factory_t *)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  const clap_plugin_t *p = factory->create_plugin(factory, &kHost, "com.lifted-truck.hypersaw");
  p->init(p);
  p->activate(p, SR, 32, 2048);
  p->start_processing(p);

  std::vector<float> L(BLK), R(BLK);
  float *chans[2] = {L.data(), R.data()};
  clap_audio_buffer_t out{};
  out.data32 = chans; out.channel_count = 2;
  EvList evl;
  evl.in.ctx = &evl; evl.in.size = evSize; evl.in.get = evGet;
  clap_output_events_t outEv{nullptr, outPush};

  std::vector<clap_event_param_value_t> pvs;
  auto param = [&](clap_id id, double v) {
    clap_event_param_value_t pv{};
    pv.header.size = sizeof(pv); pv.header.type = CLAP_EVENT_PARAM_VALUE;
    pv.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
    pv.note_id = -1; pv.port_index = -1; pv.channel = -1; pv.key = -1;
    pv.param_id = id; pv.value = v;
    pvs.push_back(pv);
  };
  // One clean partial per oscillator, both audible. Oscillator 2's volume
  // defaults to 0 (inert-by-default, hypersaw_clap.cpp) — it must be raised or
  // this probe would "pass" by osc 2 being silent, which is the exact way a
  // fan-out test lies.
  const uint32_t S = 1000;                 // ADR-082 oscillator id stride
  for (uint32_t o = 0; o < 2; o++)
  {
    param(o * S + 1, 1);                   // voices = 1
    param(o * S + 4, 0);                   // detune = 0
    param(o * S + 14, 0);                  // width = 0
    param(o * S + 9, 0);                   // drift depth = 0
    param(o * S + 17, 0.4);                // volume (osc 2 defaults to 0)
  }
  pvs.reserve(pvs.size());
  for (auto &pv : pvs) evl.ev.push_back(&pv.header);

  auto run = [&](int blocks, std::vector<float> *capture) {
    for (int b = 0; b < blocks; b++)
    {
      clap_process_t proc{};
      proc.audio_inputs_count = 0; proc.audio_outputs_count = 1; proc.audio_outputs = &out;
      proc.frames_count = BLK; proc.in_events = &evl.in; proc.out_events = &outEv;
      proc.steady_time = (int64_t)b * BLK;
      p->process(p, &proc);
      evl.ev.clear();                       // events are one-shot
      if (capture)
        for (int i = 0; i < BLK; i++) capture->push_back(0.5f * (L[i] + R[i]));
    }
  };
  run(1, nullptr);                          // params land

  // A4 on an MPE member channel, with a note id so wildcard matching is not
  // what is being relied on.
  clap_event_note_t on{};
  on.header.size = sizeof(on); on.header.type = CLAP_EVENT_NOTE_ON;
  on.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
  on.note_id = 77; on.port_index = 0; on.channel = 1; on.key = 69; on.velocity = 1.0;
  evl.ev.push_back(&on.header);
  run(20, nullptr);                         // ~230 ms: attack settles

  const double f0 = 440.0;                  // A4
  const double f1 = 440.0 * std::pow(2.0, 7.0 / 12.0);   // +7 semitones

  std::vector<float> before;
  run(20, &before);
  const double b0 = goertzel(before, f0, SR);
  char d0[64];
  std::snprintf(d0, sizeof(d0), "mag %.6g", b0);   // snprintf, not std::string:
  check(b0 > 1e-3, "unbent note is present at 440 Hz", d0);  // <string> is not
                                                             // included here and
                                                             // only libc++ leaks it

  // The gesture: +7 semitones as a TUNING note expression.
  clap_event_note_expression_t tx{};
  tx.header.size = sizeof(tx); tx.header.type = CLAP_EVENT_NOTE_EXPRESSION;
  tx.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
  tx.expression_id = CLAP_NOTE_EXPRESSION_TUNING;
  tx.note_id = 77; tx.port_index = -1; tx.channel = -1; tx.key = -1; tx.value = 7.0;
  evl.ev.push_back(&tx.header);
  run(20, nullptr);                         // let the bend settle

  std::vector<float> after;
  run(20, &after);
  const double a0 = goertzel(after, f0, SR);
  const double a1 = goertzel(after, f1, SR);

  char d[160];
  std::snprintf(d, sizeof(d), "440Hz %.5f -> %.5f (%.1f%% left), 659Hz %.5f", b0, a0,
                100.0 * a0 / (b0 > 0 ? b0 : 1), a1);
  // If the bend reached only one of two oscillators, ~half the energy stays at
  // the old pitch. Threshold at 15%: an order of magnitude above the settled
  // noise floor and far below the 50% a single stranded oscillator produces.
  check(a0 < 0.15 * b0, "every oscillator follows the TUNING expression", d);
  check(a1 > 0.3 * b0, "the bent pitch is where the energy went", d);

  // ---- all-notes-off must silence EVERY oscillator -------------------------
  // The stuck-note class. Every allOff() site addressed oscillator 0, so a
  // panic / MIDI all-notes-off left the other oscillators ringing forever —
  // and a hung note is exactly what the human reported. A CLAP note-off with
  // key < 0 is the all-notes-off path.
  {
    clap_event_note_t allOff{};
    allOff.header.size = sizeof(allOff); allOff.header.type = CLAP_EVENT_NOTE_OFF;
    allOff.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
    allOff.note_id = -1; allOff.port_index = -1; allOff.channel = -1; allOff.key = -1;
    allOff.velocity = 0.0;
    evl.ev.push_back(&allOff.header);
    // allOff() clears GATES; voices then ring out through release + dissolve
    // (0.16 s + 0.63 s by default), so a decaying tail here is correct and the
    // settle must be generous. What a stranded oscillator does instead is keep
    // its gate SET at sustain 1.0 — it plateaus and never decays at all, which
    // is why the three windows below discriminate and a single loudness reading
    // would not.
    run(120, nullptr);                      // ~1.4 s settle
    // Three successive windows, because "how loud" alone cannot tell a decaying
    // FX tail from a stuck voice — only the SHAPE can. A stranded oscillator
    // plateaus; a tail keeps falling.
    double r[3];
    for (int w = 0; w < 3; w++)
    {
      std::vector<float> tail;
      run(20, &tail);
      double a = 0;
      for (float v : tail) a += (double)v * v;
      r[w] = std::sqrt(a / (double)tail.size());
    }
    char d2[192];
    std::snprintf(d2, sizeof(d2), "rms %.3g -> %.3g -> %.3g (gated ~%.3g); decay x%.1f",
                  r[0], r[1], r[2], b0, r[2] > 0 ? r[0] / r[2] : 1e9);
    check(r[2] < 1e-4 && r[0] > r[2] * 4, "all-notes-off silences every oscillator", d2);
  }

  // ---- mono legato retarget must move EVERY oscillator ---------------------
  // Same shape again: retargetNote() addressed oscillator 0, so a legato slide
  // in mono mode dragged one oscillator to the new note and left the rest on
  // the old one.
  {
    param(32, 1);                           // voiceMono on
    param(34, 1);                           // voiceLegato on
    for (size_t i = pvs.size() - 2; i < pvs.size(); i++) evl.ev.push_back(&pvs[i].header);
    run(2, nullptr);

    clap_event_note_t m1{};
    m1.header.size = sizeof(m1); m1.header.type = CLAP_EVENT_NOTE_ON;
    m1.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
    m1.note_id = 90; m1.port_index = 0; m1.channel = 0; m1.key = 69; m1.velocity = 1.0;
    evl.ev.push_back(&m1.header);
    run(25, nullptr);
    std::vector<float> held;
    run(20, &held);
    const double m440 = goertzel(held, f0, SR);

    clap_event_note_t m2 = m1;              // legato slide up a fifth, first still held
    m2.note_id = 91; m2.key = 76;
    evl.ev.push_back(&m2.header);
    run(25, nullptr);
    std::vector<float> slid;
    run(20, &slid);
    const double s440 = goertzel(slid, f0, SR);
    const double s659 = goertzel(slid, f1, SR);
    char d3[160];
    std::snprintf(d3, sizeof(d3), "440Hz %.5f -> %.5f (%.1f%% left), 659Hz %.5f", m440, s440,
                  100.0 * s440 / (m440 > 0 ? m440 : 1), s659);
    check(m440 > 1e-3 && s440 < 0.15 * m440,
          "mono legato retarget moves every oscillator", d3);
  }

  p->stop_processing(p);
  p->deactivate(p);
  p->destroy(p);
  std::printf("mpe_check: %s (%d failures)\n", failures ? "RED" : "GREEN", failures);
  return failures ? 1 : 0;
}
