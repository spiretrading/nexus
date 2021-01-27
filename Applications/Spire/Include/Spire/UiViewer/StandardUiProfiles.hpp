#ifndef SPIRE_STANDARD_UI_PROFILES_HPP
#define SPIRE_STANDARD_UI_PROFILES_HPP
#include "Spire/UiViewer/UiViewer.hpp"

namespace Spire {

  //! Returns a UiProfile for the CheckBox.
  UiProfile make_checkbox_profile();

  //! Returns a UiProfile for the ColorSelectorButton.
  UiProfile make_color_selector_button_profile();

  //! Returns a UiProfile for the CurrencyComboBox.
  UiProfile make_currency_combo_box_profile();

  //! Returns a UiProfile for the FlatButton.
  UiProfile make_flat_button_profile();

  //! Returns a UiProfile for the IconButton.
  UiProfile make_icon_button_profile();

  //! Returns a UiProfile for the TextBox.
  UiProfile make_text_box_profile();
}

#endif
