#ifndef SPIRE_DESTINATION_LIST_BOX_HPP
#define SPIRE_DESTINATION_LIST_BOX_HPP
#include "Nexus/Definitions/Destination.hpp"
#include "Spire/Ui/TagComboBox.hpp"

namespace Spire {

  /** The type of model used for a list of Destination. */
  using DestinationListModel = ListModel<Nexus::Destination>;

  /** A TagComboBox specialized for a Nexus::Destination. */
  using DestinationListBox = TagComboBox<Nexus::Destination>;

  /**
   * Returns a new DestinationListBox using a default current model.
   * @param parent The parent widget.
   */
  DestinationListBox* make_destination_list_box(QWidget* parent = nullptr);

  /**
   * Returns a new DestinationListBox.
   * @param current The current list of selected destinations.
   * @param parent The parent widget.
   */
  DestinationListBox* make_destination_list_box(
    std::shared_ptr<DestinationListModel> current, QWidget* parent = nullptr);
}

#endif
