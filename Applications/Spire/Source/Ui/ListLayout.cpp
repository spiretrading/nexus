#include "Spire/Ui/ListLayout.hpp"
#include <QWidget>

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  int directed(const QSize& size, Qt::Orientation direction) {
    if(direction == Qt::Horizontal) {
      return size.width();
    }
    return size.height();
  }

  int perpendicular(const QSize& size, Qt::Orientation direction) {
    if(direction == Qt::Horizontal) {
      return size.height();
    }
    return size.width();
  }

  QSizePolicy::Policy perpendicular(const QSizePolicy& policy,
      Qt::Orientation direction) {
    if(direction == Qt::Horizontal) {
      return policy.verticalPolicy();
    }
    return policy.horizontalPolicy();
  }

  QSize make_directed_size(int along, int across, Qt::Orientation direction) {
    if(direction == Qt::Horizontal) {
      return QSize(along, across);
    }
    return QSize(across, along);
  }

  QRect make_directed_rect(int along_pos, int across_pos, int along_size,
      int across_size, Qt::Orientation direction) {
    if(direction == Qt::Horizontal) {
      return QRect(along_pos, across_pos, along_size, across_size);
    }
    return QRect(across_pos, along_pos, across_size, along_size);
  }

  int compute_across(QLayoutItem& item, Qt::Orientation direction,
      Overflow overflow, int perpendicular_extent) {
    if(overflow != Overflow::NONE) {
      return perpendicular(item.sizeHint(), direction);
    }
    auto min = perpendicular(item.minimumSize(), direction);
    auto max = perpendicular(item.maximumSize(), direction);
    return std::max(min, std::min(max, perpendicular_extent));
  }

  template<typename Visit>
  QSize layout_items(const std::vector<std::unique_ptr<QLayoutItem>>& items,
      Qt::Orientation direction, Overflow overflow, int item_gap,
      int overflow_gap, const QRect& rect, Visit&& visit) {
    if(items.empty()) {
      return QSize(0, 0);
    }
    auto row_capacity = [&] {
      if(overflow == Overflow::NONE) {
        return QWIDGETSIZE_MAX;
      }
      return directed(rect.size(), direction);
    }();
    auto [origin_along, origin_across] = [&] {
      if(direction == Qt::Horizontal) {
        return std::tuple(rect.left(), rect.top());
      }
      return std::tuple(rect.top(), rect.left());
    }();
    auto perpendicular_extent = perpendicular(rect.size(), direction);
    auto pos_along = 0;
    auto pos_across = 0;
    auto row_across_max = 0;
    auto items_in_row = 0;
    auto total_along_max = 0;
    for(auto& item_ptr : items) {
      auto& item = *item_ptr;
      auto hint = item.sizeHint();
      auto min = item.minimumSize();
      auto item_size = std::max(directed(min, direction),
        std::min(directed(hint, direction), row_capacity));
      auto item_across =
        compute_across(item, direction, overflow, perpendicular_extent);
      if(items_in_row > 0 &&
          pos_along + item_gap + item_size > row_capacity) {
        total_along_max = std::max(total_along_max, pos_along);
        pos_across += row_across_max + overflow_gap;
        pos_along = 0;
        row_across_max = 0;
        items_in_row = 0;
      }
      if(items_in_row > 0) {
        pos_along += item_gap;
      }
      auto item_rect = make_directed_rect(
        origin_along + pos_along, origin_across + pos_across,
        item_size, item_across, direction);
      visit(item, item_rect);
      pos_along += item_size;
      row_across_max = std::max(row_across_max, item_across);
      ++items_in_row;
    }
    total_along_max = std::max(total_along_max, pos_along);
    pos_across += row_across_max;
    return make_directed_size(total_along_max, pos_across, direction);
  }
}

ListLayout::ListLayout(QWidget* parent)
    : QLayout(parent),
      m_direction(Qt::Vertical),
      m_overflow(Overflow::NONE),
      m_item_gap(0),
      m_overflow_gap(0) {
  setContentsMargins({});
}

Qt::Orientation ListLayout::get_direction() const {
  return m_direction;
}

void ListLayout::set_direction(Qt::Orientation direction) {
  if(m_direction == direction) {
    return;
  }
  m_direction = direction;
  invalidate();
}

Overflow ListLayout::get_overflow() const {
  return m_overflow;
}

void ListLayout::set_overflow(Overflow overflow) {
  if(m_overflow == overflow) {
    return;
  }
  m_overflow = overflow;
  invalidate();
}

int ListLayout::get_item_gap() const {
  return m_item_gap;
}

