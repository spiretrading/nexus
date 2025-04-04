#include "Spire/Ui/TableCurrentController.hpp"
#include <boost/signals2/shared_connection_block.hpp>

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

const optional<TableCurrentController::Index>&
    TableCurrentController::get() const {
  return m_current->get();
}

optional<int> TableCurrentController::get_row() const {
  if(auto& index = get()) {
    return index->m_row;
  }
  return none;
}

optional<int> TableCurrentController::get_column() const {
  if(auto& index = get()) {
    return index->m_column;
  }
  return none;
}

int TableCurrentController::get_row_size() const {
  return m_row_size;
}

int TableCurrentController::get_column_size() const {
  return m_column_size;
}

void TableCurrentController::add_row(int index) {
  ++m_row_size;
}

void TableCurrentController::remove_row(int index) {
  --m_row_size;
  if(m_current->get() && m_current->get()->m_row == index) {
    m_current->set(none);
  }
}

void TableCurrentController::navigate_home() {
  if(m_row_size == 0 || m_column_size == 0) {
    return;
  }
  auto& current = m_current->get();
  for(auto row = 0; row < m_row_size; ++row) {
    for(auto column = 0; column < m_column_size; ++column) {
      auto index = Index(row, column);
      if(current == index ||
          m_current->set(index) != QValidator::State::Invalid) {
        break;
      }
    }
  }
}

void TableCurrentController::navigate_home_row() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
    return;
  }
  for(auto row = 0; row < m_row_size; ++row) {
    if(current->m_row == row || m_current->set(Index(row, current->m_column)) !=
        QValidator::State::Invalid) {
      break;
    }
  }
}

void TableCurrentController::navigate_home_column() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
    return;
  }
  for(auto column = 0; column < m_column_size; ++column) {
    if(current->m_column == column ||
        m_current->set(Index(current->m_row, column)) !=
          QValidator::State::Invalid) {
      break;
    }
  }
}

void TableCurrentController::navigate_end() {
  if(m_row_size == 0 || m_column_size == 0) {
    return;
  }
  auto& current = m_current->get();
  for(auto row = m_row_size - 1; row >= 0; --row) {
    for(auto column = m_column_size - 1; column >= 0; --column) {
      auto index = Index(row, column);
      if(current == index ||
          m_current->set(index) != QValidator::State::Invalid) {
        break;
      }
    }
  }
}

void TableCurrentController::navigate_end_row() {
  auto& current = m_current->get();
  if(!current) {
    navigate_end();
    return;
  }
  for(auto row = m_row_size - 1; row >= 0; --row) {
    if(current->m_row == row || m_current->set(Index(row, current->m_column)) !=
        QValidator::State::Invalid) {
      break;
    }
  }
}

void TableCurrentController::navigate_end_column() {
  auto& current = m_current->get();
  if(!current) {
    navigate_end();
    return;
  }
  for(auto column = m_column_size - 1; column >= 0; --column) {
    if(current->m_column == column ||
        m_current->set(Index(current->m_row, column)) !=
          QValidator::State::Invalid) {
      break;
    }
  }
}

void TableCurrentController::navigate_next_row() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
    return;
  }
  for(auto row = current->m_row + 1; row < m_row_size; ++row) {
    if(m_current->set(Index(row, current->m_column)) !=
        QValidator::State::Invalid) {
      break;
    }
  }
}

void TableCurrentController::navigate_next_column() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
    return;
  }
  for(auto column = current->m_column + 1; column < m_column_size; ++column) {
    if(m_current->set(Index(current->m_row, column)) !=
        QValidator::State::Invalid) {
      break;
    }
  }
}

void TableCurrentController::navigate_previous_row() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
    return;
  }
  for(auto row = current->m_row - 1; row >= 0; --row) {
    if(m_current->set(Index(row, current->m_column)) !=
        QValidator::State::Invalid) {
      break;
    }
  }
}

void TableCurrentController::navigate_previous_column() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
    return;
  }
  for(auto column = current->m_column - 1; column >= 0; --column) {
    if(m_current->set(Index(current->m_row, column)) !=
        QValidator::State::Invalid) {
      break;
    }
  }
}
