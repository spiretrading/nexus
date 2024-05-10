#include "Spire/Ui/TableViewItemBuilder.hpp"

using namespace Spire;

TableViewItemBuilder::TableViewItemBuilder(std::function<
  QWidget* (const std::shared_ptr<TableModel>&, int, int)> builder)
  : TableViewItemBuilder(FunctionTableViewItemBuilder(std::move(builder))) {}

QWidget* TableViewItemBuilder::mount(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  return m_builder->mount(table, row, column);
}

void TableViewItemBuilder::unmount(QWidget* widget, int row, int column) {
  m_builder->unmount(widget, row, column);
}

template<typename B>
template<typename... Args>
TableViewItemBuilder::
  WrappedTableViewItemBuilder<B>::WrappedTableViewItemBuilder(Args&&... args)
  : m_builder(std::forward<Args>(args)...) {}

template<typename B>
QWidget* TableViewItemBuilder::WrappedTableViewItemBuilder<B>::mount(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  return m_builder->mount(table, row, column);
}

template<typename B>
void TableViewItemBuilder::WrappedTableViewItemBuilder<B>::unmount(
    QWidget* widget, int row, int column) {
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
