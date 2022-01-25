#include "Spire/Spire/AnyValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

const AnyValueModel::Type& AnyValueModel::get() const {
  return m_model.get();
}

QValidator::State AnyValueModel::test(const Type& value) const {
  return m_test(value);
}

QValidator::State AnyValueModel::set(const Type& value) {
  if(get().get_type() != value.get_type()) {
    return QValidator::State::Invalid;
  }
  m_model.set(value);
  m_update_signal(get());
  return QValidator::State::Acceptable;
}

connection AnyValueModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}
