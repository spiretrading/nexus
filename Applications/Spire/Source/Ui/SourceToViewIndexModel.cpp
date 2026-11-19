#include "Spire/Ui/Details/SourceToViewIndexModel.hpp"
#include "Spire/Spire/FilteredTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  void track_rows(const TableModel::Operation& operation, bool& is_transacting,
      auto refresh) {
    visit(operation,
      [&] (const TableModel::StartTransaction&) {
        is_transacting = true;
      },
      [&] (const TableModel::EndTransaction&) {
        is_transacting = false;
        refresh();
      },
      [&] (const TableRowChange auto&) {
        if(!is_transacting) {
          refresh();
        }
      });
  }
}

TableView::SourceToViewIndexModel::SourceToViewIndexModel(
    std::shared_ptr<SortedTableModel> sorted_table,
    std::shared_ptr<FilteredTableModel> filtered_table,
    std::shared_ptr<const TranslatedListModel<int>> columns,
    std::shared_ptr<TableView::CurrentModel> current)
    : m_sorted_table(std::move(sorted_table)),
      m_filtered_table(std::move(filtered_table)),
      m_columns(std::move(columns)),
      m_current(std::move(current)),
      m_index(from_source(m_current->get())),
      m_source_size(m_filtered_table->get_source()->get_row_size()),
      m_is_source_transacting(false),
      m_is_view_transacting(false) {
  m_source_connection =
    m_filtered_table->get_source()->connect_operation_signal(
      std::bind_front(&SourceToViewIndexModel::on_source_operation, this));
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&SourceToViewIndexModel::on_current, this));
  m_table_connection = m_sorted_table->connect_operation_signal(
    std::bind_front(&SourceToViewIndexModel::on_operation, this));
  m_column_connection = m_columns->connect_operation_signal(
    [=, this] (const auto& operation) {
      visit(operation, [&] (const AnyListModel::MoveOperation&) {
        on_current(m_current->get());
      });
    });
}

const TableView::SourceToViewIndexModel::Type&
    TableView::SourceToViewIndexModel::get() const {
  if(m_is_source_transacting || m_is_view_transacting) {
    m_transaction_index = from_source(m_current->get());
    return m_transaction_index;
  }
  return m_index.get();
}

QValidator::State TableView::SourceToViewIndexModel::test(
    const Type& value) const {
  auto translated = to_source(value);
  if(value && !translated) {
    return QValidator::Invalid;
  }
  return m_current->test(translated);
}

QValidator::State TableView::SourceToViewIndexModel::set(const Type& value) {
  auto translated = to_source(value);
  if(value && !translated) {
    return QValidator::Invalid;
  }
  return m_current->set(translated);
}

connection TableView::SourceToViewIndexModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_index.connect_update_signal(slot);
}

optional<TableIndex> TableView::SourceToViewIndexModel::from_source(
    const Type& index) const {
  if(!index || index->m_column < 0 ||
      index->m_column >= m_columns->get_size()) {
    return none;
  }
  auto filtered = m_filtered_table->index_from_source(index->m_row);
  if(filtered < 0 || filtered >= m_sorted_table->get_row_size()) {
    return none;
  }
  auto row = m_sorted_table->index_from_source(filtered);
  return TableIndex(row, m_columns->index_from_source(index->m_column));
}

optional<TableIndex> TableView::SourceToViewIndexModel::to_source(
    const Type& index) const {
  if(!index || index->m_row < 0 ||
      index->m_row >= m_sorted_table->get_row_size() || index->m_column < 0 ||
      index->m_column >= m_columns->get_size()) {
    return none;
  }
  auto row = m_filtered_table->index_to_source(
    m_sorted_table->index_to_source(index->m_row));
  return TableIndex(row, m_columns->index_to_source(index->m_column));
}

void TableView::SourceToViewIndexModel::on_current(const Type& index) {
  if(m_is_source_transacting || m_is_view_transacting ||
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
  track_rows(operation, m_is_source_transacting, [&] {
    m_source_size = m_filtered_table->get_source()->get_row_size();
    on_current(m_current->get());
  });
}

void TableView::SourceToViewIndexModel::on_operation(
    const TableModel::Operation& operation) {
  track_rows(operation, m_is_view_transacting, [&] {
    on_current(m_current->get());
  });
}
