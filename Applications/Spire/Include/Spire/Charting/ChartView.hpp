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

        bool operator ==(const Region& rhs) const;

        bool operator !=(const Region& rhs) const;
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

      //! Returns the visible Region.
      const Region& get_region() const;

      //! Sets the visible region of the chart to display.
      /*!
        \param region The region the chart will display.
      */
      void set_region(const Region& region);

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
      struct Gap {
        Scalar m_start;
        Scalar m_end;
      };
      struct LineMouseOffset {
        QPoint m_first;
        QPoint m_second;
      };
      ChartModel* m_model;
      Region m_region;
      Region m_extended_region;
      QPoint m_bottom_right_pixel;
      Scalar m_x_axis_step;
      Scalar m_x_range;
      Scalar m_y_axis_step;
      Scalar m_y_range;
      QFont m_label_font;
      QFontMetrics m_font_metrics;
      CustomVariantItemDelegate* m_item_delegate;
      std::optional<QPoint> m_crosshair_pos;
      Qt::MouseButtons m_mouse_buttons;
      QPen m_dashed_line_pen;
      QPen m_label_text_color;
      std::vector<Scalar> m_x_axis_values;
      std::vector<Scalar> m_y_axis_values;
      bool m_is_auto_scaled;
      QtPromise<void> m_region_updates;
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
      std::optional<LineMouseOffset> m_line_mouse_offset;
      std::vector<Gap> m_gaps;

      static QPoint to_pixel(const Region& region, const QSize& size,
        const std::vector<Gap>& gaps, const ChartPoint& point);
      void commit_region(const Region& region);
      QtPromise<void> load_region(Region region, Scalar density,
        std::vector<Candlestick> candlesticks, std::vector<Gap> gaps);
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

  //! Adjusts the visible region of a ChartView by a QPoint.
  /*!
    \param view The ChartView to adjust.
    \param offset The offset to adjust the region by.
  */
  void translate(ChartView& view, const QPoint& offset);

  //! Zooms the visible region of a ChartView by a factor.
  /*!
    \param view The ChartView to zoom.
    \param factor The factor to zoom by.
  */
  void zoom(ChartView& view, double factor);
}

#endif
