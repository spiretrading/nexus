#include "Spire/Ui/ListSelectionController.hpp"
#include <boost/signals2/shared_connection_block.hpp>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  int find_index(int value, ListSelectionController::SelectionModel& list) {
    for(auto i = 0; i != list.get_size(); ++i) {
      if(list.get(i) == value) {
        return i;
      }
    }
    return -1;
  }

  int get_direction(int start, int end) {
    if(start <= end) {
      return 1;
    }
    return -1;
  }
}

ListSelectionController::ListSelectionController(
  std::shared_ptr<SelectionModel> selection)
  : m_mode(Mode::SINGLE),
    m_selection(std::move(selection)),
    m_size(0),
    m_connection(m_selection->connect_operation_signal(
      std::bind_front(&ListSelectionController::on_operation, this))) {}

const std::shared_ptr<ListSelectionController::SelectionModel>&
    ListSelectionController::get_selection() const {
  return m_selection;
}

ListSelectionController::Mode ListSelectionController::get_mode() const {
  return m_mode;
}

void ListSelectionController::set_mode(Mode mode) {
  m_mode = mode;
}

void ListSelectionController::add(int index) {
  ++m_size;
  auto blocker = shared_connection_block(m_connection);
  m_selection->transact([&] {
    for(auto i = 0; i != m_selection->get_size(); ++i) {
      auto selection = m_selection->get(i);
      if(selection >= index) {
        m_selection->set(i, selection + 1);
      }
    }
  });
}

void ListSelectionController::remove(int index) {
  --m_size;
  auto operation = optional<SelectionModel::Operation>();
  {
    auto blocker = shared_connection_block(m_connection);
    m_selection->transact([&] {
      auto i = 0;
      while(i != m_selection->get_size()) {
        auto selection = m_selection->get(i);
        if(selection == index) {
          m_selection->remove(i);
          operation = SelectionModel::RemoveOperation(i, index);
        } else if(selection > index) {
          m_selection->set(i, selection - 1);
          ++i;
        }
      }
    });
  }
  if(operation) {
    m_operation_signal(*operation);
  }
}

void ListSelectionController::move(int source, int destination) {
  auto direction = get_direction(destination, source);
  auto blocker = shared_connection_block(m_connection);
  m_selection->transact([&] {
    for(auto i = 0; i != m_selection->get_size(); ++i) {
      auto selection = m_selection->get(i);
      if(selection >= source || selection <= destination) {
        m_selection->set(i, selection + direction);
      }
    }
  });
}

void ListSelectionController::select_all() {
  m_selection->transact([&] {
    for(auto i = 0; i != m_size; ++i) {
      if(i != m_current && find_index(i, *m_selection) == -1) {
        m_selection->push(i);
      }
    }
    if(m_current) {
      if(find_index(*m_current, *m_selection) == -1) {
        m_selection->push(*m_current);
      }
    }
  });
}

void ListSelectionController::click(int index) {
  if(m_mode == Mode::SINGLE) {
    if(m_selection->get_size() != 1 || m_selection->get(0) != index) {
      m_selection->transact([&] {
        clear(*m_selection);
        m_selection->push(index);
      });
    }
    m_range_anchor = index;
  } else if(m_mode == Mode::INCREMENTAL) {
    auto selection_index = find_index(index, *m_selection);
    if(selection_index == -1) {
      m_selection->push(index);
    } else {
      m_selection->remove(selection_index);
    }
    m_range_anchor = index;
  } else if(m_mode == Mode::RANGE) {
    if(!m_range_anchor) {
      m_mode = Mode::SINGLE;
      click(index);
      m_mode = Mode::RANGE;
      m_range_anchor = index;
    } else if(!m_current) {
      auto direction = get_direction(*m_range_anchor, index);
      m_selection->transact([&] {
        for(auto i = *m_range_anchor; direction * i <= direction * index;
            i += direction) {
          m_selection->push(i);
        }
      });
    } else {
      auto direction = get_direction(*m_current, index);
      m_selection->transact([&] {
        for(auto i = *m_current; i != index + direction; i += direction) {
          if(direction * i >= direction * *m_range_anchor) {
            if(find_index(i, *m_selection) == -1) {
              m_selection->push(i);
            }
          } else if(i != index) {
            auto j = find_index(i, *m_selection);
            if(j != -1) {
              m_selection->remove(j);
            }
          }
        }
      });
    }
  }
  m_current = index;
}

void ListSelectionController::navigate(int index) {
  if(m_mode == Mode::SINGLE || m_mode == Mode::RANGE) {
    click(index);
  }
}

connection ListSelectionController::connect_operation_signal(
    const SelectionModel::OperationSignal::slot_type& slot) const {
  return m_operation_signal.connect(slot);
}

void ListSelectionController::on_operation(
    const SelectionModel::Operation& operation) {
  visit(operation,
    [&] (const SelectionModel::RemoveOperation& operation) {
      if(operation.get_value() == m_range_anchor) {
        m_range_anchor = none;
      }
    },
    [&] (const SelectionModel::UpdateOperation& operation) {
      if(operation.get_previous() == m_range_anchor) {
        m_range_anchor = none;
      }
    });
  m_operation_signal(operation);
}
