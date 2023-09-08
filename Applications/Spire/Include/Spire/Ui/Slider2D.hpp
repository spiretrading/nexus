#ifndef SPIRE_SLIDER_2D_HPP
#define SPIRE_SLIDER_2D_HPP
#include <QLabel>
#include "Spire/Spire/Decimal.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Slider.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects the pad of the track. */
  using TrackPad = StateSelector<void, struct TrackPadSelectorTag>;
}

  /** Represents a 2D slider. */
  class Slider2D : public QWidget {
    public:
      
      /**
       * Signals that the current value is being submitted.
       * @param x The submitted x-coordinate value.
       * @param y The submitted y-coordinate value.
       */
      using SubmitSignal = Signal<void (const Decimal& x, const Decimal& y)>;

      /**
       * Constructs a Slider2D using LocalValueModels.
       * @param parent The parent widget.
       */
      explicit Slider2D(QWidget* parent = nullptr);

      /**
       * Constructs a Slider2D with LocalValueModels.
       * @param x_modifiers The initial horizontal keyboard modifier increments.
       * @param y_modifiers The initial vertical keyboard modifier increments.
       * @param parent The parent widget.
       */
      Slider2D(QHash<Qt::KeyboardModifier, Decimal> x_modifiers,
        QHash<Qt::KeyboardModifier, Decimal> y_modifiers,
        QWidget* parent = nullptr);

      /**
       * Constructs a Slider2D.
       * @param x_current The model used for the current x-coordinate value.
       * @param y_current The model used for the current y-coordinate value.
       * @param parent The parent widget.
       */
      Slider2D(std::shared_ptr<ScalarValueModel<Decimal>> x_current,
        std::shared_ptr<ScalarValueModel<Decimal>> y_current,
        QWidget* parent = nullptr);

      /**
       * Constructs a Slider2D.
       * @param x_current The model used for the current x-coordinate value.
       * @param y_current The model used for the current y-coordinate value.
       * @param x_modifiers The initial horizontal keyboard modifier increments.
       * @param y_modifiers The initial vertical keyboard modifier increments.
       * @param parent The parent widget.
       */
      Slider2D(std::shared_ptr<ScalarValueModel<Decimal>> x_current,
        std::shared_ptr<ScalarValueModel<Decimal>> y_current,
        QHash<Qt::KeyboardModifier, Decimal> x_modifiers,
        QHash<Qt::KeyboardModifier, Decimal> y_modifiers,
        QWidget* parent = nullptr);

      /** Returns the current x-coordinate value model. */
      const std::shared_ptr<ScalarValueModel<Decimal>>& get_x_current() const;

      /** Returns the current y-coordinate value model. */
      const std::shared_ptr<ScalarValueModel<Decimal>>& get_y_current() const;

      /** Returns the horizontal step. */
      const Decimal& get_x_step() const;

      /** Returns the vertical step. */
      const Decimal& get_y_step() const;

      /** Sets the horizontal step. */
      void set_x_step(const Decimal& step);

      /** Sets the vertical step. */
      void set_y_step(const Decimal& step);

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
      struct SliderValueModel;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<SliderValueModel> m_x_current;
      std::shared_ptr<SliderValueModel> m_y_current;
      QHash<Qt::KeyboardModifier, Decimal> m_x_modifiers;
      QHash<Qt::KeyboardModifier, Decimal> m_y_modifiers;
      Decimal m_x_submission;
      Decimal m_y_submission;
      Decimal m_x_step;
      Decimal m_y_step;
      QLabel* m_track_image_container;
      Box* m_track;
      Box* m_thumb;
      QImage m_track_image;
      QImage m_thumb_image;
      FocusObserver m_focus_observer;
      bool m_is_mouse_down;
      bool m_is_modified;
      boost::signals2::scoped_connection m_x_current_connection;
      boost::signals2::scoped_connection m_y_current_connection;
      boost::signals2::scoped_connection m_track_style_connection;
      boost::signals2::scoped_connection m_thumb_icon_style_connection;

      Decimal to_x_value(int x) const;
      Decimal to_y_value(int y) const;
      int to_x_position(const Decimal& x) const;
      int to_y_position(const Decimal& y) const;
      void set_x_current(const Decimal& x);
      void set_y_current(const Decimal& y);
      void on_focus(FocusObserver::State state);
      void on_x_current(const Decimal& x);
      void on_y_current(const Decimal& y);
      void on_track_style();
      void on_thumb_icon_style();
  };
}

#endif
