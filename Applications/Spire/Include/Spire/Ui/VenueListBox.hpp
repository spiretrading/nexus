#ifndef SPIRE_VENUE_LIST_BOX_HPP
#define SPIRE_VENUE_LIST_BOX_HPP
#include "Spire/Ui/TagComboBox.hpp"

namespace Spire {

  /** The type of model used for a list of Venue. */
  using VenueListModel = ListModel<Nexus::Venue>;

  /** A TagComboBox specialized for a Nexus::Venue. */
  using VenueListBox = TagComboBox<Nexus::Venue>;

  /**
   * Returns a new VenueListBox using a default current model.
   * @param parent The parent widget.
   */
  VenueListBox* make_venue_list_box(QWidget* parent = nullptr);

  /**
   * Returns a new VenueListBox.
   * @param current The current list of selected venues.
   * @param parent The parent widget.
   */
  VenueListBox* make_venue_list_box(
    std::shared_ptr<VenueListModel> current, QWidget* parent = nullptr);
}

#endif
