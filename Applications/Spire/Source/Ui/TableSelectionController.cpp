#include "Spire/Ui/TableSelectionController.hpp"

using namespace Spire;

TableSelectionController::TableSelectionController(
    std::shared_ptr<SelectionModel> selection, int row_size,
    int column_size) {}

const std::shared_ptr<TableSelectionController::SelectionModel>&
    TableSelectionController::get_selection() const {
  return m_selection;
}

TableSelectionController::Mode TableSelectionController::get_mode() const {
  return m_mode;
}

void TableSelectionController::set_mode(Mode mode) {}

int TableSelectionController::get_row_size() const {
  return m_row_size;
}

int TableSelectionController::get_column_size() const {
  return m_column_size;
}

void TableSelectionController::add_row(int index) {}

void TableSelectionController::remove_row(int index) {}

void TableSelectionController::move_row(int source, int destination) {}

void TableSelectionController::click(Index index) {}

void TableSelectionController::navigate(Index index) {}
