#include "Spire/Ui/OrderTypeFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  std::unordered_map<OrderType, int>& populate() {
    static auto values = [] {
      auto values = std::unordered_map<OrderType, int>();
      auto i = 0;
      values[OrderType::LIMIT] = i++;
      values[OrderType::MARKET] = i++;
      values[OrderType::PEGGED] = i++;
      values[OrderType::STOP] = i++;
      return values;
    }();
    return values;
  }
}

OrderTypeFilterPanel* Spire::make_order_type_filter_panel(QWidget& parent) {
  return make_order_type_filter_panel(
    std::make_shared<ArrayListModel<OrderType>>(), parent);
}

OrderTypeFilterPanel* Spire::make_order_type_filter_panel(
    std::shared_ptr<OrderTypeListModel> selected_model, QWidget& parent) {
  auto model = std::make_shared<ArrayTableModel>();
  auto& values = populate();
  for(auto& value : values) {
    model->push({value.first, false});
  }
  for(auto i = 0; i < selected_model->get_size(); ++i) {
    auto type = selected_model->get(i);
    if(values.contains(type)) {
      model->set(values[type], 1, true);
    }
  }
  return new ClosedFilterPanel(
    std::move(model), QObject::tr("Filter by Order Type"), parent);
}
