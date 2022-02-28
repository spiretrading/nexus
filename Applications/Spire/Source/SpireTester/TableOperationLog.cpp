#include "Spire/SpireTester/TableOperationLog.hpp"

using namespace Spire;

TableOperationLog::TableOperationLog(std::shared_ptr<TableModel> model)
  : m_operation_connection(model->connect_operation_signal(
    [=] (const auto& operation) { on_operation(operation); })) {}

int TableOperationLog::operation_count() const {
  return add_count() + remove_count() + move_count() + update_count();
}

int TableOperationLog::add_count() const {
  return static_cast<int>(m_adds.size());
}

const std::vector<TableModel::AddOperation>& TableOperationLog::adds() const {
  return m_adds;
}

int TableOperationLog::remove_count() const {
  return static_cast<int>(m_removes.size());
}

const std::vector<TableModel::RemoveOperation>&
    TableOperationLog::removes() const {
  return m_removes;
}

int TableOperationLog::move_count() const {
  return static_cast<int>(m_moves.size());
}
    
const std::vector<TableModel::MoveOperation>& TableOperationLog::moves() const {
  return m_moves;
}

int TableOperationLog::update_count() const {
  return static_cast<int>(m_updates.size());
}

const std::vector<TableModel::UpdateOperation>&
    TableOperationLog::updates() const {
  return m_updates;
}

void TableOperationLog::on_operation(const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      m_adds.push_back(operation);
    },
    [&] (const TableModel::RemoveOperation& operation) {
      m_removes.push_back(operation);
    },
    [&] (const TableModel::MoveOperation& operation) {
      m_moves.push_back(operation);
    },
    [&] (const TableModel::UpdateOperation& operation) {
      m_updates.push_back(operation);
    });
}
