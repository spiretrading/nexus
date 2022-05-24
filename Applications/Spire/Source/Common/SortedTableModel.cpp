#include "Spire/Spire/SortedTableModel.hpp"
#include <algorithm>
#include <boost/iterator/counting_iterator.hpp>
#include "Spire/Ui/DecimalBox.hpp"

using namespace boost::iterators;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;

namespace {
  template<typename T>
  bool compare(const AnyRef& lhs, const AnyRef& rhs) {
    return any_cast<T>(lhs) < any_cast<T>(rhs);
  }

  bool default_comparator(const AnyRef& lhs, const AnyRef& rhs) {
    if(lhs.get_type() != rhs.get_type()) {
      return false;
    } else if(lhs.get_type() == typeid(int)) {
      return compare<int>(lhs, rhs);
    } else if(lhs.get_type() == typeid(QString)) {
      return compare<QString>(lhs, rhs);
    } else if(lhs.get_type() == typeid(Decimal)) {
      return compare<Decimal>(lhs, rhs);
    } else if(lhs.get_type() == typeid(time_duration)) {
      return compare<time_duration>(lhs, rhs);
    }
    return false;
  }
}

SortedTableModel::SortedTableModel(std::shared_ptr<TableModel> source)
  : SortedTableModel(std::move(source), std::vector<ColumnOrder>()) {}

SortedTableModel::SortedTableModel(
  std::shared_ptr<TableModel> source, std::vector<ColumnOrder> order)
  : SortedTableModel(std::move(source), std::move(order), default_comparator) {}

SortedTableModel::SortedTableModel(
  std::shared_ptr<TableModel> source, Comparator comparator)
  : SortedTableModel(std::move(source), {}, std::move(comparator)) {}

SortedTableModel::SortedTableModel(std::shared_ptr<TableModel> source,
    std::vector<ColumnOrder> order, Comparator comparator)
    : m_source(std::move(source)),
      m_translation(m_source),
      m_comparator(std::move(comparator)) {
  set_column_order(order);
  m_source_connection = m_translation.connect_operation_signal(
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
  sort();
}

int SortedTableModel::get_row_size() const {
  return m_translation.get_row_size();
}

int SortedTableModel::get_column_size() const {
  return m_translation.get_column_size();
}

AnyRef SortedTableModel::at(int row, int column) const {
  return m_translation.at(row, column);
}

QValidator::State
    SortedTableModel::set(int row, int column, const std::any& value) {
  return m_translation.set(row, column, value);
}

connection SortedTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

bool SortedTableModel::row_comparator(int lhs, int rhs) const {
  for(auto& order : m_order) {
    if(order.m_order == Ordering::NONE) {
      continue;
    }
    auto is_lesser = m_comparator(m_translation.at(lhs, order.m_index),
      m_translation.at(rhs, order.m_index));
    if(is_lesser) {
      return order.m_order == Ordering::ASCENDING;
    }
    auto is_greater = m_comparator(m_translation.at(rhs, order.m_index),
      m_translation.at(lhs, order.m_index));
    if(is_greater) {
      return order.m_order == Ordering::DESCENDING;
    }
  }
  return m_translation.get_translation_to_source(lhs) <
    m_translation.get_translation_to_source(rhs);
}

void SortedTableModel::sort() {
  m_transaction.transact([&] {
    m_translation.transact([&] {
      for(auto i = 1; i < get_row_size(); ++i) {
        auto index = find_sorted_index(i, i + 1);
        if(index != i) {
          m_translation.move(i, index);
          m_transaction.push(MoveOperation(i, index));
        }
      }
    });
  });
}

int SortedTableModel::find_sorted_index(int row, int size) const {
  if(row != 0 && row_comparator(row, row - 1)) {
    return *std::lower_bound(
      make_counting_iterator(0), make_counting_iterator(row), row,
      [&] (auto lhs, auto rhs) { return row_comparator(lhs, rhs); });
  } else if(row != size - 1 && row_comparator(row + 1, row)) {
    return *std::lower_bound(make_counting_iterator(row + 1),
      make_counting_iterator(size), row,
      [&] (auto lhs, auto rhs) { return row_comparator(lhs, rhs); }) - 1;
  }
  return row;
}

void SortedTableModel::on_operation(const Operation& operation) {
  m_transaction.transact([&] {
    visit(operation,
      [&] (const AddOperation& operation) {
        auto index = find_sorted_index(operation.m_index, get_row_size());
        m_translation.move(operation.m_index, index);
        m_transaction.push(AddOperation(index, operation.m_row));
      },
      [&] (const UpdateOperation& operation) {
        auto index = find_sorted_index(operation.m_row, get_row_size());
        if(operation.m_row != index) {
          m_translation.move(operation.m_row, index);
          m_transaction.push(MoveOperation(operation.m_row, index));
        }
        m_transaction.push(UpdateOperation(
          index, operation.m_column, operation.m_previous, operation.m_value));
      },
      [&] (const RemoveOperation& operation) {
        m_transaction.push(operation);
      });
    });
}

void Spire::adjust(SortedTableModel::ColumnOrder order,
    std::vector<SortedTableModel::ColumnOrder>& column_order) {
  auto i = std::find_if(column_order.begin(), column_order.end(),
    [&] (const auto& value) {
      return value.m_index == order.m_index;
    });
  if(i == column_order.begin() && i != column_order.end()) {
    *i = order;
    return;
  } else if(i != column_order.end()) {
    column_order.erase(i);
  }
  column_order.insert(column_order.begin(), order);
}

SortedTableModel::ColumnOrder
    Spire::cycle(SortedTableModel::ColumnOrder order) {
  if(order.m_order == SortedTableModel::Ordering::NONE) {
    order.m_order = SortedTableModel::Ordering::ASCENDING;
  } else if(order.m_order == SortedTableModel::Ordering::ASCENDING) {
    order.m_order = SortedTableModel::Ordering::DESCENDING;
  } else {
    order.m_order = SortedTableModel::Ordering::NONE;
  }
  return order;
}
