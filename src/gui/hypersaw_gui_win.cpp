/*
 * hypersaw_gui_win.cpp — Windows (WebView2 via choc) backend of the GUI seam.
 * Same shape as the macOS .mm: everything platform-specific lives here.
 *
 * Status honesty: this backend is CI-compile-verified only — no Windows
 * machine in the loop yet. Runtime validation (host embed, focus, resize)
 * is a recorded Phase 2 residual alongside Reaper/Bitwig checks.
 */
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
    web->bind("hzGrabKeys", [this](const choc::value::ValueView &) -> choc::value::Value {
      if (HWND h = (HWND)web->getViewHandle()) SetFocus(h);
      return {};
    });
  }
};

HypersawGui::HypersawGui(GuiHost host) : impl(new Impl(std::move(host))) {}

HypersawGui::~HypersawGui() { delete impl; }

bool HypersawGui::attachToParent(void *parentView)
{
  if (!impl->web) return false;
  HWND parent = (HWND)parentView;
  HWND child = (HWND)impl->web->getViewHandle();
  if (!parent || !child) return false;
  SetWindowLongPtr(child, GWL_STYLE,
                   (GetWindowLongPtr(child, GWL_STYLE) | WS_CHILD) & ~(LONG_PTR)WS_POPUP);
  SetParent(child, parent);
  RECT r;
  GetClientRect(parent, &r);
  SetWindowPos(child, nullptr, 0, 0, r.right - r.left, r.bottom - r.top,
               SWP_NOZORDER | SWP_SHOWWINDOW);
  return true;
}

void HypersawGui::getSize(uint32_t &width, uint32_t &height) const
{
  width = detail::kGuiWidth;
  height = detail::kGuiHeight;
}

}  // namespace hypersaw

#endif  // _WIN32
