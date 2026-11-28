#ifndef SPIRE_UI_VIEWER_TRADING_GROUP_PROFILES_HPP
#define SPIRE_UI_VIEWER_TRADING_GROUP_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the TradingGroupBox. */
  UiProfile make_trading_group_box_profile();

  /** Returns a UiProfile for the TradingGroupFilterPanel. */
  UiProfile make_trading_group_filter_panel_profile();

  /** Returns a UiProfile for the TradingGroupListBox. */
  UiProfile make_trading_group_list_box_profile();
}

#endif
