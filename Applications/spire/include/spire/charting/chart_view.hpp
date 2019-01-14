#ifndef SPIRE_CHART_VIEW_HPP
#define SPIRE_CHART_VIEW_HPP
#include <QPen>
#include <QWidget>
#include "spire/charting/charting.hpp"
#include "spire/charting/chart_point.hpp"
#include "spire/ui/custom_qt_variants.hpp"

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

      //! Converts a point in pixels to a point on the chart.
      /*!
        \param point The point in pixels to convert.
        \return The corresponding point on the chart.
      */
      ChartPoint convert_pixels_to_chart(const QPoint& point) const;

      //! Converts a point on the chart to a point in pixels.
      /*!
        \param point The point on the chart to convert.
        \return The corresponding point in pixels.
      */
      QPoint convert_chart_to_pixels(const ChartPoint& point) const;

      //! Sets the position of the crosshair.
      /*!
        \param position The position to place the crosshair.
      */
      void set_crosshair(const ChartPoint& position);

      //! Sets the position of the crosshair.
      /*!
        \param position The position to place the crosshair.
      */
      void set_crosshair(const QPoint& position);

      //! Removes the crosshair from the chart.
      void reset_crosshair();

      //! Sets the visible region of the chart to display.
      /*!
        \param top_left The top left point to display.
        \param bottom_right The bottom right point to display.
      */
      void set_region(const ChartPoint& top_left,
        const ChartPoint& bottom_right);

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      ChartValue::Type m_x_axis_type;
      ChartValue::Type m_y_axis_type;
      ChartPoint m_top_left;
      ChartPoint m_bottom_right;
      int m_x_origin;
      int m_y_origin;
      ChartValue m_x_axis_step;
      ChartValue m_x_range;
      ChartValue m_y_axis_step;
      ChartValue m_y_range;
      QFont m_label_font;
      std::string m_timestamp_format;
      CustomVariantItemDelegate* m_item_delegate;
      std::optional<QPoint> m_crosshair_pos;
      QPen m_dashed_line_pen;
      QPen m_label_text_color;

      ChartValue calculate_step(ChartValue::Type value_type, ChartValue range);
      QString get_string(ChartValue::Type type, ChartValue value) const;
      QString get_timestamp(ChartValue value,
        const std::string& timestamp_format) const;
  };
}

#endif
