#ifndef SPIRE_UI_VIEWER_VENUE_PROFILES_HPP
#define SPIRE_UI_VIEWER_VENUE_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the VenueBox. */
  UiProfile make_venue_box_profile();

  /** Returns a UiProfile for the VenueFilterPanel. */
  UiProfile make_venue_filter_panel_profile();

  /** Returns a UiProfile for the VenueListBox. */
  UiProfile make_venue_list_box_profile();
}

#endif
