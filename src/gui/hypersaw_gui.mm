/*
 * hypersaw_gui.mm — choc-webview implementation of the GUI seam (macOS).
 *
 * Everything platform- and webview-specific lives here and only here
 * (ADR-019 amendment). Objective-C++ so the NSView attach is plain Cocoa.
 * The JS side talks through five bound functions (hzGetViz, hzGetParams,
 * hzSetParam, hzGesture, hzGetState, hzApplyState); all of them run on the
 * main thread (choc dispatches bindings there) and only touch the GuiHost
 * callbacks — never engine internals.
 */

#import <Cocoa/Cocoa.h>

#include "hypersaw_gui.h"

#include "../../libs/choc/choc/gui/choc_WebView.h"
#include "gui_html.h"  // generated: kGuiHtml_data / kGuiHtml_size

#include <cstdio>

namespace hypersaw
{

namespace
{
constexpr uint32_t kGuiWidth = 780;
constexpr uint32_t kGuiHeight = 480;

std::string vizToJson(const VizSnapshot &v)
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
}  // namespace

struct HypersawGui::Impl
{
  GuiHost host;
  std::unique_ptr<choc::ui::WebView> web;

  explicit Impl(GuiHost h) : host(std::move(h))
  {
    choc::ui::WebView::Options opts;
    opts.enableDebugMode = false;
    web = std::make_unique<choc::ui::WebView>(opts);

    web->bind("hzGetViz", [this](const choc::value::ValueView &) -> choc::value::Value {
      return choc::value::createString(vizToJson(host.getViz()));
    });
    web->bind("hzGetParams", [this](const choc::value::ValueView &) -> choc::value::Value {
      return choc::value::createString(host.getParamsJson());
    });
    web->bind("hzSetParam", [this](const choc::value::ValueView &args) -> choc::value::Value {
      if (args.isArray() && args.size() >= 2)
        host.setParam((uint32_t)args[0].getWithDefault<int64_t>(0),
                      args[1].getWithDefault<double>(0.0));
      return {};
    });
    web->bind("hzGesture", [this](const choc::value::ValueView &args) -> choc::value::Value {
      if (args.isArray() && args.size() >= 2)
        host.gesture((uint32_t)args[0].getWithDefault<int64_t>(0),
                     args[1].getWithDefault<bool>(false));
      return {};
    });
    web->bind("hzGetState", [this](const choc::value::ValueView &) -> choc::value::Value {
      return choc::value::createString(host.getStateJson());
    });
    web->bind("hzApplyState", [this](const choc::value::ValueView &args) -> choc::value::Value {
      bool ok = false;
      if (args.isArray() && args.size() >= 1)
        ok = host.applyStateJson(std::string(args[0].getWithDefault<std::string_view>("")));
      return choc::value::createBool(ok);
    });

    web->setHTML(std::string(reinterpret_cast<const char *>(kGuiHtml_data), kGuiHtml_size));
  }
};

HypersawGui::HypersawGui(GuiHost host) : impl(new Impl(std::move(host))) {}

HypersawGui::~HypersawGui() { delete impl; }

bool HypersawGui::attachToParent(void *parentView)
{
  if (!impl->web) return false;
  NSView *parent = (__bridge NSView *)parentView;
  NSView *child = (__bridge NSView *)impl->web->getViewHandle();
  if (!parent || !child) return false;
  [child setFrame:[parent bounds]];
  [child setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  [parent addSubview:child];
  return true;
}

void HypersawGui::getSize(uint32_t &width, uint32_t &height) const
{
  width = kGuiWidth;
  height = kGuiHeight;
}

}  // namespace hypersaw
