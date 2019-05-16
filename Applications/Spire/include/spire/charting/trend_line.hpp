#ifndef SPIRE_TREND_LINE_HPP
#define SPIRE_TREND_LINE_HPP
#include <tuple>
#include <QColor>
#include "spire/charting/chart_point.hpp"

namespace Spire {

  //! Represents a chart trend line.
  struct TrendLine {

    //! The two points of the trend line.
    std::tuple<ChartPoint, ChartPoint> m_points;

    //! The color of the trend line.
    QColor m_color;

    //! The style of the trend line.
    Qt::PenStyle m_style;

    //! Constructs a TrendLine.
    TrendLine(const std::tuple<ChartPoint, ChartPoint>& points,
      const QColor& color, Qt::PenStyle style);
  };
}

#endif
