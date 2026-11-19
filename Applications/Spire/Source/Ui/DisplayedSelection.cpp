#include "Spire/Ui/Details/DisplayedSelection.hpp"
#include <QScopedValueRollback>
#include "Spire/Spire/ListIndexTracker.hpp"

using namespace boost;
using namespace Spire;

namespace {
  int& row_of(int& row) {
    return row;
  }

  int& row_of(TableIndex& index) {
    return index.m_row;
  }

  int& column_of(int& column) {
    return column;
  }

  int& column_of(TableIndex& index) {
    return index.m_column;
  }
}

template<typename T>
void TableView::DisplayedSelection<T>::refresh() {
  m_is_refresh_pending = false;
  auto values = std::vector<T>();
  m_indices.clear();
  for(auto i = 0; i != m_source->get_size();) {
    if(auto value = from_source(m_source->get(i))) {
      m_indices.push_back(i);
      values.push_back(*value);
    } else if(m_tracking_connection.connected()) {
      auto guard = QScopedValueRollback(m_is_pruning, true);
      if(m_source->remove(i) != QValidator::Invalid) {
        continue;
      }
    }
    ++i;
  }
  auto first = 0;
  auto end = this->get_size();
  auto next_end = static_cast<int>(values.size());
  while(first < end && first < next_end && this->get(first) == values[first]) {
    ++first;
  }
  while(end > first && next_end > first &&
      this->get(end - 1) == values[next_end - 1]) {
    --end;
    --next_end;
  }
  this->m_transaction.transact([&] {
    if(end == first + 1 && next_end == end) {
      ArrayListModel<T>::set(first, values[first]);
    } else {
      while(end > first) {
        ArrayListModel<T>::remove(--end);
      }
      while(first < next_end) {
        ArrayListModel<T>::insert(values[first], first);
        ++first;
      }
    }
  });
}

template<typename T>
void TableView::DisplayedSelection<T>::refresh(
    const AnyListModel::MoveOperation& column_move) {
  this->m_transaction.transact([&] {
    for(auto i = 0; i != this->get_size(); ++i) {
      auto value = ArrayListModel<T>::get(i);
      auto& column = column_of(value);
      auto tracker = ListIndexTracker(column);
      tracker.update(column_move);
      if(column != tracker.get_index()) {
        column = tracker.get_index();
        ArrayListModel<T>::set(i, value);
      }
    }
  });
}

template<typename T>
const T& TableView::DisplayedSelection<T>::get(int index) const {
  if(m_is_refresh_pending && m_source->get_size() == this->get_size()) {
    if(auto value = from_source(m_source->get(index))) {
      m_transaction_values.resize(this->get_size());
      m_transaction_values[index] = *value;
      return m_transaction_values[index];
    }
  }
  return ArrayListModel<T>::get(index);
}

template<typename T>
QValidator::State TableView::DisplayedSelection<T>::set(
    int index, const T& value) {
  if(auto translated = to_source(value)) {
    return m_source->set(m_indices.at(index), *translated);
  }
  return QValidator::Invalid;
}

template<typename T>
QValidator::State TableView::DisplayedSelection<T>::push(const T& value) {
  if(auto translated = to_source(value)) {
    return m_source->push(*translated);
  }
  return QValidator::Invalid;
}

template<typename T>
QValidator::State TableView::DisplayedSelection<T>::insert(
    const T& value, int index) {
  if(auto translated = to_source(value)) {
    if(index == this->get_size()) {
      return m_source->insert(*translated, m_source->get_size());
    }
    return m_source->insert(*translated, m_indices.at(index));
  }
  return QValidator::Invalid;
}

template<typename T>
QValidator::State TableView::DisplayedSelection<T>::move(
    int source, int destination) {
  if(!m_tracking_connection.connected()) {
    return QValidator::Invalid;
  }
  return m_source->move(m_indices.at(source), m_indices.at(destination));
}

template<typename T>
QValidator::State TableView::DisplayedSelection<T>::remove(int index) {
  if(!m_tracking_connection.connected()) {
    return QValidator::Invalid;
  }
  return m_source->remove(m_indices.at(index));
}

template<typename T>
void TableView::DisplayedSelection<T>::start_transaction() {
  if(!m_tracking_connection.connected()) {
    ++m_inactive_depth;
    return;
  }
  m_source->start_transaction();
  ArrayListModel<T>::start_transaction();
}

template<typename T>
void TableView::DisplayedSelection<T>::end_transaction() {
  if(m_inactive_depth != 0) {
    --m_inactive_depth;
    return;
  }
  try {
    ArrayListModel<T>::end_transaction();
  } catch(...) {
    m_source->end_transaction();
    throw;
  }
  m_source->end_transaction();
}

template<typename T>
bool TableView::DisplayedSelection<T>::is_tail_append(int row) const {
  if(row != m_sorted->get_row_size() - 1) {
    return false;
  }
  auto filtered = m_sorted->index_to_source(row);
  return m_filtered->index_to_source(filtered) ==
    m_filtered->get_source()->get_row_size() - 1;
}

template<typename T>
optional<T> TableView::DisplayedSelection<T>::from_source(T value) const {
  if(m_filtered) {
    auto& row = row_of(value);
    row = m_filtered->index_from_source(row);
    if(row < 0 || row >= m_sorted->get_row_size()) {
      return none;
    }
    row = m_sorted->index_from_source(row);
  }
  if(m_columns) {
    auto& column = column_of(value);
    if(column < 0 || column >= m_columns->get_size()) {
      return none;
    }
    column = m_columns->index_from_source(column);
  }
  return value;
}

template<typename T>
void TableView::DisplayedSelection<T>::append(int index) {
  if(auto value = from_source(m_source->get(index))) {
    m_indices.push_back(index);
    this->m_transaction.transact([&] {
      ArrayListModel<T>::insert(*value, this->get_size());
    });
  } else if(m_tracking_connection.connected()) {
    refresh();
  }
}

template<typename T>
optional<T> TableView::DisplayedSelection<T>::to_source(T value) const {
  if(!m_tracking_connection.connected()) {
    return none;
  }
  if(m_filtered) {
    auto& row = row_of(value);
    if(row < 0 || row >= m_sorted->get_row_size()) {
      return none;
    }
    row = m_filtered->index_to_source(m_sorted->index_to_source(row));
  }
  if(m_columns) {
    auto& column = column_of(value);
    if(column < 0 || column >= m_columns->get_size()) {
      return none;
    }
    column = m_columns->index_to_source(column);
  }
  return value;
}

template class Spire::TableView::DisplayedSelection<int>;
template class Spire::TableView::DisplayedSelection<TableIndex>;
