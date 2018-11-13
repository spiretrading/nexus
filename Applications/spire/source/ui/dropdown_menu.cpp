#include "spire/ui/dropdown_menu.hpp"
#include <QPainter>
#include <QPaintEvent>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/ui.hpp"

using namespace Spire;

DropdownMenu::DropdownMenu(const std::initializer_list<QString>& items,
    QWidget* parent)
    : QWidget(parent),
      m_dropdown_image(imageFromSvg(":/icons/arrow-down.svg", scale(6, 4))) {}

void DropdownMenu::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(hasFocus()) {
    painter.fillRect(event->rect(), QColor("#4B23A0"));
  } else {
    painter.fillRect(event->rect(), QColor("#C8C8C8"));
  }
  painter.fillRect(1, 1, width() - 2, height() - 2, Qt::white);
  auto font = QFont("Roboto", scale_height(7), QFont::Normal);
  painter.setFont(font);
  auto metrics = QFontMetrics(font);
  painter.drawText(QPoint(scale_width(8),
    (height() / 2) + (metrics.ascent() / 2) - 1), currentText());
  painter.drawImage(
    QPoint(width() - (m_dropdown_image.width() + scale_width(8)),
    scale_height(11)), m_dropdown_image);
}
