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
       * Constructs a MenuButton.
       * @param body The widget contained by the button.
       * @param parent The parent widget.
       */
      explicit MenuButton(QWidget& body, QWidget* parent = nullptr);

      /** Returns the body. */
      QWidget& get_body();

      /** Returns the menu. */
      ContextMenu& get_menu();

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      QWidget* m_body;
      ContextMenu* m_menu;
      OverlayPanel* m_menu_window;
      QTimer m_timer;
      bool m_is_mouse_down_on_button;
      int m_menu_border_size;

      void show_menu();
      void update_menu_width();
      void on_menu_window_style();
  };
}

#endif
