/* kstuck_probe — does a note fail to die at, or after, high COUPLING?
 *
 * Field report 2026-08-12: "It seems to be triggered by increasing K, and then
 * it persists even after K goes back down."
 *
 * WHY NO EXISTING GATE COULD SEE THIS. Every note-lifecycle oracle we have runs
 * at ONE value of K — notefuzz_check sends ids 21/22/32/34 (and the 2-osc
 * envelope set) and never touches id 6. So the entire coupling axis is
 * untested for note lifecycle, in an instrument whose whole thesis IS the
 * coupling. That is the same shape as the oscillator-2 blindness: a parameter
 * the oracle never moves is a parameter the oracle cannot indict.
 *
 * TWO READINGS OF "PERSISTS", AND THEY WANT DIFFERENT FIXES, so both are
 * measured separately rather than assumed:
 *   (A) a note that hung WHILE K was high stays hung after K drops — one stuck
 *       voice, latched;
 *   (B) the SUSCEPTIBILITY persists — notes played later, at low K, also hang,
 *       which means raising K corrupted state that outlives it.
 *
 * WHAT A GREEN RUN HERE DOES **NOT** RULE OUT — read this before concluding the
 * report was wrong. This probe delivers K changes as host->plugin `in_events`.
 * A GUI KNOB DOES NOT TAKE THAT PATH: it calls `enqueueParam`, and `drainQueue`
 * then pushes param events OUTWARD to the host as well. That outward flood is
 * the exact mechanism of L0022, where a full host output buffer destroyed a
 * NOTE_END permanently and presented as a stuck note "worst when I've recently
 * changed the K value". So the reported trigger — a hand on the K knob — runs
 * through code this file cannot reach. Reproducing it needs a hook onto
 * `enqueueParam`, the way trace_check got one onto panic.
 *
 * CONTROLS (L0032). The same sequence at default K must read silent, and a
 * never-released note must read LOUD — without the first, "silent" might be a
 * broken measurement; without the second, "silent" might mean nothing sounded
 * at all.
 */
#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <clap/clap.h>
#include "../src/hypersaw_clap_entry.h"

namespace {
#include "notefuzz_scaffold.inc"

int failures = 0;
void check(bool ok, const char *what, const char *detail)
{
  std::printf("%-6s %s  (%s)\n", ok ? "OK" : "FAIL", what, detail);
  if (!ok) failures++;
}
}  // namespace

