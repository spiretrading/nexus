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
      ChartValue::Type m_x_axis_type;
      ChartValue::Type m_y_axis_type;
      ChartPoint m_top_left;
      ChartPoint m_bottom_right;
      ChartValue m_x_axis_range;
      ChartValue m_y_axis_range;
      QFont m_label_font;
      std::string m_timestamp_format;

      std::vector<ChartValue> get_axis_values(const ChartValue::Type& type,
        const ChartValue& range_start, const ChartValue& range_end);
      int value_to_pixel(const ChartValue& range_start,
        const ChartValue& range_end, const ChartValue& value,
        int widget_size);
      QString drawable_timestamp(const boost::posix_time::ptime& time);
      ChartValue get_step(const ChartValue::Type& value_type,
        const ChartValue& range);
  };
}

#endif
