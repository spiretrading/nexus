#include "Spire/Spire/DeduplicatedTableModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost::signals2;
using namespace Spire;

DeduplicatedTableModel::DeduplicatedTableModel(
  std::shared_ptr<TableModel> source)
  : DeduplicatedTableModel(std::move(source),
      [] (const auto& previous, const auto& value, auto column) {
        return is_equal(previous, value);
      }) {}

DeduplicatedTableModel::DeduplicatedTableModel(
    std::shared_ptr<TableModel> source, IsEqual is_equal)
    : m_source(std::move(source)),
      m_is_equal(std::move(is_equal)) {
  m_connection = m_source->connect_operation_signal(
    std::bind_front(&DeduplicatedTableModel::on_operation, this));
}

int DeduplicatedTableModel::get_row_size() const {
  return m_source->get_row_size();
}

int DeduplicatedTableModel::get_column_size() const {
  return m_source->get_column_size();
}

AnyRef DeduplicatedTableModel::at(int row, int column) const {
  return m_source->at(row, column);
}

QValidator::State DeduplicatedTableModel::set(
    int row, int column, const std::any& value) {
  return m_source->set(row, column, value);
}

QValidator::State DeduplicatedTableModel::remove(int row) {
  return m_source->remove(row);
}

connection DeduplicatedTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void DeduplicatedTableModel::on_operation(const Operation& operation) {
  visit(operation,
    [&] (const UpdateOperation& operation) {
      if(m_is_equal(
          operation.m_previous, operation.m_value, operation.m_column)) {
        return;
      }
      m_transaction.push(operation);
    },
    [&] (const auto& operation) {
      m_transaction.push(operation);
    });
}
