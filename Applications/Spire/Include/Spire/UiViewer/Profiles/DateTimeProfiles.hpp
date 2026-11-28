#ifndef SPIRE_UI_VIEWER_DATE_TIME_PROFILES_HPP
#define SPIRE_UI_VIEWER_DATE_TIME_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the CalendarDatePicker. */
  UiProfile make_calendar_date_picker_profile();

  /** Returns a UiProfile for the DateBox. */
  UiProfile make_date_box_profile();

  /** Returns a UiProfile for the DateFilterPanel. */
  UiProfile make_date_filter_panel_profile();

  /** Returns a UiProfile for the DurationBox. */
  UiProfile make_duration_box_profile();

  /** Returns a UiProfile for the TimeBox. */
  UiProfile make_time_box_profile();
}

#endif
