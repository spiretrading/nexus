#ifndef SPIRE_UI_VIEWER_INPUT_BOX_PROFILES_HPP
#define SPIRE_UI_VIEWER_INPUT_BOX_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the EditableBox. */
  UiProfile make_editable_box_profile();

  /** Returns a UiProfile for the InputBox. */
  UiProfile make_input_box_profile();

  /** Returns a UiProfile for the SearchBox. */
  UiProfile make_search_box_profile();

  /** Returns a UiProfile for the TextAreaBox. */
  UiProfile make_text_area_box_profile();

  /** Returns a UiProfile for the TextBox. */
  UiProfile make_text_box_profile();
}

#endif
