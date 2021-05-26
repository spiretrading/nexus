#include "Spire/Ui/ArrayListModel.hpp"
#include <algorithm>
#include <stdexcept>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

void ArrayListModel::push(const std::any& value) {
  insert(value, get_size());
}

void ArrayListModel::insert(const std::any& value, int index) {
  if(index < 0 || index > get_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  m_data.insert(std::next(m_data.begin(), index), value);
  m_transaction.push(AddOperation{index});
}

void ArrayListModel::move(int source, int destination) {
  if(source < 0 || source >= get_size() || destination < 0 ||
      destination >= get_size()) {
    throw std::out_of_range("The source or destination is out of range.");
  }
  if(source == destination) {
    return;
  }
  auto source_row = std::move(m_data[source]);
  if(source < destination) {
    std::move(std::next(m_data.begin(), source + 1),
      std::next(m_data.begin(), destination + 1),
      std::next(m_data.begin(), source));
  } else {
    std::move_backward(std::next(m_data.begin(), destination),
      std::next(m_data.begin(), source), std::next(m_data.begin(), source + 1));
  }
  m_data[destination] = std::move(source_row);
  m_transaction.push(MoveOperation{source, destination});
}

void ArrayListModel::remove(int index) {
  if(index < 0 || index >= get_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  m_data.erase(std::next(m_data.begin(), index));
  m_transaction.push(RemoveOperation{index});
}

int ArrayListModel::get_size() const {
  return static_cast<int>(m_data.size());
}

const std::any& ArrayListModel::at(int index) const {
  if(index < 0 || index >= get_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  return m_data[index];
}

QValidator::State ArrayListModel::set(int index, const std::any& value) {
  if(index < 0 || index >= get_size()) {
    return QValidator::State::Invalid;
  }
  m_data[index] = value;
  m_transaction.push(UpdateOperation{index});
  return QValidator::State::Acceptable;
}

connection ArrayListModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}
