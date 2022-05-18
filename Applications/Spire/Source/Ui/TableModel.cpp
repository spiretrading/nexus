#include "Spire/Ui/TableModel.hpp"

using namespace Spire;

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
