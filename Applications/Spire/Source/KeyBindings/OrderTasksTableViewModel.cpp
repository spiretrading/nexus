#include "Spire/KeyBindings/OrderTasksTableViewModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  auto adjust_row(const AnyListModel& source) {
    auto row = std::make_shared<ArrayListModel<std::any>>();
    static auto empty_value = std::any();
    row->push(empty_value);
    for(auto i = 0; i < source.get_size(); ++i) {
      row->push(source.get(i));
    }
    return row;
  }
}

OrderTasksTableViewModel::OrderTasksTableViewModel(
  std::shared_ptr<TableModel> source)
  : m_source(std::move(source)),
    m_source_connection(m_source->connect_operation_signal(
      std::bind_front(&OrderTasksTableViewModel::on_operation, this))) {}

int OrderTasksTableViewModel::get_row_size() const {
  return m_source->get_row_size() + 1;
}

int OrderTasksTableViewModel::get_column_size() const {
  return m_source->get_column_size() + 1;
}

AnyRef OrderTasksTableViewModel::at(int row, int column) const {
  if(row < 0 || row >= get_row_size()) {
    throw std::out_of_range("The row is out of range.");
  }
  if(column == 0) {
    return {};
  }
  if(row < m_source->get_row_size()) {
    return m_source->at(row, column - 1);
  }
  return {};
}

QValidator::State OrderTasksTableViewModel::set(int row, int column,
    const std::any& value) {
  if(row < 0 || row >= get_row_size()) {
    throw std::out_of_range("The row is out of range.");
  }
  if(column == 0) {
    return QValidator::State::Invalid;
  }
  if(row < m_source->get_row_size()) {
    return m_source->set(row, column - 1, value);
  }
  return QValidator::State::Invalid;
}

connection OrderTasksTableViewModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void OrderTasksTableViewModel::on_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      m_transaction.push(TableModel::AddOperation(operation.m_index,
        adjust_row(*operation.m_row)));
    },
    [&] (const TableModel::MoveOperation& operation) {
      m_transaction.push(operation);
    },
    [&] (const TableModel::RemoveOperation& operation) {
      m_transaction.push(TableModel::RemoveOperation(operation.m_index,
        adjust_row(*operation.m_row)));
    },
    [&] (const TableModel::UpdateOperation& operation) {
      m_transaction.push(TableModel::UpdateOperation(operation.m_row,
        operation.m_column + 1, operation.m_previous, operation.m_value));
    });
}
