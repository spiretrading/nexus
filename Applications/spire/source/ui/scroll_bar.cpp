#include "spire/ui/scroll_bar.hpp"
#include <QEvent>

using namespace Spire;

#include <QDebug>

ScrollBar::ScrollBar(QScrollBar* scroll_bar)
    : QWidget(scroll_bar),
      m_parent(scroll_bar) {
  m_minimum = m_parent->minimum();
  m_maximum = m_parent->maximum();
  connect(m_parent, &QScrollBar::rangeChanged, this,
    &ScrollBar::on_range_changed);
  connect(m_parent, &QScrollBar::valueChanged, this,
    &ScrollBar::on_value_changed);
  // who determines where the mouse is?
  // when should repaint() be called (if at all)?
}

void ScrollBar::paintEvent(QPaintEvent* event) {
  // map ScrollBar geometry to parent geometry
  // draw track (if applicable)
  // draw thumb:
  // 13px if mouse is hovering
  // 6px if mouse is not hovering
}

void ScrollBar::on_range_changed(int minimum, int maximum) {
  m_minimum = minimum;
  m_maximum = maximum;
}

void ScrollBar::on_value_changed(int value) {
  qDebug() << "pos: " << mapTo(window(), m_parent->pos());
  qDebug() << "width: " << m_parent->width();
  qDebug() << "height: " << m_parent->height();
  qDebug() << value;
}
