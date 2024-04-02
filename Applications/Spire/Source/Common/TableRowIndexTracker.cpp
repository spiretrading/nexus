#include "Spire/Spire/TableRowIndexTracker.hpp"

using namespace Spire;

TableRowIndexTracker::TableRowIndexTracker(const TableModel& table, int index)
  : m_connection(table.connect_operation_signal(
      std::bind_front(&TableRowIndexTracker::on_operation, this))),
    m_index(index) {}

int TableRowIndexTracker::get_index() const {
  return m_index;
}

void TableRowIndexTracker::on_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      if(operation.m_index <= m_index) {
        ++m_index;
      }
    },
    [&] (const TableModel::RemoveOperation& operation) {
      if(operation.m_index < m_index) {
        --m_index;
      } else if(operation.m_index == m_index) {
        m_index = -1;
        m_connection.disconnect();
      }
    },
    [&] (const TableModel::MoveOperation& operation) {
      if(operation.m_source == m_index) {
        m_index = operation.m_destination;
      } else if(operation.m_source < m_index &&
          operation.m_destination >= m_index) {
        --m_index;
      } else if(operation.m_source > m_index &&
          operation.m_destination <= m_index) {
        ++m_index;
      }
    });
}
