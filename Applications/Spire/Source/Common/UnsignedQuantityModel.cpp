#include "Spire/Spire/UnsignedQuantityModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

UnsignedQuantityModel::UnsignedQuantityModel(
  std::shared_ptr<ScalarValueModel<optional<Quantity>>> model)
  : m_model(std::move(model)) {}

optional<Quantity> UnsignedQuantityModel::get_minimum() const {
  auto minimum = m_model->get_minimum();
  if(!minimum || *minimum < 0) {
    return Quantity(0);
  }
  return minimum;
}

optional<Quantity> UnsignedQuantityModel::get_maximum() const {
  auto maximum = m_model->get_maximum();
  if(maximum && *maximum < 0) {
    return Quantity(0);
  }
  return maximum;
}

optional<Quantity> UnsignedQuantityModel::get_increment() const {
  return m_model->get_increment();
}

QValidator::State UnsignedQuantityModel::get_state() const {
  return m_model->get_state();
}

const optional<Quantity>& UnsignedQuantityModel::get() const {
  return m_model->get();
}

QValidator::State UnsignedQuantityModel::test(
    const optional<Quantity>& value) const {
  if(value && *value < 0) {
    return QValidator::State::Invalid;
  }
  return m_model->test(value);
}

QValidator::State UnsignedQuantityModel::set(const optional<Quantity>& value) {
  if(value && *value < 0) {
    return QValidator::State::Invalid;
  }
  return m_model->set(value);
}

connection UnsignedQuantityModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_model->connect_update_signal(slot);
}
