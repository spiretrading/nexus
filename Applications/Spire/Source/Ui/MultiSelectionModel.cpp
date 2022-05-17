#include "Spire/Ui/MultiSelectionModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

int MultiSelectionModel::get_size() const {
  throw std::runtime_error("Not implemented.");
}

const int& MultiSelectionModel::get(int index) const {
  throw std::runtime_error("Not implemented.");
}

QValidator::State MultiSelectionModel::set(int index, const int& value) {
  throw std::runtime_error("Not implemented.");
}

QValidator::State MultiSelectionModel::insert(const int& value, int index) {
  throw std::runtime_error("Not implemented.");
}

QValidator::State MultiSelectionModel::move(int source, int destination) {
  throw std::runtime_error("Not implemented.");
}

QValidator::State MultiSelectionModel::remove(int index) {
  throw std::runtime_error("Not implemented.");
}

connection MultiSelectionModel::connect_operation_signal(
    const typename OperationSignal::slot_type& slot) const {
  throw std::runtime_error("Not implemented.");
}
