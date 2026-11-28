#ifndef SPIRE_UI_VIEWER_OBSERVER_PROFILES_HPP
#define SPIRE_UI_VIEWER_OBSERVER_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the FocusObserver. */
  UiProfile make_focus_observer_profile();

  /** Returns a UiProfile for the HoverObserver. */
  UiProfile make_hover_observer_profile();
}

#endif
