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
  if(auto start = boost::get<AnyListModel::StartTransaction>(&operation)) {
    return to_table_operation(*start);
  } else if(auto end = boost::get<AnyListModel::EndTransaction>(&operation)) {
    return to_table_operation(*end);
  } else if(auto add = boost::get<AnyListModel::AddOperation>(&operation)) {
    return to_table_operation(*add);
  } else if(auto move = boost::get<AnyListModel::MoveOperation>(&operation)) {
    return to_table_operation(*move);
  } else if(auto pre_remove =
      boost::get<AnyListModel::PreRemoveOperation>(&operation)) {
    return to_table_operation(*pre_remove);
  } else if(auto remove =
      boost::get<AnyListModel::RemoveOperation>(&operation)) {
    return to_table_operation(*remove);
  }
  return TableModel::Operation();
}
