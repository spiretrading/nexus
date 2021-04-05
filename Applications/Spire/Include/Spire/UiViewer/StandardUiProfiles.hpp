#ifndef SPIRE_STANDARD_UI_PROFILES_HPP
#define SPIRE_STANDARD_UI_PROFILES_HPP
#include <Qt>
#include "Spire/UiViewer/UiViewer.hpp"

namespace Spire {

  //! Returns a UiProfile for the Box.
  UiProfile make_box_profile();

  //! Returns a UiProfile for the CheckBox.
  UiProfile make_checkbox_profile();

  //! Returns a UiProfile for the ColorSelectorButton.
  UiProfile make_color_selector_button_profile();

  //! Returns a UiProfile for the CurrencyComboBox.
  UiProfile make_currency_combo_box_profile();

  //! Returns a UiProfile for the DecimalBox.
  UiProfile make_decimal_box_profile();

  //! Returns a UiProfile for the DurationBox.
  UiProfile make_duration_box_profile();

  //! Returns a UiProfile for the FlatButton.
  UiProfile make_flat_button_profile();

  //! Returns a UiProfile for the IconButton.
  UiProfile make_icon_button_profile();

  //! Returns a UiProfile for the IntegerBox.
  UiProfile make_integer_box_profile();

  //! Returns a UiProfile for the ScrollBar.
  UiProfile make_scroll_bar_profile();

  //! Returns a UiProfile for the TextBox.
  UiProfile make_text_box_profile();

  //! Returns a UiProfile for the TimeBox.
  UiProfile make_time_box_profile();

  //! Returns a UiProfile for the Tooltip.
  UiProfile make_tooltip_profile();
}

#endif
