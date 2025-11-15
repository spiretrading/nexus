#ifndef SPIRE_TIME_IN_FORCE_FILTER_PANEL_HPP
#define SPIRE_TIME_IN_FORCE_FILTER_PANEL_HPP
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Ui/ClosedFilterPanel.hpp"

namespace Spire {

  /** A CastListModel over a Nexus::TimeInForce. */
  using TimeInForceListModel = ListModel<Nexus::TimeInForce>;

  /** Displays a ClosedFilterPanel over a list of Nexus::TimeInForce. */
  using TimeInForceFilterPanel = ClosedFilterPanel;

  /**
   * Returns a new TimeInForceFilterPanel using an empty TimeInForceListModel.
   * @param parent The parent widget.
   */
  TimeInForceFilterPanel* make_time_in_force_filter_panel(QWidget& parent);

  /**
   * Returns a new TimeInForceFilterPanel.
   * @param selection A model including a list of selected values.
   * @param parent The parent widget.
   */
  TimeInForceFilterPanel* make_time_in_force_filter_panel(
    std::shared_ptr<TimeInForceListModel> selection, QWidget& parent);
}

#endif
