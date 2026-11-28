#ifndef SPIRE_UI_VIEWER_MENU_PROFILES_HPP
#define SPIRE_UI_VIEWER_MENU_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the CheckButtonMenuItem. */
  UiProfile make_check_button_menu_item_profile();

  /** Returns a UiProfile for the ComboBox. */
  UiProfile make_context_menu_profile();

  /** Returns a UiProfile for the MenuButton. */
  UiProfile make_menu_button_profile();

  /** Returns a UiProfile for the MenuIconButton. */
  UiProfile make_menu_icon_button_profile();

  /** Returns a UiProfile for the MenuLabelButton. */
  UiProfile make_menu_label_button_profile();
}

#endif
