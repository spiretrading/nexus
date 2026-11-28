#ifndef SPIRE_UI_VIEWER_DISPLAY_PROFILES_HPP
#define SPIRE_UI_VIEWER_DISPLAY_PROFILES_HPP
#include "Spire/UiViewer/UiProfile.hpp"

namespace Spire {

  /** Returns a UiProfile for the Identicon. */
  UiProfile make_identicon_profile();

  /** Returns a UiProfile for the Label. */
  UiProfile make_label_profile();

  /** Returns a UiProfile for displaying number in a label. */
  UiProfile make_number_label_profile();

  /** Returns a UiProfile for the ResponsiveLabel. */
  UiProfile make_responsive_label_profile();

  /** Returns a UiProfile for the TextAreaLabel. */
  UiProfile make_text_area_label_profile();

  /** Returns a UiProfile for the ProgressBar. */
  UiProfile make_progress_bar_profile();
}

#endif
