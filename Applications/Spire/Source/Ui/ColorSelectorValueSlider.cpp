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
  setStyleSheet(QString("border: %1px solid #C8C8C8").arg(scale_width(1)));
  m_gradient = create_gradient_image(width(), height(), m_current_color);
  m_last_mouse_pos = get_mouse_pos(m_current_color);
}

void ColorSelectorValueSlider::set_color(const QColor& color) {
  //m_last_mouse_x = get_mouse_x(color);
  update();
}

connection ColorSelectorValueSlider::connect_color_signal(
    const ColorSignal::slot_type& slot) const {
  return m_color_signal.connect(slot);
}

void ColorSelectorValueSlider::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    set_mouse_pos(event->pos());
    m_color_signal(m_gradient.pixelColor(m_last_mouse_pos));
  }
}

void ColorSelectorValueSlider::mouseMoveEvent(QMouseEvent* event) {
  if(event->buttons() == Qt::LeftButton) {
    set_mouse_pos(event->pos());
    m_color_signal(m_gradient.pixelColor(m_last_mouse_pos));
  }
}

void ColorSelectorValueSlider::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);
  auto painter = QPainter(this);
  painter.drawImage(1, 1, m_gradient);
  painter.drawImage(m_last_mouse_pos.x() - (m_handle.width() / 2),
    m_last_mouse_pos.y() - (m_handle.height() / 2), m_handle);
}

void ColorSelectorValueSlider::resizeEvent(QResizeEvent* event) {
  m_gradient = create_gradient_image(width() - 2, height() - 2,
    m_current_color);
}

QPoint ColorSelectorValueSlider::get_mouse_pos(const QColor& color) {
  for(auto i = 0; i < m_gradient.width(); ++i) {
    if(m_gradient.pixelColor(i, 0).value() == color.value()) {
      return {};
    }
  }
  return m_last_mouse_pos;
}

void ColorSelectorValueSlider::set_mouse_pos(const QPoint& pos) {
  m_last_mouse_pos.setX(std::max(0, std::min(pos.x(), width())));
  m_last_mouse_pos.setY(std::max(0, std::min(pos.y(), height())));
  update();
}
