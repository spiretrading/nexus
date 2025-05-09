#include "Spire/Spire/ListModel.hpp"

using namespace boost;
using namespace boost::signals2;
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

QValidator::State ListModel<std::any>::insert(
    const std::any& value, int index) {
  return QValidator::State::Invalid;
}

QValidator::State ListModel<std::any>::insert(
    const std::any& value, const_iterator i) {
  return insert(value, i - cbegin());
}

QValidator::State ListModel<std::any>::move(int source, int destination) {
  return QValidator::State::Invalid;
}

QValidator::State ListModel<std::any>::remove(int index) {
  return QValidator::State::Invalid;
}

QValidator::State ListModel<std::any>::remove(const_iterator i) {
  return remove(i - cbegin());
}

ListModel<std::any>::iterator ListModel<std::any>::begin() {
  return iterator(*this, 0);
}

ListModel<std::any>::iterator ListModel<std::any>::end() {
  return iterator(*this, get_size());
}

ListModel<std::any>::const_iterator ListModel<std::any>::begin() const {
  return const_iterator(const_cast<ListModel&>(*this), 0);
}

ListModel<std::any>::const_iterator ListModel<std::any>::end() const {
  return const_iterator(const_cast<ListModel&>(*this), get_size());
}

ListModel<std::any>::const_iterator ListModel<std::any>::cbegin() const {
  return const_iterator(const_cast<ListModel&>(*this), 0);
}

ListModel<std::any>::const_iterator ListModel<std::any>::cend() const {
  return const_iterator(const_cast<ListModel&>(*this), get_size());
}

ListModel<std::any>::reverse_iterator ListModel<std::any>::rbegin() {
  return reverse_iterator(end());
}

ListModel<std::any>::reverse_iterator ListModel<std::any>::rend() {
  return reverse_iterator(begin());
}

ListModel<std::any>::const_reverse_iterator ListModel<std::any>::rbegin() const {
  return const_reverse_iterator(end());
}

ListModel<std::any>::const_reverse_iterator ListModel<std::any>::rend() const {
  return const_reverse_iterator(begin());
}

ListModel<std::any>::const_reverse_iterator
    ListModel<std::any>::crbegin() const {
  return const_reverse_iterator(end());
}

ListModel<std::any>::const_reverse_iterator ListModel<std::any>::crend() const {
  return const_reverse_iterator(begin());
}

connection ListModel<std::any>::connect_operation_signal(
    const AnyListModel::OperationSignal::slot_type& slot) const {
  return connect_operation_signal([=] (const Operation& operation) {
    slot(static_cast<const AnyListModel::Operation&>(operation));
  });
}

ListModel<std::any>::UpdateOperation::UpdateOperation(
  int index, Type previous, Type value)
  : AnyListModel::UpdateOperation(
      index, std::move(previous), std::move(value)) {}

const ListModel<std::any>::Type&
    ListModel<std::any>::UpdateOperation::get_previous() const {
  return m_previous;
}

const ListModel<std::any>::Type&
    ListModel<std::any>::UpdateOperation::get_value() const {
  return m_value;
}

void Spire::clear(AnyListModel& model) {
  auto size = model.get_size();
  model.transact([&] {
    for(auto i = size - 1; i >= 0; --i) {
      model.remove(i);
    }
  });
}

std::ostream& Spire::operator <<(
    std::ostream& out, const AnyListModel::AddOperation& operation) {
  return out << operation.m_index;
}

std::ostream& Spire::operator <<(
    std::ostream& out, const AnyListModel::PreRemoveOperation& operation) {
  return out << operation.m_index;
}

std::ostream& Spire::operator <<(
    std::ostream& out, const AnyListModel::RemoveOperation& operation) {
  return out << operation.m_index;
}

std::ostream& Spire::operator <<(
    std::ostream& out, const AnyListModel::MoveOperation& operation) {
  return out <<
    '(' << operation.m_source << ' ' << operation.m_destination << ')';
}

std::ostream& Spire::operator <<(
    std::ostream& out, const AnyListModel::UpdateOperation& operation) {
  return out << operation.m_index;
}

std::ostream& Spire::operator <<(
    std::ostream& out, const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::StartTransaction& operation) {
      out << "StartTransaction";
    },
    [&] (const AnyListModel::EndTransaction& operation) {
      out << "EndTransaction";
    },
    [&] (const AnyListModel::AddOperation& operation) {
      out << "(AddOperation " << operation << ')';
    },
    [&] (const AnyListModel::PreRemoveOperation& operation) {
      out << "(PreRemoveOperation " << operation << ')';
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
      out << "(RemoveOperation " << operation << ')';
    },
    [&] (const AnyListModel::MoveOperation& operation) {
      out << "(MoveOperation " <<
        operation.m_source << ' ' << operation.m_destination << ')';
    },
    [&] (const AnyListModel::UpdateOperation& operation) {
      out << "(UpdateOperation " << operation << ')';
    });
  return out;
}
