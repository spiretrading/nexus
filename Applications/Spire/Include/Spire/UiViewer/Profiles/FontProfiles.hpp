#ifndef SPIRE_UI_VIEWER_FONT_PROFILES_HPP
#define SPIRE_UI_VIEWER_FONT_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the FontBox. */
  UiProfile make_font_box_profile();

  /** Returns a UiProfile for the FontFamilyBox. */
  UiProfile make_font_family_box_profile();

  /** Returns a UiProfile for the FontStyleBox. */
  UiProfile make_font_style_box_profile();
}

#endif
