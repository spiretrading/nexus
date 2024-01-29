#ifndef SPIRE_ORDER_TYPE_BOX_HPP
#define SPIRE_ORDER_TYPE_BOX_HPP
#include "Nexus/Definitions/OrderType.hpp"
#include "Spire/Ui/EnumBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A ValueModel over a Nexus::OrderType. */
  using OrderTypeModel = ValueModel<Nexus::OrderType>;

  /** A LocalValueModel over a Nexus::OrderType. */
  using LocalOrderTypeModel = LocalValueModel<Nexus::OrderType>;

  /** An EnumBox specialized for a Nexus::OrderType. */
  using OrderTypeBox = EnumBox<Nexus::OrderType>;

  /**
   * Returns a new OrderTypeBox using a LocalOrderTypeModel.
   * @param parent The parent widget.
   */
  OrderTypeBox* make_order_type_box(QWidget* parent = nullptr);

  /**
   * Returns a new OrderTypeBox using a LocalOrderTypeModel and
   * an initial current value.
   * @param current The initial current value.
   * @param parent The parent widget.
   */
  OrderTypeBox* make_order_type_box(
    Nexus::OrderType current, QWidget* parent = nullptr);

  /**
   * Returns an OrderTypeBox.
   * @param current The current value's model.
   * @param parent The parent widget.
   */
   OrderTypeBox* make_order_type_box(
    std::shared_ptr<OrderTypeModel> current, QWidget* parent = nullptr);
}

#endif
