#include "Spire/Ui/ScrollableLayer.hpp"
#include <QApplication>
#include <QKeyEvent>
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBar.hpp"

using namespace Spire;
using namespace Spire::Styles;

ScrollableLayer::ScrollableLayer(QWidget* parent)
    : QWidget(parent),
      m_vertical_scroll_bar(new ScrollBar(Qt::Orientation::Vertical, this)),
      m_horizontal_scroll_bar(
        new ScrollBar(Qt::Orientation::Horizontal, this)),
      m_corner_box(new Box(nullptr)) {
  auto layout = make_grid_layout(this);
  layout->setColumnStretch(0, 1);
  layout->addItem(
    new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 0,
    0);
  layout->addWidget(m_vertical_scroll_bar, 0, 1);
  layout->addWidget(m_horizontal_scroll_bar, 1, 0);
  update_style(*m_corner_box, [&] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFF, 0xFF, 0xFF)));
  });
  m_corner_box->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  layout->addWidget(m_corner_box, 1, 1);
  setAttribute(Qt::WA_TransparentForMouseEvents);
  m_vertical_scroll_bar->installEventFilter(this);
  m_horizontal_scroll_bar->installEventFilter(this);
  m_corner_box->installEventFilter(this);
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
    scroll_line_up(*m_horizontal_scroll_bar);
  } else if(event->key() == Qt::Key_PageUp) {
    scroll_page_up(*m_vertical_scroll_bar);
  } else if(event->key() == Qt::Key_PageDown) {
    scroll_page_down(*m_vertical_scroll_bar);
  } else if(event->key() == Qt::Key_Home) {
    scroll_to_start(*m_vertical_scroll_bar);
  } else if(event->key() == Qt::Key_End) {
    scroll_to_end(*m_vertical_scroll_bar);
  } else {
    event->ignore();
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

bool ScrollableLayer::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Resize || event->type() == QEvent::Move ||
      event->type() == QEvent::Show || event->type() == QEvent::Hide) {
    update_mask();
  }
  return QWidget::eventFilter(watched, event);
}

void ScrollableLayer::resizeEvent(QResizeEvent* event) {
  update_mask();
}

void ScrollableLayer::update_mask() {
  if(m_vertical_scroll_bar->isHidden() && m_horizontal_scroll_bar->isHidden()) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    return;
  }
  auto region = [&] {
    if(m_vertical_scroll_bar->isVisible() &&
        m_horizontal_scroll_bar->isVisible()) {
      return QPolygon(m_vertical_scroll_bar->geometry()).united(
        m_horizontal_scroll_bar->geometry()).united(m_corner_box->geometry());
    } else if(m_vertical_scroll_bar->isVisible()) {
      return QPolygon(m_vertical_scroll_bar->geometry());
    }
    return QPolygon(m_horizontal_scroll_bar->geometry());
  }();
  if(region.isEmpty()) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
  } else {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMask(region);
  }
}
