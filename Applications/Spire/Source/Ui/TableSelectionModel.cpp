#include "Spire/Ui/TableSelectionModel.hpp"

using namespace Spire;

TableSelectionModel::TableSelectionModel(
  std::shared_ptr<ListModel<Index>> item_selection,
  std::shared_ptr<ListModel<int>> row_selection,
  std::shared_ptr<ListModel<int>> column_selection)
  : m_item_selection(std::move(item_selection)),
    m_row_selection(std::move(row_selection)),
    m_column_selection(std::move(column_selection)) {}

std::shared_ptr<const ListModel<TableSelectionModel::Index>>
    TableSelectionModel::get_item_selection() const {
  return m_item_selection;
}

const std::shared_ptr<ListModel<TableSelectionModel::Index>>&
    TableSelectionModel::get_item_selection() {
  return m_item_selection;
}

std::shared_ptr<const ListModel<int>>
    TableSelectionModel::get_row_selection() const {
  return m_row_selection;
}

const std::shared_ptr<ListModel<int>>& TableSelectionModel::get_row_selection() {
  return m_row_selection;
}

std::shared_ptr<const ListModel<int>>
    TableSelectionModel::get_column_selection() const {
  return m_column_selection;
}

const std::shared_ptr<ListModel<int>>&
    TableSelectionModel::get_column_selection() {
  return m_column_selection;
}
