#ifndef SPIRE_CHART_VIEW_HPP
#define SPIRE_CHART_VIEW_HPP
#include <deque>
#include <optional>
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

      //! Sets the position of the crosshair and sets the status of the mouse
      //! buttons.
      /*!
        \param position The position to place the crosshair.
        \param buttons The status of the mouse buttons.
      */
      void set_crosshair(const QPoint& position, Qt::MouseButtons buttons);

      //! Removes the crosshair from the chart.
      void reset_crosshair();

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

      //! Shifts the view to a different region.
      /*!
        \param dx The number of pixels to move the region by along X.
        \param dy The number of pixels to move the region by along X.
      */
      void shift(int dx, int dy);

      //! Expands or shrinks the region to a different size.
      /*!
        \param factor The scaling factor.
        \detail The view is expanded if factor > 1 and shrinked if factor < 1.
      */
      void zoom(double factor);

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
      class PeggedCandlestick : public Candlestick {
        public:
          PeggedCandlestick(Candlestick candlestick, Scalar location);
          void set_location(Scalar location);
          Scalar get_location() const;

        private:
          Scalar m_location;
      };
      struct CandlestickLayout {
        QPoint open;
        QPoint close;
        QPoint high;
        QPoint low;
      };
      struct Gap {
        Scalar m_start;
        Scalar m_end;
      };
      struct GapInfo {
        int gap_count;
        Scalar total_gaps_value;
      };
      struct LineMouseOffset {
        QPoint m_first;
        QPoint m_second;
      };
      ChartModel* m_model;
      std::optional<Region> m_region;
      ChartPoint m_gap_adjusted_bottom_right;
      Scalar m_x_axis_step;
      Scalar m_y_axis_step;
      QFont m_label_font;
      QFontMetrics m_font_metrics;
      CustomVariantItemDelegate* m_item_delegate;
      std::optional<QPoint> m_crosshair_pos;
      Qt::MouseButtons m_mouse_buttons;
      QPen m_dashed_line_pen;
      QPen m_label_text_color;
      bool m_is_auto_scaled;
      double m_time_per_point;
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
      std::deque<PeggedCandlestick> m_visible_candlesticks;
      std::optional<PeggedCandlestick> m_left_candlestick;
      std::optional<PeggedCandlestick> m_right_candlestick;
      QtPromise<std::nullopt_t> m_data_update_promise;

      static GapInfo update_gaps(std::vector<ChartView::Gap>& gaps,
        std::vector<Candlestick>& candlesticks, Scalar start);
      static Scalar get_candlestick_time(const Candlestick&
        candlestick);
      //void draw_gap(QPainter& paitner, int start, int end);
      //void draw_point(QPainter& painter, const QColor& color,
      //  const QPoint& pos);
      //void draw_points(int id, QPainter& painter);
      void set_region(const Region& region);
      const std::optional<ChartView::Region>& get_region() const;
      bool intersects_gap(int x) const;
      void update_auto_scale();
      int update_intersection(const QPoint& mouse_pos);
      void update_origins();
      void update_selected_line_styles();
      void on_left_mouse_button_press(const QPoint& pos);
      void on_left_mouse_button_release();
      void on_right_mouse_button_press();
      void update_candlesticks();
      void drop_left_candlesticks();
      void drop_right_candlesticks();
      void load_left_candlesticks();
      void load_right_candlesticks();
      QtPromise<bool> load_first_candlestick();
      void insert_left_candlestick(Candlestick candlestick);
      void insert_right_candlestick(Candlestick candlestick);
      bool is_visible(Scalar location) const;
      std::optional<QPoint> to_pixel(const ChartPoint& point) const;
      std::optional<CandlestickLayout> get_candlestick_layout(
        const PeggedCandlestick& candlestick) const;
      QPoint get_top_right_pixel() const;
      std::optional<Scalar> get_time_by_location(Scalar location) const;
      std::optional<PeggedCandlestick*> get_leftmost_candlestick();
      std::optional<const PeggedCandlestick*> get_leftmost_candlestick() const;
      std::optional<PeggedCandlestick*> get_rightmost_candlestick();
      std::optional<const PeggedCandlestick*>
        get_rightmost_candlestick() const;
      bool is_empty() const;
  };
}

#endif
