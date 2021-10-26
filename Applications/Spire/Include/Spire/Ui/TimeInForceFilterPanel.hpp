#ifndef SPIRE_TIME_IN_FORCE_FILTER_PANEL_HPP
#define SPIRE_TIME_IN_FORCE_FILTER_PANEL_HPP
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Ui/CastListModel.hpp"
#include "Spire/Ui/ClosedFilterPanel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A CastListModel over a Nexus::TimeInForce. */
  using TimeInForceListModel = CastListModel<Nexus::TimeInForce>;

  /** Displays a ClosedFilterPanel over a list of Nexus::TimeInForce. */
  using TimeInForceFilterPanel = ClosedFilterPanel;

  /**
   * Returns a new TimeInForceFilterPanel with no selected values.
   * @param parent The parent widget.
   */
  TimeInForceFilterPanel* make_time_in_force_filter_panel(QWidget& parent);

  /**
   * Returns a new TimeInForceFilterPanel.
   * @param selected_model A model including a list of selected values.
   * @param parent The parent widget.
   */
  TimeInForceFilterPanel* make_time_in_force_filter_panel(
    std::shared_ptr<TimeInForceListModel> selected_model, QWidget& parent);
}

#endif
