#include "Spire/Ui/TimeInForceFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Nexus;
using namespace Spire;

TimeInForceFilterPanel* Spire::make_time_in_force_filter_panel(
    QWidget* parent) {
  return make_time_in_force_filter_panel(
    std::make_shared<ArrayListModel<TimeInForce>>(), parent);
}

TimeInForceFilterPanel* Spire::make_time_in_force_filter_panel(
    std::shared_ptr<TimeInForceListModel> selection, QWidget* parent) {
  return make_closed_filter_panel(std::vector<TimeInForce>{
    TimeInForce::Type::DAY, TimeInForce::Type::GTC, TimeInForce::Type::OPG,
    TimeInForce::Type::IOC, TimeInForce::Type::FOK, TimeInForce::Type::GTX,
    TimeInForce::Type::GTD, TimeInForce::Type::MOC},
    std::move(selection), parent);
}
