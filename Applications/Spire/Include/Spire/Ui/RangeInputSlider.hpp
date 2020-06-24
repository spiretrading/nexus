#ifndef SPIRE_RANGE_INPUT_SLIDER_HPP
#define SPIRE_RANGE_INPUT_SLIDER_HPP
#include <QWidget>
#include "Spire/Spire/RangeInputModel.hpp"
#include "Spire/Spire/Signal.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a ranged slider with movable handles.
  class RangeInputSlider : public QWidget {
    public:

      //! Signals that a slider handle was moved by the user.
      using ChangedSignal = Signal<void (Scalar value)>;

      //! Constructs a RangeInputSlider.
      /*
        \param min_value The left-most (minimum) value of the slider.
        \param max_value The right-most (maximum) value of the slider.
        \param step The minimum step between consecutive slider values.
        \param parent The parent widget to the RangeInputSlider.
      */
      RangeInputSlider(Scalar min_value, Scalar max_value, Scalar step,
        QWidget* parent = nullptr);

      //! Sets the minimum value of the slider without triggering the changed
      //! signal.
      /*
        \param value The minimum value.
      */
      void set_min_value(Scalar value);

      //! Sets the maximum value of the slider without triggering the changed
      //! signal.
      /*
        \param value The maximum value.
      */
      void set_max_value(Scalar value);

      //! Sets the displayed histogram.
      /*
        \param histogram The histogram to display.
      */
      void set_histogram(const RangeInputModel::Histogram& histogram);

      //! Connects a slot to the minimum value changed signal.
      boost::signals2::connection connect_min_changed_signal(
        const ChangedSignal::slot_type& slot) const;

      //! Connects a slot to the maximum value changed signal.
      boost::signals2::connection connect_max_changed_signal(
        const ChangedSignal::slot_type& slot) const;

    protected:
      void leaveEvent(QEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable ChangedSignal m_min_changed_signal;
      mutable ChangedSignal m_max_changed_signal;
      Scalar m_min_value;
      Scalar m_max_value;
      Scalar m_step;
      Scalar m_current_min_value;
      Scalar m_current_max_value;
      RangeInputModel::Histogram m_histogram;
      bool m_is_dragging_handle;
      bool m_is_dragging_min;
      bool m_is_dragging_max;
      bool m_is_hovering_handle;
      bool m_is_hovering_min;
      bool m_is_hovering_max;
      int m_last_mouse_x;
      int m_mouse_offset;
      int m_min_handle_x;
      int m_max_handle_x;

      void draw_handle(QPainter& painter, bool is_highlighted, int x);
      Scalar map_x_to_value(int x);
      void move_min_handle(Scalar value);
      void move_max_handle(Scalar value);
      Scalar rounded_value(Scalar value);
  };
}

#endif
