#include "Spire/Ui/TimeInForceFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  const auto& get_time_in_forces() {
    static const auto values = std::vector<TimeInForce>{
      TimeInForce::Type::DAY, TimeInForce::Type::GTC, TimeInForce::Type::OPG,
      TimeInForce::Type::IOC, TimeInForce::Type::FOK, TimeInForce::Type::GTX,
      TimeInForce::Type::GTD, TimeInForce::Type::MOC};
    return values;
  }
}

TimeInForceFilterPanel* Spire::make_time_in_force_filter_panel(
    QWidget* parent) {
  return make_time_in_force_filter_panel(
    std::make_shared<ArrayListModel<TimeInForce>>(), parent);
}

TimeInForceFilterPanel* Spire::make_time_in_force_filter_panel(
    std::shared_ptr<TimeInForceListModel> selection, QWidget* parent) {
  auto model = std::make_shared<ArrayTableModel>();
  auto& values = get_time_in_forces();
  for(auto& value : values) {
    model->push({value, false});
  }
  for(auto i = 0; i < selection->get_size(); ++i) {
    if(auto iterator = std::find(values.begin(), values.end(),
        selection->get(i)); iterator != values.end()) {
      model->set(std::distance(values.begin(), iterator), 1, true);
    }
  }
  return new ClosedFilterPanel(std::move(model), parent);
}
