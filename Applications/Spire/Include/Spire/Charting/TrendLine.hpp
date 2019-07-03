#ifndef SPIRE_TREND_LINE_HPP
#define SPIRE_TREND_LINE_HPP
#include <tuple>
#include <QColor>
#include <QPainter>
#include "Spire/Charting/ChartPoint.hpp"

namespace Spire {

  //! Defines the TrendLine styles.
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
    /*!
      \param points The two points of the trend line.
      \param color The color of the trend line.
      \param style The style of the trend line.
    */
    TrendLine(const std::tuple<ChartPoint, ChartPoint>& points,
      const QColor& color, TrendLineStyle style);

    /** Returns <code>true</code> iff two TrendLines are equal. */
    bool operator ==(const TrendLine& rhs) const;

    /** Returns <code>true</code> iff two TrendLines are not equal. */
    bool operator !=(const TrendLine& rhs) const;
  };

  //! Draws a TrendLine using the provided QPainter.
  /*!
    \param painter The painter used for drawing the trend line.
    \param style The trend line style to draw.
    \param color The color of the trend line.
    \param x1 The x coordinate of the first point.
    \param y2 The y coordinate of the first point.
    \param x2 The x coordinate of the second point.
    \param y2 The y coordinate of the second point.
  */
  void draw_trend_line(QPainter& painter, TrendLineStyle style,
    const QColor& color, int x1, int y1, int x2, int y2);
}

#endif
