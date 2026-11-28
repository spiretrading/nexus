#ifndef SPIRE_UI_VIEWER_BUTTON_PROFILES_HPP
#define SPIRE_UI_VIEWER_BUTTON_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the CheckBox. */
  UiProfile make_check_box_profile();

  /** Returns a UiProfile for the DeleteIconButton. */
  UiProfile make_delete_icon_button_profile();

  /** Returns a UiProfile for the IconButton. */
  UiProfile make_icon_button_profile();

  /** Returns a UiProfile for the IconToggleButton. */
  UiProfile make_icon_toggle_button_profile();

  /** Returns a UiProfile for the LabelButton. */
  UiProfile make_label_button_profile();

  /** Returns a UiProfile for the RadioButton. */
  UiProfile make_radio_button_profile();

  /** Returns a UiProfile for the SwitchButton. */
  UiProfile make_switch_button_profile();
}

#endif
