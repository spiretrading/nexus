#include "Spire/Ui/ColumnViewListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

ColumnViewListModel::ColumnViewListModel(std::shared_ptr<TableModel> source,
    int column)
    : m_source(std::move(source)) {
  if(column < 0 || column >= m_source->get_column_size()) {
    m_column = -1;
    m_source = nullptr;
  } else {
    m_column = column;
    m_source_connection = m_source->connect_operation_signal(
      std::bind_front(&ColumnViewListModel::on_operation, this));
  }
}

int ColumnViewListModel::get_size() const {
  if(m_column == -1) {
    return 0;
  }
  return m_source->get_row_size();
}

const std::any& ColumnViewListModel::at(int index) const {
  if(m_column == -1) {
    throw std::out_of_range("The column is out of range.");
  }
  return m_source->at(index, m_column);
}

QValidator::State ColumnViewListModel::set(int index, const std::any& value) {
  if(m_column == -1) {
    return QValidator::State::Invalid;
  }
  return m_source->set(index, m_column, value);
}

connection ColumnViewListModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void ColumnViewListModel::on_operation(const TableModel::Operation& operation) {
  m_transaction.transact([&] {
    visit(operation,
      [&] (const TableModel::AddOperation& operation) {
        m_transaction.push(AddOperation{operation.m_index});
      },
      [&] (const TableModel::MoveOperation& operation) {
        m_transaction.push(
          MoveOperation{operation.m_source, operation.m_destination});
      },
      [&] (const TableModel::RemoveOperation& operation) {
        m_transaction.push(RemoveOperation{operation.m_index});
      },
      [&] (const TableModel::UpdateOperation& operation) {
        if(m_column == operation.m_column) {
          m_transaction.push(UpdateOperation{operation.m_row});
        }
      });
    });
}
