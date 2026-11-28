#ifndef SPIRE_UI_VIEWER_LIST_PROFILES_HPP
#define SPIRE_UI_VIEWER_LIST_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the DeletableListItem. */
  UiProfile make_deletable_list_item_profile();

  /** Returns a UiProfile for the ListItem. */
  UiProfile make_list_item_profile();

  /** Returns a UiProfile for the ListView. */
  UiProfile make_list_view_profile();

  /** Returns a UiProfile for the ScrollableListBox. */
  UiProfile make_scrollable_list_box_profile();
}

#endif