int main()
{
  hypersaw_entry_init("");
  auto *factory = (const clap_plugin_factory_t *)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  const clap_plugin_t *p = factory->create_plugin(factory, &kHost, "com.lifted-truck.hypersaw");
  p->init(p);
  p->activate(p, kSR, 32, 2048);
  p->start_processing(p);

  std::vector<float> L(kBlock), R(kBlock);
  float *chans[2] = {L.data(), R.data()};
  clap_audio_buffer_t out{};
  out.data32 = chans;
  out.channel_count = 2;
  int64_t blkIdx = 0;
  double lastPeak = 0;

  auto run = [&](EvList &e, int blocks) {
    e.finalize();
    lastPeak = 0;
    for (int b = 0; b < blocks; b++)
    {
      clap_process_t proc{};
      proc.audio_inputs_count = 0;
      proc.audio_outputs_count = 1;
      proc.audio_outputs = &out;
      proc.frames_count = kBlock;
      proc.in_events = &e.list;
      proc.out_events = &kOut;
      proc.steady_time = blkIdx++ * kBlock;
      p->process(p, &proc);
      e.notes.clear();
      e.params.clear();
      e.finalize();
      for (int i = 0; i < kBlock; i++)
      {
        const double m = std::fmax(std::fabs((double)L[i]), std::fabs((double)R[i]));
        if (m > lastPeak) lastPeak = m;
      }
    }
    return lastPeak;
  };
  auto idle = [&](double seconds) { EvList e; return run(e, Math_blocks(seconds)); };
  /* SETTLE FIRST, THEN MEASURE. `run` returns the peak over its WHOLE window,
     so measuring across the second that begins at note-off reads the release
     tail and calls it a stuck note — which is exactly what the first version of
     this probe did, and its control caught it. A residue measured beside the
     event that produces it is not a residue (L0016). */
  auto tailAfter = [&](double settle, double window) {
    idle(settle);
    return idle(window);
  };
  auto note = [&](uint16_t type, int key) {
    EvList e;
    e.notes.push_back(mkNote(type, 0, (int16_t)key, -1));
    return run(e, 1);
  };
  auto setK = [&](double k) {
    EvList e;
    e.params.push_back(mkParam(6, k));
    return run(e, 1);
  };
  // A sweep, not a jump: the report says "increasing K", and a knob move is a
  // stream of values. If the latch needs the intermediate states, a single
  // store would miss it entirely.
  auto sweepK = [&](double from, double to, int steps) {
    for (int i = 0; i <= steps; i++)
    {
      EvList e;
      e.params.push_back(mkParam(6, from + (to - from) * (double)i / steps));
      run(e, 1);
    }
  };

  /* CONFIGURATION IS THE EXPERIMENT, NOT SCENERY. The first version of this
     probe left oscillator 2 at vol 0 and ran poly — i.e. it tested a
     one-oscillator poly instrument, which is precisely the blindness that hid
     the last stuck note for weeks. `hard` turns on the conditions that bug
     actually needed: oscillator 2 AUDIBLE (id 1017 — vol is per-oscillator, so
     the base id would only re-set oscillator 1) with its envelope DIVERGED from
     oscillator 1's, plus mono + legato. A negative result from the easy
     configuration is worth very little; both are run. */
  auto freshPlugin = [&](bool hard) {
    p->reset(p);
    EvList e;
    e.params.push_back(mkParam(22, 0.02));   // short release: dead means dead
    e.params.push_back(mkParam(21, 1.0));    // sustain full
    e.params.push_back(mkParam(6, 0.0));     // K = 0
    if (hard)
    {
      e.params.push_back(mkParam(32, 1.0));    // mono
      e.params.push_back(mkParam(34, 1.0));    // legato
      e.params.push_back(mkParam(1017, 0.4));  // oscillator 2 AUDIBLE
      e.params.push_back(mkParam(1019, 0.15)); // ...envelope diverged from osc 1
      e.params.push_back(mkParam(1020, 0.40));
      e.params.push_back(mkParam(1021, 0.60));
      e.params.push_back(mkParam(1022, 0.03));
    }
    run(e, 4);
  };

  // How loud is "still sounding"? A held note gives the scale.
  freshPlugin(false);
  note(CLAP_EVENT_NOTE_ON, 60);
  const double heldLoud = idle(0.20);
  note(CLAP_EVENT_NOTE_OFF, 60);
  const double afterOff = tailAfter(0.5, 0.5);
  char d[240];
  std::snprintf(d, sizeof(d), "held peak %.4f, then %.6f one second after note-off", heldLoud, afterOff);
  check(heldLoud > 0.02 && afterOff < 1e-3,
        "CONTROL at K=0: a released note goes silent (and a held one is loud)", d);
  const double kSilent = 1e-3;

  for (int cfg = 0; cfg < 2; cfg++)
  {
  const bool hard = cfg == 1;
  const char *tag = hard ? " [mono+legato, osc2 audible, envelopes diverged]" : " [poly, 1 osc]";
  std::string suffix(tag);

  // ---- A. a note released WHILE K is high -------------------------------
  {
    freshPlugin(hard);
    sweepK(0.0, 1.0, 24);
    note(CLAP_EVENT_NOTE_ON, 60);
    idle(0.30);
    note(CLAP_EVENT_NOTE_OFF, 60);
    const double hi = tailAfter(0.5, 0.5);
    sweepK(1.0, 0.0, 24);                    // ...and K comes back down
    const double afterDrop = tailAfter(0.5, 0.5);
    std::snprintf(d, sizeof(d), "1 s after note-off at high K: %.6f; after K returns to 0: %.6f",
                  hi, afterDrop);
    check(hi < kSilent && afterDrop < kSilent,
          (std::string("A: a note released at HIGH K dies, and stays dead when K drops") + suffix).c_str(), d);
  }

  // ---- B. does raising K poison later notes? ------------------------------
  {
    freshPlugin(hard);
    sweepK(0.0, 1.0, 24);
    note(CLAP_EVENT_NOTE_ON, 67);
    idle(0.30);
    note(CLAP_EVENT_NOTE_OFF, 67);
    idle(0.50);
    sweepK(1.0, 0.0, 24);                    // K all the way back down
    idle(0.30);
    note(CLAP_EVENT_NOTE_ON, 62);            // a NEW note, at low K
    idle(0.30);
    note(CLAP_EVENT_NOTE_OFF, 62);
    const double after = tailAfter(0.5, 0.5);
    std::snprintf(d, sizeof(d), "note played AFTER a K excursion, 1 s past its release: %.6f", after);
    check(after < kSilent, (std::string("B: a note played after K goes back down still dies") + suffix).c_str(), d);
  }

  // ---- C. K swept WHILE the note is held ---------------------------------
  // The report says "increasing K" triggers it, and a player raises K with a
  // note sounding. That is a different ordering from A, where K was already
  // high before the note existed.
  {
    freshPlugin(hard);
    note(CLAP_EVENT_NOTE_ON, 64);
    idle(0.15);
    sweepK(0.0, 1.0, 24);                    // raise K under a sounding note
    idle(0.30);
    note(CLAP_EVENT_NOTE_OFF, 64);
    const double held = tailAfter(0.5, 0.5);
    sweepK(1.0, 0.0, 24);
    const double dropped = tailAfter(0.5, 0.5);
    std::snprintf(d, sizeof(d), "K raised UNDER a held note; 1 s after its release %.6f, "
                                "after K returns %.6f", held, dropped);
    check(held < kSilent && dropped < kSilent,
          (std::string("C: a note whose K was raised while sounding still dies") + suffix).c_str(), d);
  }

  // ---- D. the same, polyphonically and repeatedly -------------------------
  // One note may not fill the pool or reach the tier-2/3 paths. A chord played
  // repeatedly across a K excursion is closer to what a player actually did.
  {
    freshPlugin(hard);
    for (int round = 0; round < 4; round++)
    {
      sweepK(0.0, 1.0, 12);
      for (int k = 0; k < 6; k++) { note(CLAP_EVENT_NOTE_ON, 60 + 2 * k); idle(0.02); }
      idle(0.20);
      for (int k = 0; k < 6; k++) { note(CLAP_EVENT_NOTE_OFF, 60 + 2 * k); idle(0.02); }
      sweepK(1.0, 0.0, 12);
      idle(0.20);
    }
    const double after = tailAfter(0.5, 1.0);
    std::snprintf(d, sizeof(d), "4 rounds of chord + K sweep, 1.5 s after the last release: %.6f", after);
    check(after < kSilent, (std::string("D: repeated chords across K excursions all die") + suffix).c_str(), d);
  }

  }  // config loop

  p->stop_processing(p);
  p->deactivate(p);
  p->destroy(p);
  std::printf("kstuck_probe: %s (%d failures)\n", failures ? "RED" : "GREEN", failures);
  return failures ? 1 : 0;
}
