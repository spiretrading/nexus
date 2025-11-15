#ifndef SPIRE_ORDER_TYPE_FILTER_PANEL_HPP
#define SPIRE_ORDER_TYPE_FILTER_PANEL_HPP
#include "Nexus/Definitions/OrderType.hpp"
#include "Spire/Ui/ClosedFilterPanel.hpp"

namespace Spire {

  /** A CastListModel over a Nexus::OrderType. */
  using OrderTypeListModel = ListModel<Nexus::OrderType>;

  /** Displays a ClosedFilterPanel over a list of Nexus::OrderType. */
  using OrderTypeFilterPanel = ClosedFilterPanel;

  /**
   * Returns a new OrderTypeFilterPanel using an empty OrderTypeListModel.
   * @param parent The parent widget.
   */
  OrderTypeFilterPanel* make_order_type_filter_panel(QWidget& parent);

  /**
   * Returns a new OrderTypeFilterPanel.
   * @param selection A model including a list of selected values.
   * @param parent The parent widget.
   */
  OrderTypeFilterPanel* make_order_type_filter_panel(
    std::shared_ptr<OrderTypeListModel> selection, QWidget& parent);
}

#endif
