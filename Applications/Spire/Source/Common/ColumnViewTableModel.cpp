#include "Spire/Spire/ColumnViewTableModel.hpp"

using namespace boost::signals2;
using namespace Spire;

ColumnViewTableModel::ColumnViewTableModel(std::shared_ptr<TableModel> source,
  int column)
  : m_source(std::move(source)),
    m_column(column),
    m_source_connection(m_source->connect_operation_signal(
      std::bind_front(&ColumnViewTableModel::on_operation, this))) {}

int ColumnViewTableModel::get_row_size() const {
  return m_source->get_row_size();
}

int ColumnViewTableModel::get_column_size() const {
  return 1;
}

AnyRef ColumnViewTableModel::at(int row, int column) const {
  if(column != 0) {
    throw std::out_of_range("The column is out of range.");
  }
  return m_source->at(row, m_column);
}

connection ColumnViewTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void ColumnViewTableModel::on_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::UpdateOperation& operation) {
      if(operation.m_column != m_column) {
        return;
      }
      m_transaction.push(TableModel::UpdateOperation(operation.m_row, 0,
        operation.m_previous, operation.m_value));
    },
    [&] (const auto& operation) {
      m_transaction.push(operation);
    });
}
