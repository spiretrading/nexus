#include "Spire/Ui/Details/DisplayedSelection.hpp"
#include <boost/signals2/shared_connection_block.hpp>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  int& row_of(int& row) {
    return row;
  }

  int& row_of(TableIndex& index) {
    return index.m_row;
  }
}

template<typename T>
void TableView::DisplayedSelection<T>::refresh() {
  auto values = std::vector<T>();
  m_indices.clear();
  for(auto i = 0; i != m_source->get_size();) {
    if(auto value = from_source(m_source->get(i))) {
      m_indices.push_back(i);
      values.push_back(*value);
    } else if(m_tracking_connection.connected()) {
      auto blocker = shared_connection_block(m_connection);
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
  auto apply = std::function<void ()>([&] {
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
  ArrayListModel<T>::transact(apply);
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
void TableView::DisplayedSelection<T>::transact(
    const std::function<void ()>& transaction) {
  m_source->transact([&] {
    ArrayListModel<T>::transact(transaction);
  });
}

template<typename T>
optional<T> TableView::DisplayedSelection<T>::from_source(T value) const {
  auto& row = row_of(value);
  row = m_filtered->index_from_source(row);
  if(row < 0 || row >= m_sorted->get_row_size()) {
    return none;
  }
  row = m_sorted->index_from_source(row);
  return value;
}

template<typename T>
void TableView::DisplayedSelection<T>::append(int index) {
  if(auto value = from_source(m_source->get(index))) {
    m_indices.push_back(index);
    ArrayListModel<T>::transact({[&] {
      ArrayListModel<T>::insert(*value, this->get_size());
    }});
  } else if(m_tracking_connection.connected()) {
    refresh();
  }
}

template<typename T>
optional<T> TableView::DisplayedSelection<T>::to_source(T value) const {
  auto& row = row_of(value);
  if(!m_tracking_connection.connected() || row < 0 ||
      row >= m_sorted->get_row_size()) {
    return none;
  }
  row = m_filtered->index_to_source(m_sorted->index_to_source(row));
  return value;
}

template class Spire::TableView::DisplayedSelection<int>;
template class Spire::TableView::DisplayedSelection<TableIndex>;
