#ifndef SPIRE_SLIDER_2D_HPP
#define SPIRE_SLIDER_2D_HPP
#include <QGridLayout>
#include <QLabel>
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects the track. */
  using Track = StateSelector<void, struct TrackSelectorTag>;

  /** Selects the thumb. */
  using Thumb = StateSelector<void, struct ThumbSelectorTag>;
}

  /**
   * Represents a range of values along a track which can be horizontal or
   * vertical.
   */
  class Slider2D : public QWidget {
    public:

      /**
       * Signals that the current value is being submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const QPoint& submission)>;

      /**
       * Constructs a Slider2D using a LocalValueModel with an increment of 1.
       * @param parent The parent widget.
       */
      explicit Slider2D(QWidget* parent = nullptr);

      /**
       * Constructs a Slider2D with a LocalValueModel.
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
      explicit Slider2D(std::shared_ptr<ScalarValueModel<QPoint>> current,
        QWidget* parent = nullptr);

      /**
       * Constructs a Slider2D
       * @param current The model used for the current value.
       * @param modifiers The initial keyboard modifier increments.
       * @param parent The parent widget.
       */
      Slider2D(std::shared_ptr<ScalarValueModel<QPoint>> current,
        QHash<Qt::KeyboardModifier, QPoint> modifiers,
        QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<ScalarValueModel<QPoint>>& get_current() const;

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
      std::shared_ptr<ScalarValueModel<QPoint>> m_current;
      QHash<Qt::KeyboardModifier, QPoint> m_modifiers;
      QPoint m_submission;
      QPoint m_step_size;
      QLabel* m_track_body;
      Box* m_track;
      Box* m_thumb;
      //QBoxLayout* m_track_layout;
      FocusObserver m_focus_observer;
      bool m_is_dragging;
      bool m_is_modified;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_track_style_connection;
      boost::signals2::scoped_connection m_thumb_style_connection;
      boost::signals2::scoped_connection m_thumb_icon_style_connection;

      //int get_increment(int modifier_flag) const;
      //double get_range() const;
      //double to_value(int position) const;
      //int to_position(double value) const;
      //void set_current(double value);
      void on_focus(FocusObserver::State state);
      void on_current(const QPoint& current);
      void on_track_style();
      void on_thumb_style();
      void on_thumb_icon_style();
  };

  bool operator <(const QPoint& point1, const QPoint& point2);

  bool operator >(const QPoint& point1, const QPoint& point2);
}

#endif
