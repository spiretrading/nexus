#include "Spire/Ui/FixedHorizontalLayout.hpp"

using namespace boost;
using namespace Spire;

FixedHorizontalLayout::FixedHorizontalLayout(QWidget* parent)
    : QLayout(parent) {
  setSpacing(0);
  setContentsMargins({});
}

void FixedHorizontalLayout::move(int source, int destination) {
  move_element(m_items, source, destination);
  update();
}

void FixedHorizontalLayout::addItem(QLayoutItem* item) {
  m_items.emplace_back(item);
  invalidate();
}

QSize FixedHorizontalLayout::sizeHint() const {
  if(m_size_hint) {
    return *m_size_hint;
  }
  auto margins = contentsMargins();
  auto width =
    std::max(0, (count() - 1) * spacing() + margins.left() + margins.right());
  auto height = std::max(0, margins.top() + margins.bottom());
  for(auto& item : m_items) {
    width += item->sizeHint().width();
    height = std::max(height, item->sizeHint().height());
  }
  m_size_hint = QSize(width, height);
  return *m_size_hint;
}

void FixedHorizontalLayout::setGeometry(const QRect& rect) {
  QLayout::setGeometry(rect);
  auto geometry = contentsRect();
  auto size_hint = sizeHint();
  auto x = geometry.left();
  for(auto i = 0; i < count() - 1; ++i) {
    auto& item = m_items[i];
    auto width = item->sizeHint().width();
    item->setGeometry(QRect(x, geometry.y(), width, size_hint.height()));
    if(width > 0) {
      x += width + spacing();
    }
  }
  if(!isEmpty()) {
    auto& item = m_items.back();
    auto width = geometry.width() - x + geometry.left();
    item->setGeometry(QRect(x, geometry.y(), width, size_hint.height()));
  }
}

QLayoutItem* FixedHorizontalLayout::itemAt(int index) const {
  if(index >= 0 && index < count()) {
    return m_items[index].get();
  }
  return nullptr;
}

QLayoutItem* FixedHorizontalLayout::takeAt(int index) {
  if(index < count()) {
    auto item = std::move(m_items[index]);
    m_items.erase(m_items.begin() + index);
    invalidate();
    return item.release();
  }
  return nullptr;
}

int FixedHorizontalLayout::count() const {
  return static_cast<int>(m_items.size());
}

void FixedHorizontalLayout::invalidate() {
  QLayout::invalidate();
  m_size_hint = none;
  for(auto& item : m_items) {
    item->invalidate();
  }
}
