#include "Spire/Ui/TableHeader.hpp"
#include <QEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/FixedHorizontalLayout.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

TableHeader::TableHeader(
    std::shared_ptr<ListModel<TableHeaderItem::Model>> items, QWidget* parent)
    : QWidget(parent),
      m_items(items) {
  m_widths = std::make_shared<ArrayListModel<int>>();
  auto body = new QWidget();
  auto layout = new FixedHorizontalLayout(body);
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
    if(!is_last) {
      if(m_widths->get_size() > i) {
        item->setFixedWidth(m_widths->get(i));
      } else {
        std::static_pointer_cast<ArrayListModel<int>>(m_widths)->push(
          item->sizeHint().width());
      }
    }
    item->set_is_resizeable(!is_last);
    item->connect_sort_signal(std::bind_front(&TableHeader::on_sort, this, i));
    item->is_filtered()->connect_update_signal(
      std::bind_front(&TableHeader::on_filtered, this, i));
    layout->addWidget(item);
    m_item_views.push_back(item);
    link(*this, *item);
    item->installEventFilter(this);
  }
  auto box = new Box(body);
  update_style(*box, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderBottomSize(scale_width(1))).
      set(BorderBottomColor(0xE0E0E0)).
      set(PaddingLeft(scale_width(1)));
  });
  enclose(*this, *box);
  proxy_style(*this, *box);
  m_widths_connection = m_widths->connect_operation_signal(
    std::bind_front(&TableHeader::on_widths_operation, this));
}

const std::shared_ptr<ListModel<TableHeaderItem::Model>>&
    TableHeader::get_items() const {
  return m_items;
}

const std::shared_ptr<ListModel<int>>& TableHeader::get_widths() const {
  return m_widths;
}

TableHeaderItem* TableHeader::get_item(int column) {
  if(column < 0 || column >= std::ssize(m_item_views)) {
    return nullptr;
  }
  return m_item_views[column];
}

optional<int> TableHeader::get_index(TableHeaderItem* item) const {
  auto i = std::find(m_item_views.begin(), m_item_views.end(), item);
  if(i != m_item_views.end()) {
    return std::distance(m_item_views.begin(), i);
  }
  return {};
}

connection TableHeader::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

connection TableHeader::connect_toggle_filter_signal(
    const ToggleFilterSignal::slot_type& slot) const {
  return m_filter_signal.connect(slot);
}

bool TableHeader::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Resize) {
    if(auto index = get_index(static_cast<TableHeaderItem*>(watched));
        index && *index < m_widths->get_size()) {
      auto width = static_cast<TableHeaderItem*>(watched)->width();
      if(width != m_widths->get(*index)) {
        m_widths->set(*index, width);
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TableHeader::on_widths_operation(
    const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::UpdateOperation& operation) {
      m_item_views[operation.m_index]->setFixedWidth(
        operation.get_value());
    });
}

void TableHeader::on_sort(int index, TableHeaderItem::Order order) {
  m_sort_signal(index, order);
}

void TableHeader::on_filtered(int index, bool is_filtered) {
  m_filter_signal(index, is_filtered);
}
