#ifndef SPIRE_UI_VIEWER_OVERLAY_PROFILES_HPP
#define SPIRE_UI_VIEWER_OVERLAY_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the IntoPanel. */
  UiProfile make_info_panel_profile();

  /** Returns a UiProfile for the IntoTip. */
  UiProfile make_info_tip_profile();

  /** Returns a UiProfile for the OverlayPanel. */
  UiProfile make_overlay_panel_profile();

  /** Returns a UiProfile for the PopupBox. */
  UiProfile make_popup_box_profile();

  /** Returns a UiProfile for the Tooltip. */
  UiProfile make_tooltip_profile();
}

#endif
