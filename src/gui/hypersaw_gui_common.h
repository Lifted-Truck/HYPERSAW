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

constexpr uint32_t kGuiWidth = 920;
constexpr uint32_t kGuiHeight = 600;

inline std::string vizToJson(const VizSnapshot &v)
{
  std::string out;
  out.reserve(1024);
  char buf[64];
  std::snprintf(buf, sizeof(buf), "{\"active\":%s,\"n\":%d,\"centerIdx\":%d,",
                v.active ? "true" : "false", v.n, v.centerIdx);
  out += buf;
  std::snprintf(buf, sizeof(buf), "\"R\":%.4f,\"RN\":%.4f,\"psi\":%.4f,\"sigma\":%.2f,", v.R,
                v.RN, v.psi, v.sigma);
  out += buf;
  std::snprintf(buf, sizeof(buf), "\"KsmS\":%.3f,\"KsmP\":%.3f,\"phase\":[", v.KsmS, v.KsmP);
  out += buf;
  for (int i = 0; i < v.n && i < 32; i++)
  {
    std::snprintf(buf, sizeof(buf), i ? ",%.5f" : "%.5f", v.phase[i]);
    out += buf;
  }
  out += "]}";
  return out;
}

inline std::unique_ptr<choc::ui::WebView> makeWebView(GuiHost &host)
{
  choc::ui::WebView::Options opts;
  opts.enableDebugMode = false;
  auto web = std::make_unique<choc::ui::WebView>(opts);

  web->bind("hzGetViz", [&host](const choc::value::ValueView &) -> choc::value::Value {
    return choc::value::createString(vizToJson(host.getViz()));
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
