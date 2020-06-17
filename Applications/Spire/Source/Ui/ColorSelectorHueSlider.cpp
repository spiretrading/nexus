#include "Spire/Ui/ColorSelectorHueSlider.hpp"
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto HANDLE_Y() {
    static auto y = scale_height(3);
    return y;
  }

  auto create_gradient_image(int width, int height) {
    auto image = QImage(width, height, QImage::Format_RGB32);
    for(auto x = 0; x < image.width(); ++x) {
      auto hue = static_cast<int>(map_to(x, 0, image.width() - 1, 0.0, 359.0));
      for(auto y = 0; y < image.height(); ++y) {
        image.setPixelColor(x, y, QColor::fromHsv(hue, 255, 255));
      }
    }
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
  m_gradient = create_gradient_image(width(), height());
  m_last_mouse_x = get_mouse_x(color.hue());
}

void ColorSelectorHueSlider::set_color(const QColor& color) {
  if(color.hue() == -1) {
    return;
  }
  m_current_color = color;
  m_last_mouse_x = get_mouse_x(color.hue());
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
  m_gradient = create_gradient_image(width() - 2, height() - 2);
}

int ColorSelectorHueSlider::get_mouse_x(int hue) {
  return static_cast<int>(map_to(hue, 0, 359, static_cast<double>(1),
    static_cast<double>(m_gradient.width() - 1)));
}

void ColorSelectorHueSlider::set_mouse_x(int x) {
  m_last_mouse_x = std::max(1, std::min(x, m_gradient.width() - 1));
  update();
}
