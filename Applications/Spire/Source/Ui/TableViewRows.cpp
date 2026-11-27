#include "Spire/Ui/Details/TableViewRows.hpp"

using namespace Spire;

TableView::Rows::Rows(std::shared_ptr<FilteredTableModel> filtered_table,
    std::shared_ptr<SortedTableModel> sorted_table) noexcept
    : m_filtered_table(std::move(filtered_table)),
      m_sorted_table(std::move(sorted_table)) {}

const std::shared_ptr<FilteredTableModel>&
    TableView::Rows::get_filtered_table() const {
  return m_filtered_table;
}

std::shared_ptr<TableModel> TableView::Rows::get_table() const {
  if(m_sorted_table) {
    return m_sorted_table;
  }
  return m_filtered_table;
}

int TableView::Rows::index_from_source(int row) const {
  row = m_filtered_table->index_from_source(row);
  if(m_sorted_table && row >= 0) {
    if(row >= m_sorted_table->get_row_size()) {
      return -1;
    }
    row = m_sorted_table->index_from_source(row);
  }
  return row;
}

int TableView::Rows::index_to_source(int row) const {
  if(m_sorted_table) {
    row = m_sorted_table->index_to_source(row);
  }
  return m_filtered_table->index_to_source(row);
}
