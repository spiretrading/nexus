#include "Spire/Ui/FilteredTableModel.hpp"
#include <algorithm>
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost::signals2;
using namespace Spire;

FilteredTableModel::FilteredTableModel(
    std::shared_ptr<TableModel> source, Filter filter)
    : m_source(std::move(source)),
      m_filter(std::move(filter)) {
  for(auto i = 0; i != m_source->get_row_size(); ++i) {
    if(!m_filter(*m_source, i)) {
      m_filtered_data.push_back(i);
    }
  }
  m_source_connection = m_source->connect_operation_signal(
    [=] (const auto& operation) { on_operation(operation); });
}

void FilteredTableModel::set_filter(const Filter& filter) {
  m_filter = filter;
  auto source_row = 0;
  auto filtered_row = 0;
  m_transaction.transact([&] {
    while(source_row != m_source->get_row_size() &&
        filtered_row != static_cast<int>(m_filtered_data.size())) {
      if(!m_filter(*m_source, source_row)) {
        if(m_filtered_data[filtered_row] != source_row) {
          m_filtered_data.insert(
            m_filtered_data.begin() + filtered_row, source_row);
          auto value = std::make_shared<ArrayListModel<std::any>>();
          for(auto i = 0; i != get_column_size(); ++i) {
            value->push(at(filtered_row, i));
          }
          m_transaction.push(AddOperation(filtered_row, std::move(value)));
        }
        ++filtered_row;
      } else {
        if(m_filtered_data[filtered_row] == source_row) {
          auto value = std::make_shared<ArrayListModel<std::any>>();
          for(auto i = 0; i != get_column_size(); ++i) {
            value->push(at(filtered_row, i));
          }
          m_filtered_data.erase(m_filtered_data.begin() + filtered_row);
          m_transaction.push(RemoveOperation(filtered_row, std::move(value)));
        }
      }
      ++source_row;
    }
    while(filtered_row != static_cast<int>(m_filtered_data.size())) {
      auto value = std::make_shared<ArrayListModel<std::any>>();
      for(auto i = 0; i != get_column_size(); ++i) {
        value->push(at(filtered_row, i));
      }
      m_filtered_data.erase(m_filtered_data.begin() + filtered_row);
      m_transaction.push(RemoveOperation(filtered_row, std::move(value)));
    }
    while(source_row != m_source->get_row_size()) {
      if(!m_filter(*m_source, source_row)) {
        m_filtered_data.push_back(source_row);
        auto value = std::make_shared<ArrayListModel<std::any>>();
        for(auto i = 0; i != get_column_size(); ++i) {
          value->push(at(m_filtered_data.size() - 1, i));
        }
        m_transaction.push(
          AddOperation(m_filtered_data.size() - 1, std::move(value)));
      }
      ++source_row;
    }
  });
}

int FilteredTableModel::get_row_size() const {
  return m_filtered_data.size();
}

int FilteredTableModel::get_column_size() const {
  return m_source->get_column_size();
}

AnyRef FilteredTableModel::at(int row, int column) const {
  if(row < 0 || row >= get_row_size()) {
    throw std::out_of_range("The row is out of range.");
  }
  return m_source->at(m_filtered_data[row], column);
}

QValidator::State FilteredTableModel::set(
    int row, int column, const std::any& value) {
  if(row < 0 || row >= get_row_size()) {
    throw std::out_of_range("The row is out of range.");
  }
  return m_source->set(m_filtered_data[row], column, value);
}

connection FilteredTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

std::tuple<bool, std::vector<int>::iterator> FilteredTableModel::find(
    int index) {
  auto i = std::lower_bound(m_filtered_data.begin(), m_filtered_data.end(),
    index);
  if(i != m_filtered_data.end() && *i == index) {
    return {true, i};
  }
  return {false, i};
}

void FilteredTableModel::on_operation(const Operation& operation) {
  m_transaction.transact([&] {
    visit(operation,
      [&] (const AddOperation& operation) {
        if(operation.m_index >= m_source->get_row_size() - 1) {
          if(!m_filter(*m_source, operation.m_index)) {
            m_filtered_data.push_back(operation.m_index);
            m_transaction.push(AddOperation(
              static_cast<int>(m_filtered_data.size()) - 1, operation.m_row));
          }
        } else {
          auto i = std::get<1>(find(operation.m_index));
          std::for_each(i, m_filtered_data.end(),
            [] (int& value) { ++value; });
          if(!m_filter(*m_source, operation.m_index)) {
            m_transaction.push(AddOperation(
              static_cast<int>(m_filtered_data.insert(i, operation.m_index) -
                m_filtered_data.begin()), operation.m_row));
          }
        }
      },
      [&] (const MoveOperation& operation) {
        auto [is_found, source] = find(operation.m_source);
        if(is_found) {
          auto destination = source;
          if(operation.m_source < operation.m_destination) {
            destination = std::upper_bound(source, m_filtered_data.end(),
              operation.m_destination) - 1;
            for(auto i = source; i != destination; ++i) {
              *i = *(i + 1) - 1;
            }
          } else {
            destination = std::lower_bound(m_filtered_data.begin(), source,
              operation.m_destination);
            for(auto i = source; i != destination; --i) {
              *i = *(i - 1) + 1;
            }
          }
          *destination = operation.m_destination;
          m_transaction.push(MoveOperation(
            static_cast<int>(source - m_filtered_data.begin()),
            static_cast<int>(destination - m_filtered_data.begin())));
        } else if(operation.m_source < operation.m_destination) {
            auto destination = std::upper_bound(source, m_filtered_data.end(),
              operation.m_destination);
            std::for_each(source, destination, [] (int& value) { --value; });
        } else {
          auto destination = std::lower_bound(m_filtered_data.begin(), source,
            operation.m_destination);
          std::for_each(destination, source, [] (int& value) { ++value; });
        }
      },
      [&] (const RemoveOperation& operation) {
        auto [is_found, i] = find(operation.m_index);
        std::for_each(i, m_filtered_data.end(), [] (int& value) { --value; });
        if(is_found) {
          auto index = static_cast<int>(i - m_filtered_data.begin());
          m_filtered_data.erase(i);
          m_transaction.push(RemoveOperation(index, operation.m_row));
        }
      },
      [&] (const UpdateOperation& operation) {
        auto [is_found, i] = find(operation.m_row);
        if(!m_filter(*m_source, operation.m_row)) {
          if(is_found) {
            m_transaction.push(UpdateOperation(
              static_cast<int>(i - m_filtered_data.begin()),
              operation.m_column, operation.m_previous, operation.m_value));
          } else {
            auto filtered_row = static_cast<int>(m_filtered_data.insert(
              i, operation.m_row) - m_filtered_data.begin());
            auto value = std::make_shared<ArrayListModel<std::any>>();
            for(auto i = 0; i != get_column_size(); ++i) {
              value->push(at(filtered_row, i));
            }
            m_transaction.push(AddOperation(filtered_row, std::move(value)));
          }
        } else if(is_found) {
          auto index = static_cast<int>(i - m_filtered_data.begin());
          auto value = std::make_shared<ArrayListModel<std::any>>();
          for(auto i = 0; i != get_column_size(); ++i) {
            value->push(at(index, i));
          }
          m_filtered_data.erase(i);
          m_transaction.push(RemoveOperation(index, std::move(value)));
        }
      });
  });
}
