#ifndef SPIRE_DESTINATION_FILTER_PANEL_HPP
#define SPIRE_DESTINATION_FILTER_PANEL_HPP
#include "Nexus/Definitions/Destination.hpp"
#include "Spire/Ui/OpenFilterPanelTemplate.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * An OpenFilterPanelTemplate specialized for a Nexus::Destination.
   */
  using DestinationFilterPanel = OpenFilterPanelTemplate<Nexus::Destination>;

  /**
   * Returns a new DestinationFilterPanel using an empty list of matches.
   * @param query_model The model used to query matches.
   * @param parent The parent widget.
   */
  DestinationFilterPanel* make_destination_filter_panel(
    std::shared_ptr<ComboBox::QueryModel> query_model, QWidget& parent);

  /**
   * Returns a new DestinationFilterPanel.
   * @param query_model The model used to query matches.
   * @param matches The list of destination values to match against.
   * @param mode The filter mode.
   * @param parent The parent widget.
   */
  DestinationFilterPanel* make_destination_filter_panel(
    std::shared_ptr<ComboBox::QueryModel> query_model,
    std::shared_ptr<ListModel<Nexus::Destination>> matches,
    std::shared_ptr<ValueModel<DestinationFilterPanel::Mode>> mode,
    QWidget& parent);
}

#endif
