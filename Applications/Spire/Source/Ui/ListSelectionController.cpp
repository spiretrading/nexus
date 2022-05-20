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
}

ListSelectionController::ListSelectionController(
  std::shared_ptr<SelectionModel> selection)
  : m_mode(Mode::SINGLE),
    m_selection(std::move(selection)),
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
  auto operation = optional<SelectionModel::Operation>();
  {
    auto blocker = shared_connection_block(m_connection);
    m_selection->transact([&] {
      for(auto i = 0; i != m_selection->get_size(); ++i) {
        auto selection = m_selection->get(i);
        if(selection == index) {
          m_selection->remove(i);
          operation = SelectionModel::RemoveOperation(i, index);
        } else if(selection > index) {
          m_selection->set(i, selection - 1);
        }
      }
    });
  }
  if(operation) {
    m_operation_signal(*operation);
  }
}

void ListSelectionController::move(int source, int destination) {
  auto direction = [&] {
    if(source < destination) {
      return -1;
    }
    return 1;
  }();
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

void ListSelectionController::click(int index) {
  if(m_mode == Mode::SINGLE) {
    if(find_index(index, *m_selection) != -1) {
      return;
    }
    m_selection->transact([&] {
      clear(*m_selection);
      m_selection->push(index);
    });
  } else if(m_mode == Mode::INCREMENTAL) {
    auto selection_index = find_index(index, *m_selection);
    if(selection_index == -1) {
      m_selection->push(index);
    } else {
      m_selection->remove(selection_index);
    }
  }
}

void ListSelectionController::navigate(int index) {
  if(m_mode == Mode::SINGLE) {
    click(index);
  }
}

connection ListSelectionController::connect_operation_signal(
    const SelectionModel::OperationSignal::slot_type& slot) const {
  return m_operation_signal.connect(slot);
}

void ListSelectionController::on_operation(
    const SelectionModel::Operation& operation) {
  m_operation_signal(operation);
}
