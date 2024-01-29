#include "Spire/Ui/TimeInForceFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  std::unordered_map<TimeInForce::Type, int>& populate() {
    static auto values = [] {
      auto values = std::unordered_map<TimeInForce::Type, int>();
      auto i = 0;
      values[TimeInForce::Type::DAY] = i++;
      values[TimeInForce::Type::GTC] = i++;
      values[TimeInForce::Type::OPG] = i++;
      values[TimeInForce::Type::IOC] = i++;
      values[TimeInForce::Type::FOK] = i++;
      values[TimeInForce::Type::GTX] = i++;
      values[TimeInForce::Type::GTD] = i++;
      values[TimeInForce::Type::MOC] = i++;
      return values;
    }();
    return values;
  }
}

TimeInForceFilterPanel* Spire::make_time_in_force_filter_panel(
    QWidget& parent) {
  return make_time_in_force_filter_panel(
    std::make_shared<ArrayListModel<TimeInForce>>(), parent);
}

TimeInForceFilterPanel* Spire::make_time_in_force_filter_panel(
    std::shared_ptr<TimeInForceListModel> selected_model, QWidget& parent) {
  auto model = std::make_shared<ArrayTableModel>();
  auto& values = populate();
  for(auto& value : values) {
    model->push({TimeInForce(value.first), false});
  }
  for(auto i = 0; i < selected_model->get_size(); ++i) {
    auto type = selected_model->get(i).GetType();
    if(values.contains(type)) {
      model->set(values[type], 1, true);
    }
  }
  return new ClosedFilterPanel(std::move(model),
    QObject::tr("Filter by Time in Force"), parent);
}
