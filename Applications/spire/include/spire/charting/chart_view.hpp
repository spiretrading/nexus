#ifndef SPIRE_CHART_VIEW_HPP
#define SPIRE_CHART_VIEW_HPP
#include <QWidget>
#include "spire/charting/charting.hpp"
#include "spire/charting/chart_point.hpp"

namespace Spire {

  //! Displays a chart of financial data.
  class ChartView : public QWidget {
    public:

      //! Constructs a ChartView.
      /*!
        \param x_axis_type The type of data represented on the x-axis.
        \param y_axis_type The type of data represented on the y-axis.
        \param parent Parent to this widget.
      */
      ChartView(ChartValue::Type x_axis_type, ChartValue::Type y_axis_type,
        QWidget* parent = nullptr);

      //! Sets the visible region of the chart to display.
      /*!
        \param top_left The top left point to display.
        \param bottom_right The bottom right point to display.
      */
      void set_region(ChartPoint top_left, ChartPoint bottom_right);

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      ChartPoint m_top_left;
      ChartPoint m_bottom_right;
  };
}

#endif
