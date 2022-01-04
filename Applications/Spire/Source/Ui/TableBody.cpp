#include "Spire/Ui/TableBody.hpp"
#include <QGridLayout>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Spire;

QWidget* TableBody::default_view_builder(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  return make_label(displayTextAny(table->at(row, column)));
}

TableBody::TableBody(std::shared_ptr<TableModel> table,
  std::shared_ptr<ListModel<int>> widths, QWidget* parent)
  : TableBody(
      std::move(table), std::make_shared<LocalValueModel<optional<Index>>>(),
      std::move(widths), &default_view_builder, parent) {}

TableBody::TableBody(std::shared_ptr<TableModel> table,
  std::shared_ptr<CurrentModel> current, std::shared_ptr<ListModel<int>> widths,
  QWidget* parent)
  : TableBody(std::move(table), std::move(current), std::move(widths),
      &default_view_builder, parent) {}

TableBody::TableBody(
  std::shared_ptr<TableModel> table, std::shared_ptr<ListModel<int>> widths,
  ViewBuilder view_builder, QWidget* parent)
  : TableBody(std::move(table),
      std::make_shared<LocalValueModel<optional<Index>>>(), std::move(widths),
      std::move(view_builder), parent) {}

TableBody::TableBody(
    std::shared_ptr<TableModel> table, std::shared_ptr<CurrentModel> current,
    std::shared_ptr<ListModel<int>> widths, ViewBuilder view_builder,
    QWidget* parent)
    : QWidget(parent),
      m_table(std::move(table)),
      m_current(std::move(current)),
      m_widths(std::move(widths)),
      m_view_builder(std::move(view_builder)) {
  auto layout = new QGridLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  for(auto row = 0; row != m_table->get_row_size(); ++row) {
    for(auto column = 0; column != m_table->get_column_size(); ++column) {
      auto item = m_view_builder(m_table, row, column);
      if(column != m_table->get_column_size() - 1) {
        item->setSizePolicy(
          QSizePolicy::Fixed, item->sizePolicy().verticalPolicy());
        item->setFixedWidth(m_widths->get(column));
      } else {
        item->setSizePolicy(
          QSizePolicy::Expanding, item->sizePolicy().verticalPolicy());
      }
      layout->addWidget(item, row, column);
    }
  }
  m_table_connection = m_table->connect_operation_signal(
    std::bind_front(&TableBody::on_table_operation, this));
  m_widths_connection = m_widths->connect_operation_signal(
    std::bind_front(&TableBody::on_widths_update, this));
}

const std::shared_ptr<TableModel>& TableBody::get_table() const {
  return m_table;
}

const std::shared_ptr<TableBody::CurrentModel>& TableBody::get_current() const {
  return m_current;
}

void TableBody::on_table_operation(const TableModel::Operation& operation) {
  auto& layout = static_cast<QGridLayout&>(*this->layout());
  visit(operation,
    [&] (const TableModel::MoveOperation& operation) {
      for(auto column = 0; column != m_table->get_column_size(); ++column) {
        auto source = layout.itemAtPosition(operation.m_source, column);
        layout.removeItem(source);
        if(operation.m_source < operation.m_destination) {
          layout
        }
      }
    });
}

void TableBody::on_widths_update(const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::UpdateOperation& operation) {
      auto layout = static_cast<QGridLayout*>(this->layout());
      for(auto row = 0; row != m_table->get_row_size(); ++row) {
        auto item = layout->itemAtPosition(row, operation.m_index)->widget();
        item->setFixedWidth(m_widths->get(operation.m_index));
      }
    });
}
