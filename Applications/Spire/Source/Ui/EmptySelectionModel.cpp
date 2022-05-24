#include "Spire/Ui/EmptySelectionModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

int EmptySelectionModel::get_size() const {
  return 0;
}

const int& EmptySelectionModel::get(int index) const {
  throw std::out_of_range("The index is out of range.");
}

QValidator::State EmptySelectionModel::set(int index, const int& value) {
  throw std::out_of_range("The index is out of range.");
}

QValidator::State EmptySelectionModel::insert(const int& value, int index) {
  if(index == 0) {
    return QValidator::State::Invalid;
  }
  throw std::out_of_range("The index is out of range.");
}

QValidator::State EmptySelectionModel::move(int source, int destination) {
  throw std::out_of_range("The source or destination is out of range.");
}

QValidator::State EmptySelectionModel::remove(int index) {
  throw std::out_of_range("The index is out of range.");
}

connection EmptySelectionModel::connect_operation_signal(
    const typename OperationSignal::slot_type& slot) const {
  return {};
}

void EmptySelectionModel::transact(const std::function<void ()>& transaction) {
  transaction();
}
