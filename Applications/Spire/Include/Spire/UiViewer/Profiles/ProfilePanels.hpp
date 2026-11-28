#ifndef SPIRE_UI_VIEWER_PROFILE_PANELS_HPP
#define SPIRE_UI_VIEWER_PROFILE_PANELS_HPP
#include "Spire/Ui/FilterPanel.hpp"

namespace Spire {

  /** Builds a sample filter panel with minimum and maximum fields. */
  FilterPanel* make_filter_panel();

  /**
   * Builds the body of a sample panel with open and close buttons.
   * @param button_name The label for the button that opens a panel.
   */
  QWidget* create_panel_body(const QString& button_name);
}

#endif
