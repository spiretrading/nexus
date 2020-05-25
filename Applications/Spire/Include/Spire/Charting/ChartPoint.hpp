#ifndef SPIRE_CHART_POINT_HPP
#define SPIRE_CHART_POINT_HPP
#include "Spire/Charting/Charting.hpp"
#include "Spire/Spire/Scalar.hpp"

namespace Spire {

  /** Stores a single point on a chart. */
  struct ChartPoint {

    //! The position on the x-axis.
    Scalar m_x;

    //! The position on the y-axis.
    Scalar m_y;

    //! Constructs a point at position (0, 0).
    ChartPoint() = default;

    //! Constructs a ChartPoint.
    /*!
      \param x The position on the x-axis.
      \param y The position on the y-axis.
    */
    ChartPoint(Scalar x, Scalar y);

    /** Returns <code>true</code> iff two ChartPoints are equal. */
    bool operator ==(const ChartPoint& rhs) const;

    /** Returns <code>true</code> iff two ChartPoints are not equal. */
    bool operator !=(const ChartPoint& rhs) const;

    /** Returns the sum of two ChartPoints. */
    ChartPoint operator +(const ChartPoint& rhs) const;

    /** Adds a point from <i>this</i>. */
    ChartPoint& operator +=(const ChartPoint& rhs);

    /** Returns the difference of two ChartPoints. */
    ChartPoint operator -(const ChartPoint& rhs) const;

    /** Subtracts a point from <i>this</i>. */
    ChartPoint& operator -=(const ChartPoint& rhs);
  };
}

#endif
