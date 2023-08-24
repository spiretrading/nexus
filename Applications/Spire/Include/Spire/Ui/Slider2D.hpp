#ifndef SPIRE_SLIDER_2D_HPP
#define SPIRE_SLIDER_2D_HPP
#include <QLabel>
#include <QWidget>
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Slider.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects the pad of the track. */
  using TrackPad = StateSelector<void, struct TrackPadSelectorTag>;
}

  /**
   * Represents a 2D slider.
   */
  class Slider2D : public QWidget {
    public:

      /**
       * Signals that the current value is being submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const QPoint& point)>;

      /**
       * Constructs a Slider2D using LocalValueModels.
       * @param parent The parent widget.
       */
      explicit Slider2D(QWidget* parent = nullptr);

      /**
       * Constructs a Slider2D with LocalValueModels.
       * @param modifiers The initial keyboard modifier increments.
       * @param parent The parent widget.
       */
      explicit Slider2D(QHash<Qt::KeyboardModifier, QPoint> modifiers,
        QWidget* parent = nullptr);

      /**
       * Constructs a Slider2D with an increment determined
       * by the model's increment.
       * @param current The model used for the current value.
       * @param parent The parent widget.
       */
      Slider2D(std::shared_ptr<ScalarValueModel<int>> current_x,
        std::shared_ptr<ScalarValueModel<int>> current_y,
        QWidget* parent = nullptr);

      /**
       * Constructs a Slider2D
       * @param current The model used for the current value.
       * @param modifiers The initial keyboard modifier increments.
       * @param parent The parent widget.
       */
      Slider2D(std::shared_ptr<ScalarValueModel<int>> current_x,
        std::shared_ptr<ScalarValueModel<int>> current_y,
        QHash<Qt::KeyboardModifier, QPoint> modifiers,
        QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<ScalarValueModel<int>>& get_current_x() const;

      const std::shared_ptr<ScalarValueModel<int>>& get_current_y() const;

      /** Returns the size of a step. */
      QPoint get_step_size() const;

      /** Sets the size of a step. */
      void set_step_size(const QPoint& step_size);

      /** Connects a slot to the value submission signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<ScalarValueModel<int>> m_current_x;
      std::shared_ptr<ScalarValueModel<int>> m_current_y;
      QHash<Qt::KeyboardModifier, QPoint> m_modifiers;
      QPoint m_submission;
      QPoint m_step_size;
      QLabel* m_track_image_container;
      Box* m_track;
      Box* m_thumb;
      QImage m_track_image;
      QImage m_thumb_image;
      FocusObserver m_focus_observer;
      bool m_is_mouse_down;
      bool m_is_modified;
      boost::signals2::scoped_connection m_current_x_connection;
      boost::signals2::scoped_connection m_current_y_connection;
      boost::signals2::scoped_connection m_track_style_connection;
      boost::signals2::scoped_connection m_thumb_icon_style_connection;

      QPoint get_increment(int modifier_flag) const;
      double to_value_x(int x) const;
      double to_value_y(int y) const;
      int to_position_x(double x) const;
      int to_position_y(double y) const;
      void set_current_x(double x);
      void set_current_y(double y);
      void on_focus(FocusObserver::State state);
      void on_current_x(int x);
      void on_current_y(int y);
      void on_track_style();
      void on_thumb_icon_style();
  };
}

#endif
