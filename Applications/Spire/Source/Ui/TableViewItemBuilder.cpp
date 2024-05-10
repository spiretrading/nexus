#include "Spire/Ui/TableViewItemBuilder.hpp"

using namespace Spire;

QWidget* TableViewItemBuilder::mount(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  return m_builder->mount(table, row, column);
}

void TableViewItemBuilder::unmount(QWidget* widget, int row, int column) {
  m_builder->unmount(widget, row, column);
}

FunctionTableViewItemBuilder::FunctionTableViewItemBuilder(
  std::function<QWidget* (const std::shared_ptr<TableModel>&, int, int)>
    builder)
  : m_builder(std::move(builder)) {}

QWidget* FunctionTableViewItemBuilder::mount(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  return m_builder(table, row, column);
}

void FunctionTableViewItemBuilder::unmount(
    QWidget* widget, int row, int column) {
  delete widget;
}
