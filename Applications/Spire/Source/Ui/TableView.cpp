#include "Spire/Ui/TableView.hpp"
#include <QVBoxLayout>
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TableHeader.hpp"
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

QWidget* TableView::default_view_builder(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  return make_label(displayTextAny(table->at(row, column)));
}

TableView::TableView(std::shared_ptr<TableModel> table,
  std::shared_ptr<HeaderModel> header, QWidget* parent)
  : TableView(
      std::move(table), std::move(header), &default_view_builder, parent) {}

TableView::TableView(std::shared_ptr<TableModel> table,
    std::shared_ptr<HeaderModel> header, ViewBuilder view_builder,
    QWidget* parent)
    : QWidget(parent),
      m_table(std::move(table)),
      m_view_builder(std::move(view_builder)) {
  auto body = new QWidget();
  auto body_layout = new QVBoxLayout(body);
  body_layout->setContentsMargins({});
  body_layout->addWidget(new TableHeader(header));
  auto box = new Box(body);
  update_style(*box, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  auto layout = new QVBoxLayout(this);
  layout->addWidget(box);
}

const std::shared_ptr<TableModel>& TableView::get_table() const {
  return m_table;
}
