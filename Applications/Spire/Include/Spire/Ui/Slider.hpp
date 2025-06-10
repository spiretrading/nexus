#ifndef SPIRE_SLIDER_HPP
#define SPIRE_SLIDER_HPP
#include <QBoxLayout>
#include <QLabel>
#include <QWidget>
#include "Spire/Spire/Decimal.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects the track. */
  using Track = StateSelector<void, struct TrackSelectorTag>;

  /** Selects the thumb. */
  using Thumb = StateSelector<void, struct ThumbSelectorTag>;

  /** Selects the rail of the track. */
  using TrackRail = StateSelector<void, struct TrackRailSelectorTag>;

  /** Selects the fill of the track. */
  using TrackFill = StateSelector<void, struct TrackFillSelectorTag>;
}

  /**
   * Represents a range of values along a track which can be horizontal or
   * vertical.
   */
  class Slider : public QWidget {
    public:

      /**
       * Signals that the current value is being submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const Decimal& submission)>;

      /**
       * Constructs a Slider with a LocalValueModel.
       * @param parent The parent widget.
       */
      explicit Slider(QWidget* parent = nullptr);

      /**
       * Constructs a Slider with a LocalValueModel.
       * @param modifiers The initial keyboard modifier increments.
       * @param parent The parent widget.
       */
      explicit Slider(QHash<Qt::KeyboardModifier, Decimal> modifiers,
        QWidget* parent = nullptr);

      /**
       * Constructs a Slider with an increment determined
       * by the model's increment.
       * @param current The model used for the current value.
       * @param parent The parent widget.
       */
      explicit Slider(std::shared_ptr<ScalarValueModel<Decimal>> current,
        QWidget* parent = nullptr);

      /**
       * Constructs a Slider
       * @param current The model used for the current value.
       * @param modifiers The initial keyboard modifier increments.
       * @param parent The parent widget.
       */
      Slider(std::shared_ptr<ScalarValueModel<Decimal>> current,
        QHash<Qt::KeyboardModifier, Decimal> modifiers,
        QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<ScalarValueModel<Decimal>>& get_current() const;

      /** Returns the size of a step. */
      Decimal get_step() const;

      /** Sets the size of a step. */
      void set_step(const Decimal& step);

      /**
       * Converts a pixel position to the corresponding slider value.
       * @param position The pixel position relative to the slider's coordinate
       *        system.
       * @return The corresponding slider value, guaranteed to be within
       *         the slider's minimum and maximum range.
       */
      Decimal to_value(const QPoint& position) const;

      /** Connects a slot to the value submission signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;

    private:
      struct SliderValueModel;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<SliderValueModel> m_current;
      QHash<Qt::KeyboardModifier, Decimal> m_modifiers;
      Qt::Orientation m_orientation;
      Decimal m_submission;
      Decimal m_step;
      QLabel* m_track_image_container;
      Box* m_track_fill;
      Box* m_track;
      Box* m_thumb;
      QWidget* m_body;
      QBoxLayout* m_track_layout;
      QImage m_track_image;
      QImage m_thumb_image;
      FocusObserver m_focus_observer;
      bool m_is_mouse_down;
      bool m_is_modified;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_track_style_connection;
      boost::signals2::scoped_connection m_thumb_icon_style_connection;
      boost::signals2::scoped_connection m_style_connection;

      Decimal get_increment(int modifier_flag) const;
      Decimal to_value(int position) const;
      int to_position(const Decimal& value) const;
      void set_current(const Decimal& value);
      void update_track();
      void update_thumb();
      void on_focus(FocusObserver::State state);
      void on_current(const Decimal& current);
      void on_track_style();
      void on_thumb_icon_style();
      void on_style();
  };
}

#endif
