#include "Spire/Ui/OrderStatusBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  OrderStatusBox::Settings setup() {
    static auto settings = [] {
      auto settings = OrderStatusBox::Settings();
      auto cases = std::make_shared<ArrayListModel<OrderStatus>>();
      cases->push(OrderStatus::PENDING_NEW);
      cases->push(OrderStatus::REJECTED);
      cases->push(OrderStatus::NEW);
      cases->push(OrderStatus::PARTIALLY_FILLED);
      cases->push(OrderStatus::EXPIRED);
      cases->push(OrderStatus::CANCELED);
      cases->push(OrderStatus::SUSPENDED);
      cases->push(OrderStatus::STOPPED);
      cases->push(OrderStatus::FILLED);
      cases->push(OrderStatus::DONE_FOR_DAY);
      cases->push(OrderStatus::PENDING_CANCEL);
      cases->push(OrderStatus::CANCEL_REJECT);
      settings.m_cases = std::move(cases);
      return settings;
    }();
    return settings;
  }
}

OrderStatusBox* Spire::make_order_status_box(QWidget* parent) {
  return make_order_status_box(OrderStatus::PENDING_NEW, parent);
}

OrderStatusBox* Spire::make_order_status_box(OrderStatus current,
    QWidget* parent) {
  return make_order_status_box(
    std::make_shared<LocalOrderStatusModel>(current), parent);
}

OrderStatusBox* Spire::make_order_status_box(
    std::shared_ptr<OrderStatusModel> current, QWidget* parent) {
  auto settings = setup();
  settings.m_current = std::move(current);
  return new OrderStatusBox(std::move(settings), parent);
}
