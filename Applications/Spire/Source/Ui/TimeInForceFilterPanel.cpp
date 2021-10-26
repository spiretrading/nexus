#include "Spire/Ui/TimeInForceFilterPanel.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/ArrayTableModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  std::unordered_map<TimeInForce::Type, int> get_mapping(
      std::shared_ptr<TableModel> table_model) {
    static auto mapping = [=] {
      auto mapping = std::unordered_map<TimeInForce::Type, int>();
      for(auto i = 0; i < table_model->get_row_size(); ++i) {
        mapping[table_model->get<TimeInForce>(i, 0).GetType()] = i;
      }
      return mapping;
    }();
    return mapping;
  }
}

TimeInForceFilterPanel* Spire::make_time_in_force_filter_panel(
    QWidget& parent) {
  return make_time_in_force_filter_panel(
    std::make_shared<TimeInForceListModel>(std::make_shared<ArrayListModel>()),
      parent);
}

TimeInForceFilterPanel* Spire::make_time_in_force_filter_panel(
    std::shared_ptr<TimeInForceListModel> selected_model, QWidget& parent) {
  auto model = std::make_shared<ArrayTableModel>();
  model->push({TimeInForce(TimeInForce::Type::DAY), false});
  model->push({TimeInForce(TimeInForce::Type::GTC), false});
  model->push({TimeInForce(TimeInForce::Type::OPG), false});
  model->push({TimeInForce(TimeInForce::Type::IOC), false});
  model->push({TimeInForce(TimeInForce::Type::FOK), false});
  model->push({TimeInForce(TimeInForce::Type::GTX), false});
  model->push({TimeInForce(TimeInForce::Type::GTD), false});
  model->push({TimeInForce(TimeInForce::Type::MOC), false});
  for(auto i = 0; i < selected_model->get_size(); ++i) {
    if(get_mapping(model).contains(selected_model->get(i).GetType())) {
      model->set(get_mapping(model)[selected_model->get(i).GetType()], 1, true);
    }
  }
  return new ClosedFilterPanel(model, QObject::tr("Filter by Time in Force"),
    parent);
}
