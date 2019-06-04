#include "Spire/Charting/StyleDropDownMenuItem.hpp"
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

StyleDropDownMenuItem::StyleDropDownMenuItem(TrendLineStyle style,
    QWidget* parent)
    : QWidget(parent),
      m_style(style),
      m_is_highlighted(false),
      m_padding(scale_width(8)),
      m_line_length(scale_width(52)) {
  setMouseTracking(true);
  setFixedHeight(scale_height(20));
}

TrendLineStyle StyleDropDownMenuItem::get_style() const {
  return m_style;
}

void StyleDropDownMenuItem::set_highlight() {
  m_is_highlighted = true;
}

void StyleDropDownMenuItem::remove_highlight() {
  m_is_highlighted = false;
}

connection StyleDropDownMenuItem::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

void StyleDropDownMenuItem::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    m_selected_signal(m_style);
  }
}

void StyleDropDownMenuItem::leaveEvent(QEvent* event) {
  m_is_highlighted = false;
  update();
}

void StyleDropDownMenuItem::mouseMoveEvent(QMouseEvent* event) {
  m_is_highlighted = true;
  update();
}

void StyleDropDownMenuItem::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_selected_signal(m_style);
  }
}

void StyleDropDownMenuItem::paintEvent(QPaintEvent* event) {
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
