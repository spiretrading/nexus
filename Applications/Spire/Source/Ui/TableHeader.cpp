#include "Spire/Ui/TableHeader.hpp"
#include <QMouseEvent>
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
      m_items(items),
      m_resize_index(-1) {
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
    item->connect_start_resize_signal(
      std::bind_front(&TableHeader::on_start_resize, this, i));
    item->connect_end_resize_signal(
      std::bind_front(&TableHeader::on_end_resize, this, i));
    item->connect_sort_signal(std::bind_front(&TableHeader::on_sort, this, i));
    item->connect_filter_signal(
      std::bind_front(&TableHeader::on_filter, this, i));
    layout->addWidget(item);
    m_item_views.push_back(item);
    link(*this, *item);
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

Button& TableHeader::get_filter_button(int column) {
  return m_item_views[column]->get_filter_button();
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

connection TableHeader::connect_filter_signal(
    const FilterSignal::slot_type& slot) const {
  return m_filter_signal.connect(slot);
}

void TableHeader::mouseMoveEvent(QMouseEvent* event) {
  if(m_resize_index == -1) {
    return QWidget::mouseMoveEvent(event);
  }
  auto& item = *m_item_views[m_resize_index];
  auto width =
    std::max(scale_width(10), item.mapFromGlobal(QCursor::pos()).x());
  if(width != m_widths->get(m_resize_index)) {
    m_widths->set(m_resize_index, width);
  }
}

void TableHeader::on_widths_operation(
    const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::UpdateOperation& operation) {
      m_item_views[operation.m_index]->setFixedWidth(
        operation.get_value());
    });
}

void TableHeader::on_start_resize(int index) {
  m_resize_index = index;
}

void TableHeader::on_end_resize(int index) {
  m_resize_index = -1;
}

void TableHeader::on_sort(int index, TableHeaderItem::Order order) {
  m_sort_signal(index, order);
}

void TableHeader::on_filter(int index) {
  m_filter_signal(index);
}
