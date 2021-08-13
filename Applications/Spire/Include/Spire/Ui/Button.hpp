#ifndef SPIRE_BUTTON_HPP
#define SPIRE_BUTTON_HPP
#include "Spire/Styles/ComponentSelector.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects a pressed Button. */
  using Press = StateSelector<void, struct PressTag>;
}

  /** Represents a button built upon a box. */
  class Button : public QWidget {
    public:

      /** Signals that the button is clicked. */
      using ClickedSignal = Signal<void ()>;

      /**
       * Constructs a Button.
       * @param body The widget contained by the button.
       * @param parent The parent widget.
       */
      explicit Button(QWidget* body, QWidget* parent = nullptr);

      const QWidget& get_body() const;

      QWidget& get_body();

      /** Connects a slot to the click signal. */
      boost::signals2::connection connect_clicked_signal(
        const ClickedSignal::slot_type& slot) const;

    protected:
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      mutable ClickedSignal m_clicked_signal;
      QWidget* m_body;
      bool m_is_down;
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
   * @param tooltip_text The text of the Tooltip to display.
   * @param parent The parent widget.
   */
  Button* make_icon_button(QImage icon, QString tooltip_text,
    QWidget* parent = nullptr);

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

namespace Spire::Styles {
  template<>
  struct ComponentFinder<Button, Body> : BaseComponentFinder<Button, Body> {};
}

#endif
