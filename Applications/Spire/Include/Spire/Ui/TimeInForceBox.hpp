#ifndef SPIRE_TIME_IN_FORCE_BOX_HPP
#define SPIRE_TIME_IN_FORCE_BOX_HPP
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Ui/EnumBox.hpp"

namespace Spire {

  /** A ValueModel over a Nexus::TimeInForce. */
  using TimeInForceModel = ValueModel<Nexus::TimeInForce>;

  /** A LocalValueModel over a Nexus::TimeInForce. */
  using LocalTimeInForceModel = LocalValueModel<Nexus::TimeInForce>;

  /**
   * An EnumBox specialized for a Nexus::TimeInForce.
   */
  using TimeInForceBox = EnumBox<Nexus::TimeInForce>;

  /**
   * Returns a new TimeInForceBox using a LocalTimeInForceModel.
   * @param parent The parent widget.
   */
  TimeInForceBox* make_time_in_force_box(QWidget* parent = nullptr);

  /**
   * Returns a new TimeInForceBox using a LocalTimeInForceModel and
   * an initial current value.
   * @param current The initial current value.
   * @param parent The parent widget.
   */
  TimeInForceBox* make_time_in_force_box(
    Nexus::TimeInForce current, QWidget* parent = nullptr);

  /**
   * Returns a TimeInForceBox.
   * @param current The current value model.
   * @param parent The parent widget.
   */
   TimeInForceBox* make_time_in_force_box(
     std::shared_ptr<TimeInForceModel> current, QWidget* parent = nullptr);
}

#endif
