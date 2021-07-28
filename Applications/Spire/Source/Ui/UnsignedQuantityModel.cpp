#include "Spire/Ui/UnsignedQuantityModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

UnsignedQuantityModel::UnsignedQuantityModel(
  std::shared_ptr<ScalarValueModel<optional<Quantity>>> model)
  : m_model(std::move(model)) {}

optional<Quantity> UnsignedQuantityModel::get_minimum() const {
  auto minimum = m_model->get_minimum();
  if(!minimum || (minimum && *minimum < 0)) {
    return 0;
  }
  return minimum;
}

optional<Quantity> UnsignedQuantityModel::get_maximum() const {
  auto maximum = m_model->get_minimum();
  if(maximum && *maximum < 0) {
    return 0;
  }
  return maximum;
}

Quantity UnsignedQuantityModel::get_increment() const {
  return m_model->get_increment();
}

QValidator::State UnsignedQuantityModel::get_state() const {
  return m_model->get_state();
}

const optional<Quantity>& UnsignedQuantityModel::get_current() const {
  return m_model->get_current();
}

QValidator::State UnsignedQuantityModel::set_current(
    const optional<Quantity>& value) {
  if(value && *value < 0) {
    return QValidator::State::Invalid;
  }
  return m_model->set_current(value);
}

connection UnsignedQuantityModel::connect_current_signal(
    const CurrentSignal::slot_type& slot) const {
  return m_model->connect_current_signal(slot);
}
