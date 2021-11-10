#ifndef SPIRE_SIDE_FILTER_PANEL_HPP
#define SPIRE_SIDE_FILTER_PANEL_HPP
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Ui/CastListModel.hpp"
#include "Spire/Ui/ClosedFilterPanel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A CastListModel over a Nexus::Side. */
  using SideListModel = CastListModel<Nexus::Side>;

  /** Displays a ClosedFilterPanel over a list of Nexus::Side. */
  using SideFilterPanel = ClosedFilterPanel;

  /**
   * Returns a new SideFilterPanel using an empty SideListModel.
   * @param parent The parent widget.
   */
  SideFilterPanel* make_side_filter_panel(QWidget& parent);

  /**
   * Returns a new SideFilterPanel.
   * @param selected_model A model including a list of selected values.
   * @param parent The parent widget.
   */
  SideFilterPanel* make_side_filter_panel(
    std::shared_ptr<SideListModel> selected_model, QWidget& parent);
}

#endif
