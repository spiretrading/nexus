#include "Spire/Ui/SortedTableModel.hpp"
#include <algorithm>
#include "Spire/Ui/DecimalBox.hpp"

using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;

SortedTableModel::ColumnOrder SortedTableModel::ColumnOrder::cycle() const {
  if(m_order == Ordering::NONE) {
    return ColumnOrder{m_index, Ordering::ASCENDING};
  } else if(m_order == Ordering::ASCENDING) {
    return ColumnOrder{m_index, Ordering::DESCENDING};
  } 
  return ColumnOrder{m_index, Ordering::NONE};
}

SortedTableModel::SortedTableModel(std::shared_ptr<TableModel> source)
  : SortedTableModel(source, m_order) {}

SortedTableModel::SortedTableModel(std::shared_ptr<TableModel> source,
  std::vector<ColumnOrder> order)
    : SortedTableModel(source, order, get_default_comparator()) {}

SortedTableModel::SortedTableModel(std::shared_ptr<TableModel> source,
  Comparator comparator)
    : SortedTableModel(source, m_order, comparator) {}

SortedTableModel::SortedTableModel(std::shared_ptr<TableModel> source,
  std::vector<ColumnOrder> order, Comparator comparator)
    : m_source(std::move(source)),
      m_order(std::move(order)),
      m_comparator(std::move(comparator)),
      m_mapping(m_source->get_row_size()),
      m_reverse_mapping(m_source->get_row_size()) {
  std::iota(m_mapping.begin(), m_mapping.end(), 0);
  m_reverse_mapping = m_mapping;
  sort();
  m_source_connection = m_source->connect_operation_signal(
    [=] (const auto& operation) { on_operation(operation); });
}

const SortedTableModel::Comparator& SortedTableModel::get_comparator() const {
  return m_comparator;
}

const std::vector<SortedTableModel::ColumnOrder>&
    SortedTableModel::get_column_order() const {
  return m_order;
}

void SortedTableModel::set_column_order(const std::vector<ColumnOrder>& order) {
  m_order = order;
  std::iota(m_mapping.begin(), m_mapping.end(), 0);
  m_reverse_mapping = m_mapping;
  sort();
}

int SortedTableModel::get_row_size() const {
  return m_source->get_row_size();
}

int SortedTableModel::get_column_size() const {
  return m_source->get_column_size();
}

const std::any& SortedTableModel::at(int row, int column) const {
  return m_source->at(m_mapping[row], column);
}

QValidator::State SortedTableModel::set(int row, int column,
    const std::any& value) {
  return m_source->set(m_mapping[row], column, value);
}

connection SortedTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

SortedTableModel::Comparator SortedTableModel::get_default_comparator() {
  return [&] (const std::any& lhs, const std::any& rhs) {
    if(lhs.type() == rhs.type()) {
      if(lhs.type() == typeid(int)) {
        return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
      } else if(lhs.type() == typeid(QString)) {
        return std::any_cast<QString>(lhs) < std::any_cast<QString>(rhs);
      } else if(lhs.type() == typeid(DecimalBox::Decimal)) {
        return std::any_cast<DecimalBox::Decimal>(lhs) <
          std::any_cast<DecimalBox::Decimal>(rhs);
      } else if(lhs.type() == typeid(time_duration)) {
        return std::any_cast<time_duration>(lhs) <
          std::any_cast<time_duration>(rhs);
      }
    }
    return false;
  };
}

void SortedTableModel::sort() {
  if(!m_order.empty() && m_order.front().m_order != Ordering::NONE) {
    std::stable_sort(m_mapping.begin(), m_mapping.end(),
      [&] (int lhs, int rhs) {
        for(auto i = m_order.begin(); i != m_order.end(); ++i) {
          if(i->m_index < 0 || i->m_index > get_column_size()) {
            break;
          }
          if(i->m_order == Ordering::ASCENDING) {
            if(m_comparator(m_source->at(lhs, i->m_index),
              m_source->at(rhs, i->m_index))) {
              return true;
            } else if(m_comparator(m_source->at(rhs, i->m_index),
              m_source->at(lhs, i->m_index))) {
              return false;
            }
          } else if(i->m_order == Ordering::DESCENDING) {
            if(m_comparator(m_source->at(rhs, i->m_index),
              m_source->at(lhs, i->m_index))) {
              return true;
            } else if(m_comparator(m_source->at(lhs, i->m_index),
              m_source->at(rhs, i->m_index))) {
              return false;
            }
          } else {
            return false;
          }
        }
        return false;
      });
    for(auto i = m_mapping.begin(); i != m_mapping.end(); ++i) {
      m_reverse_mapping[*i] = static_cast<int>(i - m_mapping.begin());
    }
  }
}

void SortedTableModel::translate(int direction, int row) {
  auto row_index = m_reverse_mapping[row];
  for(auto i = 0; i != static_cast<int>(m_mapping.size()); ++i) {
    if(m_mapping[i] >= row) {
      m_mapping[i] += direction;
    }
    if(m_reverse_mapping[i] >= row_index) {
      m_reverse_mapping[i] += direction;
    }
  }
}

void SortedTableModel::on_operation(const Operation& operation) {
  m_transaction.transact([&] {
    visit(operation,
      [&] (const AddOperation& operation) {
        auto added_index = [&] {
          if(operation.m_index >= static_cast<int>(m_mapping.size())) {
            m_mapping.push_back(operation.m_index);
            m_reverse_mapping.push_back(operation.m_index);
            return operation.m_index;
          } else {
            auto row_index = m_reverse_mapping[operation.m_index];
            translate(1, operation.m_index);
            m_mapping.insert(m_mapping.begin() + row_index,
              operation.m_index);
            m_reverse_mapping.insert(
              m_reverse_mapping.begin() + operation.m_index, row_index);
            return row_index;
          }
        }();
        if(m_order.empty() || m_order.front().m_order == Ordering::NONE) {
          m_transaction.push(AddOperation{added_index});
        } else {
          sort();
          m_transaction.push(AddOperation{m_reverse_mapping[operation.m_index]});
        }
      },
      [&] (const RemoveOperation& operation) {
        auto row_index = m_reverse_mapping[operation.m_index];
        translate(-1, operation.m_index);
        m_mapping.erase(m_mapping.begin() + row_index);
        m_reverse_mapping.erase(m_reverse_mapping.begin() + operation.m_index);
        m_transaction.push(RemoveOperation{row_index});
      },
      [&] (const UpdateOperation& operation) {
        auto row_index = m_reverse_mapping[operation.m_row];
        m_transaction.push(UpdateOperation{row_index, operation.m_column});
        auto is_sorted_column = [&] {
          for(auto i = m_order.begin(); i != m_order.end(); ++i) {
            if(i->m_order == Ordering::NONE) {
              break;
            } else if(i->m_index == operation.m_column) {
              return true;
            }
          }
          return false;
        }();
        if(!is_sorted_column) {
          return;
        }
        sort();
        if(row_index != m_reverse_mapping[operation.m_row]) {
          m_transaction.push(MoveOperation{row_index,
            m_reverse_mapping[operation.m_row]});
        }
      });
    });
}

void Spire::adjust(SortedTableModel::ColumnOrder order,
    std::vector<SortedTableModel::ColumnOrder>& column_order) {
  auto i = std::find_if(column_order.begin(), column_order.end(),
    [&] (const SortedTableModel::ColumnOrder& value) {
      return value.m_index == order.m_index;
    });
  if(i != column_order.end()) {
    column_order.erase(i);
  }
  column_order.insert(column_order.begin(), order);
}
