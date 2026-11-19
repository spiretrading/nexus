#include "Spire/Ui/Details/DisplayedColumnSelection.hpp"

using namespace boost::signals2;
using namespace Spire;

TableView::DisplayedColumnSelection::DisplayedColumnSelection(
  std::shared_ptr<TableSelectionModel> selection, connection tracking)
  : m_selection(std::move(selection)),
    m_inactive_depth(0),
    m_tracking_connection(std::move(tracking)) {}

int TableView::DisplayedColumnSelection::get_size() const {
  return m_selection->get_column_selection()->get_size();
}

const int& TableView::DisplayedColumnSelection::get(int index) const {
  return m_selection->get_column_selection()->get(index);
}

QValidator::State TableView::DisplayedColumnSelection::set(
    int index, const int& value) {
  if(!m_tracking_connection.connected()) {
    return QValidator::Invalid;
  }
  return m_selection->get_column_selection()->set(index, value);
}

QValidator::State TableView::DisplayedColumnSelection::push(const int& value) {
  if(!m_tracking_connection.connected()) {
    return QValidator::Invalid;
  }
  return m_selection->get_column_selection()->push(value);
}

QValidator::State TableView::DisplayedColumnSelection::insert(
    const int& value, int index) {
  if(!m_tracking_connection.connected()) {
    return QValidator::Invalid;
  }
  return m_selection->get_column_selection()->insert(value, index);
}

QValidator::State TableView::DisplayedColumnSelection::move(
    int source, int destination) {
  if(!m_tracking_connection.connected()) {
    return QValidator::Invalid;
  }
  return m_selection->get_column_selection()->move(source, destination);
}

QValidator::State TableView::DisplayedColumnSelection::remove(int index) {
  if(!m_tracking_connection.connected()) {
    return QValidator::Invalid;
  }
  return m_selection->get_column_selection()->remove(index);
}

connection TableView::DisplayedColumnSelection::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_selection->get_column_selection()->
    connect_operation_signal<>(slot);
}

void TableView::DisplayedColumnSelection::start_transaction() {
  if(m_tracking_connection.connected()) {
    m_selection->get_column_selection()->start_transaction();
  } else {
    ++m_inactive_depth;
  }
}

void TableView::DisplayedColumnSelection::end_transaction() {
  if(m_inactive_depth != 0) {
    --m_inactive_depth;
  } else {
    m_selection->get_column_selection()->end_transaction();
  }
}
