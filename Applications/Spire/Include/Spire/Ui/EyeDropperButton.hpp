#ifndef SPIRE_EYE_DROPPER_BUTTON_HPP
#define SPIRE_EYE_DROPPER_BUTTON_HPP
#include "Spire/Ui/ColorBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a button that uses the EyeDropper to pick a color. */
  class EyeDropperButton : public QWidget {
    public:

      /**
       * Signals that the current color is being submitted.
       * @param submission The submitted color.
       */
      using SubmitSignal = Signal<void (const QColor& submission)>;

      /**
       * Signals that the current color is being rejected.
       * @param color The rejected color.
       */
      using RejectSignal = Signal<void (const QColor& color)>;

      /**
       * Constructs an EyeDropperButton.
       * @param parent The parent widget.
       */
      explicit EyeDropperButton(QWidget* parent = nullptr);

      /** Returns the current color model. */
      const std::shared_ptr<ColorModel>& get_current() const;

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectSignal. */
      boost::signals2::connection connect_reject_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<ColorModel> m_current;
      ToggleButton* m_button;

      void on_checked(bool checked);
      void on_current(const QColor& current);
      void on_submit(const QColor& submission);
      void on_reject(const QColor& color);
  };
}

#endif
