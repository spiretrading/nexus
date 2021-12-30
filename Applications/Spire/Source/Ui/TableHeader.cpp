#include "Spire/Ui/TableHeader.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/ListValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
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
  m_cells_connection = m_cells->connect_operation_signal(
    std::bind_front(&TableHeader::on_cells_update, this));
}

const std::shared_ptr<ListModel<TableHeaderCell::Model>>&
    TableHeader::get_cells() const {
  return m_cells;
}

connection TableHeader::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

connection TableHeader::connect_filter_signal(
    const FilterSignal::slot_type& slot) const {
  return m_filter_signal.connect(slot);
}

void TableHeader::on_cells_update(
    const ListModel<TableHeaderCell::Model>::Operation& operation) {
}
