#ifndef SPIRE_MENU_BUTTON_HPP
#define SPIRE_MENU_BUTTON_HPP
#include <QTimer>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents a button that can open a menu */
  class MenuButton : public QWidget {
    public:

      /**
       * The type of function used to build a menu used by MenuButton.
       * @param parent The parent of the menu.
       * @return The OverlayPanel that represents a menu.
       */
      using MenuBuilder = std::function<OverlayPanel* (QWidget* parent)>;

      /**
       * Constructs a MenuButton.
       * @param body The component that opens a menu.
       * @param menu_builder The MenuBuilder to use.
       * @param parent The parent widget.
       */
      MenuButton(QWidget* body, MenuBuilder menu_builder,
        QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      OverlayPanel* m_menu;
      QTimer m_timer;
      bool m_is_mouse_down_on_button_with_menu;
      bool m_is_mouse_down_on_button_without_menu;
  };

  /**
   * Makes a MenuLabelButton.
   * @param label The text displayed on the button.
   * @param menu_builder The MenuBuilder to use.
   * @param parent The parent widget.
   */
  MenuButton* make_menu_label_button(const QString& label,
    MenuButton::MenuBuilder menu_builder, QWidget* parent = nullptr);
}

#endif
