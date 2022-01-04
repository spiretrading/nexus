#include "Spire/Spire/ListModel.hpp"

using namespace Spire;

std::any AnyListModel::get(int index) const {
  return at(index);
}

QValidator::State ListModel<std::any>::set(int index, const Type& value) {
  return QValidator::State::Invalid;
}

std::any ListModel<std::any>::at(int index) const {
  return get(index);
}
