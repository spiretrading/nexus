#ifndef SPIRE_CHARTPOINT_HPP
#define SPIRE_CHARTPOINT_HPP
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartValue.hpp"

namespace Spire {

  /*! \struct ChartPoint
      \brief Stores a single point on a chart.
   */
  struct ChartPoint {

    //! The position on the x-axis.
    ChartValue m_x;

    //! The position on the y-axis.
    ChartValue m_y;

    //! Constructs a default ChartPoint at position (0, 0).
    ChartPoint();

    //! Constructs a ChartPoint.
    /*!
      \param x The position on the x-axis.
      \param y The position on the y-axis.
    */
    ChartPoint(ChartValue x, ChartValue y);
  };
}

#endif