void ListLayout::set_item_gap(int gap) {
  if(m_item_gap == gap) {
    return;
  }
  m_item_gap = gap;
  invalidate();
}

int ListLayout::get_overflow_gap() const {
  return m_overflow_gap;
}

void ListLayout::set_overflow_gap(int gap) {
  if(m_overflow_gap == gap) {
    return;
  }
  m_overflow_gap = gap;
  invalidate();
}

void ListLayout::insert_item(int index, QLayoutItem& item) {
  if(index < 0 || index > count()) {
    throw std::out_of_range("The index is out of range.");
  }
  m_items.emplace(m_items.begin() + index, &item);
  invalidate();
}

void ListLayout::insert_widget(int index, QWidget& widget) {
  if(index < 0 || index > count()) {
    throw std::out_of_range("The index is out of range.");
  }
  addChildWidget(&widget);
  insert_item(index, *new QWidgetItem(&widget));
}

void ListLayout::addItem(QLayoutItem* item) {
  if(item == nullptr) {
    return;
  }
  m_items.emplace_back(item);
  invalidate();
}

QLayoutItem* ListLayout::itemAt(int index) const {
  if(index < 0 || index >= count()) {
    return nullptr;
  }
  return m_items[index].get();
}

QLayoutItem* ListLayout::takeAt(int index) {
  if(index < 0 || index >= count()) {
    return nullptr;
  }
  auto item = std::move(m_items[index]);
  m_items.erase(m_items.begin() + index);
  invalidate();
  return item.release();
}

int ListLayout::count() const {
  return static_cast<int>(m_items.size());
}

QSize ListLayout::sizeHint() const {
  if(m_size_hint) {
    return *m_size_hint;
  }
  auto margins = contentsMargins();
  auto margin_size = QSize(margins.left() + margins.right(),
    margins.top() + margins.bottom());
  if(m_overflow == Overflow::WRAP) {
    if(auto parent = parentWidget()) {
      if(auto size = directed(parent->size(), m_direction); size > 0) {
        auto inner_size = size - directed(margin_size, m_direction);
        auto inner_rect = QRect(QPoint(),
          make_directed_size(inner_size, 0, m_direction));
        return compute_size(inner_rect) + margin_size;
      }
    }
  }
  auto direction_size = 0;
  auto perpendicular_size = 0;
  for(auto& item : m_items) {
    auto hint = item->sizeHint();
    if(m_overflow == Overflow::WRAP) {
      direction_size = std::max(direction_size, directed(hint, m_direction));
      perpendicular_size += perpendicular(hint, m_direction);
    } else {
      direction_size += directed(hint, m_direction);
      perpendicular_size =
        std::max(perpendicular_size, perpendicular(hint, m_direction));
    }
  }
  if(count() > 1) {
    auto gap_count = count() - 1;
    if(m_overflow == Overflow::WRAP) {
      perpendicular_size += gap_count * m_overflow_gap;
    } else {
      direction_size += gap_count * m_item_gap;
    }
  }
  m_size_hint = make_directed_size(direction_size, perpendicular_size,
    m_direction) + margin_size;
  return *m_size_hint;
}

Qt::Orientations ListLayout::expandingDirections() const {
  if(m_overflow == Overflow::NONE) {
    if(m_direction == Qt::Horizontal) {
      return Qt::Vertical;
    }
    return Qt::Horizontal;
  }
  return m_direction;
}

bool ListLayout::hasHeightForWidth() const {
  return m_overflow == Overflow::WRAP && m_direction == Qt::Horizontal;
}

int ListLayout::heightForWidth(int width) const {
  if(!hasHeightForWidth()) {
    return -1;
  }
  auto margins = contentsMargins();
  auto inner_width = width - margins.left() - margins.right();
  auto size = compute_size(QRect(0, 0, inner_width, 0));
  return size.height() + margins.top() + margins.bottom();
}

void ListLayout::setGeometry(const QRect& rect) {
  QLayout::setGeometry(rect);
  apply_layout(contentsRect());
}

void ListLayout::invalidate() {
  m_size_hint = none;
  QLayout::invalidate();
}

QSize ListLayout::compute_size(const QRect& rect) const {
  return layout_items(m_items, m_direction, m_overflow, m_item_gap,
    m_overflow_gap, rect, [] (QLayoutItem&, const QRect&) {});
}

void ListLayout::apply_layout(const QRect& rect) {
  layout_items(m_items, m_direction, m_overflow, m_item_gap, m_overflow_gap,
    rect, [] (QLayoutItem& item, const QRect& item_rect) {
      item.setGeometry(item_rect);
    });
}
