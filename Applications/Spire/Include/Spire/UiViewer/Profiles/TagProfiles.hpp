#ifndef SPIRE_UI_VIEWER_TAG_PROFILES_HPP
#define SPIRE_UI_VIEWER_TAG_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the Tag. */
  UiProfile make_tag_profile();

  /** Returns a UiProfile for the TagBox. */
  UiProfile make_tag_box_profile();

  /** Returns a UiProfile for the TagComboBox. */
  UiProfile make_tag_combo_box_profile();
}

#endif
