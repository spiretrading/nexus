#include "Spire/Ui/Checkbox.hpp"
#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;

namespace {
  auto BOX_SIZE() {
    static auto size = scale(16, 16);
    return size;
  }

  auto PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }

  const auto& CHECK_ICON() {
    static auto icon = imageFromSvg(":/Icons/check.svg", scale(10, 8));
    return icon;
  }

  const auto& BOX_ENABLED_COLOR() {
    static auto color = QColor("#FFFFFF");
    return color;
  }

  const auto& BOX_DISABLED_COLOR() {
    static auto color = QColor("#F5F5F5");
    return color;
  }

  const auto& CHECK_ENABLED_COLOR() {
    static auto color = QColor("#4B23A0");
    return color;
  }

  const auto& CHECK_DISABLED_COLOR() {
    static auto color = QColor("#C8C8C8");
    return color;
  }

  const auto& TEXT_COLOR() {
    static auto color = QColor("#000000");
    return color;
  }
}

Checkbox::Checkbox(QWidget* parent)
  : QCheckBox("", parent) {}

Checkbox::Checkbox(const QString& label, QWidget* parent)
    : QCheckBox(label, parent),
      m_is_read_only(false) {
  setAttribute(Qt::WA_Hover);
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  setFont(font);
}

void Checkbox::keyPressEvent(QKeyEvent* event) {
  if(m_is_read_only) {
    event->accept();
    return;
  }
  QCheckBox::keyPressEvent(event);
}

void Checkbox::mousePressEvent(QMouseEvent* event) {
  event->accept();
  if(m_is_read_only) {
    return;
  }
  if(event->button() == Qt::LeftButton) {
    setChecked(!isChecked());
  }
}

void Checkbox::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.fillRect(QRect(get_box_position(), BOX_SIZE()), get_box_color());
  painter.fillRect(get_inner_box_rect(), get_inner_box_color());
  if(isChecked()) {
    auto icon = QPixmap::fromImage(CHECK_ICON());
    auto image_painter = QPainter(&icon);
    image_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    image_painter.fillRect(icon.rect(), get_check_color());
    painter.drawPixmap(get_check_position(), icon);
  }
  if(!text().isEmpty()) {
    painter.setPen(TEXT_COLOR());
    painter.drawText(QRect(get_text_position(), size()), 0, text());
  }
}

void Checkbox::set_read_only(bool is_read_only) {
  if(is_read_only != m_is_read_only) {
    m_is_read_only = is_read_only;
    update();
  }
}

QColor Checkbox::get_box_color() const {
  if(!isEnabled() || m_is_read_only || (!hasFocus() && !underMouse())) {
    return CHECK_DISABLED_COLOR();
  }
  return CHECK_ENABLED_COLOR();
}

QPoint Checkbox::get_box_position() const {
  auto box_y = (height() - BOX_SIZE().height()) / 2;
  if(layoutDirection() == Qt::LeftToRight) {
    return {0, box_y};
  }
  return {width() - BOX_SIZE().width(), box_y};
}

QColor Checkbox::get_check_color() const {
  if(!isEnabled()) {
    return CHECK_DISABLED_COLOR();
  }
  return CHECK_ENABLED_COLOR();
}

QPoint Checkbox::get_check_position() const {
  auto box_pos = get_box_position();
  return {box_pos.x() + ((BOX_SIZE().width() - CHECK_ICON().width()) / 2),
    box_pos.y() + ((BOX_SIZE().height() - CHECK_ICON().height()) / 2)};
}

QColor Checkbox::get_inner_box_color() const {
  if(!isEnabled() || m_is_read_only) {
    return BOX_DISABLED_COLOR();
  }
  return BOX_ENABLED_COLOR();
}

QRect Checkbox::get_inner_box_rect() const {
  return {get_box_position() + QPoint(scale_width(1), scale_height(1)),
    BOX_SIZE().shrunkBy({scale_width(1), scale_height(1), scale_width(1),
    scale_width(1)})};
}

QPoint Checkbox::get_text_position() const {
  auto text_y = (height() - font().pixelSize()) / 2;
  if(layoutDirection() == Qt::LeftToRight) {
    return {BOX_SIZE().width() + PADDING(), text_y};
  }
  return {0, text_y};
}
