#ifndef SPIRE_STANDARD_UI_PROFILES_HPP
#define SPIRE_STANDARD_UI_PROFILES_HPP
#include <Qt>
#include "Spire/UiViewer/UiViewer.hpp"

namespace Spire {

  /** Returns a UiProfile for the Box. */
  UiProfile make_box_profile();

  /** Returns a UiProfile for the CheckBox. */
  UiProfile make_check_box_profile();

  /** Returns a UiProfile for the ColorSelectorButton. */
  UiProfile make_color_selector_button_profile();

  /** Returns a UiProfile for the CurrencyComboBox. */
  UiProfile make_currency_combo_box_profile();

  /** Returns a UiProfile for the DecimalBox. */
  UiProfile make_decimal_box_profile();

  /** Returns a UiProfile for the DecimalFilterPanel. */
  UiProfile make_decimal_filter_panel_profile();

  /** Returns a UiProfile for the DurationBox. */
  UiProfile make_duration_box_profile();

  /** Returns a UiProfile for the FilterPanel. */
  UiProfile make_filter_panel_profile();

  /** Returns a UiProfile for the IconButton. */
  UiProfile make_icon_button_profile();

  /** Returns a UiProfile for the IntegerBox. */
  UiProfile make_integer_box_profile();

  /** Returns a UiProfile for the IntegerFilterPanel. */
  UiProfile make_integer_filter_panel_profile();

  /** Returns a UiProfile for the KeyTag. */
  UiProfile make_key_tag_profile();

  /** Returns a UiProfile for the LabelButton. */
  UiProfile make_label_button_profile();

  /** Returns a UiProfile for the ListItem. */
  UiProfile make_list_item_profile();

  /** Returns a UiProfile for the ListView. */
  UiProfile make_list_view_profile();

  /** Returns a UiProfile for the MoneyBox. */
  UiProfile make_money_box_profile();

  /** Returns a UiProfile for the MoneyFilterPanel. */
  UiProfile make_money_filter_panel_profile();

  /** Returns a UiProfile for the OverlayPanel. */
  UiProfile make_overlay_panel_profile();

  /** Returns a UiProfile for the ScrollBar. */
  UiProfile make_scroll_bar_profile();

  /** Returns a UiProfile for the ScrollBox. */
  UiProfile make_scroll_box_profile();

  /** Returns a UiProfile for the TextBox. */
  UiProfile make_text_box_profile();

  /** Returns a UiProfile for the TimeBox. */
  UiProfile make_time_box_profile();

  /** Returns a UiProfile for the Tooltip. */
  UiProfile make_tooltip_profile();
}

#endif
