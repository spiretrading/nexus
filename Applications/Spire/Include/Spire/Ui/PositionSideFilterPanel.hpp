#ifndef SPIRE_POSITION_SIDE_FILTER_PANEL_HPP
#define SPIRE_POSITION_SIDE_FILTER_PANEL_HPP
#include "Spire/Ui/SideFilterPanel.hpp"

namespace Spire {

  /** Type alias for a ClosedFilterPanel filtering on position sides. */
  using PositionSideFilterPanel = ClosedFilterPanel;

  /**
   * Returns a new PositionSideFilterPanel using an empty SideListModel.
   * @param parent The parent widget.
   */
  PositionSideFilterPanel* make_position_side_filter_panel(
    QWidget* parent = nullptr);

  /**
   * Returns a new PositionSideFilterPanel.
   * @param selection A model including a list of selected values.
   * @param parent The parent widget.
   */
  PositionSideFilterPanel* make_position_side_filter_panel(
    std::shared_ptr<SideListModel> selection, QWidget* parent = nullptr);
}

#endif
