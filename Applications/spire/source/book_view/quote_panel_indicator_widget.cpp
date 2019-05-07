#include "spire/book_view/quote_panel_indicator_widget.hpp"
#include <QPainter>

using namespace Spire;

namespace {
  constexpr auto FADE_TIME_MS = 400;
  constexpr auto STARTING_ALPHA = 100;

  double map_to(double value, double a1, double a2, double b1, double b2) {
    return ((value - a1) * ((b2 - b1) / (a2 - a1))) + b1;
  }
}

QuotePanelIndicatorWidget::QuotePanelIndicatorWidget(QWidget* parent)
    : QWidget(parent),
      m_color(Qt::black) {
  m_animation_timer.setInterval(33);
  m_animation_timer.setSingleShot(true);
  connect(&m_animation_timer, &QTimer::timeout, this,
    &QuotePanelIndicatorWidget::on_animation_timer);
}

void QuotePanelIndicatorWidget::set_color(const QColor& color) {
  m_color = color;
  update();
}

void QuotePanelIndicatorWidget::animate_color(const QColor& color) {
  m_color = color;
  m_animation_start = std::chrono::steady_clock::now();
  on_animation_timer();
}

void QuotePanelIndicatorWidget::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.fillRect(rect(), m_color);
}

void QuotePanelIndicatorWidget::on_animation_timer() {
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::steady_clock::now() - m_animation_start);
  if(elapsed.count() >= FADE_TIME_MS) {
    m_color.setAlpha(255);
  } else {
    m_color.setAlpha(static_cast<int>(map_to(
      static_cast<double>(elapsed.count()), 0, FADE_TIME_MS, STARTING_ALPHA,
      255)));
    m_animation_timer.start();
  }
  update();
}
