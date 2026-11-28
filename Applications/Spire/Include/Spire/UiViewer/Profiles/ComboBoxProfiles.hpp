#ifndef SPIRE_UI_VIEWER_COMBO_BOX_PROFILES_HPP
#define SPIRE_UI_VIEWER_COMBO_BOX_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the ComboBox. */
  UiProfile make_combo_box_profile();

  /** Returns a UiProfile for the DropDownBox. */
  UiProfile make_drop_down_box_profile();

  /** Returns a UiProfile for the DropDownList. */
  UiProfile make_drop_down_list_profile();
}

#endif
