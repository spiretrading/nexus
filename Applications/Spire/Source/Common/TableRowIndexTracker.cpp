#include "Spire/Spire/TableRowIndexTracker.hpp"

using namespace Spire;

TableRowIndexTracker::TableRowIndexTracker(int index)
  : m_tracker(index) {}

int TableRowIndexTracker::get_index() const {
  return m_tracker.get_index();
}

void TableRowIndexTracker::set(int index) {
  m_tracker.set(index);
}

void TableRowIndexTracker::update(const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      m_tracker.update(AnyListModel::AddOperation(operation.m_index));
    },
    [&] (const TableModel::PreRemoveOperation& operation) {
      m_tracker.update(AnyListModel::PreRemoveOperation(operation.m_index));
    },
    [&] (const TableModel::MoveOperation& operation) {
      m_tracker.update(AnyListModel::MoveOperation(
        operation.m_source, operation.m_destination));
    });
}
