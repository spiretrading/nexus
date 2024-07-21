#include "Spire/Spire/TableToListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

AnyRef RowView::get(int column) const {
  return m_table->at(m_row, column);
}

QValidator::State RowView::set(int column, const std::any& value) const {
  return m_table->set(m_row, column, value);
}

RowView::RowView(TableModel& table, int row)
  : m_table(&table),
    m_row(row) {}

TableToListModel::TableToListModel(std::shared_ptr<TableModel> table)
    : m_table(std::move(table)),
      m_update(nullptr) {
  m_connection = m_table->connect_operation_signal(
    std::bind_front(&TableToListModel::on_operation, this));
}

const TableModel::UpdateOperation& TableToListModel::get_update() const {
  if(m_update) {
    return *m_update;
  }
  static const auto NONE = TableModel::UpdateOperation();
  return NONE;
}

int TableToListModel::get_size() const {
  return m_table->get_row_size();
}

const TableToListModel::Type& TableToListModel::get(int index) const {
  m_last.emplace(RowView(*m_table, index));
  return *m_last;
}

QValidator::State TableToListModel::set(int index, const Type& value) {
  return QValidator::State::Invalid;
}

QValidator::State TableToListModel::push(const Type& value) {
  return QValidator::State::Invalid;
}

QValidator::State TableToListModel::insert(const Type& value, int index) {
  return QValidator::State::Invalid;
}

QValidator::State TableToListModel::move(int source, int destination) {
  return QValidator::State::Invalid;
}

QValidator::State TableToListModel::remove(int index) {
  return m_table->remove(index);
}

connection TableToListModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void TableToListModel::transact(const std::function<void ()>& transaction) {
  m_transaction.transact(transaction);
}

void TableToListModel::on_operation(const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::StartTransaction&) {
      m_transaction.start();
    },
    [&] (const TableModel::EndTransaction&) {
      m_transaction.end();
    },
    [&] (const TableModel::AddOperation& operation) {
      m_transaction.push(AddOperation(operation.m_index));
    },
    [&] (const TableModel::PreRemoveOperation& operation) {
      m_transaction.push(PreRemoveOperation(operation.m_index));
    },
    [&] (const TableModel::RemoveOperation& operation) {
      m_transaction.push(RemoveOperation(operation.m_index));
    },
    [&] (const TableModel::MoveOperation& operation) {
      m_transaction.push(
        MoveOperation(operation.m_source, operation.m_destination));
    },
    [&] (const TableModel::UpdateOperation& operation) {
      m_update = &operation;
      m_transaction.push(UpdateOperation(operation.m_row,
        RowView(*m_table, operation.m_row),
        RowView(*m_table, operation.m_row)));
      m_update = nullptr;
    });
}
