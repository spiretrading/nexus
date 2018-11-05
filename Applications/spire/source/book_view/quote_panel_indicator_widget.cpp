#include "spire/book_view/quote_panel_indicator_widget.hpp"
#include <QPainter>

using namespace Spire;

QuotePanelIndicatorWidget::QuotePanelIndicatorWidget(QWidget* parent)
    : QWidget(parent),
      m_color(Qt::black) {}

void QuotePanelIndicatorWidget::set_color(const QColor& color) {
  m_color = color;
  repaint();
}

void QuotePanelIndicatorWidget::animate_color(const QColor& color) {
  m_color = color;
  repaint();
}

void QuotePanelIndicatorWidget::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(m_animation_timer.isActive()) {
    painter.fillRect(this->rect(), m_color);
  } else {
    painter.fillRect(this->rect(), m_color);
  }
}
