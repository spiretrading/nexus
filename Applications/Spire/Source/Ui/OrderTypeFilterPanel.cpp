#include "Spire/Ui/OrderTypeFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Nexus;
using namespace Spire;

OrderTypeFilterPanel* Spire::make_order_type_filter_panel(QWidget* parent) {
  return make_order_type_filter_panel(
    std::make_shared<ArrayListModel<OrderType>>(), parent);
}

OrderTypeFilterPanel* Spire::make_order_type_filter_panel(
    std::shared_ptr<OrderTypeListModel> selection, QWidget* parent) {
  return make_closed_filter_panel(std::vector<OrderType>{
    OrderType::LIMIT, OrderType::MARKET, OrderType::PEGGED, OrderType::STOP},
    std::move(selection), parent);
}
