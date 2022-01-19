#include "Spire/Spire/AnyValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

const AnyValueModel::Type& AnyValueModel::get() const {
  throw std::runtime_error("Not implemented.");
}

QValidator::State AnyValueModel::test(const Type& value) const {
  return QValidator::State::Invalid;
}

QValidator::State AnyValueModel::set(const Type& value) {
  return QValidator::State::Invalid;
}

connection AnyValueModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return {};
}
