#include "Spire/Ui/TimeInForceInputWidget.hpp"

using namespace Nexus;
using namespace Spire;

TimeInForceInputWidget::TimeInForceInputWidget(QWidget* parent)
  : FilteredDropDownMenu({
      QVariant::fromValue<TimeInForce>({TimeInForce::Type::DAY}),
      QVariant::fromValue<TimeInForce>({TimeInForce::Type::FOK}),
      QVariant::fromValue<TimeInForce>({TimeInForce::Type::GTC}),
      QVariant::fromValue<TimeInForce>({TimeInForce::Type::GTD}),
      QVariant::fromValue<TimeInForce>({TimeInForce::Type::GTX}),
      QVariant::fromValue<TimeInForce>({TimeInForce::Type::IOC}),
      QVariant::fromValue<TimeInForce>({TimeInForce::Type::MOC}),
      QVariant::fromValue<TimeInForce>({TimeInForce::Type::OPG})},
    parent) {}

TimeInForce TimeInForceInputWidget::get_time_in_force() const {
  return get_item().value<TimeInForce>();
}
