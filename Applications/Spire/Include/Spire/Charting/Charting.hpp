#ifndef SPIRE_CHARTING_HPP
#define SPIRE_CHARTING_HPP
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  class CachedChartModel;
  class ChartModel;
  struct ChartPoint;
  class ChartValue;
  class ChartView;
  class ChartingTechnicalsPanel;
  class ChartingWindow;
  class ColorPicker;
  class DropDownColorPicker;
  class StyleDropDownMenuItem;
  class StyleDropDownMenuList;
  struct TrendLine;
  class TrendLineEditor;
  class TrendLineModel;
  class TrendLineStyleDropDownMenu;
  using Candlestick =
    Nexus::TechnicalAnalysis::Candlestick<ChartValue, ChartValue>;

  //! Returns the slope of the line reprsented by the given points.
  /*!
    \param x1 The x-coordinate of the first point.
    \param y1 The y-cooridnate of the first point.
    \param x2 The x-coordinate of the second point.
    \param y2 The y-coordinate of the second point.
  */
  template <typename T>
  T slope(T x1, T y1, T x2, T y2) {
    return (y2 - y1) / (x2 - x1);
  }

  //! Returns the squared distance between the given points, as a real number.
  /*!
    \param x1 The x-coordinate of the first point.
    \param y1 The y-cooridnate of the first point.
    \param x2 The x-coordinate of the second point.
    \param y2 The y-coordinate of the second point.
  */
  template <typename T>
  T distance_squared(T x1, T y1, T x2, T y2) {
    return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
  }

  //! Returns true if the given value is between:
  //! [start - threshold, end + threshold].
  /*!
    \param value The value to compare.
    \param start The starting value of the range.
    \param end The ending value of the range.
    \param threshold Value subtract from and added to the starting and ending
                      values of the range, respectively.
  */
  template <typename T>
  bool is_within_interval(T value, T start, T end, T threshold) {
    if(start > end) {
      return is_within_interval(value, end, start, threshold);
    }
    return start - threshold <= value && value <= end + threshold;
  }

  //! Returns true if the given value is between [start, end].
  /*!
    \param value The value to compare.
    \param start The starting value of the range.
    \param end The ending value of the range.
  */
  template <typename T>
  bool is_within_interval(T value, T start, T end) {
    return is_within_interval(value, start, end, static_cast<T>(0));
  }

  //! Calculates the y-intercept of a line, given a point and the slope.
  /*!
    \param x The x-coordinate of the given point.
    \param y The y-coordinate of the given point.
    \param slpoe The slope ofthe given line.
  */
  template <typename T>
  T y_intercept(T x, T y,T slope) {
    return y - x * slope;
  }

  //! Calculates the y value of a line at the given x value.
  /*!
    \param m The slope of the line.
    \param x The x value.
    \param b The y-intercept of the line.
  */
  template <typename T>
  T calculate_y(T m, T x, T b) {
    return m * x + b;
  }

  //! Returns the shortest distance between a point and the endpoints of a
  //! TrendLine.
  /*!
    \param x The x-coordinate of the point.
    \param y The y-coordinate of the point.
    \param line The TrendLine.
  */
  template <typename T>
  T closest_point_distance_squared(T x, T y, const TrendLine& line) {
    auto pt1_distance = distance_squared(x, y,
      static_cast<T>(std::get<0>(line.m_points).m_x),
      static_cast<T>(std::get<0>(line.m_points).m_y));
    auto pt2_distance = distance_squared(x, y,
      static_cast<T>(std::get<1>(line.m_points).m_x),
      static_cast<T>(std::get<1>(line.m_points).m_y));
    return std::min(pt1_distance, pt2_distance);
  }
}

#endif
