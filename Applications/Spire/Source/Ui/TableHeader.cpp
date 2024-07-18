#include "Spire/Ui/TableHeader.hpp"
#include <QMouseEvent>
#include <QTimer>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
class HorizontalLayout : public QLayout {
public:
    HorizontalLayout(QWidget *parent = nullptr) : QLayout(parent) {}

    ~HorizontalLayout() {
        while (!items.empty())
            delete items.takeAt(0);
    }

    void addItem(QLayoutItem *item) override {
        items.push_back(item);
    }

    QSize sizeHint() const override {
        int width = 0;
        int height = 0;
        for (auto item : items) {
            width += item->sizeHint().width();
            height = std::max(height, item->sizeHint().height());
        }
        return QSize(width, height);
    }

    void setGeometry(const QRect &rect) override {
        QLayout::setGeometry(rect);
        int x = rect.x();
        for (auto item : items) {
            int itemWidth = item->sizeHint().width();
            int itemHeight = item->sizeHint().height();
            item->setGeometry(QRect(x, rect.y(), itemWidth, itemHeight));
            x += itemWidth;
        }
    }

    QLayoutItem* itemAt(int index) const override {
        if (index < items.size())
            return items.at(index);
        return nullptr;
    }

    QLayoutItem* takeAt(int index) override {
        if (index < items.size())
            return items.takeAt(index);
        return nullptr;
    }

    int count() const override {
        return items.size();
    }

    void invalidate() override {
        QLayout::invalidate();
        for (auto item : items) {
            item->invalidate();
        }
    }

private:
    QVector<QLayoutItem*> items;
};
}

TableHeader::TableHeader(
    std::shared_ptr<ListModel<TableHeaderItem::Model>> items, QWidget* parent)
    : QWidget(parent),
      m_items(items),
      m_width_update_count(0),
      m_resize_index(-1) {
  m_widths = std::make_shared<ArrayListModel<int>>();
  auto layout = new HorizontalLayout(this);
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
  }
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
