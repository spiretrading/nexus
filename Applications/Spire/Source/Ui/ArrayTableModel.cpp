#include "Spire/Ui/ArrayTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

ArrayTableModel::ArrayTableModel() {}

void ArrayTableModel::push(const std::vector<std::any>& row) {}

void ArrayTableModel::insert(const TableModel& model, int index) {}

void ArrayTableModel::move(int source, int destination) {}

void ArrayTableModel::remove(int index) {}

void ArrayTableModel::set(int row, int column, const std::any& value) {}

int ArrayTableModel::get_row_size() const {
  return -1;
}

int ArrayTableModel::get_column_size() const {
  return -1;
}

const std::any& ArrayTableModel::at(int row, int column) const {
  static const auto dummy = std::any();
  return dummy;
}

connection ArrayTableModel::connect_operation_signal(
    const typename OperationSignal::slot_type& slot) const {
  return {};
}
