#include "Spire/Charting/StyleDropDownMenuItem.hpp"
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto LINE_LENGTH() {
    static auto length = scale_width(52);
    return length;
  }

  auto PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }
}

StyleDropDownMenuItem::StyleDropDownMenuItem(TrendLineStyle style,
  QWidget* parent)
  : DropDownItem(QVariant::fromValue(style), parent),
    m_style(style) {}

void StyleDropDownMenuItem::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(underMouse() || is_highlighted()) {
    painter.fillRect(rect(), QColor("#F2F2FF"));
  } else {
    painter.fillRect(rect(), Qt::white);
  }
  auto y = height() / 2;
  draw_trend_line(painter, m_style, Qt::black, PADDING(), y,
    PADDING() + LINE_LENGTH(), y);
}
