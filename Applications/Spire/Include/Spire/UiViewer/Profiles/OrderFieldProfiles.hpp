#ifndef SPIRE_UI_VIEWER_ORDER_FIELD_PROFILES_HPP
#define SPIRE_UI_VIEWER_ORDER_FIELD_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the OrderFieldInfoTip. */
  UiProfile make_order_field_info_tip_profile();

  /** Returns a UiProfile for the OrderStatusBox. */
  UiProfile make_order_status_box_profile();

  /** Returns a UiProfile for the OrderStatusFilterPanel. */
  UiProfile make_order_status_filter_panel_profile();

  /** Returns a UiProfile for the OrderStatusListBox. */
  UiProfile make_order_status_list_box_profile();

  /** Returns a UiProfile for the OrderTypeBox. */
  UiProfile make_order_type_box_profile();

  /** Returns a UiProfile for the OrderTypeFilterPanel. */
  UiProfile make_order_type_filter_panel_profile();

  /** Returns a UiProfile for the PositionSideBox. */
  UiProfile make_position_side_box_profile();

  /** Returns a UiProfile for the PositionSideFilterPanel. */
  UiProfile make_position_side_filter_panel_profile();

  /** Returns a UiProfile for the SideBox. */
  UiProfile make_side_box_profile();

  /** Returns a UiProfile for the SideFilterPanel. */
  UiProfile make_side_filter_panel_profile();

  /** Returns a UiProfile for the TaskStateBox. */
  UiProfile make_task_state_box_profile();

  /** Returns a UiProfile for the TimeInForceBox. */
  UiProfile make_time_in_force_box_profile();

  /** Returns a UiProfile for the TimeInForceFilterPanel. */
  UiProfile make_time_in_force_filter_panel_profile();
}

#endif
