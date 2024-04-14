#include "Spire/LegacyUI/ExpandButton.hpp"
#include <QStyleOption>
#include <QStylePainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::LegacyUI;

ExpandButton::ExpandButton(QWidget* parent)
    : QWidget(parent),
      m_expanded(false) {
  Setup();
}

ExpandButton::ExpandButton(bool expanded, QWidget* parent)
    : QWidget(parent),
      m_expanded(expanded) {
  Setup();
}

ExpandButton::~ExpandButton() {}

bool ExpandButton::IsExpanded() const {
  return m_expanded;
}

void ExpandButton::SetExpanded(bool expanded) {
  if(expanded == m_expanded) {
    return;
  }
  m_expanded = expanded;
  if(m_expanded) {
    m_expandedSignal();
  } else {
    m_collapsedSignal();
  }
  update();
}

connection ExpandButton::ConnectExpandedSignal(
    const ExpandedSignal::slot_type& slot) const {
  return m_expandedSignal.connect(slot);
}

connection ExpandButton::ConnectCollapsedSignal(
    const CollapsedSignal::slot_type& slot) const {
  return m_collapsedSignal.connect(slot);
}

void ExpandButton::paintEvent(QPaintEvent* event) {
  static const int DECORATION_WIDTH = scale_width(9);
  static const int DECORATION_HEIGHT = scale_height(9);
  QWidget::paintEvent(event);
  QStylePainter painter(this);
  QStyleOption option;
  option.initFrom(this);
  int x = option.rect.x() + DECORATION_WIDTH / 2;
  int y = option.rect.y() + option.rect.height() / 2 - DECORATION_HEIGHT / 2;
  painter.drawLine(x + scale_width(2), y + scale_height(4), x + scale_width(6),
    y + scale_height(4));
  if(!m_expanded) {
    painter.drawLine(x + scale_width(4), y + scale_height(2),
      x + scale_width(4), y + scale_height(6));
  }
  QPen oldPen = painter.pen();
  painter.setPen(option.palette.dark().color());
  painter.drawRect(x, y, DECORATION_WIDTH - scale_width(1),
    DECORATION_HEIGHT - scale_height(1));
  painter.setPen(oldPen);
}

void ExpandButton::mousePressEvent(QMouseEvent* event) {
  m_expanded = !m_expanded;
  if(m_expanded) {
    m_expandedSignal();
  } else {
    m_collapsedSignal();
  }
  update();
}

void ExpandButton::Setup() {
  QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
  setSizePolicy(sizePolicy);
  setMinimumSize(scale(16, 16));
  setMaximumSize(scale(16, 16));
  update();
}
