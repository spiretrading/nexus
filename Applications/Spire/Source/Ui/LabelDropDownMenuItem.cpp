#include "Spire/Ui/LabelDropDownMenuItem.hpp"
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Spire;

namespace {
  const auto& FONT() {
    static auto font = [] {
      auto font = QFont("Roboto");
      font.setPixelSize(scale_height(12));
      return font;
    }();
    return font;
  }

  auto LEFT_PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }
  
  auto RIGHT_PADDING() {
    static auto padding = scale_width(12);
    return padding;
  }
}

LabelDropDownMenuItem::LabelDropDownMenuItem(QVariant value, QWidget* parent)
  : DropDownMenuItem2(std::move(value), parent) {}

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
}
