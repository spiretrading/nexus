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
  this->m_transaction.transact([&] {
    auto displayed = 0;
    for(auto i = 0; i != m_source->get_size();) {
      refresh_entry(i, displayed);
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
  if(row != m_rows.get_table()->get_row_size() - 1) {
    return false;
  }
  return m_rows.index_to_source(row) ==
    m_rows.get_filtered_table()->get_source()->get_row_size() - 1;
}

template<typename T>
optional<T> TableView::DisplayedSelection<T>::from_source(T value) const {
  if(m_rows.get_filtered_table()) {
    auto& row = row_of(value);
    row = m_rows.index_from_source(row);
    if(row < 0 || row >= m_rows.get_table()->get_row_size()) {
      return none;
    }
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
void TableView::DisplayedSelection<T>::refresh_entry(
    int& source, int& displayed) {
  auto has_entry =
    displayed < this->get_size() && m_indices[displayed] == source;
  if(auto value = from_source(m_source->get(source))) {
    if(!has_entry) {
      m_indices.insert(m_indices.begin() + displayed, source);
      ArrayListModel<T>::insert(*value, displayed);
    } else if(*value != ArrayListModel<T>::get(displayed)) {
      ArrayListModel<T>::set(displayed, *value);
    }
    ++displayed;
  } else {
    if(m_tracking_connection.connected()) {
      auto guard = QScopedValueRollback(m_is_pruning, true);
      if(m_source->remove(source) != QValidator::Invalid) {
        return;
      }
    }
    if(has_entry) {
      m_indices.erase(m_indices.begin() + displayed);
      ArrayListModel<T>::remove(displayed);
    }
  }
  ++source;
}

template<typename T>
void TableView::DisplayedSelection<T>::update_source(int index) {
  if(m_is_refresh_pending) {
    refresh();
    return;
  }
  auto displayed = index;
  if(this->get_size() != m_source->get_size()) {
    auto i = std::ranges::lower_bound(m_indices, index);
    displayed = static_cast<int>(i - m_indices.begin());
  }
  this->m_transaction.transact([&] {
    refresh_entry(index, displayed);
  });
}

template<typename T>
void TableView::DisplayedSelection<T>::remove_source(int index) {
  if(!m_indices.empty() && m_indices.back() == index) {
    m_indices.pop_back();
    ArrayListModel<T>::remove(this->get_size() - 1);
    return;
  }
  auto i = std::ranges::lower_bound(m_indices, index);
  auto displayed = static_cast<int>(i - m_indices.begin());
  auto is_displayed = i != m_indices.end() && *i == index;
  if(is_displayed) {
    i = m_indices.erase(i);
  }
  for(; i != m_indices.end(); ++i) {
    --*i;
  }
  if(is_displayed) {
    ArrayListModel<T>::remove(displayed);
  }
}

template<typename T>
void TableView::DisplayedSelection<T>::move_source(
    const AnyListModel::MoveOperation& move) {
  if(this->get_size() == m_source->get_size()) {
    ArrayListModel<T>::move(move.m_source, move.m_destination);
    return;
  }
  auto [low, high] = std::minmax(move.m_source, move.m_destination);
  auto first = std::ranges::lower_bound(m_indices, low);
  auto end = std::upper_bound(first, m_indices.end(), high);
  if(first == end) {
    return;
  }
  auto source = first;
  auto destination = end - 1;
  if(move.m_source > move.m_destination) {
    std::swap(source, destination);
  }
  auto is_displayed = *source == move.m_source;
  for(auto i = first; i != end; ++i) {
    auto tracker = ListIndexTracker(*i);
    tracker.update(move);
    *i = tracker.get_index();
  }
  if(is_displayed) {
    auto from = static_cast<int>(source - m_indices.begin());
    auto to = static_cast<int>(destination - m_indices.begin());
    move_element(m_indices, from, to);
    ArrayListModel<T>::move(from, to);
  }
}

template<typename T>
optional<T> TableView::DisplayedSelection<T>::to_source(T value) const {
  if(!m_tracking_connection.connected()) {
    return none;
  }
  if(m_rows.get_filtered_table()) {
    auto& row = row_of(value);
    if(row < 0 || row >= m_rows.get_table()->get_row_size()) {
      return none;
    }
    row = m_rows.index_to_source(row);
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
