#include "spire/charting/trend_line.hpp"

using namespace Spire;

TrendLine::TrendLine()
    : m_style(TrendLineStyle::NULL_STYLE) {}

TrendLine::TrendLine(const std::tuple<ChartPoint, ChartPoint>& points,
    const QColor& color, TrendLineStyle style)
    : m_points(points),
      m_color(color),
      m_style(style) {}
