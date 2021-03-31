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
  return m_state;
}

const Duration& LocalDurationModel::get_current() const {
  return LocalValueModel<Duration>::get_current();
}

QValidator::State LocalDurationModel::set_current(const Duration& value) {
  if(value.is_special()) {
    m_state = QValidator::State::Invalid;
  } else {
    if(m_minimum && value < *m_minimum ||
      m_maximum && value > *m_maximum) {
      m_state = QValidator::Intermediate;
    } else {
      m_state = QValidator::Acceptable;
    }
  }
  LocalValueModel<Duration>::set_current(value);
  return m_state;
}

connection LocalDurationModel::connect_current_signal(
    const typename CurrentSignal::slot_type& slot) const {
  return LocalValueModel<Duration>::connect_current_signal(slot);
}
