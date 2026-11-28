#ifndef SPIRE_UI_VIEWER_ACCOUNT_PROFILES_HPP
#define SPIRE_UI_VIEWER_ACCOUNT_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the AccountBox. */
  UiProfile make_account_box_profile();

  /** Returns a UiProfile for the AccountFilterPanel. */
  UiProfile make_account_filter_panel_profile();

  /** Returns a UiProfile for the AccountListBox. */
  UiProfile make_account_list_box_profile();

  /** Returns a UiProfile for the AccountListItem. */
  UiProfile make_account_list_item_profile();
}

#endif
