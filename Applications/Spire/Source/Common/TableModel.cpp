#include "Spire/Spire/TableModel.hpp"

using namespace Spire;

TableModel::AddOperation::AddOperation(
  int index, std::shared_ptr<const AnyListModel> row)
  : m_index(index),
    m_row(std::move(row)) {}

TableModel::RemoveOperation::RemoveOperation(
  int index, std::shared_ptr<const AnyListModel> row)
  : m_index(index),
    m_row(std::move(row)) {}

TableModel::MoveOperation::MoveOperation(int source, int destination)
  : m_source(source),
    m_destination(destination) {}

TableModel::UpdateOperation::UpdateOperation(
  int row, int column, std::any previous, std::any value)
  : m_row(row),
    m_column(column),
    m_previous(std::move(previous)),
    m_value(std::move(value)) {}

TableModel::Operation::Operation(AddOperation operation)
  : m_operation(std::move(operation)) {}

TableModel::Operation::Operation(RemoveOperation operation)
  : m_operation(std::move(operation)) {}

TableModel::Operation::Operation(MoveOperation operation)
  : m_operation(std::move(operation)) {}

TableModel::Operation::Operation(UpdateOperation operation)
  : m_operation(std::move(operation)) {}

TableModel::Operation::Operation(std::vector<Operation> operation)
  : m_operation(std::move(operation)) {}

QValidator::State TableModel::set(int row, int column, const std::any& value) {
  return QValidator::State::Invalid;
}
