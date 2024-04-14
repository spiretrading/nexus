#include "Spire/Spire/ListIndexTracker.hpp"

using namespace Spire;

ListIndexTracker::ListIndexTracker(int index)
  : m_index(index) {}

int ListIndexTracker::get_index() const {
  return m_index;
}

void ListIndexTracker::set(int index) {
  m_index = index;
}

void ListIndexTracker::update(const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::AddOperation& operation) {
      if(operation.m_index <= m_index) {
        ++m_index;
      }
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
      if(operation.m_index < m_index) {
        --m_index;
      } else if(operation.m_index == m_index) {
        m_index = -1;
      }
    },
    [&] (const AnyListModel::MoveOperation& operation) {
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
