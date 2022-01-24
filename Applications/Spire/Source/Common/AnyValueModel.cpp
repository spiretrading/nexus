#include "Spire/Spire/AnyValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

const AnyValueModel::Type& AnyValueModel::get() const {
  return m_value;
}

QValidator::State AnyValueModel::test(const Type& value) const {
  if(m_value.has_value()) {
    return QValidator::State::Acceptable;
  }
  return QValidator::State::Invalid;
}

QValidator::State AnyValueModel::set(const Type& value) {
  if(m_value.get_type() != value.get_type()) {
    return QValidator::State::Invalid;
  }
  m_set_value(value);
  m_update_signal(m_value);
  return QValidator::State::Acceptable;
}

connection AnyValueModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}
