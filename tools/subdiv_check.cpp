/* subdiv_check — the engine must not care how a buffer is SUBDIVIDED.
 *
 * A host may deliver 2048 frames or 33; a plugin may split them further (the
 * multi-oscillator mix renders oscillator 0 whole and the rest in kMixChunk
 * pieces). None of that is a musical choice, so none of it may change a sample.
 *
 * The bug this exists for (ADR-086): gravity integrated once per render call
 * with dt = the block length. Explicit Euler on a nonlinear ODE, so one step of
 * dt and two of dt/2 disagreed — the same patch sounded different at different
 * host buffer sizes, and oscillator 0 drifted from oscillators 1..N.
 *
 * WHY THIS CANNOT BE A GOLDEN TEST. The golden generator renders a fixed buffer
 * and parity_check renders kBlock, so BOTH SIDES USE THE SAME SUBDIVISION and
 * parity agrees with itself. All 147 scenarios passed throughout. The property
 * is invisible to the oracle by construction; it needs its own check.
 *
 * There are at least two per-render-call integrators in this core (gravity, and
 * pan motion which is deliberately per-call — see swarm_core.h). A third added
 * without thought would silently reintroduce this, which is what the gate is
 * for.
 */
#include <cmath>
#include <cstdio>
#include <vector>
#include "../src/swarm_core.h"

static int failures = 0;
static void check(bool ok, const char *what, const char *detail)
{
  std::printf("%-6s %s  (%s)\n", ok ? "OK" : "FAIL", what, detail);
  if (!ok) failures++;
}

static std::vector<float> run(int chunk, double grav, double panMotion, int total)
{
  hypersaw::SwarmCore c(44100.0);
  c.setParam("seed", 1234);
  c.setParam("n", 5);
  c.setParam("grav", grav);
  c.setParam("basin", 50);
  c.setParam("panMotion", panMotion);
  c.noteOn(60, 261.6255653005986);
  c.noteOn(64, 329.6275569128699);
  c.noteOn(67, 391.99543598174927);
  std::vector<float> out(total);
  // Sized to `total`, not a fixed 4096: the reference case renders the whole
  // buffer in ONE call, and a fixed scratch overflowed it (SIGABRT on the first
  // run). A probe that crashes is at least honest; one that overflows quietly
  // would have "passed".
  std::vector<float> bL(total), bR(total);
  int done = 0;
  while (done < total)
  {
    const int m = total - done < chunk ? total - done : chunk;
    c.render(bL.data(), bR.data(), m);
    for (int i = 0; i < m; i++) out[done + i] = bL[i];
    done += m;
  }
  return out;
}

static double maxdiff(const std::vector<float> &a, const std::vector<float> &b)
{
  double m = 0;
  for (size_t i = 0; i < a.size() && i < b.size(); i++) m = std::fmax(m, std::fabs(a[i] - b[i]));
  return m;
}

int main()
{
  const int N = 44100;
  // Subdivisions a host or the mix stage might plausibly produce, including
  // sizes that are NOT multiples of the gravity grid — an accumulator that only
  // works on aligned blocks is not an accumulator.
  const int chunks[] = {N, 2048, 1024, 512, 256, 333, 127, 64};
  /* `ratified` says whether a failure here is a REGRESSION or a KNOWN open
     instance. ADR-086 ratified the fixed grid for GRAVITY only. Pan motion
     (ADR-064) is the same defect in a different parameter — measured 0.191 at
     chunk 333 — and keeping it per-call is what confined ADR-086's blast radius
     to what was approved. It is excluded LOUDLY rather than quietly: an
     undeclared exclusion is how a gate rots into decoration, and this file
     would otherwise assert something the codebase does not do. Fold it in the
     moment pan motion is ruled. */
  struct Case { const char *name; double grav; double pan; bool ratified; };
  const Case cases[] = {
      {"inert (no per-call integrator engaged)", 0.0, 0.0, true},
      {"gravity engaged (ADR-086)", 0.7, 0.0, true},
      {"pan motion engaged (ADR-064 — NOT yet ruled)", 0.0, 0.6, false},
      {"both engaged", 0.7, 0.6, false},
  };
  for (const auto &cs : cases)
  {
    const std::vector<float> ref = run(N, cs.grav, cs.pan, N);   // one whole call
    double worst = 0;
    int worstChunk = 0;
    for (int ch : chunks)
    {
      const double d = maxdiff(ref, run(ch, cs.grav, cs.pan, N));
      if (d > worst) { worst = d; worstChunk = ch; }
    }
    char detail[160];
    std::snprintf(detail, sizeof(detail), "worst %.10g at chunk %d", worst, worstChunk);
    if (cs.ratified) check(worst == 0.0, cs.name, detail);
    else
      std::printf("%-6s %s  (%s)\n", worst == 0.0 ? "OK" : "KNOWN", cs.name, detail);
  }
  std::printf("subdiv_check: %s (%d failures; pan motion excluded pending a ruling)\n",
              failures ? "RED" : "GREEN", failures);
  return failures ? 1 : 0;
}
