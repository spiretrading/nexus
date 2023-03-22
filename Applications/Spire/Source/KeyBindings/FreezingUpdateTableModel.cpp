#include "Spire/KeyBindings/FreezingUpdateTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

FreezingUpdateTableModel::FreezingUpdateTableModel(
  std::shared_ptr<TableModel> source)
  : m_source(std::move(source)),
    m_is_frozen(false),
    m_source_connection(m_source->connect_operation_signal(
      std::bind_front(&FreezingUpdateTableModel::on_operation, this))) {}

void FreezingUpdateTableModel::freeze() {
  m_is_frozen = true;
}

void FreezingUpdateTableModel::unfreeze() {
  if(m_is_frozen && m_frozen_update) {
    m_transaction.push(*m_frozen_update);
  }
  clear();
}

int FreezingUpdateTableModel::get_row_size() const {
  return m_source->get_row_size();
}

int FreezingUpdateTableModel::get_column_size() const {
  return m_source->get_column_size();
}

AnyRef FreezingUpdateTableModel::at(int row, int column) const {
  return m_source->at(row, column);
}

QValidator::State FreezingUpdateTableModel::set(
    int row, int column, const std::any& value) {
  return m_source->set(row, column, value);
}

connection FreezingUpdateTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void FreezingUpdateTableModel::clear() {
  m_is_frozen = false;
  m_frozen_update.reset();
}

void FreezingUpdateTableModel::on_operation(const Operation& operation) {
  visit(operation,
    [&] (const StartTransaction&) {
      m_transaction.start();
    },
    [&] (const EndTransaction&) {
      m_transaction.end();
    },
    [&] (const UpdateOperation& operation) {
      if(m_is_frozen) {
        m_frozen_update = operation;
        return;
      }
      m_transaction.push(operation);
    },
    [&] (const MoveOperation& operation) {
      if(m_frozen_update) {
        if(m_frozen_update->m_row == operation.m_source) {
          m_frozen_update->m_row = operation.m_destination;
        } else if(operation.m_source < operation.m_destination) {
          if(m_frozen_update->m_row > operation.m_source &&
              m_frozen_update->m_row <= operation.m_destination) {
            --m_frozen_update->m_row;
          }
        } else if(m_frozen_update->m_row >= operation.m_destination &&
            m_frozen_update->m_row < operation.m_source) {
          ++m_frozen_update->m_row;
        }
      }
      m_transaction.push(operation);
    },
    [&] (const auto& operation) {
      clear();
      m_transaction.push(operation);
    });
}
