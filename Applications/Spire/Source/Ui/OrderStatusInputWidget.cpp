#include "Spire/Ui/OrderStatusInputWidget.hpp"

using namespace Nexus;
using namespace Spire;

OrderStatusInputWidget::OrderStatusInputWidget(QWidget* parent)
  : FilteredDropDownMenu({
      QVariant::fromValue<OrderStatus>(OrderStatus::CANCELED),
      QVariant::fromValue<OrderStatus>(OrderStatus::CANCEL_REJECT),
      QVariant::fromValue<OrderStatus>(OrderStatus::DONE_FOR_DAY),
      QVariant::fromValue<OrderStatus>(OrderStatus::EXPIRED),
      QVariant::fromValue<OrderStatus>(OrderStatus::FILLED),
      QVariant::fromValue<OrderStatus>(OrderStatus::NEW),
      QVariant::fromValue<OrderStatus>(OrderStatus::PARTIALLY_FILLED),
      QVariant::fromValue<OrderStatus>(OrderStatus::PENDING_CANCEL),
      QVariant::fromValue<OrderStatus>(OrderStatus::PENDING_NEW),
      QVariant::fromValue<OrderStatus>(OrderStatus::REJECTED),
      QVariant::fromValue<OrderStatus>(OrderStatus::STOPPED),
      QVariant::fromValue<OrderStatus>(OrderStatus::SUSPENDED)}, parent) {}

OrderStatus OrderStatusInputWidget::get_order_status() const {
  return get_item().value<OrderStatus>();
}
