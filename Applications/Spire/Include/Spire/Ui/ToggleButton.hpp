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
       * @param parent The parent widget.
       */
      ToggleButton(QWidget* body, QWidget* parent = nullptr);

      /**
       * Constructs a ToggleButton.
       * @param body The widget displayed inside of the button.
       * @param current Whether the button is checked.
       * @param parent The parent widget.
       */
      ToggleButton(QWidget* body, std::shared_ptr<BooleanModel> current,
        QWidget* parent = nullptr);

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
      Button* m_button;
      std::shared_ptr<BooleanModel> m_current;
      boost::signals2::scoped_connection m_connection;

      void on_click();
      void on_update(bool current);
  };

  /**
   * Makes a ToggleButton whose body displays an icon.
   * @param icon The icon to display.
   * @param parent The parent widget.
   */
  ToggleButton* make_icon_toggle_button(QImage icon, QWidget* parent = nullptr);

  /**
   * Makes a ToggleButton whose body displays an icon.
   * @param icon The icon to display.
   * @param current Whether the button is checked.
   * @param parent The parent widget.
   */
  ToggleButton* make_icon_toggle_button(QImage icon,
    std::shared_ptr<BooleanModel> current, QWidget* parent = nullptr);

  /**
   * Makes a ToggleButton whose body displays an icon and displays a tooltip.
   * @param icon The icon used within the button.
   * @param tooltip The text of the Tooltip to display.
   * @param parent The parent widget.
   */
  ToggleButton* make_icon_toggle_button(
    QImage icon, QString tooltip, QWidget* parent = nullptr);

  /**
   * Makes a ToggleButton whose body displays an icon and displays a tooltip.
   * @param icon The icon used within the button.
   * @param current Whether the button is checked.
   * @param tooltip The text of the Tooltip to display.
   * @param parent The parent widget.
   */
  ToggleButton* make_icon_toggle_button(
    QImage icon, std::shared_ptr<BooleanModel> current, QString tooltip,
    QWidget* parent = nullptr);

  /**
   * Makes a ToggleButton whose body displays different icons when
   * checked/unchecked.
   * @param icon The icon displayed when unchecked.
   * @param checked_icon The icon displayed when checked.
   * @param current Whether the button is checked.
   * @param parent The parent widget.
   */
  ToggleButton* make_icon_toggle_button(QImage icon, QImage checked_icon,
    std::shared_ptr<BooleanModel> current, QWidget* parent = nullptr);
}

#endif
