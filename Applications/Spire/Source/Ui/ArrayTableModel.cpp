#include "Spire/Ui/ArrayTableModel.hpp"
#include <algorithm>
#include <stdexcept>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

ArrayTableModel::ArrayTableModel()
  : m_transaction_level(0) {}

void ArrayTableModel::push(const std::vector<std::any>& row) {
  if(!m_data.empty() && row.size() != get_column_size()) {
    throw std::out_of_range("row.size() != get_column_size()");
  }
  if(row.empty()) {
    return;
  }
  m_data.push_back(row);
  auto operation = AddOperation{static_cast<int>(m_data.size()) - 1};
  if(m_transaction_level > 0) {
    m_transaction.m_operations.push_back(std::move(operation));
  } else {
    m_operation_signal(operation);
  }
}

void ArrayTableModel::insert(const std::vector<std::any>& row, int index) {
  if(!row.empty() && !m_data.empty() && row.size() != get_column_size()) {
    throw std::out_of_range("row.size() != get_column_size()");
  }
  if(index < 0 || index > get_row_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  if(row.empty()) {
    return;
  }
  m_data.insert(std::next(m_data.begin(), index), row);
  auto operation = AddOperation{index};
  if(m_transaction_level > 0) {
    m_transaction.m_operations.push_back(std::move(operation));
  } else {
    m_operation_signal(operation);
  }
}

void ArrayTableModel::move(int source, int destination) {
  if(source < 0 || source >= get_row_size() || destination < 0 ||
      destination >= get_row_size()) {
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
  auto operation = MoveOperation{source, destination};
  if(m_transaction_level > 0) {
    m_transaction.m_operations.push_back(std::move(operation));
  } else {
    m_operation_signal(operation);
  }
}

void ArrayTableModel::remove(int index) {
  if(index < 0 || index >= get_row_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  m_data.erase(std::next(m_data.begin(), index));
  auto operation = RemoveOperation{index};
  if(m_transaction_level > 0) {
    m_transaction.m_operations.push_back(std::move(operation));
  } else {
    m_operation_signal(operation);
  }
}

int ArrayTableModel::get_row_size() const {
  return m_data.size();
}

int ArrayTableModel::get_column_size() const {
  if(m_data.empty()) {
    return 0;
  } else {
    return m_data[0].size();
  }
}

const std::any& ArrayTableModel::at(int row, int column) const {
  if(row < 0 || row >= get_row_size() || column < 0 ||
      column >= get_column_size()) {
    throw std::out_of_range("The row or column is out of range.");
  }
  return m_data[row][column];
}

QValidator::State ArrayTableModel::set(int row, int column,
    const std::any& value) {
  if(row < 0 || row >= get_row_size() || column < 0 ||
      column >= get_column_size()) {
    return QValidator::State::Invalid;
  }
  m_data[row][column] = value;
  auto operation = UpdateOperation{row, column};
  if(m_transaction_level > 0) {
    m_transaction.m_operations.push_back(std::move(operation));
  } else {
    m_operation_signal(operation);
  }
  return QValidator::State::Acceptable;
}

connection ArrayTableModel::connect_operation_signal(
    const typename OperationSignal::slot_type& slot) const {
  return m_operation_signal.connect(slot);
}
