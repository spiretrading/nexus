#ifndef SPIRE_CHART_VIEW_HPP
#define SPIRE_CHART_VIEW_HPP
#include <QPen>
#include <QWidget>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartPoint.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  //! Displays a chart of financial data.
  class ChartView : public QWidget {
    public:

      //! Constructs a ChartView.
      /*!
        \param model The model containing the data to display.
        \param parent Parent to this widget.
      */
      ChartView(ChartModel& model, QWidget* parent = nullptr);

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

      //! Returns a pair (top left, bottom right) of ChartPoints
      //! representing the region of the ChartView.
      std::tuple<ChartPoint, ChartPoint> get_region() const;

      //! Sets the visible region of the chart to display.
      /*!
        \param top_left The top left point to display.
        \param bottom_right The bottom right point to display.
      */
      void set_region(const ChartPoint& top_left,
        const ChartPoint& bottom_right);

      //! Sets auto scale on or off.
      /*!
        \param auto_scale Sets the auto scale on (true) or off (false).
      */
      void set_auto_scale(bool auto_scale);

    protected:
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      ChartModel* m_model;
      ChartPoint m_top_left;
      ChartPoint m_bottom_right;
      int m_x_origin;
      int m_y_origin;
      ChartValue m_x_axis_step;
      ChartValue m_x_range;
      ChartValue m_y_axis_step;
      ChartValue m_y_range;
      QFont m_label_font;
      QFontMetrics m_font_metrics;
      CustomVariantItemDelegate* m_item_delegate;
      QCursor m_crosshair;
      std::optional<QPoint> m_crosshair_pos;
      QPen m_dashed_line_pen;
      QPen m_label_text_color;
      std::vector<ChartValue> m_x_axis_values;
      int m_x_axis_text_width;
      std::vector<ChartValue> m_y_axis_values;
      bool m_is_auto_scaled;
      QtPromise<std::vector<Spire::Candlestick>> m_candlestick_promise;
      std::vector<Candlestick> m_candlesticks;

      void update_auto_scale();
      void update_origins();
  };
}

#endif
