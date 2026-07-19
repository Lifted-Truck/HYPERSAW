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
  // SPECTRA per-partial strip feed (empty in SAW mode — v.spectra gates it).
  obj.addMember("spectra", v.spectra);
  if (v.spectra)
  {
    obj.addMember("partials", (int32_t)v.partials);
    obj.addMember("cloud", (int32_t)v.cloud);
    auto pr = choc::value::createEmptyArray();
    auto pa = choc::value::createEmptyArray();
    auto pp = choc::value::createEmptyArray();
    for (int k = 0; k < v.partials && k < 24; k++)
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
  web->bind("hzGetParams", [&host](const choc::value::ValueView &) -> choc::value::Value {
    return choc::value::createString(host.getParamsJson());
  });
  web->bind("hzSetParam", [&host](const choc::value::ValueView &args) -> choc::value::Value {
    if (args.isArray() && args.size() >= 2)
      host.setParam((uint32_t)args[0].getWithDefault<int64_t>(0),
                    args[1].getWithDefault<double>(0.0));
    return {};
  });
  web->bind("hzGesture", [&host](const choc::value::ValueView &args) -> choc::value::Value {
    if (args.isArray() && args.size() >= 2)
      host.gesture((uint32_t)args[0].getWithDefault<int64_t>(0),
                   args[1].getWithDefault<bool>(false));
    return {};
  });
  web->bind("hzGetSpec", [&host](const choc::value::ValueView &) -> choc::value::Value {
    float bins[96];
    host.getSpectrum(bins, 96);
    auto arr = choc::value::createEmptyArray();
    for (int i = 0; i < 96; i++) arr.addArrayElement(bins[i]);
    return arr;
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
