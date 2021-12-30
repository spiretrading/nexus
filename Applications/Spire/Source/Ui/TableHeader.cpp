#include "Spire/Ui/TableHeader.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/ListValueModel.hpp"

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
    layout->addWidget(
      new TableHeaderCell(make_list_value_model(m_cells, i)));
  }
}

const std::shared_ptr<ListModel<TableHeaderCell::Model>>&
    TableHeader::get_cells() const {
  return m_cells;
}
