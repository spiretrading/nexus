#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"

using namespace Spire;

namespace {
  auto to_index(Qt::KeyboardModifier modifier) {
    if(modifier == Qt::NoModifier) {
      return 0;
    } else if(modifier == Qt::ShiftModifier) {
      return 1;
    } else if(modifier == Qt::ControlModifier) {
      return 2;
    } else if(modifier == Qt::AltModifier) {
      return 3;
    }
    return -1;
  }
}

InteractionsKeyBindingsModel::InteractionsKeyBindingsModel()
    : m_default_quantity(std::make_shared<LocalQuantityModel>(100)),
      m_is_cancel_on_fill(std::make_shared<LocalBooleanModel>(false)) {
  for(auto& increment : m_quantity_increments) {
    increment = std::make_shared<LocalQuantityModel>(100);
  }
  for(auto& increment : m_price_increments) {
    increment = std::make_shared<LocalMoneyModel>(Nexus::Money(0.01));
  }
}

std::shared_ptr<QuantityModel>
    InteractionsKeyBindingsModel::get_default_quantity() const {
  return m_default_quantity;
}

std::shared_ptr<QuantityModel>
    InteractionsKeyBindingsModel::get_quantity_increment(
      Qt::KeyboardModifier modifier) const {
  auto index = to_index(modifier);
  if(index < 0) {
    throw std::out_of_range("The index is out of range.");
  }
  return m_quantity_increments[index];
}

std::shared_ptr<MoneyModel>
    InteractionsKeyBindingsModel::get_price_increment(
      Qt::KeyboardModifier modifier) const {
  auto index = to_index(modifier);
  if(index < 0) {
    throw std::out_of_range("The index is out of range.");
  }
  return m_price_increments[index];
}

std::shared_ptr<BooleanModel>
    InteractionsKeyBindingsModel::is_cancel_on_fill() const {
  return m_is_cancel_on_fill;
}
