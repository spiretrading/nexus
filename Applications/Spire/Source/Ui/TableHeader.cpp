#include "Spire/Ui/TableHeader.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QMouseEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TableHeader::TableHeader(
    std::shared_ptr<ListModel<TableHeaderItem::Model>> items, QWidget* parent)
    : QWidget(parent),
      m_items(items),
      m_resize_index(-1) {
  m_widths = std::make_shared<ArrayListModel<int>>();
  auto layout = make_hbox_layout(this);
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
        m_widths->push(item->sizeHint().width());
      }
      item->installEventFilter(this);
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
  }
  m_items_connection = m_items->connect_operation_signal(
    std::bind_front(&TableHeader::on_items_operation, this));
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

connection TableHeader::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

connection TableHeader::connect_filter_signal(
    const FilterSignal::slot_type& slot) const {
  return m_filter_signal.connect(slot);
}

bool TableHeader::eventFilter(QObject* watched, QEvent* event) {
  auto find_item_index = [&] (QObject* item) {
    auto i = std::find(m_item_views.begin(), m_item_views.end(), item);
    if(i != m_item_views.end()) {
      return std::distance(m_item_views.begin(), i);
    }
    return -1;
  };
  if(event->type() == QEvent::HideToParent) {
    auto index = find_item_index(watched);
    if(index != -1) {
      auto current_blocker = shared_connection_block(m_widths_connection);
      m_widths->set(index, 0);
    }
  } else if(event->type() == QEvent::ShowToParent) {
    auto index = find_item_index(watched);
    if(index != -1) {
      auto current_blocker = shared_connection_block(m_widths_connection);
      m_widths->set(index, m_item_views[index]->width());
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TableHeader::mouseMoveEvent(QMouseEvent* event) {
  if(m_resize_index == -1) {
    return QWidget::mouseMoveEvent(event);
  }
  auto position = QCursor::pos();
  auto delta = position.x() - m_resize_position.x();
  if(delta < 0) {
    auto width = m_widths->get(m_resize_index);
    auto new_width = std::max(scale_width(10), width + delta);
    delta = new_width - width;
    position.rx() = delta + m_resize_position.x();
    if(delta != 0) {
      m_widths->set(m_resize_index, m_widths->get(m_resize_index) + delta);
      auto sibling_index = get_next_visible_sibling_index(m_resize_index);
      if(sibling_index != m_widths->get_size() - 1) {
        m_widths->set(sibling_index, m_widths->get(sibling_index) - delta);
      }
    }
  } else if(delta > 0) {
    auto sibling_index = get_next_visible_sibling_index(m_resize_index);
    auto sibling_width = [&] {
      if(sibling_index == m_widths->get_size() - 1) {
        auto w = 0;
        for(auto i = 0; i != m_widths->get_size(); ++i) {
          w += m_widths->get(i);
        }
        return width() - w;
      }
      return m_widths->get(sibling_index);
    }();
    auto new_sibling_width = std::max(scale_width(10), sibling_width - delta);
    delta = new_sibling_width - sibling_width;
    position.rx() = -delta + m_resize_position.x();
    if(delta != 0) {
      if(sibling_index != m_widths->get_size() - 1) {
        m_widths->set(sibling_index, m_widths->get(sibling_index) + delta);
      }
      m_widths->set(m_resize_index, m_widths->get(m_resize_index) - delta);
    }
  }
  m_resize_position = position;
}

int TableHeader::get_next_visible_sibling_index(int index) const {
  if(index == m_widths->get_size() - 1) {
    return index;
  }
  ++index;
  if(!m_item_views[index]->isVisible()) {
    return get_next_visible_sibling_index(index);
  }
  return index;
}

void TableHeader::on_items_operation(
    const ListModel<TableHeaderItem::Model>::Operation& operation) {
}

void TableHeader::on_widths_operation(
    const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::UpdateOperation& operation) {
      m_item_views[operation.m_index]->setFixedWidth(
        m_widths->get(operation.m_index));
      if(!m_item_views[operation.m_index]->isVisible()) {
        auto current_blocker = shared_connection_block(m_widths_connection);
        m_widths->set(operation.m_index, 0);
      }
    });
}

void TableHeader::on_start_resize(int index) {
  m_resize_index = index;
  m_resize_position = QCursor::pos();
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
