#include "Spire/Spire/ValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

QValidator::State AnyValueModel::get_state() const {
  return QValidator::State::Acceptable;
}

AnyRef AnyValueModel::get() const {
  return get_ref();
}

QValidator::State AnyValueModel::test(const AnyRef& value) const {
  return test_ref(std::move(value));
}

QValidator::State AnyValueModel::set(const AnyRef& value) {
  return set_ref(std::move(value));
}

connection AnyValueModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return connect_update_signal_ref(slot);
}
