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
  if(m_icon.isNull()) {
    return;
  }
  auto painter = QPainter(this);
  m_painter.paint(painter);
  auto icon = QPixmap::fromImage(m_icon);
  auto image_painter = QPainter(&icon);
  image_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  if(m_fill) {
    image_painter.fillRect(icon.rect(), *m_fill);
  }
  painter.drawPixmap((width() - icon.width()) / 2,
    (height() - icon.height()) / 2, icon);
}

void Icon::on_style() {
  auto& stylist = find_stylist(*this);
  m_fill = QColor(0x755EEC);
  for(auto& property : stylist.get_computed_block()) {
    apply(property, m_painter, stylist);
    property.visit(
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
      });
  }
}
