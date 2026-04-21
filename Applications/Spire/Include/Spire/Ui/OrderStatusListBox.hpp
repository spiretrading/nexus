#ifndef SPIRE_ORDER_STATUS_LIST_BOX_HPP
#define SPIRE_ORDER_STATUS_LIST_BOX_HPP
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Spire/Ui/TagComboBox.hpp"

namespace Spire {

  /** The type of model used for a list of OrderStatus. */
  using OrderStatusListModel = ListModel<Nexus::OrderStatus>;

  /** A TagComboBox specialized for a Nexus::OrderStatus. */
  using OrderStatusListBox = TagComboBox<Nexus::OrderStatus>;

  /**
   * Returns a new OrderStatusListBox using a default current model.
   * @param parent The parent widget.
   */
  OrderStatusListBox* make_order_status_list_box(QWidget* parent = nullptr);

  /**
   * Returns a new OrderStatusListBox.
   * @param current The current list of selected order statuses.
   * @param parent The parent widget.
   */
  OrderStatusListBox* make_order_status_list_box(
    std::shared_ptr<OrderStatusListModel> current, QWidget* parent = nullptr);
}

#endif
