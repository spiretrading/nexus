#include "Spire/Ui/LocalDurationModel.hpp"
#include "Spire/Ui/LocalScalarValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

LocalDurationModel::LocalDurationModel()
  : LocalDurationModel(boost::posix_time::not_a_date_time) {}

LocalDurationModel::LocalDurationModel(const Duration& current)
    : m_state(LocalValueModel<Duration>::get_state()),
      m_hour_model(std::make_shared<LocalIntegerModel>()),
      m_minute_model(std::make_shared<LocalIntegerModel>()),
      m_second_model(
        std::make_shared<LocalScalarValueModel<DecimalBox::Decimal>>()) {
  LocalValueModel<Duration>::set_current(current);
  m_hour_model->set_minimum(0);
  m_minute_model->set_minimum(0);
  m_minute_model->set_maximum(59);
  m_second_model->set_minimum(DecimalBox::Decimal(0));
  m_second_model->set_maximum(DecimalBox::Decimal(59.999));
  m_second_model->set_increment(pow(DecimalBox::Decimal(10), -3));
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

const std::shared_ptr<LocalIntegerModel>&
    LocalDurationModel::get_hour_model() const {
  return m_hour_model;
}

const std::shared_ptr<LocalIntegerModel>&
    LocalDurationModel::get_minute_model() const {
  return m_minute_model;
}

const std::shared_ptr<LocalScalarValueModel<DecimalBox::Decimal>>&
    LocalDurationModel::get_second_model() const {
  return m_second_model;
}

connection LocalDurationModel::connect_current_signal(
    const typename CurrentSignal::slot_type& slot) const {
  return LocalValueModel<Duration>::connect_current_signal(slot);
}
