#include "Spire/Ui/OrderTypeFilterPanel.hpp"
#include <algorithm>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  const auto& get_order_types() {
    static const auto values = std::vector<OrderType>{
      OrderType::LIMIT, OrderType::MARKET,
      OrderType::PEGGED, OrderType::STOP};
    return values;
  }
}

OrderTypeFilterPanel* Spire::make_order_type_filter_panel(QWidget* parent) {
  return make_order_type_filter_panel(
    std::make_shared<ArrayListModel<OrderType>>(), parent);
}

OrderTypeFilterPanel* Spire::make_order_type_filter_panel(
    std::shared_ptr<OrderTypeListModel> selection, QWidget* parent) {
  auto model = std::make_shared<ArrayTableModel>();
  auto& types = get_order_types();
  for(auto& type : types) {
    model->push({type, false});
  }
  for(auto i = 0; i < selection->get_size(); ++i) {
    auto selected = selection->get(i);
    if(auto iterator = std::find(types.begin(), types.end(), selected);
        iterator != types.end()) {
      model->set(std::distance(types.begin(), iterator), 1, true);
    }
  }
  return new ClosedFilterPanel(std::move(model), parent);
}
