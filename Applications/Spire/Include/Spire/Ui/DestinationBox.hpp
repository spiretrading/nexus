#ifndef SPIRE_DESTINATION_BOX_HPP
#define SPIRE_DESTINATION_BOX_HPP
#include "Nexus/Definitions/Destination.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/EnumBox.hpp"
#include "Spire/Ui/RegionBox.hpp"
#include "Spire/Ui/Ui.hpp"

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
   * @param markets The markets used to resolve destinations.
   * @param region The region to restrict the destinations to.
   */
  std::shared_ptr<ListModel<Nexus::DestinationDatabase::Entry>>
    make_region_filtered_destination_list(
      Nexus::DestinationDatabase destinations, Nexus::MarketDatabase markets,
      std::shared_ptr<RegionModel> region);

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
