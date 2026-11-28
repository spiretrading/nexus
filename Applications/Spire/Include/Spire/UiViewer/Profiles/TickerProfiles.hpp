#ifndef SPIRE_UI_VIEWER_TICKER_PROFILES_HPP
#define SPIRE_UI_VIEWER_TICKER_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the TickerBox. */
  UiProfile make_ticker_box_profile();

  /** Returns a UiProfile for the TickerFilterPanel. */
  UiProfile make_ticker_filter_panel_profile();

  /** Returns a UiProfile for the TickerListBox. */
  UiProfile make_ticker_list_box_profile();

  /** Returns a UiProfile for the TickerListItem. */
  UiProfile make_ticker_list_item_profile();

  /** Returns a UiProfile for the TickerView. */
  UiProfile make_ticker_view_profile();
}

#endif
