#ifndef SPIRE_UI_VIEWER_BOX_PROFILES_HPP
#define SPIRE_UI_VIEWER_BOX_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the AdaptiveBox. */
  UiProfile make_adaptive_box_profile();

  /** Returns a UiProfile for the Box. */
  UiProfile make_box_profile();
}

#endif
