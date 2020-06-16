#include "Spire/Ui/ColorSelectorValueSlider.hpp"
#include <algorithm>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto create_gradient_image(int width, int height, const QColor& color) {
    auto scene = QGraphicsScene(0, 0, width, height);
    auto color_gradient = QLinearGradient(0, 0, width, 0);
    color_gradient.setColorAt(0, QColor(255,255,255));
    color_gradient.setColorAt(1, color);
    scene.setBackgroundBrush(color_gradient);
    auto black_gradient = QLinearGradient(0, 0, 0, height);
    black_gradient.setColorAt(0, QColor(0,0,0,0));
    black_gradient.setColorAt(1, QColor(0,0,0,255));
    scene.setForegroundBrush(black_gradient);
    auto image = QImage(scene.sceneRect().size().toSize(),
      QImage::Format_RGB32);
    auto painter = QPainter(&image);
    scene.render(&painter);
    return image;
  }
}

ColorSelectorValueSlider::ColorSelectorValueSlider(const QColor& current_color,
    QWidget* parent)
    : QWidget(parent),
      m_current_color(current_color),
      m_handle(
        imageFromSvg(":/Icons/color-picker-cursor.svg", scale(14, 14))) {
  m_gradient = create_gradient_image(width(), height(), m_current_color);
  m_last_mouse_pos = get_mouse_pos(m_current_color);
}

void ColorSelectorValueSlider::set_color(const QColor& color) {
  if(color.hue() == m_current_color.hue()) {
    return;
  }
  m_current_color = color;
  m_gradient = create_gradient_image(width(), height(), m_current_color);
  m_last_mouse_pos = get_mouse_pos(color);
  update();
}

connection ColorSelectorValueSlider::connect_color_signal(
    const ColorSignal::slot_type& slot) const {
  return m_color_signal.connect(slot);
}

void ColorSelectorValueSlider::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    set_mouse_pos(event->pos());
    m_current_color = m_gradient.pixelColor(m_last_mouse_pos);
    m_color_signal(m_current_color);
  }
}

void ColorSelectorValueSlider::mouseMoveEvent(QMouseEvent* event) {
  if(event->buttons() == Qt::LeftButton) {
    set_mouse_pos(event->pos());
    m_current_color = m_gradient.pixelColor(m_last_mouse_pos);
    m_color_signal(m_current_color);
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
  m_gradient = create_gradient_image(width() - 2, height() - 2,
    m_current_color);
}

QPoint ColorSelectorValueSlider::get_mouse_pos(const QColor& color) {
  for(auto x = 0; x < m_gradient.width(); ++x) {
    for(auto y = 0; y < m_gradient.height(); ++y) {
      if(m_gradient.pixelColor(x, y).value() == color.value()) {
        return {x, y};
      }
    }
  }
  return m_last_mouse_pos;
}

void ColorSelectorValueSlider::set_mouse_pos(const QPoint& pos) {
  m_last_mouse_pos.setX(std::max(1, std::min(pos.x(), width() - 1)));
  m_last_mouse_pos.setY(std::max(1, std::min(pos.y(), height() - 1)));
  update();
}
