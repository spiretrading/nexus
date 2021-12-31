#include "Spire/Ui/TableHeader.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/ListValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TableHeader::TableHeader(
    std::shared_ptr<ListModel<TableHeaderItem::Model>> items,
    QWidget* parent)
    : QWidget(parent),
      m_items(items) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  for(auto i = 0; i != m_items->get_size(); ++i) {
    auto item = new TableHeaderItem(make_list_value_model(m_items, i));
    auto is_last = i == m_items->get_size() - 1;
    auto horizontal_policy = [&] {
      if(is_last) {
        return QSizePolicy::Expanding;
      }
      return QSizePolicy::Fixed;
    }();
    item->setSizePolicy(horizontal_policy, QSizePolicy::Preferred);
    item->set_is_resizeable(!is_last);
    layout->addWidget(item);
  }
  m_items_connection = m_items->connect_operation_signal(
    std::bind_front(&TableHeader::on_items_update, this));
}

const std::shared_ptr<ListModel<TableHeaderItem::Model>>&
    TableHeader::get_items() const {
  return m_items;
}

connection TableHeader::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

connection TableHeader::connect_filter_signal(
    const FilterSignal::slot_type& slot) const {
  return m_filter_signal.connect(slot);
}

void TableHeader::on_items_update(
    const ListModel<TableHeaderItem::Model>::Operation& operation) {
}
