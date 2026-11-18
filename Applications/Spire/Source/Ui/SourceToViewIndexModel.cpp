#include "Spire/Ui/Details/SourceToViewIndexModel.hpp"
#include "Spire/Spire/FilteredTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TableView::SourceToViewIndexModel::SourceToViewIndexModel(
    std::shared_ptr<SortedTableModel> sorted_table,
    std::shared_ptr<FilteredTableModel> filtered_table,
    std::shared_ptr<TableView::CurrentModel> current)
    : m_sorted_table(std::move(sorted_table)),
      m_filtered_table(std::move(filtered_table)),
      m_current(std::move(current)),
      m_index(from_source(m_current->get())),
      m_source_size(m_filtered_table->get_source()->get_row_size()),
      m_is_transacting(false) {
  m_source_connection =
    m_filtered_table->get_source()->connect_operation_signal(
      std::bind_front(&SourceToViewIndexModel::on_source_operation, this));
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&SourceToViewIndexModel::on_current, this));
  m_table_connection = m_sorted_table->connect_operation_signal(
    std::bind_front(&SourceToViewIndexModel::on_operation, this));
}

const TableView::SourceToViewIndexModel::Type&
    TableView::SourceToViewIndexModel::get() const {
  if(m_is_transacting) {
    m_transaction_index = from_source(m_current->get());
    return m_transaction_index;
  }
  return m_index.get();
}

QValidator::State TableView::SourceToViewIndexModel::test(
    const Type& value) const {
  return m_current->test(to_source(value));
}

QValidator::State TableView::SourceToViewIndexModel::set(const Type& value) {
  return m_current->set(to_source(value));
}

connection TableView::SourceToViewIndexModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_index.connect_update_signal(slot);
}

optional<TableIndex> TableView::SourceToViewIndexModel::from_source(
    const Type& index) const {
  if(!index) {
    return none;
  }
  auto filtered = m_filtered_table->index_from_source(index->m_row);
  if(filtered < 0 || filtered >= m_sorted_table->get_row_size()) {
    return none;
  }
  auto row = m_sorted_table->index_from_source(filtered);
  return TableIndex(row, index->m_column);
}

optional<TableIndex> TableView::SourceToViewIndexModel::to_source(
    const Type& index) const {
  if(!index) {
    return none;
  }
  auto row = m_filtered_table->index_to_source(
    m_sorted_table->index_to_source(index->m_row));
  return TableIndex(row, index->m_column);
}

void TableView::SourceToViewIndexModel::on_current(const Type& index) {
  if(m_is_transacting ||
      m_source_size != m_filtered_table->get_source()->get_row_size()) {
    return;
  }
  auto displayed = from_source(index);
  if(displayed != m_index.get()) {
    m_index.set(displayed);
  }
}

void TableView::SourceToViewIndexModel::on_source_operation(
    const TableModel::Operation& operation) {
  if(std::holds_alternative<TableModel::StartTransaction>(operation)) {
    m_is_transacting = true;
    return;
  } else if(std::holds_alternative<TableModel::EndTransaction>(operation)) {
    m_is_transacting = false;
  } else if(m_is_transacting || !is_row_change(operation)) {
    return;
  }
  m_source_size = m_filtered_table->get_source()->get_row_size();
  on_current(m_current->get());
}

void TableView::SourceToViewIndexModel::on_operation(
    const TableModel::Operation& operation) {
  if(is_row_change(operation)) {
    on_current(m_current->get());
  }
}
