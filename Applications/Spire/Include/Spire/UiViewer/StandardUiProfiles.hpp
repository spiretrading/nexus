#ifndef SPIRE_STANDARD_UI_PROFILES_HPP
#define SPIRE_STANDARD_UI_PROFILES_HPP
#include <span>
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Constructs a profile for a widget in the UI Viewer. */
  using UiProfileFactory = UiProfile (*)();

  /** Returns the standard profile factories in viewer display order. */
  std::span<const UiProfileFactory> get_standard_ui_profile_factories();
}

#endif
