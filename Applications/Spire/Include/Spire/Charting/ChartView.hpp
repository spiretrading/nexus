#ifndef SPIRE_CHART_VIEW_HPP
#define SPIRE_CHART_VIEW_HPP
#include <QPen>
#include <QWidget>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartPoint.hpp"
#include "Spire/Charting/TrendLineModel.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  //! Displays a chart of financial data.
  class ChartView : public QWidget {
    public:

      //! Represents a region defined by two points.
      struct Region {

        /** The top left point of the region. */
        ChartPoint m_top_left;

        /** The bottom right point of the region. */
        ChartPoint m_bottom_right;
      };

      //! Represents a non-contiguous gap between two ChartValues.
      struct Gap {

        /** The start value of the gap. */
        ChartValue m_start;

        /** The end value of the gap. */
        ChartValue m_end;
      };

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
      ChartPoint to_chart_point(const QPoint& point) const;

      //! Converts a point on the chart to a point in pixels.
      /*!
        \param point The point on the chart to convert.
        \return The corresponding point in pixels.
      */
      QPoint to_pixel(const ChartPoint& point) const;

      //! Sets the position of the crosshair and sets the status of the mouse
      //! buttons.
      /*!
        \param position The position to place the crosshair.
        \param buttons The status of the mouse buttons.
      */
      void set_crosshair(const ChartPoint& position, Qt::MouseButtons buttons);

      //! Sets the position of the crosshair and sets the status of the mouse
      //! buttons.
      /*!
        \param position The position to place the crosshair.
        \param buttons The status of the mouse buttons.
      */
      void set_crosshair(const QPoint& position, Qt::MouseButtons buttons);

      //! Removes the crosshair from the chart.
      void reset_crosshair();

      //! Returns the region of the ChartView.
      ChartView::Region get_region() const;

      //! Sets the visible region of the chart to display.
      /*!
        \param top_left The top left point to display.
        \param bottom_right The bottom right point to display.
      */
      void set_region(const ChartPoint& top_left,
        const ChartPoint& bottom_right);

      //! Returns true if auto scale is enabled, false otherwise.
      bool is_auto_scale_enabled() const;

      //! Sets auto scale on or off.
      /*!
        \param auto_scale Sets the auto scale on (true) or off (false).
      */
      void set_auto_scale(bool auto_scale);

      //! Returns true if draw mode is enabled, false otherwise.
      bool is_draw_mode_enabled() const;

      //! Sets draw mode on or off.
      /*!
        \param edit_mode Sets the draw mode on (true) or off (false).
      */
      void set_draw_mode(bool draw_mode);

      //! Sets the color to apply selected and new trend lines.
      /*!
        \param color The color to apply.
      */
      void set_trend_line_color(const QColor& color);

      //! Sets the style to apply to selected and new trend lines.
      /*!
        \param style The style to apply.
      */
      void set_trend_line_style(TrendLineStyle style);

      //! Deletes the selected trend lines.
      void remove_selected_trend_lines();

      //! Sets the multi-select feature on (true) or off (false).
      /*!
        \param on The on/off status of the multi-select feature.
      */
      void set_multi_select(bool on);

    protected:
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      enum class DrawState {
        HOVER,
        IDLE,
        LINE,
        NEW,
        OFF,
        POINT
      };
      ChartModel* m_model;
      ChartPoint m_top_left;
      ChartPoint m_bottom_right;
      ChartPoint m_gap_adjusted_bottom_right;
      QPoint m_bottom_right_pixel;
      ChartValue m_x_axis_step;
      ChartValue m_x_range;
      ChartValue m_y_axis_step;
      ChartValue m_y_range;
      QFont m_label_font;
      QFontMetrics m_font_metrics;
      CustomVariantItemDelegate* m_item_delegate;
      std::optional<QPoint> m_crosshair_pos;
      QPoint m_last_crosshair_pos;
      Qt::MouseButtons m_mouse_buttons;
      QPen m_dashed_line_pen;
      QPen m_label_text_color;
      std::vector<ChartValue> m_x_axis_values;
      std::vector<ChartValue> m_y_axis_values;
      bool m_is_auto_scaled;
      QtPromise<std::vector<Spire::Candlestick>> m_candlestick_promise;
      std::vector<Candlestick> m_candlesticks;
      TrendLineModel m_trend_line_model;
      DrawState m_draw_state;
      int m_current_trend_line_id;
      ChartPoint m_current_trend_line_point;
      ChartPoint m_current_stationary_point;
      QColor m_current_trend_line_color;
      TrendLineStyle m_current_trend_line_style;
      int m_line_hover_distance_squared;
      bool m_is_multi_select_enabled;
      std::vector<Gap> m_gaps;

      void draw_gap(QPainter& paitner, int start, int end);
      void draw_point(QPainter& painter, const QColor& color,
        const QPoint& pos);
      void draw_points(int id, QPainter& painter);
      bool intersects_gap(int x) const;
      void update_auto_scale();
      int update_intersection(const QPoint& mouse_pos);
      void update_origins();
      void update_selected_line_styles();
      void on_left_mouse_button_press(const QPoint& pos);
      void on_left_mouse_button_release();
      void on_right_mouse_button_press();
  };
}

#endif
