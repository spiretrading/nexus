#ifndef SPIRE_BUTTON_HPP
#define SPIRE_BUTTON_HPP
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/ClickObserver.hpp"
#include "Spire/Ui/PressObserver.hpp"

namespace Spire {
namespace Styles {

  /** Selects a pressed Button. */
  using Press = StateSelector<void, struct PressTag>;
}

  /** Represents a button built upon a box. */
  class Button : public QWidget {
    public:

      /** Signals that the button is clicked. */
      using ClickSignal = Signal<void ()>;

      /**
       * Constructs a Button.
       * @param body The widget contained by the button.
       * @param parent The parent widget.
       */
      explicit Button(QWidget* body, QWidget* parent = nullptr);

      /** Returns the body of the Button. */
      const QWidget& get_body() const;

      /** Returns the body of the Button. */
      QWidget& get_body();

      /** Connects a slot to the click signal. */
      boost::signals2::connection connect_click_signal(
        const ClickSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      mutable ClickSignal m_click_signal;
      QWidget* m_body;
      ClickObserver m_click_observer;
      PressObserver m_press_observer;

      void on_press_start(PressObserver::Reason reason);
      void on_press_end(PressObserver::Reason reason);
  };

  /**
   * Returns a newly constructed Button displaying an Icon and no tooltip.
   * @param icon The icon used within the button.
   * @param parent The parent widget.
   */
  Button* make_icon_button(QImage icon, QWidget* parent = nullptr);

  /**
   * Returns a newly constructed Button displaying an Icon and a tooltip.
   * @param icon The icon used within the button.
   * @param tooltip The text of the Tooltip to display.
   * @param parent The parent widget.
   */
  Button* make_icon_button(
    QImage icon, QString tooltip, QWidget* parent = nullptr);

  /**
   * Returns a newly constructed DeleteIconButton.
   * @param parent The parent widget.
   */
  Button* make_delete_icon_button(QWidget* parent = nullptr);

  /**
   * Returns a newly constructed Button displaying a text label.
   * @param label The button's text label.
   * @param parent The parent widget.
   */
  Button* make_label_button(const QString& label, QWidget* parent = nullptr);
}

#endif
