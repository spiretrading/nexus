#include "Spire/Ui/ListSelectionController.hpp"
#include <boost/signals2/shared_connection_block.hpp>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

ListSelectionController::ListSelectionController(
  std::shared_ptr<ListModel<int>> selection)
  : m_mode(Mode::SINGLE),
    m_selection(std::move(selection)),
    m_connection(m_selection->connect_operation_signal(
      std::bind_front(&ListSelectionController::on_operation, this))) {}

const std::shared_ptr<ListModel<int>>&
    ListSelectionController::get_selection() const {
  return m_selection;
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
  auto blocker = shared_connection_block(m_connection);
  m_selection->transact([&] {
    for(auto i = 0; i != m_selection->get_size(); ++i) {
      auto selection = m_selection->get(i);
      if(selection == index) {
        m_selection->remove(i);
      } else if(selection > index) {
        m_selection->set(i, selection - 1);
      }
    }
  });
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
  m_selection->push(index);
}

void ListSelectionController::navigate(int index) {
}

connection ListSelectionController::connect_operation_signal(
    const ListModel<int>::OperationSignal::slot_type& slot) const {
  return m_operation_signal.connect(slot);
}

void ListSelectionController::on_operation(
    const ListModel<int>::Operation& operation) {
  m_operation_signal(operation);
}
