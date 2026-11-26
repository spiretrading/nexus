#include "Spire/Spire/ReadOnlyTableModel.hpp"

using namespace boost::signals2;
using namespace Spire;

ReadOnlyTableModel::ReadOnlyTableModel(
  std::shared_ptr<const TableModel> source)
  : m_source(std::move(source)),
    m_source_connection(
      m_source->connect_operation_signal(m_operation_signal)) {}

int ReadOnlyTableModel::get_row_size() const {
  return m_source->get_row_size();
}

int ReadOnlyTableModel::get_column_size() const {
  return m_source->get_column_size();
}

AnyRef ReadOnlyTableModel::at(int row, int column) const {
  auto value = m_source->at(row, column);
  value.set_const();
  return value;
}

connection ReadOnlyTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_operation_signal.connect(slot);
}

void ReadOnlyTableModel::start_transaction() {}

void ReadOnlyTableModel::end_transaction() {}
