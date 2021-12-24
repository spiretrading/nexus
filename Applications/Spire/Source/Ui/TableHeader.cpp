#include "Spire/Ui/TableHeader.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/LocalCompositeValueModel.hpp"

using namespace Spire;

TableHeader::TableHeader(
    std::shared_ptr<ListModel<TableHeaderCell::Model>> cells,
    QWidget* parent)
    : QWidget(parent),
      m_cells(cells) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  for(auto i = 0; i != m_cells->get_size(); ++i) {
    auto cell = new TableHeaderCell(
      std::make_shared<LocalCompositeValueModel<TableHeaderCell::Model>>(
        m_cells->get(i)));
    layout->addWidget(cell);
  }
}

const std::shared_ptr<ListModel<TableHeaderCell::Model>>&
    TableHeader::get_cells() const {
  return m_cells;
}
