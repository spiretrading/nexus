#include "spire/charting/trend_line.hpp"

using namespace Spire;

TrendLine::TrendLine(const std::tuple<ChartPoint, ChartPoint>& points,
    const QColor& color, Qt::PenStyle style)
    : m_points(points),
      m_color(color),
      m_style(style) {}
