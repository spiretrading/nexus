#ifndef SPIRE_UI_VIEWER_DESTINATION_PROFILES_HPP
#define SPIRE_UI_VIEWER_DESTINATION_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the DestinationBox. */
  UiProfile make_destination_box_profile();

  /** Returns a UiProfile for the DestinationFilterPanel. */
  UiProfile make_destination_filter_panel_profile();

  /** Returns a UiProfile for the DestinationListBox. */
  UiProfile make_destination_list_box_profile();

  /** Returns a UiProfile for the DestinationListItem. */
  UiProfile make_destination_list_item_profile();
}

#endif
