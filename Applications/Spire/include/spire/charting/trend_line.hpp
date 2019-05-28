#ifndef SPIRE_TREND_LINE_HPP
#define SPIRE_TREND_LINE_HPP
#include <tuple>
#include <QColor>
#include "spire/charting/chart_point.hpp"

namespace Spire {

  enum class TrendLineStyle {
    SOLID,
    SOLID_WIDE,
    DASHED,
    DASHED_WIDE,
    DOTTED,
    DOTTED_WIDE,
    NULL_STYLE
  };

  //! Represents a chart trend line.
  struct TrendLine {

    //! The two points of the trend line.
    std::tuple<ChartPoint, ChartPoint> m_points;

    //! The color of the trend line.
    QColor m_color;

    //! The style of the trend line.
    TrendLineStyle m_style;

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
      const QColor& color, TrendLineStyle style);
  };
}

#endif
