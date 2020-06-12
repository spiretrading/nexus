#include "Spire/Ui/ColorSelectorSlider.hpp"
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

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

ColorSelectorSlider::ColorSelectorSlider(const QColor& color,
    const QGradientStops& stops, QWidget* parent)
    : QWidget(parent),
      m_gradient_stops(stops),
      m_handle(
        imageFromSvg(":/Icons/color-picker-cursor.svg", scale(14, 14))) {
  setStyleSheet(QString("border: %1px solid #C8C8C8").arg(scale_width(1)));
  m_gradient = create_gradient_image(width(), height(), m_gradient_stops);
  // TODO: set last mouse x based on current/starting color
  m_last_mouse_x = 20;
}

void ColorSelectorSlider::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    set_mouse_x(event->x());
  }
}

void ColorSelectorSlider::mouseMoveEvent(QMouseEvent* event) {
  if(event->buttons() == Qt::LeftButton) {
    set_mouse_x(event->x());
  }
}

void ColorSelectorSlider::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);
  auto painter = QPainter(this);
  painter.drawImage(1, 1, m_gradient);
  painter.drawImage(m_last_mouse_x - (m_handle.width() / 2), HANDLE_Y(),
    m_handle);
}

void ColorSelectorSlider::resizeEvent(QResizeEvent* event) {
  m_gradient = create_gradient_image(width() - 2, height() - 2, m_gradient_stops);
}

void ColorSelectorSlider::set_mouse_x(int x) {
  m_last_mouse_x = std::max(0, std::min(x, width()));
  update();
}
