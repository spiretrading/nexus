#ifndef SPIRE_VENUE_FILTER_PANEL_HPP
#define SPIRE_VENUE_FILTER_PANEL_HPP
#include "Spire/Ui/OpenFilterPanel.hpp"
#include "Spire/Ui/VenueListBox.hpp"

namespace Spire {

  /** An OpenFilterPanel specialized for a VenueListBox. */
  using VenueFilterPanel = OpenFilterPanel<VenueListBox>;

  /**
   * Returns a new VenueFilterPanel with a default current model.
   * @param parent The parent widget.
   */
  VenueFilterPanel* make_venue_filter_panel(QWidget* parent = nullptr);

  /**
   * Returns a new VenueFilterPanel.
   * @param current The current list of selected venues.
   * @param parent The parent widget.
   */
  VenueFilterPanel* make_venue_filter_panel(
    std::shared_ptr<VenueListModel> current, QWidget* parent = nullptr);
}

#endif
