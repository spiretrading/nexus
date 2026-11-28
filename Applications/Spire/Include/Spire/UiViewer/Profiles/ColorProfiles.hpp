#ifndef SPIRE_UI_VIEWER_COLOR_PROFILES_HPP
#define SPIRE_UI_VIEWER_COLOR_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the ColorBox. */
  UiProfile make_color_box_profile();

  /** Returns a UiProfile for the ColorCodePanel. */
  UiProfile make_color_code_panel_profile();

  /** Returns a UiProfile for the ColorPicker. */
  UiProfile make_color_picker_profile();

  /** Returns a UiProfile for the EyeDropper. */
  UiProfile make_eye_dropper_profile();

  /** Returns a UiProfile for the HexColorBox. */
  UiProfile make_hex_color_box_profile();

  /** Returns a UiProfile for the HighlightBox. */
  UiProfile make_highlight_box_profile();

  /** Returns a UiProfile for the HighlightPicker. */
  UiProfile make_highlight_picker_profile();

  /** Returns a UiProfile for the HighlightSwatch. */
  UiProfile make_highlight_swatch_profile();
}

#endif
