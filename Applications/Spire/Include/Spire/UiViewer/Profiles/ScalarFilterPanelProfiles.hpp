#ifndef SPIRE_UI_VIEWER_SCALAR_FILTER_PANEL_PROFILES_HPP
#define SPIRE_UI_VIEWER_SCALAR_FILTER_PANEL_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the DecimalFilterPanel. */
  UiProfile make_decimal_filter_panel_profile();

  /** Returns a UiProfile for the DurationFilterPanel. */
  UiProfile make_duration_filter_panel_profile();

  /** Returns a UiProfile for the IntegerFilterPanel. */
  UiProfile make_integer_filter_panel_profile();

  /** Returns a UiProfile for the MoneyFilterPanel. */
  UiProfile make_money_filter_panel_profile();

  /** Returns a UiProfile for the PercentFilterPanel. */
  UiProfile make_percent_filter_panel_profile();

  /** Returns a UiProfile for the QuantityFilterPanel. */
  UiProfile make_quantity_filter_panel_profile();
}

#endif
