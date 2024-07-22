#include "Spire/Spire/FilteredTableModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto make_filter(const TableModel& table, FilteredTableModel::Filter filter) {
    return [=, &table] (const ListModel<RowView>& list, int index) {
      return filter(table, index);
    };
  }
}

FilteredTableModel::FilteredTableModel(
    std::shared_ptr<TableModel> source, Filter filter)
    : m_source(std::move(source)),
      m_list(std::make_shared<TableToListModel>(m_source)),
      m_filtered_list(m_list, make_filter(*m_source, std::move(filter))) {
  m_connection = m_filtered_list.connect_operation_signal(
    std::bind_front(&FilteredTableModel::on_operation, this));
}

void FilteredTableModel::set_filter(const Filter& filter) {
  m_filtered_list.set_filter(make_filter(*m_source, filter));
}

int FilteredTableModel::get_row_size() const {
  return m_filtered_list.get_size();
}

int FilteredTableModel::get_column_size() const {
  return m_source->get_column_size();
}

AnyRef FilteredTableModel::at(int row, int column) const {
  return m_filtered_list.get(row).get(column);
}

QValidator::State FilteredTableModel::set(
    int row, int column, const std::any& value) {
  return m_filtered_list.get(row).set(column, value);
}

QValidator::State FilteredTableModel::remove(int row) {
  return m_filtered_list.remove(row);
}

connection FilteredTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void FilteredTableModel::on_operation(
    const ListModel<RowView>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<RowView>::UpdateOperation& operation) {
      m_transaction.push(UpdateOperation(operation.m_index,
        m_list->get_update().m_column, m_list->get_update().m_previous,
        m_list->get_update().m_value));
    },
    [&] (const auto& operation) {
      m_transaction.push(to_table_operation(operation));
    });
}
