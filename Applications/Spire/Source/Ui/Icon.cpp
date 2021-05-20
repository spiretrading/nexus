#include "Spire/Ui/Icon.hpp"
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"

using namespace boost;
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
    : QWidget(parent),
      m_icon(std::move(icon)),
      m_background_color(QColor::fromRgb(0xF5, 0xF5, 0xF5)),
      m_fill(QColor::fromRgb(0x75, 0x5E, 0xEC)) {
  setAttribute(Qt::WA_Hover);
  set_style(*this, DEFAULT_STYLE());
  connect_style_signal(*this, [=] { on_style(); });
}

QSize Icon::sizeHint() const {
  static auto size = scale(26, 26);
  return size;
}

void Icon::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.fillRect(rect(), m_background_color);
  auto icon = QPixmap::fromImage(m_icon);
  auto image_painter = QPainter(&icon);
  image_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  image_painter.fillRect(icon.rect(), m_fill);
  painter.drawPixmap((width() - icon.width()) / 2,
    (height() - icon.height()) / 2, icon);
  if(m_border_color) {
    draw_border(rect(), *m_border_color, &painter);
  }
}

void Icon::on_style() {
  auto& stylist = find_stylist(*this);
  auto computed_style = stylist.compute_style();
  m_background_color = QColor::fromRgb(0xF5, 0xF5, 0xF5);
  m_fill = QColor::fromRgb(0x75, 0x5E, 0xEC);
  m_border_color = none;
  for(auto& property : computed_style.get_properties()) {
    property.visit(
      [&] (const BackgroundColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_background_color = color;
          update();
        });
      },
      [&] (const Fill& fill) {
        stylist.evaluate(fill, [=] (auto color) {
          m_fill = color;
          update();
        });
      },
      [&] (const BorderTopColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_border_color = color;
          update();
        });
      });
  }
  update();
}
