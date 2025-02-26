#include "Spire/Spire/TableCurrentIndexModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TableCurrentIndexModel::TableCurrentIndexModel(
  std::shared_ptr<TableModel> table)
  : TableCurrentIndexModel(std::move(table), none) {}

TableCurrentIndexModel::TableCurrentIndexModel(
    std::shared_ptr<TableModel> table, optional<TableIndex> index)
    : m_table(std::move(table)),
      m_index(index),
      m_tracker(index.value_or(TableIndex(-1, -1)).m_row) {
  m_connection = m_table->connect_operation_signal(
    std::bind_front(&TableCurrentIndexModel::on_operation, this));
}

const TableCurrentIndexModel::Type& TableCurrentIndexModel::get() const {
  return m_index.get();
}

QValidator::State TableCurrentIndexModel::test(const Type& value) const {
  if(value == none || value->m_row >= 0 &&
      value->m_row < m_table->get_row_size() &&
      value->m_column >= 0 && value->m_column < m_table->get_column_size()) {
    return QValidator::Acceptable;
  }
  return QValidator::Invalid;
}

QValidator::State TableCurrentIndexModel::set(const Type& value) {
  if(test(value) == QValidator::Invalid) {
    return QValidator::Invalid;
  }
  m_tracker.set(value.value_or(TableIndex(-1,-1)).m_row);
  return m_index.set(value);
}

connection TableCurrentIndexModel::connect_update_signal(
    const typename UpdateSignal::slot_type& slot) const {
  return m_index.connect_update_signal(slot);
}

void TableCurrentIndexModel::on_operation(
    const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::PreRemoveOperation& operation) {
      if(auto index = get()) {
        m_tracker.update(operation);
        if(m_table->get_row_size() == 0) {
          m_tracker.set(-1);
          m_index.set(none);
        } else if(index->m_row >= m_table->get_row_size()) {
          m_tracker.set(m_table->get_row_size() - 1);
          m_index.set(TableIndex(m_table->get_row_size() - 1, index->m_column));
        } else if(operation.m_index == index->m_row) {
          m_tracker.set(index->m_row);
          m_index.set(*index);
        } else if(m_tracker.get_index() == -1) {
          m_index.set(none);
        } else {
          m_index.set(TableIndex(m_tracker.get_index(), index->m_column));
        }
      }
    },
    [&] (const auto& operation) {
      auto index = get();
      m_tracker.update(operation);
      if(m_tracker.get_index() != index.value_or(TableIndex(-1, -1)).m_row) {
        if(m_tracker.get_index() == -1) {
          m_index.set(none);
        } else {
          m_index.set(TableIndex(m_tracker.get_index(), index->m_column));
        }
      }
    });
}
