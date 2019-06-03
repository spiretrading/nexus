#include "spire/charting/style_dropdown_menu_item.hpp"
#include <QMouseEvent>
#include <QPainter>
#include "spire/spire/dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

StyleDropdownMenuItem::StyleDropdownMenuItem(TrendLineStyle style,
    QWidget* parent)
    : QWidget(parent),
      m_style(style),
      m_is_highlighted(false),
      m_padding(scale_width(8)),
      m_line_length(scale_width(52)) {
  setMouseTracking(true);
  setFixedHeight(scale_height(20));
}

TrendLineStyle StyleDropdownMenuItem::get_style() const {
  return m_style;
}

void StyleDropdownMenuItem::set_highlight() {
  m_is_highlighted = true;
}

void StyleDropdownMenuItem::remove_highlight() {
  m_is_highlighted = false;
}

connection StyleDropdownMenuItem::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

void StyleDropdownMenuItem::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    m_selected_signal(m_style);
  }
}

void StyleDropdownMenuItem::leaveEvent(QEvent* event) {
  m_is_highlighted = false;
  update();
}

void StyleDropdownMenuItem::mouseMoveEvent(QMouseEvent* event) {
  m_is_highlighted = true;
  update();
}

void StyleDropdownMenuItem::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_selected_signal(m_style);
  }
}

void StyleDropdownMenuItem::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(m_is_highlighted) {
    painter.fillRect(event->rect(), QColor("#F2F2FF"));
  } else  {
    painter.fillRect(event->rect(), QColor("#FFFFFF"));
  }
  auto y = event->rect().height() / 2;
  draw_trend_line(painter, m_style, Qt::black, m_padding, y,
    m_padding + m_line_length, y);
}
