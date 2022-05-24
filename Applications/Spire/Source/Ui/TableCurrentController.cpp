#include "Spire/Ui/TableCurrentController.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TableCurrentController::TableCurrentController(
  std::shared_ptr<CurrentModel> current, int row_size, int column_size)
  : m_current(std::move(current)),
    m_row_size(row_size),
    m_column_size(column_size) {}

const std::shared_ptr<TableCurrentController::CurrentModel>&
    TableCurrentController::get_current() const {
  return m_current;
}

int TableCurrentController::get_row_size() const {
  return m_row_size;
}

int TableCurrentController::get_column_size() const {
  return m_column_size;
}

void TableCurrentController::add_row(int index) {
}

void TableCurrentController::remove_row(int index) {
}

void TableCurrentController::move_row(int source, int destination) {
}

void TableCurrentController::navigate_home() {
  if(m_row_size > 0 && m_column_size > 0 && m_current->get() != Index(0, 0)) {
    m_current->set(Index(0, 0));
  }
}

void TableCurrentController::navigate_home_row() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
  } else if(current->m_row != 0) {
    m_current->set(Index(0, current->m_column));
  }
}

void TableCurrentController::navigate_home_column() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
  } else if(current->m_column != 0) {
    m_current->set(Index(current->m_row, 0));
  }
}

void TableCurrentController::navigate_end() {
  if(m_row_size > 0 && m_column_size > 0 &&
      m_current->get() != Index(m_row_size - 1, m_column_size - 1)) {
    m_current->set(Index(m_row_size - 1, m_column_size - 1));
  }
}

void TableCurrentController::navigate_end_row() {
  auto& current = m_current->get();
  if(!current) {
    navigate_end();
  } else if(current->m_row != m_row_size - 1) {
    m_current->set(Index(m_row_size - 1, current->m_column));
  }
}

void TableCurrentController::navigate_end_column() {
  auto& current = m_current->get();
  if(!current) {
    navigate_end();
  } else if(current->m_column != m_column_size - 1) {
    m_current->set(Index(current->m_row, m_column_size - 1));
  }
}

void TableCurrentController::navigate_next_row() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
  } else if(current->m_row != m_row_size - 1) {
    m_current->set(Index(current->m_row + 1, current->m_column));
  }
}

void TableCurrentController::navigate_next_column() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
  } else if(current->m_column != m_column_size - 1) {
    m_current->set(Index(current->m_row, current->m_column + 1));
  }
}

void TableCurrentController::navigate_previous_row() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
  } else if(current->m_row != 0) {
    m_current->set(Index(current->m_row - 1, current->m_column));
  }
}

void TableCurrentController::navigate_previous_column() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
  } else if(current->m_column != 0) {
    m_current->set(Index(current->m_row, current->m_column - 1));
  }
}

connection TableCurrentController::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}
