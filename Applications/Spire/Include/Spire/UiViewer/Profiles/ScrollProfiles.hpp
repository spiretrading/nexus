#ifndef SPIRE_UI_VIEWER_SCROLL_PROFILES_HPP
#define SPIRE_UI_VIEWER_SCROLL_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the ScrollBar. */
  UiProfile make_scroll_bar_profile();

  /** Returns a UiProfile for the ScrollBox. */
  UiProfile make_scroll_box_profile();
}

#endif
