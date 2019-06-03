#include "Spire/Charting/TrendLine.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

TrendLine::TrendLine()
    : m_style(TrendLineStyle::NULL_STYLE) {}

TrendLine::TrendLine(const std::tuple<ChartPoint, ChartPoint>& points,
    const QColor& color, TrendLineStyle style)
    : m_points(points),
      m_color(color),
      m_style(style) {}

void Spire::draw_trend_line(QPainter& painter, TrendLineStyle style,
    const QColor& color, int x1, int y1, int x2, int y2) {
  auto pen = QPen();
  pen.setColor(color);
  if(style == TrendLineStyle::DASHED) {
    pen.setStyle(Qt::DashLine);
    pen.setDashPattern({static_cast<double>(scale_width(2)),
      static_cast<double>(scale_width(1))});
    pen.setWidth(2);
  } else if(style == TrendLineStyle::DASHED_WIDE) {
    pen.setStyle(Qt::DashLine);
    pen.setDashPattern({static_cast<double>(scale_width(2)),
      static_cast<double>(scale_width(1))});
    pen.setWidth(4);
  } else if(style == TrendLineStyle::DOTTED) {
    pen.setStyle(Qt::DotLine);
    pen.setDashPattern({static_cast<double>(scale_width(1)),
      static_cast<double>(scale_width(1))});
    pen.setWidth(2);
  } else if(style == TrendLineStyle::DOTTED_WIDE) {
    pen.setStyle(Qt::DotLine);
    pen.setDashPattern({static_cast<double>(scale_width(1)),
      static_cast<double>(scale_width(1))});
    pen.setWidth(4);
  } else if(style == TrendLineStyle::SOLID) {
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(2);
  } else if(style == TrendLineStyle::SOLID_WIDE) {
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(4);
  }
  pen.setCapStyle(Qt::FlatCap);
  painter.setPen(pen);
  painter.drawLine(x1, y1, x2, y2);
}
