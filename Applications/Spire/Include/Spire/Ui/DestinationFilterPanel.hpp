#ifndef SPIRE_DESTINATION_FILTER_PANEL_HPP
#define SPIRE_DESTINATION_FILTER_PANEL_HPP
#include "Spire/Ui/DestinationListBox.hpp"
#include "Spire/Ui/OpenFilterPanel.hpp"

namespace Spire {

  /** An OpenFilterPanel specialized for a DestinationListBox. */
  using DestinationFilterPanel = OpenFilterPanel<DestinationListBox>;

  /**
   * Returns a new DestinationFilterPanel with a default current model.
   * @param parent The parent widget.
   */
  DestinationFilterPanel* make_destination_filter_panel(
    QWidget* parent = nullptr);

  /**
   * Returns a new DestinationFilterPanel.
   * @param current The current list of selected destinations.
   * @param parent The parent widget.
   */
  DestinationFilterPanel* make_destination_filter_panel(
    std::shared_ptr<DestinationListModel> current,
    QWidget* parent = nullptr);
}

#endif
