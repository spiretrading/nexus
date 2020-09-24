#include "Spire/Spire/SpinBoxModel.hpp"

using namespace Nexus;
using namespace Spire;

std::shared_ptr<DecimalSpinBoxModel> Spire::make_decimal_spin_box_model(
    double initial, double minimum, double maximum) {
  auto model = std::make_shared<DecimalSpinBoxModel>(initial, minimum,
    maximum);
  model->set_increment(Qt::NoModifier, 1.0);
  return model;
}

std::shared_ptr<IntegerSpinBoxModel> Spire::make_integer_spin_box_model(
    std::int64_t initial, std::int64_t minimum, std::int64_t maximum) {
  auto model = std::make_shared<IntegerSpinBoxModel>(initial, minimum,
    maximum);
  model->set_increment(Qt::NoModifier, 1);
  return model;
}

std::shared_ptr<MoneySpinBoxModel> Spire::make_money_spin_box_model(
    Nexus::Money initial, Nexus::Money minimum, Nexus::Money maximum) {
  auto model = std::make_shared<MoneySpinBoxModel>(initial, minimum,
    maximum);
  // TODO: workaround for compiler bug where Money::CENT is 0.
  model->set_increment(Qt::NoModifier, *Nexus::Money::FromValue("0.01"));
  return model;
}

std::shared_ptr<QuantitySpinBoxModel> Spire::make_quantity_spin_box_model(
    Nexus::Quantity initial, Nexus::Quantity minimum, Nexus::Quantity maximum) {
  auto model = std::make_shared<QuantitySpinBoxModel>(initial, minimum,
    maximum);
  model->set_increment(Qt::NoModifier, 1);
  return model;
}
