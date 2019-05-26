#include "Spire/Charting/TrendLine.hpp"

using namespace Spire;

TrendLine::TrendLine()
    : m_style(Qt::NoPen) {}

TrendLine::TrendLine(const std::tuple<ChartPoint, ChartPoint>& points,
    const QColor& color, Qt::PenStyle style)
    : m_points(points),
      m_color(color),
      m_style(style) {}
