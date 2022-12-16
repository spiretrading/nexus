#include "Spire/Ui/TableCurrentController.hpp"
#include <boost/signals2/shared_connection_block.hpp>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TableCurrentController::TableCurrentController(
  std::shared_ptr<CurrentModel> current, int row_size, int column_size)
  : m_current(std::move(current)),
    m_row_size(row_size),
    m_column_size(column_size),
    m_last_current(m_current->get()),
    m_connection(m_current->connect_update_signal(
      std::bind_front(&TableCurrentController::on_current, this))) {}

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
  ++m_row_size;
  if(m_current->get() && m_current->get()->m_row >= index &&
      m_current->get()->m_row < m_row_size - 1) {
    auto blocker = shared_connection_block(m_connection);
    m_last_current =
      Index(m_current->get()->m_row + 1, m_current->get()->m_column);
    m_current->set(m_last_current);
  }
}

void TableCurrentController::remove_row(int index) {
  --m_row_size;
  if(m_current->get()) {
    if(m_current->get()->m_row == index) {
      m_current->set(Index(index, m_current->get()->m_column));
    } else if(m_current->get()->m_row > index) {
      auto blocker = shared_connection_block(m_connection);
      m_last_current =
        Index(m_current->get()->m_row - 1, m_current->get()->m_column);
      m_current->set(m_last_current);
    }
  }
}

void TableCurrentController::move_row(int source, int destination) {
  if(source == destination) {
    return;
  }
  auto direction = [&] {
    if(source < destination) {
      return -1;
    }
    return 1;
  }();
  auto adjust = [&] (auto& value) {
    if(value) {
      if(value->m_row == source) {
        value->m_row = destination;
        return true;
      } else if(direction == 1 && value->m_row >= destination && value->m_row < source ||
        direction == -1 && value->m_row > source && value->m_row <= destination) {
        value->m_row += direction;
        return true;
      }
    }
    return false;
  };
  adjust(m_last_current);
  auto current = m_current->get();
  if(adjust(current)) {
    auto blocker = shared_connection_block(m_connection);
    m_current->set(current);
  }
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

void TableCurrentController::on_current(const optional<Index>& current) {
  auto previous = m_last_current;
  m_last_current = current;
  m_update_signal(previous, current);
}
