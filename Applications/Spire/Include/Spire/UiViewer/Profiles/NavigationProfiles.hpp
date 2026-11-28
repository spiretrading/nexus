#ifndef SPIRE_UI_VIEWER_NAVIGATION_PROFILES_HPP
#define SPIRE_UI_VIEWER_NAVIGATION_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the NavigationView. */
  UiProfile make_navigation_view_profile();

  /** Returns a UiProfile for the TabView. */
  UiProfile make_tab_view_profile();

  /** Returns a UiProfile for the TransitionView. */
  UiProfile make_transition_view_profile();
}

#endif
