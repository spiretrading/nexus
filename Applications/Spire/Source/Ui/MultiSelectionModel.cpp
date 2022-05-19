#include "Spire/Ui/MultiSelectionModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

int MultiSelectionModel::get_size() const {
  return m_list.get_size();
}

const int& MultiSelectionModel::get(int index) const {
  return m_list.get(index);
}

QValidator::State MultiSelectionModel::set(int index, const int& value) {
  auto previous = m_list.get(index);
  if(previous == value) {
    return QValidator::State::Acceptable;
  }
  if(m_selection.insert(value).second) {
    m_selection.erase(previous);
    return m_list.set(index, value);
  }
  return m_list.remove(index);
}

QValidator::State MultiSelectionModel::insert(const int& value, int index) {
  if(m_selection.insert(value).second) {
    return m_list.insert(value, index);
  }
  return QValidator::State::Acceptable;
}

QValidator::State MultiSelectionModel::move(int source, int destination) {
  return m_list.move(source, destination);
}

QValidator::State MultiSelectionModel::remove(int index) {
  m_selection.erase(m_list.get(index));
  return m_list.remove(index);
}

connection MultiSelectionModel::connect_operation_signal(
    const typename OperationSignal::slot_type& slot) const {
  return m_list.connect_operation_signal(slot);
}

void MultiSelectionModel::transact(const std::function<void ()>& transaction) {
  m_list.transact([&] { transaction(); });
}
