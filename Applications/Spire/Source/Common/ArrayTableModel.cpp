#include "Spire/Spire/ArrayTableModel.hpp"
#include <algorithm>
#include <stdexcept>
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

void ArrayTableModel::push(const std::vector<std::any>& row) {
  insert(row, get_row_size());
}

void ArrayTableModel::insert(const std::vector<std::any>& row, int index) {
  if(!row.empty() && !m_data.empty() && row.size() != get_column_size()) {
    throw std::out_of_range("row.size() != get_column_size()");
  }
  if(index < 0 || index > get_row_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  m_data.insert(std::next(m_data.begin(), index), row);
  m_transaction.push(AddOperation(index));
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
  m_transaction.push(MoveOperation(source, destination));
}

int ArrayTableModel::get_row_size() const {
  return static_cast<int>(m_data.size());
}

int ArrayTableModel::get_column_size() const {
  if(m_data.empty()) {
    return 0;
  } else {
    return static_cast<int>(m_data.front().size());
  }
}

AnyRef ArrayTableModel::at(int row, int column) const {
  if(row < 0 || row >= get_row_size() || column < 0 ||
      column >= get_column_size()) {
    throw std::out_of_range("The row or column is out of range.");
  }
  return m_data[row][column];
}

QValidator::State ArrayTableModel::set(
    int row, int column, const std::any& value) {
  if(row < 0 || row >= get_row_size() || column < 0 ||
      column >= get_column_size()) {
    return QValidator::State::Invalid;
  }
  auto previous = std::move(m_data[row][column]);
  m_data[row][column] = value;
  m_transaction.push(UpdateOperation(row, column, std::move(previous), value));
  return QValidator::State::Acceptable;
}

QValidator::State ArrayTableModel::remove(int row) {
  if(row < 0 || row >= get_row_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  m_transaction.push(PreRemoveOperation(row));
  m_data.erase(std::next(m_data.begin(), row));
  m_transaction.push(RemoveOperation(row));
  return QValidator::Acceptable;
}

connection ArrayTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}
