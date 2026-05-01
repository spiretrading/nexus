#ifndef SPIRE_ORDER_STATUS_BOX_HPP
#define SPIRE_ORDER_STATUS_BOX_HPP
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Spire/Ui/EnumBox.hpp"

namespace Spire {

  /** A ValueModel over a Nexus::OrderStatus. */
  using OrderStatusModel = ValueModel<Nexus::OrderStatus>;

  /** A LocalValueModel over a Nexus::OrderStatus. */
  using LocalOrderStatusModel = LocalValueModel<Nexus::OrderStatus>;

  /** An EnumBox specialized for a Nexus::OrderStatus. */
  using OrderStatusBox = EnumBox<Nexus::OrderStatus>;

  /**
   * Returns an OrderStatusBox using a LocalOrderStatusModel.
   * @param parent The parent widget.
   */
  OrderStatusBox* make_order_status_box(QWidget* parent = nullptr);

  /**
   * Returns an OrderStatusBox using a LocalOrderStatusModel and initial current
   * value.
   * @param current The initial current value.
   * @param parent The parent widget.
   */
  OrderStatusBox* make_order_status_box(
    Nexus::OrderStatus current, QWidget* parent = nullptr);

  /**
   * Returns an OrderStatusBox.
   * @param current The current value model.
   * @param parent The parent widget.
   */
   OrderStatusBox* make_order_status_box(
    std::shared_ptr<OrderStatusModel> current, QWidget* parent = nullptr);
}

#endif
