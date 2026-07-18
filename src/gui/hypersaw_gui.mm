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

  explicit Impl(GuiHost h) : host(std::move(h)) { web = detail::makeWebView(host); }
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
