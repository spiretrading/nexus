#include "Spire/Ui/ColorSelectorHueSlider.hpp"
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto HANDLE_Y() {
    static auto y = scale_height(3);
    return y;
  }

  auto create_gradient_image(int width, int height,
      const QGradientStops& stops) {
    auto image = QImage(width, height, QImage::Format_RGB32);
    auto painter = QPainter(&image);
    auto gradient = QLinearGradient(0, height / 2, width, height);
    gradient.setStops(stops);
    painter.fillRect(0, 0, width, height, gradient);
    painter.end();
    return image;
  }
}

ColorSelectorHueSlider::ColorSelectorHueSlider(const QColor& color,
    QWidget* parent)
    : QWidget(parent),
      m_current_color(color),
      m_handle(
        imageFromSvg(":/Icons/color-picker-cursor.svg", scale(14, 14))) {
  m_gradient_stops = QGradientStops({{0.0, QColor("#FF00FF")},
      {0.16, QColor("#0000FF")}, {0.32, QColor("#00FFFF")},
      {0.48, QColor("#00FF00")}, {0.64, QColor("#FFFF00")},
      {0.8, QColor("#FF0000")}, {1.0, QColor("#FF00FF")}});
  m_gradient = create_gradient_image(width(), height(), m_gradient_stops);
  m_last_mouse_x = get_mouse_x(color);
}

void ColorSelectorHueSlider::set_color(const QColor& color) {
  if(color.value() == m_current_color.value()) {
    return;
  }
  m_current_color = color;
  m_last_mouse_x = get_mouse_x(color);
  update();
}

connection ColorSelectorHueSlider::connect_color_signal(
    const ColorSignal::slot_type& slot) const {
  return m_color_signal.connect(slot);
}

void ColorSelectorHueSlider::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    set_mouse_x(event->x());
    m_current_color = m_gradient.pixelColor(m_last_mouse_x, 0);
    m_color_signal(m_current_color);
  }
}

void ColorSelectorHueSlider::mouseMoveEvent(QMouseEvent* event) {
  if(event->buttons() == Qt::LeftButton) {
    set_mouse_x(event->x());
    m_current_color = m_gradient.pixelColor(m_last_mouse_x, 0);
    m_color_signal(m_current_color);
  }
}

void ColorSelectorHueSlider::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);
  auto painter = QPainter(this);
  painter.drawImage(1, 1, m_gradient);
  painter.drawImage(m_last_mouse_x - (m_handle.width() / 2), HANDLE_Y(),
    m_handle);
  painter.setPen({QColor("#C8C8C8"), 1});
  painter.drawRect(0, 0, width() - 1, height() - 1);
}

void ColorSelectorHueSlider::resizeEvent(QResizeEvent* event) {
  m_gradient = create_gradient_image(width() - 2, height() - 2,
    m_gradient_stops);
}

int ColorSelectorHueSlider::get_mouse_x(const QColor& color) {
  for(auto i = 0; i < m_gradient.width(); ++i) {
    if(m_gradient.pixelColor(i, 0).hue() == color.hue()) {
      return i;
    }
  }
  return m_last_mouse_x;
}

void ColorSelectorHueSlider::set_mouse_x(int x) {
  m_last_mouse_x = std::max(1, std::min(x, width() - 1));
  update();
}
