#include "Spire/Ui/LabelDropDownMenuItem.hpp"
#include <QPainter>
#include <QPaintEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Spire;

namespace {
  const auto& FONT() {
    static const auto FONT = [] {
      auto font = QFont("Roboto");
      font.setPixelSize(scale_height(12));
      return font;
    }();
    return FONT;
  }

  auto LEFT_PADDING() {
    static const auto PADDING = scale_width(8);
    return PADDING;
  }

  auto RIGHT_PADDING() {
    static const auto PADDING = scale_width(12);
    return PADDING;
  }
}

LabelDropDownMenuItem::LabelDropDownMenuItem(QVariant value, QWidget* parent)
  : DropDownMenuItem2(std::move(value), parent) {}

void LabelDropDownMenuItem::enterEvent(QEvent* event) {
  update();
  DropDownMenuItem2::enterEvent(event);
}

void LabelDropDownMenuItem::leaveEvent(QEvent* event) {
  update();
  DropDownMenuItem2::leaveEvent(event);
}

void LabelDropDownMenuItem::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(underMouse() || is_highlighted()) {
    painter.fillRect(rect(), QColor("#F2F2FF"));
  } else {
    painter.fillRect(rect(), Qt::white);
  }
  auto metrics = QFontMetrics(FONT());
  auto shortened_text = metrics.elidedText(
    CustomVariantItemDelegate().displayText(get_value()), Qt::ElideRight,
    width() - RIGHT_PADDING());
  painter.setFont(FONT());
  painter.drawText(LEFT_PADDING(), metrics.height(), shortened_text);
  DropDownMenuItem2::paintEvent(event);
}
