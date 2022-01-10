#include "Spire/Ui/OrderTypeBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  OrderTypeBox::Settings setup() {
    static auto settings = [] {
      auto settings = OrderTypeBox::Settings();
      auto cases = std::make_shared<ArrayListModel<OrderType>>();
      cases->push(OrderType::LIMIT);
      cases->push(OrderType::MARKET);
      cases->push(OrderType::PEGGED);
      cases->push(OrderType::STOP);
      settings.m_cases = std::move(cases);
      return settings;
    }();
    return settings;
  }
}

OrderTypeBox* Spire::make_order_type_box(QWidget* parent) {
  return make_order_type_box(OrderType::LIMIT, parent);
}

OrderTypeBox* Spire::make_order_type_box(OrderType current, QWidget* parent) {
  return make_order_type_box(
    std::make_shared<LocalOrderTypeModel>(current), parent);
}

OrderTypeBox* Spire::make_order_type_box(std::shared_ptr<OrderTypeModel> model,
    QWidget* parent) {
  auto settings = setup();
  settings.m_current = std::move(model);
  return new OrderTypeBox(std::move(settings), parent);
}
