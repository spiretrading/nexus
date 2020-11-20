#ifndef SPIRE_CHARTING_HPP
#define SPIRE_CHARTING_HPP
#include <QPointF>
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  class CachedChartModel;
  class ChartModel;
  struct ChartPoint;
  class ChartView;
  class ChartingTechnicalsPanel;
  class ChartingWindow;
  class ColorPicker;
  class DropDownColorPicker;
  class StyleDropDownMenuItem;
  struct TrendLine;
  class TrendLineEditor;
  class TrendLineModel;
  class TrendLineStyleDropDownMenu;
  using Candlestick = Nexus::TechnicalAnalysis::Candlestick<Scalar, Scalar>;

  //! Returns the slope of the line represented by the given points.
  /*!
    \param first The first point of the line.
    \param second The second point of the line.
  */
  double slope(const QPointF& first, const QPointF& second);

  //! Returns the squared distance between the given points, as a real number.
  /*!
    \param first The first point.
    \param second The second point.
  */
  double distance_squared(const QPointF& first, const QPointF& second);

  //! Returns true if the given value is between:
  //! [start - threshold, end + threshold].
  /*!
    \param value The value to compare.
    \param start The starting value of the range.
    \param end The ending value of the range.
    \param threshold Value subtract from and added to the starting and ending
                      values of the range, respectively.
  */
  bool is_within_interval(double value, double start, double end,
    double threshold);

  //! Returns true if the given value is between [start, end].
  /*!
    \param value The value to compare.
    \param start The starting value of the range.
    \param end The ending value of the range.
  */
  bool is_within_interval(double value, double start, double end);

  //! Calculates the y-intercept of a line, given a point and the slope.
  /*!
    \param point A point on the line.
    \param slope The slope of the given line.
  */
  double y_intercept(const QPointF& point, double slope);

  //! Calculates the y value of a line at the given x value.
  /*!
    \param m The slope of the line.
    \param x The x value.
    \param b The y-intercept of the line.
  */
  double calculate_y(double m, double x, double b);

  //! Returns the shortest distance squared between a given point and two other
  //! points.
  /*!
    \param point The given point.
    \param first The first point.
    \param second The second point.
  */
  double closest_point_distance_squared(QPointF point, const QPointF& first,
    const QPointF& second);
}

#endif
