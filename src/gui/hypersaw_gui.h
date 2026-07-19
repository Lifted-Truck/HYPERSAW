/*
 * hypersaw_gui.h — the GUI seam (ADR-019 ratification amendment).
 *
 * This header is the ONLY thing the plugin shell knows about the GUI, and it
 * contains no webview, platform, or CLAP types. Swapping the webview for a
 * native backend (iPlug2/ImGui, the recorded fallback) means reimplementing
 * hypersaw_gui.mm against this same interface — nothing else moves.
 *
 * - VizSnapshot: plain-struct visualization feed, engine -> GUI.
 * - GuiHost: transport-agnostic callbacks, GUI -> engine (param sets are
 *   QUEUED by the shell onto the audio thread, never applied directly).
 */
#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace hypersaw
{

struct VizSnapshot
{
  bool active = false;
  int n = 0;
  int centerIdx = 0;
  double R = 0, RN = 0, psi = 0, sigma = 0, KsmS = 0, KsmP = 0;
  double phase[32] = {0};
  // dynamics layer (Phase 3)
  int topo = 0, poles = 1;
  double RA = 0, RB = 0, RQ = 0;
  int gravCount = 0;
  int gravRatio[4] = {0};
  int gravOct[4] = {0};
  double gravErr[4] = {0};
  bool gridActive = false, gridLockWarn = false;
  double gridU = 0;
  int gridRungs = 0;
  // SPECTRA layer (Phase 4): per-partial strip visualizer feed. partials/cloud
  // are the live P/M; partR/partAmp index by partial; partPhase is the cloud
  // phase grid, row-major [partial*cloud + voice] with cloud <= 7.
  bool spectra = false;
  int partials = 0, cloud = 0;
  double partR[24] = {0};
  double partAmp[24] = {0};
  double partPhase[24 * 7] = {0};
};

struct GuiHost
{
  std::function<VizSnapshot()> getViz;
  std::function<void(float *, int)> getSpectrum;  // log-spaced 0..1 bins
  std::function<std::string()> getParamsJson;            // {"<id>":value,...}
  std::function<void(uint32_t, double)> setParam;        // by frozen CLAP id
  std::function<void(uint32_t, bool)> gesture;           // id, begin
  std::function<std::string()> getStateJson;             // full provenance dump
  std::function<bool(const std::string &)> applyStateJson;
};

class HypersawGui
{
 public:
  explicit HypersawGui(GuiHost host);
  ~HypersawGui();

  HypersawGui(const HypersawGui &) = delete;
  HypersawGui &operator=(const HypersawGui &) = delete;

  bool attachToParent(void *parentView);  // NSView* on macOS
  void getSize(uint32_t &width, uint32_t &height) const;

 private:
  struct Impl;
  Impl *impl;
};

}  // namespace hypersaw
