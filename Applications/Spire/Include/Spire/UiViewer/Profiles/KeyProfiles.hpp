#ifndef SPIRE_UI_VIEWER_KEY_PROFILES_HPP
#define SPIRE_UI_VIEWER_KEY_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the KeyFilterPanel. */
  UiProfile make_key_filter_panel_profile();

  /** Returns a UiProfile for the KeyInputBox. */
  UiProfile make_key_input_box_profile();

  /** Returns a UiProfile for the KeyListBox. */
  UiProfile make_key_list_box_profile();

  /** Returns a UiProfile for the KeyTag. */
  UiProfile make_key_tag_profile();
}

#endif
