/*
 * hypersaw_gui.mm — macOS (WKWebView via choc) backend of the GUI seam.
 * Objective-C++ so the NSView attach is plain Cocoa; the JS bridge and viz
 * serializer are shared with the Windows backend (hypersaw_gui_common.h).
 * All bindings run on the main thread and touch only GuiHost callbacks.
 */

#import <Cocoa/Cocoa.h>

#include "hypersaw_gui_common.h"

namespace hypersaw
{

struct HypersawGui::Impl
{
  GuiHost host;
  std::unique_ptr<choc::ui::WebView> web;

  explicit Impl(GuiHost h) : host(std::move(h))
  {
    web = detail::makeWebView(host);
    // Hosts often do not hand the plugin view keyboard focus on click; the
    // GUI's text-entry path requests it explicitly (2026-07-18 report: edit
    // boxes lost focus instantly in Live — the INVERSE of the classic
    // webview-steals-keys problem).
    web->bind("hzGrabKeys", [this](const choc::value::ValueView &) -> choc::value::Value {
      NSView *v = (__bridge NSView *)web->getViewHandle();
      if (v && v.window)
      {
        // makeFirstResponder on a non-key window never receives keys — Live
        // keeps key status on its main window, so claim it first. Live also
        // re-takes it moments later (2026-07-18 report: "focus for a split
        // second"), which is why the JS side re-grabs for the edit's lifetime
        // rather than trusting one call.
        if (![v.window isKeyWindow]) [v.window makeKeyWindow];
        [v.window makeFirstResponder:v];
      }
      return {};
    });
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
  width = detail::kGuiWidth;
  height = detail::kGuiHeight;
}

}  // namespace hypersaw
