#include "Spire/Spire/ListModel.hpp"

using namespace Spire;

std::any AnyListModel::get(int index) const {
  return at(index);
}

QValidator::State AnyListModel::push(const std::any& value) {
  return insert(value, get_size());
}

QValidator::State ListModel<std::any>::set(int index, const Type& value) {
  return QValidator::State::Invalid;
}

std::any ListModel<std::any>::at(int index) const {
  return get(index);
}

QValidator::State ListModel<std::any>::push(const std::any& value) {
  return insert(value, get_size());
}

QValidator::State ListModel<std::any>::insert(const std::any& value,
    int index) {
  return QValidator::State::Invalid;
}

QValidator::State ListModel<std::any>::move(int source, int destination) {
  return QValidator::State::Invalid;
}

QValidator::State ListModel<std::any>::remove(int index) {
  return QValidator::State::Invalid;
}

void Spire::clear(AnyListModel& model) {
  auto size = model.get_size();
  model.transact([&] {
    for(auto i = size - 1; i >= 0; --i) {
      model.remove(i);
    }
  });
}
