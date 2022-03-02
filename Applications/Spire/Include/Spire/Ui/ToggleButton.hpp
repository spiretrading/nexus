#ifndef SPIRE_TOGGLE_BUTTON_HPP
#define SPIRE_TOGGLE_BUTTON_HPP
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a button that toggles between two states. */
  class ToggleButton : public QWidget {
    public:

      /**
       * Constructs a ToggleButton using a local model initialized to
       * <code>false</code>.
       * @param body The widget displayed inside of the button.
       * @param tooltip The contents of the tooltip.
       * @param parent The parent widget.
       */
      ToggleButton(QWidget* body, QString tooltip, QWidget* parent = nullptr);

      /**
       * Constructs a ToggleButton using a local model initialized to
       * <code>false</code> and no tooltip.
       * @param body The widget displayed inside of the button.
       * @param parent The parent widget.
       */
      explicit ToggleButton(QWidget* body, QWidget* parent = nullptr);

      /**
       * Constructs a ToggleButton.
       * @param body The widget displayed inside of the button.
       * @param current Whether the button is checked.
       * @param tooltip The contents of the tooltip.
       * @param parent The parent widget.
       */
      ToggleButton(QWidget* body, std::shared_ptr<BooleanModel> current,
        QString tooltip, QWidget* parent = nullptr);

      /** Returns the body. */
      const QWidget& get_body() const;

      /** Returns the body. */
      QWidget& get_body();

      /**
       * Returns the current value, <code>true</code> iff this button is
       * checked.
       */
      const std::shared_ptr<BooleanModel>& get_current() const;

    private:
      QWidget* m_body;
      std::shared_ptr<BooleanModel> m_current;
  };

  /**
   * Makes a ToggleButton whose body displays an icon and no tooltip.
   * @param icon The icon to display.
   * @param parent The parent widget.
   */
  ToggleButton* make_icon_toggle_button(QImage icon, QWidget* parent = nullptr);

  /**
   * Makes a ToggleButton whose body displays an icon.
   * @param icon The icon used within the button.
   * @param tooltip The text of the Tooltip to display.
   * @param parent The parent widget.
   */
  ToggleButton* make_icon_toggle_button(
    QImage icon, QString tooltip, QWidget* parent = nullptr);
}

#endif
