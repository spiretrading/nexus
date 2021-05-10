#include "Spire/Ui/FilteredTableModel.hpp"
#include <algorithm>

using namespace boost::signals2;
using namespace Spire;

FilteredTableModel::FilteredTableModel(std::shared_ptr<TableModel> source,
  std::function<bool(const TableModel&, int)> filter)
    : m_source(source),
      m_filter(filter) {
  for(auto i = 0; i != m_source->get_row_size(); ++i) {
    if(!m_filter(*m_source.get(), i)) {
      m_filtered_data.push_back(i);
    }
  }
  m_source_connection = m_source->connect_operation_signal(
    [=] (const auto& operation) { on_operation(operation); });
}

int FilteredTableModel::get_row_size() const {
  return m_filtered_data.size();
}

int FilteredTableModel::get_column_size() const {
  return m_source->get_column_size();
}

const std::any& FilteredTableModel::at(int row, int column) const {
  if(row < 0 || row >= get_row_size()) {
    throw std::out_of_range("The row is out of range.");
  }
  return m_source->at(m_filtered_data[row], column);
}

QValidator::State FilteredTableModel::set(int row, int column,
    const std::any& value) {
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
  auto iter = std::lower_bound(m_filtered_data.begin(), m_filtered_data.end(),
    index);
  if(iter != m_filtered_data.end() && *iter == index) {
    return std::make_tuple(true, iter); 
  }
  return std::make_tuple(false, iter);
}

void FilteredTableModel::on_operation(const Operation& operation) {
  m_transaction.transact([&] {
    visit(operation,
      [&] (const AddOperation& operation) {
        if(operation.m_index >= m_source->get_row_size() - 1) {
          if(!m_filter(*m_source.get(), operation.m_index)) {
            m_filtered_data.push_back(operation.m_index);
            m_transaction.push(AddOperation{
              static_cast<int>(m_filtered_data.size()) - 1});
          }
        } else {
          auto iter = std::get<1>(find(operation.m_index));
          std::for_each(iter, m_filtered_data.end(),
            [] (int& value) { ++value; });
          if(!m_filter(*m_source.get(), operation.m_index)) {
            m_transaction.push(AddOperation{m_filtered_data.insert(iter,
              operation.m_index) - m_filtered_data.begin()});
          }
        }
      },
      [&] (const MoveOperation& operation) {
        auto source_row = find(operation.m_source);
        auto source = std::get<1>(source_row);
        if(std::get<0>(source_row)) {
          auto destination = source;
          if(operation.m_source < operation.m_destination) {
            destination = std::upper_bound(source, m_filtered_data.end(),
              operation.m_destination) - 1;
            for(auto iter = source; iter != destination; ++iter) {
              *iter = *(iter + 1) - 1;
            }
          } else {
            destination = std::lower_bound(m_filtered_data.begin(), source,
              operation.m_destination);
            for(auto iter = source; iter != destination; --iter) {
              *iter = *(iter - 1) + 1;
            }
          }
          *destination = operation.m_destination;
          m_transaction.push(MoveOperation{source - m_filtered_data.begin(),
            destination - m_filtered_data.begin()});
        } else {
          if(operation.m_source < operation.m_destination) {
            auto destination = std::upper_bound(source, m_filtered_data.end(),
              operation.m_destination);
            std::for_each(source, destination, [] (int& value) { --value; });
          } else {
            auto destination = std::lower_bound(m_filtered_data.begin(), source,
              operation.m_destination);
            std::for_each(destination, source, [] (int& value) { ++value; });
          }
        }
      },
      [&] (const RemoveOperation& operation) {
        auto data = find(operation.m_index);
        auto iter = std::get<1>(data);
        std::for_each(iter, m_filtered_data.end(), [] (int& value) { --value; });
        if(std::get<0>(data)) {
          auto index = iter - m_filtered_data.begin();
          m_filtered_data.erase(iter);
          m_transaction.push(RemoveOperation{index});
        }
      },
      [&] (const UpdateOperation& operation) {
        auto data = find(operation.m_row);
        auto iter = std::get<1>(data);
        if(!m_filter(*m_source.get(), operation.m_row)) {
          if(std::get<0>(data)) {
            m_transaction.push(UpdateOperation{iter - m_filtered_data.begin(),
              operation.m_column});
          } else {
            m_transaction.push(AddOperation{m_filtered_data.insert(iter,
              operation.m_row) - m_filtered_data.begin()});
          }
        } else {
          if(std::get<0>(data)) {
            auto index = iter - m_filtered_data.begin();
            m_filtered_data.erase(iter);
            m_transaction.push(RemoveOperation{index});
          }
        }
      });
  });
}
