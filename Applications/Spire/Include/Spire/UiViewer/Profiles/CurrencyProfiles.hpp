#ifndef SPIRE_UI_VIEWER_CURRENCY_PROFILES_HPP
#define SPIRE_UI_VIEWER_CURRENCY_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the CurrencyBox. */
  UiProfile make_currency_box_profile();

  /** Returns a UiProfile for the CurrencyFilterPanel. */
  UiProfile make_currency_filter_panel_profile();

  /** Returns a UiProfile for the CurrencyListBox. */
  UiProfile make_currency_list_box_profile();

  /** Returns a UiProfile for the CurrencyListItem. */
  UiProfile make_currency_list_item_profile();
}

#endif
