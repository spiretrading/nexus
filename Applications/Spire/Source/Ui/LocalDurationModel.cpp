#include "Spire/Ui/LocalDurationModel.hpp"
#include "Spire/Ui/LocalScalarValueModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;

LocalDurationModel::LocalDurationModel()
  : LocalDurationModel({0, 0, 0, 0}) {}

LocalDurationModel::LocalDurationModel(time_duration current)
    : m_state(LocalValueModel<time_duration>::get_state()),
      m_hour_model(std::make_shared<LocalIntegerModel>()),
      m_minute_model(std::make_shared<LocalIntegerModel>()),
      m_second_model(
        std::make_shared<LocalScalarValueModel<DecimalBox::Decimal>>()) {
  LocalValueModel<time_duration>::set_current(current);
  m_hour_model->set_minimum(0);
  m_minute_model->set_minimum(0);
  m_minute_model->set_maximum(59);
  m_second_model->set_minimum(DecimalBox::Decimal(0));
  m_second_model->set_maximum(DecimalBox::Decimal(59.999));
  m_second_model->set_increment(pow(DecimalBox::Decimal(10), -3));
}

void LocalDurationModel::set_minimum(const optional<time_duration>& minimum) {
  m_minimum = minimum;
}

void LocalDurationModel::set_maximum(const optional<time_duration>& maximum) {
  m_maximum = maximum;
  if(m_maximum) {
    m_hour_model->set_maximum(static_cast<int>(m_maximum->hours()));
  }
}

optional<time_duration> LocalDurationModel::get_minimum() const {
  return m_minimum;
}

optional<time_duration> LocalDurationModel::get_maximum() const {
  return m_maximum;
}

QValidator::State LocalDurationModel::get_state() const {
  return m_state;
}

const time_duration& LocalDurationModel::get_current() const {
  return LocalValueModel<time_duration>::get_current();
}

QValidator::State LocalDurationModel::set_current(const time_duration& value) {
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
  LocalValueModel<time_duration>::set_current(value);
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
  return LocalValueModel<time_duration>::connect_current_signal(slot);
}
