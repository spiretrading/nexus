#include "Spire/Ui/ColonWidget.hpp"
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

ColonWidget::ColonWidget(QWidget* parent)
  : QWidget(parent),
    m_border_color("#C8C8C8") {}

void ColonWidget::set_default_style() {
  m_border_color = QColor("#C8C8C8");
  update();
}

void ColonWidget::set_active_style() {
  m_border_color = QColor("#4B23A0");
  update();
}

void ColonWidget::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.fillRect(0, 0, width(), height(), Qt::white);
  painter.fillRect(0, 0, width(), scale_height(1), m_border_color);
  painter.fillRect(0, height() - scale_height(1), width(),
    scale_height(1), m_border_color);
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  painter.setFont(font);
  painter.setPen(Qt::black);
  painter.drawText(QPoint(0, scale_height(17)), ":");
}
