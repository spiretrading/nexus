#include "Spire/Ui/FixedHorizontalLayout.hpp"

using namespace boost;
using namespace Spire;

FixedHorizontalLayout::FixedHorizontalLayout(QWidget* parent)
    : QLayout(parent) {
  setSpacing(0);
}

void FixedHorizontalLayout::addItem(QLayoutItem* item) {
  m_items.emplace_back(item);
}

QSize FixedHorizontalLayout::sizeHint() const {
  if(m_size_hint) {
    return *m_size_hint;
  }
  auto width = std::max(0, (count() - 1) * spacing());
  auto height = 0;
  for(auto& item : m_items) {
    width += item->sizeHint().width();
    height = std::max(height, item->sizeHint().height());
  }
  m_size_hint = QSize(width, height);
  return *m_size_hint;
}

void FixedHorizontalLayout::setGeometry(const QRect& rect) {
  QLayout::setGeometry(rect);
  auto size_hint = sizeHint();
  auto x = rect.x();
  for(auto i = 0; i < count() - 1; ++i) {
    auto& item = m_items[i];
    auto width = item->sizeHint().width();
    item->setGeometry(QRect(x, rect.y(), width, size_hint.height()));
    x += width + spacing();
  }
  if(!isEmpty()) {
    auto& item = m_items.back();
    auto width = rect.right() - x;
    item->setGeometry(QRect(x, rect.y(), width, size_hint.height()));
  }
}

QLayoutItem* FixedHorizontalLayout::itemAt(int index) const {
  if(index < count()) {
    return m_items[index].get();
  }
  return nullptr;
}

QLayoutItem* FixedHorizontalLayout::takeAt(int index) {
  if(index < count()) {
    auto item = std::move(m_items[index]);
    m_items.erase(m_items.begin() + index);
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
