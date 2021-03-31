#include "Spire/Ui/LocalDurationModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

LocalDurationModel::LocalDurationModel() {}

LocalDurationModel::LocalDurationModel(const Duration& current) {
  LocalValueModel<Duration>::set_current(current);
}

void LocalDurationModel::set_minimum(const optional<Duration>& minimum) {
  m_minimum = minimum;
}

void LocalDurationModel::set_maximum(const optional<Duration>& maximum) {
  m_maximum = maximum;
}

optional<Duration> LocalDurationModel::get_minimum() const {
  return m_minimum;
}

optional<Duration> LocalDurationModel::get_maximum() const {
  return m_maximum;
}

QValidator::State LocalDurationModel::get_state() const {
  return LocalValueModel<Duration>::get_state();
}

const Duration& LocalDurationModel::get_current() const {
  return LocalValueModel<Duration>::get_current();
}

QValidator::State LocalDurationModel::set_current(const Duration& value) {
  if(value != get_current() && value.is_special()) {
    return QValidator::State::Invalid;
  }
  auto m_state = LocalValueModel<Duration>::set_current(value);
  if(m_minimum && value < *m_minimum ||
      m_maximum && value > *m_maximum) {
    m_state = QValidator::Intermediate;
  }
  return m_state;
}

connection LocalDurationModel::connect_current_signal(
    const typename CurrentSignal::slot_type& slot) const {
  return LocalValueModel<Duration>::connect_current_signal(slot);
}
