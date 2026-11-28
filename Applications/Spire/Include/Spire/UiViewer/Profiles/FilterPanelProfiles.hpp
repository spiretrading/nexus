#ifndef SPIRE_UI_VIEWER_FILTER_PANEL_PROFILES_HPP
#define SPIRE_UI_VIEWER_FILTER_PANEL_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the ClosedFilterPanel. */
  UiProfile make_closed_filter_panel_profile();

  /** Returns a UiProfile for the FilterPanel. */
  UiProfile make_filter_panel_profile();

  /** Returns a UiProfile for the OpenFilterPanel. */
  UiProfile make_open_filter_panel_profile();
}

#endif
