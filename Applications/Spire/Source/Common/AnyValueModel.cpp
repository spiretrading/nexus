#include "Spire/Spire/AnyValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

QValidator::State AnyValueModel::get_state() const {
  return m_model->get_state();
}

const AnyValueModel::Type& AnyValueModel::get() const {
  return m_model->get();
}

QValidator::State AnyValueModel::test(const Type& value) const {
  return m_model->test(value);
}

QValidator::State AnyValueModel::set(const Type& value) {
  return m_model->set(value);
}

connection AnyValueModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_model->connect_update_signal(slot);
}
