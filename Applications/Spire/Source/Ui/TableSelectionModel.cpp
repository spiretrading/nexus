#include "Spire/Ui/TableSelectionModel.hpp"

using namespace Spire;

TableSelectionModel::TableSelectionModel(
  std::shared_ptr<ItemSelectionModel> item_selection,
  std::shared_ptr<RowSelectionModel> row_selection,
  std::shared_ptr<ColumnSelectionModel> column_selection)
  : m_item_selection(std::move(item_selection)),
    m_row_selection(std::move(row_selection)),
    m_column_selection(std::move(column_selection)) {}

std::shared_ptr<const TableSelectionModel::ItemSelectionModel>
    TableSelectionModel::get_item_selection() const {
  return m_item_selection;
}

const std::shared_ptr<TableSelectionModel::ItemSelectionModel>&
    TableSelectionModel::get_item_selection() {
  return m_item_selection;
}

std::shared_ptr<const TableSelectionModel::RowSelectionModel>
    TableSelectionModel::get_row_selection() const {
  return m_row_selection;
}

const std::shared_ptr<TableSelectionModel::RowSelectionModel>&
    TableSelectionModel::get_row_selection() {
  return m_row_selection;
}

std::shared_ptr<const TableSelectionModel::ColumnSelectionModel>
    TableSelectionModel::get_column_selection() const {
  return m_column_selection;
}

const std::shared_ptr<TableSelectionModel::ColumnSelectionModel>&
    TableSelectionModel::get_column_selection() {
  return m_column_selection;
}
