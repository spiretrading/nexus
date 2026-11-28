#ifndef SPIRE_UI_VIEWER_WINDOW_PROFILES_HPP
#define SPIRE_UI_VIEWER_WINDOW_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the TitleBar. */
  UiProfile make_title_bar_profile();

  /** Returns a UiProfile for the WindowHighlight. */
  UiProfile make_window_highlight_profile();
}

#endif
