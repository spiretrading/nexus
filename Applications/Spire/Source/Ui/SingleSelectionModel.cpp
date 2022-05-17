#include "Spire/Ui/SingleSelectionModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

int SingleSelectionModel::get_size() const {
  if(m_value) {
    return 1;
  }
  return 0;
}

const int& SingleSelectionModel::get(int index) const {
  if(index < 0 || index >= get_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  return *m_value;
}

QValidator::State SingleSelectionModel::set(int index, const int& value) {
  if(index < 0 || index >= get_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  *m_value = value;
  m_transaction.push(UpdateOperation(index));
  return QValidator::State::Acceptable;
}

QValidator::State SingleSelectionModel::insert(const int& value, int index) {
  if(!m_value && index == 0) {
    m_value = value;
    m_transaction.push(AddOperation(index));
    return QValidator::State::Acceptable;
  } else if(m_value && (index == 0 || index == 1)) {
    m_value = value;
    m_transaction.push(UpdateOperation(0));
    return QValidator::State::Acceptable;
  }
  throw std::out_of_range("The index is out of range.");
}

QValidator::State SingleSelectionModel::move(int source, int destination) {
  if(source < 0 || source >= get_size() || destination < 0 ||
      destination >= get_size()) {
    throw std::out_of_range("The source or destination is out of range.");
  }
  return QValidator::State::Acceptable;
}

QValidator::State SingleSelectionModel::remove(int index) {
  if(index < 0 || index >= get_size()) {
    throw std::out_of_range("The index is out of range.");
  }
  m_value = none;
  m_transaction.push(RemoveOperation(index));
  return QValidator::State::Acceptable;
}

connection SingleSelectionModel::connect_operation_signal(
    const typename OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void SingleSelectionModel::transact(const std::function<void ()>& transaction) {
  m_transaction.transact(transaction);
}
