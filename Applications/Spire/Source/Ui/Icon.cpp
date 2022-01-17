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
    style.get(Any()).set(Fill(QColor(0x755EEC)));
    style.get(Hover()).set(Fill(QColor(0x4B23AB)));
    style.get(Disabled()).set(Fill(QColor(0xD0D0D0)));
    return style;
  }
}

Icon::Icon(QImage icon, QWidget* parent)
    : QWidget(parent),
      m_icon(std::move(icon)),
      m_background_color(QColor(0xF5F5F5)),
      m_fill(QColor(0x755EEC)) {
  setAttribute(Qt::WA_Hover);
  set_style(*this, DEFAULT_STYLE());
  m_style_connection = connect_style_signal(*this, [=] { on_style(); });
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
  if(m_fill) {
    image_painter.fillRect(icon.rect(), *m_fill);
  }
  painter.drawPixmap((width() - icon.width()) / 2,
    (height() - icon.height()) / 2, icon);
  if(m_border_color) {
    auto border_region = QRegion(rect()).subtracted(rect().adjusted(
      scale_width(1), scale_height(1), -scale_width(1), -scale_height(1)));
    for(auto& rect : border_region) {
      painter.fillRect(rect, *m_border_color);
    }
  }
}

void Icon::on_style() {
  auto& stylist = find_stylist(*this);
  auto& block = stylist.get_computed_block();
  m_background_color = QColor(0xF5F5F5);
  m_fill = QColor(0x755EEC);
  m_border_color = none;
  for(auto& property : block) {
    property.visit(
      [&] (const BackgroundColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_background_color = color;
          update();
        });
      },
      [&] (const IconImage& image) {
        stylist.evaluate(image, [=] (auto image) {
          m_icon = std::move(image);
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
