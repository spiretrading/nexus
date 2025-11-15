#ifndef SPIRE_VENUE_BOX_HPP
#define SPIRE_VENUE_BOX_HPP
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/Ui/EnumBox.hpp"

namespace Spire {

  /** Defines a model over a Venue. */
  using VenueModel = ValueModel<Nexus::Venue>;

  /** Defines a local model for a Venue. */
  using LocalVenueModel = LocalValueModel<Nexus::Venue>;

  /** An EnumBox specialized for a Nexus::Venue. */
  using VenueBox = EnumBox<Nexus::Venue>;

  /**
   * Returns a new VenueBox using a LocalVenueModel.
   * @param parent The parent widget.
   */
  VenueBox* make_venue_box(QWidget* parent = nullptr);

  /**
   * Returns a new VenueBox using a LocalVenueModel and initial current value.
   * @param current The initial current value.
   * @param parent The parent widget.
   */
  VenueBox* make_venue_box(Nexus::Venue current, QWidget* parent = nullptr);

  /**
   * Returns a VenueBox.
   * @param current The current value.
   * @param parent The parent widget.
   */
   VenueBox* make_venue_box(
    std::shared_ptr<VenueModel> current, QWidget* parent = nullptr);
}

#endif
