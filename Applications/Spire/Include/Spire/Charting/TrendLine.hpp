#ifndef SPIRE_TREND_LINE_HPP
#define SPIRE_TREND_LINE_HPP
#include <tuple>
#include <QColor>
#include "Spire/Charting/ChartPoint.hpp"

namespace Spire {

  //! Represents a chart trend line.
  struct TrendLine {

    //! The two points of the trend line.
    std::tuple<ChartPoint, ChartPoint> m_points;

    //! The color of the trend line.
    QColor m_color;

    //! The style of the trend line.
    Qt::PenStyle m_style;

    //! Constructs a TrendLine with two points at (0, 0), black color,
    //! and no style.
    TrendLine();

    //! Constructs a TrendLine.
    /*
      \param points The two points of the trend line.
      \param color The color of the trend line.
      \param style The style of the trend line.
    */
    TrendLine(const std::tuple<ChartPoint, ChartPoint>& points,
      const QColor& color, Qt::PenStyle style);
  };
}

#endif
