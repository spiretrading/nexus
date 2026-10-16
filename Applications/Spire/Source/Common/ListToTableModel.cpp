#include "Spire/Spire/ListToTableModel.hpp"

using namespace Spire;

TableModel::StartTransaction Spire::to_table_operation(
    const AnyListModel::StartTransaction& operation) {
  return TableModel::StartTransaction();
}

TableModel::EndTransaction Spire::to_table_operation(
    const AnyListModel::EndTransaction& operation) {
  return TableModel::EndTransaction();
}

TableModel::AddOperation Spire::to_table_operation(
    const AnyListModel::AddOperation& operation) {
  return TableModel::AddOperation(operation.m_index);
}

TableModel::MoveOperation Spire::to_table_operation(
    const AnyListModel::MoveOperation& operation) {
  return TableModel::MoveOperation(operation.m_source, operation.m_destination);
}

TableModel::PreRemoveOperation Spire::to_table_operation(
    const AnyListModel::PreRemoveOperation& operation) {
  return TableModel::PreRemoveOperation(operation.m_index);
}

TableModel::RemoveOperation Spire::to_table_operation(
    const AnyListModel::RemoveOperation& operation) {
  return TableModel::RemoveOperation(operation.m_index);
}

TableModel::Operation Spire::to_table_operation(
    const AnyListModel::Operation& operation) {
  if(auto start = std::get_if<AnyListModel::StartTransaction>(&operation)) {
    return to_table_operation(*start);
  } else if(auto end = std::get_if<AnyListModel::EndTransaction>(&operation)) {
    return to_table_operation(*end);
  } else if(auto add = std::get_if<AnyListModel::AddOperation>(&operation)) {
    return to_table_operation(*add);
  } else if(auto move = std::get_if<AnyListModel::MoveOperation>(&operation)) {
    return to_table_operation(*move);
  } else if(auto pre_remove =
      std::get_if<AnyListModel::PreRemoveOperation>(&operation)) {
    return to_table_operation(*pre_remove);
  } else if(auto remove =
      std::get_if<AnyListModel::RemoveOperation>(&operation)) {
    return to_table_operation(*remove);
  }
  return TableModel::Operation();
}
