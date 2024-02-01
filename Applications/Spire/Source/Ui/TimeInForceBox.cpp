#include "Spire/Ui/TimeInForceBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  TimeInForceBox::Settings setup() {
    static auto settings = [] {
      auto settings = TimeInForceBox::Settings();
      auto cases = std::make_shared<ArrayListModel<TimeInForce>>();
      cases->push(TimeInForce(TimeInForce::Type::DAY));
      cases->push(TimeInForce(TimeInForce::Type::GTC));
      cases->push(TimeInForce(TimeInForce::Type::OPG));
      cases->push(TimeInForce(TimeInForce::Type::IOC));
      cases->push(TimeInForce(TimeInForce::Type::FOK));
      cases->push(TimeInForce(TimeInForce::Type::GTX));
      cases->push(TimeInForce(TimeInForce::Type::GTD));
      cases->push(TimeInForce(TimeInForce::Type::MOC));
      settings.m_cases = std::move(cases);
      return settings;
    }();
    return settings;
  }
}

TimeInForceBox* Spire::make_time_in_force_box(QWidget* parent) {
  return make_time_in_force_box(TimeInForce(TimeInForce::Type::DAY), parent);
}

TimeInForceBox* Spire::make_time_in_force_box(TimeInForce current,
    QWidget* parent) {
  return make_time_in_force_box(
    std::make_shared<LocalTimeInForceModel>(current), parent);
}

TimeInForceBox* Spire::make_time_in_force_box(
    std::shared_ptr<TimeInForceModel> model, QWidget* parent) {
  auto settings = setup();
  settings.m_current = std::move(model);
  return new TimeInForceBox(std::move(settings), parent);
}
