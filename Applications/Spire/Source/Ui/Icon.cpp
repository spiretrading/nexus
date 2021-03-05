#include "Spire/Ui/Icon.hpp"
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;
using namespace Spire::Styles;

auto DEFAULT_STYLE() {
  auto style = StyleSheet();
  style.get(Any()).
    set(BackgroundColor(QColor::fromRgb(255, 255, 255))).
    set(Fill(QColor::fromRgb(117, 94, 236)));
  style.get(Hover()).
    set(BackgroundColor(QColor::fromRgb(227, 227, 227))).
    set(Fill(QColor::fromRgb(75, 35, 160)));
  style.get(Focus()).
    set(border_color(QColor::fromRgb(75, 35, 160))).
    set(border_size(scale_width(1)));
  style.get(Disabled()).
    set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
    set(border_size(0)).
    set(Fill(QColor::fromRgb(208, 208, 208)));
  return style;
}

Icon::Icon(QImage icon, QWidget* parent)
  : StyledWidget(parent),
    m_icon(std::move(icon)) {}

void Icon::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  auto icon = QPixmap::fromImage(m_icon);
  auto image_painter = QPainter(&icon);
  image_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  auto computed_style = compute_style();
  image_painter.fillRect(icon.rect(), );
  painter.drawPixmap((width() - icon.width()) / 2,
    (height() - icon.height()) / 2, icon);
  StyledWidget::paintEvent(event);
}
