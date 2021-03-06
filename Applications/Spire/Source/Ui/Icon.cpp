#include "Spire/Ui/Icon.hpp"
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).set(Fill(QColor::fromRgb(0x75, 0x5E, 0xEC)));
    style.get(Hover()).set(Fill(QColor::fromRgb(0x4B, 0x23, 0xAB)));
    style.get(Disabled()).set(Fill(QColor::fromRgb(0xD0, 0xD0, 0xD0)));
    return style;
  }
}

Icon::Icon(QImage icon, QWidget* parent)
    : StyledWidget(parent),
      m_icon(std::move(icon)) {
  setAttribute(Qt::WA_Hover);
  set_style(DEFAULT_STYLE());
}

QSize Icon::sizeHint() const {
  static auto size = scale(26, 26);
  return size;
}

void Icon::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  auto computed_style = compute_style();
  auto background_color = [&] {
    if(auto background_color = Styles::find<BackgroundColor>(computed_style)) {
      return background_color->get_expression().as<QColor>();
    }
    return QColor::fromRgb(0xF5, 0xF5, 0xF5);
  }();
  painter.fillRect(rect(), background_color);
  auto icon = QPixmap::fromImage(m_icon);
  auto image_painter = QPainter(&icon);
  image_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  auto fill = [&] {
    if(auto fill = Styles::find<Fill>(computed_style)) {
      return fill->get_expression().as<QColor>();
    }
    return QColor::fromRgb(0x75, 0x5E, 0xEC);
  }();
  image_painter.fillRect(icon.rect(), fill);
  painter.drawPixmap((width() - icon.width()) / 2,
    (height() - icon.height()) / 2, icon);
  if(auto border_color = Styles::find<BorderTopColor>(computed_style)) {
    draw_border(rect(), border_color->get_expression().as<QColor>(), &painter);
  }
}
