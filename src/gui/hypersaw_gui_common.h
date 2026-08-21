/*
 * hypersaw_gui_common.h — platform-independent half of the webview backend:
 * the JS bridge bindings and the viz JSON serializer. Included ONLY by the
 * per-platform backends (hypersaw_gui.mm, hypersaw_gui_win.cpp); the plugin
 * shell still sees nothing but hypersaw_gui.h (the ADR-019 seam).
 */
#pragma once

#include "hypersaw_gui.h"

#include "../../libs/choc/choc/gui/choc_WebView.h"
#include "gui_html.h"  // generated: kGuiHtml_data / kGuiHtml_size

#include <cstdio>
#include <memory>

namespace hypersaw::detail
{

constexpr uint32_t kGuiWidth = 980;
constexpr uint32_t kGuiHeight = 720;

inline choc::value::Value vizToValue(const VizSnapshot &v)
{
  // Structured choc::value crosses the bridge as a native JS object — no JSON
  // string on the C++ side, no JSON.parse per frame on the JS side. (The
  // former string path also hid a truncation bug — 2026-07-18.)
  auto obj = choc::value::createObject("Viz");
  obj.addMember("active", v.active);
  obj.addMember("n", (int32_t)v.n);
  obj.addMember("centerIdx", (int32_t)v.centerIdx);
  obj.addMember("R", v.R);
  obj.addMember("RN", v.RN);
  obj.addMember("psi", v.psi);
  obj.addMember("sigma", v.sigma);
  obj.addMember("KsmS", v.KsmS);
  obj.addMember("KsmP", v.KsmP);
  obj.addMember("topo", (int32_t)v.topo);
  obj.addMember("poles", (int32_t)v.poles);
  obj.addMember("RA", v.RA);
  obj.addMember("RB", v.RB);
  obj.addMember("RQ", v.RQ);
  obj.addMember("gridActive", v.gridActive);
  obj.addMember("gridLockWarn", v.gridLockWarn);
  obj.addMember("gridU", v.gridU);
  obj.addMember("gridRungs", (int32_t)v.gridRungs);
  auto grav = choc::value::createEmptyArray();
  for (int i = 0; i < v.gravCount; i++)
  {
    auto pair = choc::value::createEmptyArray();
    pair.addArrayElement((int32_t)v.gravRatio[i]);
    pair.addArrayElement((int32_t)v.gravOct[i]);
    pair.addArrayElement(v.gravErr[i]);
    grav.addArrayElement(pair);
  }
  obj.addMember("grav", grav);
  auto phase = choc::value::createEmptyArray();
  for (int i = 0; i < v.n && i < 32; i++) phase.addArrayElement(v.phase[i]);
  obj.addMember("phase", phase);
  // voice map + note monitor (SAW mode; empty arrays in SPECTRA mode)
  obj.addMember("sr", v.sampleRate);
  obj.addMember("vmF0", v.vmF0);
  auto vmVf = choc::value::createEmptyArray();
  auto vmEff = choc::value::createEmptyArray();
  auto vmPan = choc::value::createEmptyArray();
  if (!v.spectra)
    for (int i = 0; i < v.n && i < 32; i++)
    {
      vmVf.addArrayElement(v.vmVf[i]);
      vmEff.addArrayElement(v.vmEff[i]);
      vmPan.addArrayElement(v.vmPan[i]);
    }
  obj.addMember("vmVf", vmVf);
  obj.addMember("vmEff", vmEff);
  obj.addMember("vmPan", vmPan);
  auto notes = choc::value::createEmptyArray();
  for (int i = 0; i < v.nmCount; i++)
  {
    auto row = choc::value::createEmptyArray();
    row.addArrayElement((int32_t)v.nmMidi[i]);
    row.addArrayElement((int32_t)v.nmGate[i]);
    row.addArrayElement(v.nmEnv[i]);
    notes.addArrayElement(row);
  }
  obj.addMember("outPeak", v.outPeak);
  // Per-oscillator meters (B24). Serialised as an array so the strip count
  // is data, not a shape the GUI hardcodes — a third oscillator needs no
  // change here.
  auto peaks = choc::value::createEmptyArray();
  for (double pk : v.oscPeak) peaks.addArrayElement(pk);
  obj.addMember("oscPeak", peaks);
  auto eOn = choc::value::createEmptyArray();
  auto eAt = choc::value::createEmptyArray();
  auto eRe = choc::value::createEmptyArray();
  for (int i = 0; i < v.envCount && i < 32; i++)
  {
    eOn.addArrayElement(v.envOnsetMs[i]);
    eAt.addArrayElement(v.envAtkMs[i]);
    eRe.addArrayElement(v.envRelMs[i]);
  }
  obj.addMember("envOnset", eOn);
  obj.addMember("envAtk", eAt);
  obj.addMember("envRel", eRe);
  obj.addMember("notes", notes);
  // SPECTRA per-partial strip feed (empty in SAW mode — v.spectra gates it).
  obj.addMember("spectra", v.spectra);
  if (v.spectra)
  {
    obj.addMember("partials", (int32_t)v.partials);
    obj.addMember("cloud", (int32_t)v.cloud);
    auto pr = choc::value::createEmptyArray();
    auto pa = choc::value::createEmptyArray();
    auto pp = choc::value::createEmptyArray();
    for (int k = 0; k < v.partials && k < 32; k++)
    {
      pr.addArrayElement(v.partR[k]);
      pa.addArrayElement(v.partAmp[k]);
      for (int m = 0; m < v.cloud && m < 7; m++) pp.addArrayElement(v.partPhase[k * 7 + m]);
    }
    obj.addMember("partR", pr);
    obj.addMember("partAmp", pa);
    obj.addMember("partPhase", pp);  // flat [partial*cloud + voice]
  }
  return obj;
}

inline std::unique_ptr<choc::ui::WebView> makeWebView(GuiHost &host)
{
  choc::ui::WebView::Options opts;
  opts.enableDebugMode = false;
  opts.acceptsFirstMouseClick = true;  // click-through focus in hosts
  auto web = std::make_unique<choc::ui::WebView>(opts);

  web->bind("hzGetViz", [&host](const choc::value::ValueView &) -> choc::value::Value {
    return vizToValue(host.getViz());
  });
  web->bind("hzPanic", [&host](const choc::value::ValueView &) -> choc::value::Value {
    if (host.panic) host.panic();
    return choc::value::createInt32(1);
  });
  web->bind("hzGetBuild", [&host](const choc::value::ValueView &) -> choc::value::Value {
    return choc::value::createString(host.getBuildId ? host.getBuildId() : std::string("?"));
  });
  web->bind("hzReleaseKeyFocus", [&host](const choc::value::ValueView &) -> choc::value::Value {
    if (host.releaseKeyFocus) host.releaseKeyFocus();
    return choc::value::createInt32(1);
  });
  web->bind("hzGetHostHint", [&host](const choc::value::ValueView &) -> choc::value::Value {
    // Empty, not "?": absent hint means NOTHING TO SAY. A placeholder here would
    // render as a permanent warning badge on every load.
    return choc::value::createString(host.getHostHint ? host.getHostHint() : std::string());
  });
  web->bind("hzGetParams", [&host](const choc::value::ValueView &) -> choc::value::Value {
    return choc::value::createString(host.getParamsJson());
  });
  web->bind("hzGetBendCurve", [&host](const choc::value::ValueView &) -> choc::value::Value {
    return choc::value::createString(host.getBendCurveJson ? host.getBendCurveJson()
                                                           : std::string("{}"));
  });
  web->bind("hzMorphCapture", [&host](const choc::value::ValueView &args) -> choc::value::Value {
    if (host.morphCapture && args.isArray() && args.size() >= 1)
      host.morphCapture((uint32_t)args[0].getWithDefault<int64_t>(0));
    return {};
  });
  web->bind("hzGetShapeWave", [&host](const choc::value::ValueView &) -> choc::value::Value {
    return choc::value::createString(host.getShapeWaveJson ? host.getShapeWaveJson()
                                                           : std::string("{}"));
  });
  web->bind("hzGetDefaults", [&host](const choc::value::ValueView &) -> choc::value::Value {
    return choc::value::createString(host.getDefaultsJson ? host.getDefaultsJson()
                                                          : std::string("{}"));
  });
  web->bind("hzSetParam", [&host](const choc::value::ValueView &args) -> choc::value::Value {
    if (args.isArray() && args.size() >= 2)
      host.setParam((uint32_t)args[0].getWithDefault<int64_t>(0),
                    args[1].getWithDefault<double>(0.0));
    return {};
  });
  // The visuals follow the GUI's active oscillator. THIS BIND WAS MISSING until
  // 2026-08-08: the GuiHost member and the plugin-side assignment both landed,
  // the build was green, and window.hzSetVizOsc simply did not exist — so every
  // tab click threw and the viz stayed pinned to oscillator 0. A callback with
  // no bind is invisible to the compiler.
  web->bind("hzSetVizOsc", [&host](const choc::value::ValueView &args) -> choc::value::Value {
    if (host.setVizOsc && args.isArray() && args.size() >= 1)
      host.setVizOsc((uint32_t)args[0].getWithDefault<int64_t>(0));
    return {};
  });
  web->bind("hzGesture", [&host](const choc::value::ValueView &args) -> choc::value::Value {
    if (args.isArray() && args.size() >= 2)
      host.gesture((uint32_t)args[0].getWithDefault<int64_t>(0),
                   args[1].getWithDefault<bool>(false));
    return {};
  });
  // 256 bins, not 96 (human 2026-08-08: "choppy and ugly"). The analyzer is a
  // 2048-point FFT — ~1024 usable magnitudes — so 96 log bins were throwing
  // away most of what had already been computed; the cost of more bins is the
  // array marshal, not the transform.
  web->bind("hzGetSpec", [&host](const choc::value::ValueView &) -> choc::value::Value {
    constexpr int kBins = 256;
    float bins[kBins];
    host.getSpectrum(bins, kBins);
    auto arr = choc::value::createEmptyArray();
    for (int i = 0; i < kBins; i++) arr.addArrayElement(bins[i]);
    return arr;
  });
  web->bind("hzGetScope", [&host](const choc::value::ValueView &) -> choc::value::Value {
    // 1536, not 512: at D2 one period is ~604 samples, so a 512-sample window
    // cannot even hold one — there was nothing for a trigger to lock onto.
    constexpr int kN = 1536;
    float l[kN], r[kN];
    if (host.getScope) host.getScope(l, r, kN);
    else { for (int i = 0; i < kN; i++) { l[i] = 0; r[i] = 0; } }
    auto L = choc::value::createEmptyArray(), R = choc::value::createEmptyArray();
    for (int i = 0; i < kN; i++) { L.addArrayElement(l[i]); R.addArrayElement(r[i]); }
    auto obj = choc::value::createObject("Scope");
    obj.addMember("l", L); obj.addMember("r", R);
    return obj;
  });
  web->bind("hzGetState", [&host](const choc::value::ValueView &) -> choc::value::Value {
    return choc::value::createString(host.getStateJson());
  });
  web->bind("hzApplyState", [&host](const choc::value::ValueView &args) -> choc::value::Value {
    bool ok = false;
    if (args.isArray() && args.size() >= 1)
      ok = host.applyStateJson(std::string(args[0].getWithDefault<std::string_view>("")));
    return choc::value::createBool(ok);
  });

  web->setHTML(std::string(reinterpret_cast<const char *>(kGuiHtml_data), kGuiHtml_size));
  return web;
}

}  // namespace hypersaw::detail
