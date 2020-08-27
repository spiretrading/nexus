#include "Spire/Ui/OrderTypeInputWidget.hpp"

using namespace Nexus;
using namespace Spire;

OrderTypeInputWidget::OrderTypeInputWidget(QWidget* parent)
  : FilteredDropDownMenu({
      QVariant::fromValue<OrderType>(OrderType::LIMIT),
      QVariant::fromValue<OrderType>(OrderType::MARKET),
      QVariant::fromValue<OrderType>(OrderType::PEGGED),
      QVariant::fromValue<OrderType>(OrderType::STOP)}, parent) {}

OrderType OrderTypeInputWidget::get_order_type() const {
  return get_item().value<OrderType>();
}
