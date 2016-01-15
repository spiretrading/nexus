#ifndef SPIRE_POINTCHARTPLOT_HPP
#define SPIRE_POINTCHARTPLOT_HPP
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartPlot.hpp"
#include "Spire/Charting/ChartValue.hpp"

namespace Spire {

  /*! \class PointChartPlot
      \brief Implements a ChartPlot for a single 2d point.
   */
  class PointChartPlot : public ChartPlot {
    public:

      //! Constructs a PointChartPlot at the origin.
      PointChartPlot();

      //! Constructs a PointChartPlot.
      /*!
        \param x The x-value.
        \param y The y-value.
      */
      PointChartPlot(ChartValue x, ChartValue y);

      virtual ~PointChartPlot();

      //! Returns the x-value.
      ChartValue GetX() const;

      //! Sets the x-value.
      /*!
        \param value The x-value to represent.
      */
      void SetX(ChartValue value);

      //! Returns the y-value.
      ChartValue GetY() const;

      //! Sets the y-value.
      /*!
        \param value The y-value to represent.
      */
      void SetY(ChartValue value);

      virtual boost::signals2::connection ConnectUpdateSignal(
        const UpdateSignal::slot_function_type& slot) const;

    private:
      ChartValue m_x;
      ChartValue m_y;
      mutable UpdateSignal m_updateSignal;
  };
}

#endif
