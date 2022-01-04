#include "Spire/Ui/TableView.hpp"
#include <QVBoxLayout>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableHeader.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

QWidget* TableView::default_view_builder(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  return TableBody::default_view_builder(table, row, column);
}

TableView::TableView(std::shared_ptr<TableModel> table,
  std::shared_ptr<HeaderModel> header, QWidget* parent)
  : TableView(std::move(table), std::move(header),
      std::make_shared<LocalValueModel<optional<Index>>>(),
      &default_view_builder, parent) {}

TableView::TableView(std::shared_ptr<TableModel> table,
  std::shared_ptr<HeaderModel> header, std::shared_ptr<CurrentModel> current,
  QWidget* parent)
  : TableView(std::move(table), std::move(header), std::move(current),
      &default_view_builder, parent) {}

TableView::TableView(std::shared_ptr<TableModel> table,
  std::shared_ptr<HeaderModel> header, ViewBuilder view_builder,
  QWidget* parent)
  : TableView(std::move(table), std::move(header),
      std::make_shared<LocalValueModel<optional<Index>>>(),
      std::move(view_builder), parent) {}

TableView::TableView(std::shared_ptr<TableModel> table,
    std::shared_ptr<HeaderModel> header, std::shared_ptr<CurrentModel> current,
    ViewBuilder view_builder, QWidget* parent)
    : QWidget(parent) {
  auto box_body = new QWidget();
  auto box_body_layout = new QVBoxLayout(box_body);
  box_body_layout->setContentsMargins({});
  auto header_view = new TableHeader(header);
  box_body_layout->addWidget(header_view);
  auto box = new Box(box_body);
  update_style(*box, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  proxy_style(*this, *box);
  m_body = new TableBody(std::move(table), std::move(current),
    header_view->get_widths(), std::move(view_builder));
  auto layout = new QVBoxLayout(this);
  layout->addWidget(box);
  layout->addWidget(m_body);
}

const std::shared_ptr<TableModel>& TableView::get_table() const {
  return m_body->get_table();
}

const std::shared_ptr<TableView::CurrentModel>& TableView::get_current() const {
  return m_body->get_current();
}
