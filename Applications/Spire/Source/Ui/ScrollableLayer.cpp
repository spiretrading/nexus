#include "Spire/Ui/ScrollableLayer.hpp"
#include <QApplication>
#include <QGridLayout>
#include <QKeyEvent>
#include "Spire/Ui/ScrollBar.hpp"

using namespace Spire;
using namespace Spire::Styles;

ScrollableLayer::ScrollableLayer(QWidget* parent)
  : StyledWidget(parent),
    m_vertical_scroll_bar(new ScrollBar(Qt::Orientation::Vertical, this)),
    m_horizontal_scroll_bar(new ScrollBar(Qt::Orientation::Horizontal, this)) {
  m_vertical_scroll_bar->set_range(0, 1000);
  auto layout = new QGridLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout->setColumnStretch(0, 1);
  layout->addItem(new QSpacerItem(
    1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 0);
  layout->addWidget(m_vertical_scroll_bar, 0, 1);
  layout->addWidget(m_horizontal_scroll_bar, 1, 0);
  layout->addItem(new QSpacerItem(
    1, 1, QSizePolicy::Minimum, QSizePolicy::Minimum), 1, 1);
}

ScrollBar& ScrollableLayer::get_vertical_scroll_bar() {
  return *m_vertical_scroll_bar;
}

ScrollBar& ScrollableLayer::get_horizontal_scroll_bar() {
  return *m_horizontal_scroll_bar;
}

void ScrollableLayer::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Up) {
    scroll_line_up(*m_vertical_scroll_bar);
  } else if(event->key() == Qt::Key_Down) {
    scroll_line_down(*m_vertical_scroll_bar);
  } else if(event->key() == Qt::Key_Right) {
    scroll_line_down(*m_horizontal_scroll_bar);
  } else if(event->key() == Qt::Key_Left) {
    scroll_line_down(*m_horizontal_scroll_bar);
  } else if(event->key() == Qt::Key_PageUp) {
    scroll_page_up(*m_vertical_scroll_bar);
  } else if(event->key() == Qt::Key_PageDown) {
    scroll_page_down(*m_vertical_scroll_bar);
  } else if(event->key() == Qt::Key_Home) {
    scroll_to_start(*m_vertical_scroll_bar);
  } else if(event->key() == Qt::Key_End) {
    scroll_to_end(*m_vertical_scroll_bar);
  }
}

void ScrollableLayer::wheelEvent(QWheelEvent* event) {
  auto scroll_bar = [&] {
    if(event->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier)) {
      return m_horizontal_scroll_bar;
    } else {
      return m_vertical_scroll_bar;
    }
  }();
  if(event->angleDelta().y() < 0) {
    scroll_line_down(*scroll_bar, QApplication::wheelScrollLines());
  } else if(event->angleDelta().y() > 0) {
    scroll_line_up(*scroll_bar, QApplication::wheelScrollLines());
  }
  if(event->angleDelta().x() < 0) {
    scroll_line_down(*m_horizontal_scroll_bar);
  } else if(event->angleDelta().x() > 0) {
    scroll_line_up(*m_horizontal_scroll_bar);
  }
}
