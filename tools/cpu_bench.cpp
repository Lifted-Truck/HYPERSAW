/*
 * cpu_bench.cpp — what fraction of one core does the real voice path cost?
 *
 * WHY THIS EXISTS. ADR-082 makes a CPU claim about two oscillators that rests
 * on an ESTIMATE: current cost x oscillator count x a x4 min-spec derate
 * borrowed from the ADR-018 spike. A derate is not a measurement, and the
 * conclusion it produces ("3 oscillators + 2x oversampling is over budget") is
 * exactly the kind of comfortable-looking number that should be checked before
 * it decides anything. This runs the ACTUAL SwarmCore, not a model of it, so
 * pointing it at min-spec hardware answers the question directly.
 *
 * Reports % of ONE core against the E-6 envelope (44.1 kHz, 128-sample buffer,
 * budget 50%). Deterministic: fixed seed, fixed note order, no wall-clock
 * inside the render path — the timer only wraps it.
 *
 * Usage: cpu_bench [voices] [notes] [seconds]
 */
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cmath>
#include "../src/swarm_core.h"

int main(int argc, char **argv)
{
  const int voices = argc > 1 ? std::atoi(argv[1]) : 7;
  const int notes = argc > 2 ? std::atoi(argv[2]) : 8;
  const double secs = argc > 3 ? std::atof(argv[3]) : 8.0;
  const double sr = 44100.0;
  const int block = 128;               // the E-6 envelope's buffer size

  hypersaw::SwarmCore core(sr);
  core.setParam("n", voices);
  core.setParam("seed", 1234);
  // spread the notes so voices do not share a frequency and get optimised
  // into a degenerate case the real instrument never sees
  for (int i = 0; i < notes; i++)
    core.noteOn(45 + i * 3, 440.0 * std::pow(2.0, (45 + i * 3 - 69) / 12.0));

  std::vector<float> L(block), R(block);
  const long blocks = (long)(secs * sr / block);
  // one untimed pass so page faults and first-touch allocation land outside
  // the measurement
  for (int i = 0; i < 20; i++) core.render(L.data(), R.data(), block);

  const auto t0 = std::chrono::steady_clock::now();
  for (long b = 0; b < blocks; b++) core.render(L.data(), R.data(), block);
  const auto t1 = std::chrono::steady_clock::now();

  const double cpuS = std::chrono::duration<double>(t1 - t0).count();
  const double audioS = blocks * block / sr;
  const double pct = cpuS / audioS * 100.0;
  std::printf("cpu_bench: %d voices x %d notes = %d oscillators\n", voices, notes, voices * notes);
  std::printf("  audio rendered   %.2f s\n", audioS);
  std::printf("  cpu consumed     %.3f s\n", cpuS);
  std::printf("  %% of one core    %.2f%%   (E-6 budget 50%%, %.1fx realtime)\n",
              pct, audioS / cpuS);
  return 0;
}
