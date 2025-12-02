#ifndef SPIRE_DESTINATION_BOX_HPP
#define SPIRE_DESTINATION_BOX_HPP
#include "Nexus/Definitions/Destination.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/EnumBox.hpp"
#include "Spire/Ui/RegionBox.hpp"

namespace Spire {

  /** Defines a model over a Destination. */
  using DestinationModel = ValueModel<Nexus::Destination>;

  /** Defines a local model for a Destination. */
  using LocalDestinationModel = LocalValueModel<Nexus::Destination>;

  /** An EnumBox specialized for a Nexus::Destination. */
  using DestinationBox = EnumBox<Nexus::Destination>;

  /**
   * Makes a list of destinations that are restricted to a region.
   * @param destinations The destinations available.
   */
  std::shared_ptr<ListModel<Nexus::DestinationDatabase::Entry>>
    make_region_filtered_destination_list(std::shared_ptr<RegionModel> region);

  /**
   * Returns a DestinationBox.
   * @param current The current value model.
   * @param destinations The list of destinations available to select.
   * @param parent The parent widget.
   */
   DestinationBox* make_destination_box(
    std::shared_ptr<DestinationModel> current,
    std::shared_ptr<ListModel<Nexus::DestinationDatabase::Entry>> destinations,
    QWidget* parent = nullptr);
}

#endif
