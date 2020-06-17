#include "Spire/Ui/ColorSelectorValueSlider.hpp"
#include <algorithm>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto create_gradient_image(int width, int height, int hue) {
    auto image = QImage(width, height, QImage::Format_RGB32);
    for(auto x = 0; x < image.width(); ++x) {
      for(auto y = 0; y < image.height(); ++y) {
        image.setPixelColor(x, height - 1 - y, QColor::fromHsv(hue,
          static_cast<int>(map_to(x, 0, width - 1, 0.0, 255.0)),
          static_cast<int>(map_to(y, 0, height - 1, 0.0, 255.0))));
      }
    }
    return image;
  }
}

ColorSelectorValueSlider::ColorSelectorValueSlider(const QColor& current_color,
    QWidget* parent)
    : QWidget(parent),
      m_current_color(current_color),
      m_handle(
        imageFromSvg(":/Icons/color-picker-cursor.svg", scale(14, 14))) {
  update_gradient();
  m_last_mouse_pos = get_mouse_pos(m_current_color);
}

void ColorSelectorValueSlider::set_color(const QColor& color) {
  if(color.hue() == -1) {
    m_current_color = QColor::fromHsv(m_current_color.hue(),
      color.saturation(), color.value());
  } else {
    m_current_color = color;
  }
  update_gradient();
  m_last_mouse_pos = get_mouse_pos(color);
  update();
}

void ColorSelectorValueSlider::set_hue(int hue) {
  m_current_color.setHsv(hue, m_current_color.saturation(),
    m_current_color.value());
  update_gradient();
  update();
}

connection ColorSelectorValueSlider::connect_color_signal(
    const ColorSignal::slot_type& slot) const {
  return m_color_signal.connect(slot);
}

void ColorSelectorValueSlider::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    set_mouse_pos(event->pos());
    on_color_changed();
  }
}

void ColorSelectorValueSlider::mouseMoveEvent(QMouseEvent* event) {
  if(event->buttons() == Qt::LeftButton) {
    set_mouse_pos(event->pos());
    on_color_changed();
  }
}

void ColorSelectorValueSlider::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);
  auto painter = QPainter(this);
  painter.drawImage(1, 1, m_gradient);
  painter.drawImage(m_last_mouse_pos.x() - (m_handle.width() / 2),
    m_last_mouse_pos.y() - (m_handle.height() / 2), m_handle);
  painter.setPen({QColor("#C8C8C8"), 1});
  painter.drawRect(0, 0, width() - 1, height() - 1);
}

void ColorSelectorValueSlider::resizeEvent(QResizeEvent* event) {
  update_gradient();
}

QPoint ColorSelectorValueSlider::get_mouse_pos(const QColor& color) {
  return { static_cast<int>(map_to(color.saturation(), 0, 255, 1.0,
    static_cast<double>(m_gradient.width() - 1))),
    m_gradient.height() - static_cast<int>(map_to(color.value(), 0, 255, 1.0,
    static_cast<double>(m_gradient.height() - 1)))};
}

void ColorSelectorValueSlider::set_mouse_pos(const QPoint& pos) {
  m_last_mouse_pos.setX(std::max(1, std::min(pos.x(),
    m_gradient.width() - 1)));
  m_last_mouse_pos.setY(std::max(1, std::min(pos.y(),
    m_gradient.height() - 1)));
  update();
}

void ColorSelectorValueSlider::update_gradient() {
  m_gradient = create_gradient_image(width() - 2, height() - 2,
    m_current_color.hue());
}

void ColorSelectorValueSlider::on_color_changed() {
  m_current_color = QColor::fromHsv(m_current_color.hue(),
    static_cast<int>(map_to(m_last_mouse_pos.x(), 1, m_gradient.width() - 1,
      0.0, 255.0)),
    255 - static_cast<int>(map_to(m_last_mouse_pos.y(), 1,
      m_gradient.height() - 1, 0.0, 255.0)));
  m_color_signal(m_current_color);
}
