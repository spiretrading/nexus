#ifndef SPIRE_UI_VIEWER_SCOPE_PROFILES_HPP
#define SPIRE_UI_VIEWER_SCOPE_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the ScopeBox. */
  UiProfile make_scope_box_profile();

  /** Returns a UiProfile for the ScopeDropDownBox. */
  UiProfile make_scope_drop_down_box_profile();

  /** Returns a UiProfile for the ScopeFilterPanel. */
  UiProfile make_scope_filter_panel_profile();

  /** Returns a UiProfile for the ScopeListItem. */
  UiProfile make_scope_list_item_profile();
}

#endif
