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
       * @param menu The menu that is revealed when the button is clicked.
       * @param parent The parent widget.
       */
      MenuButton(QWidget& body, OverlayPanel& menu, QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      OverlayPanel* m_menu;
      QTimer m_timer;
      bool m_is_mouse_down_on_button;
  };

  /**
   * Makes a MenuLabelButton.
   * @param label The text displayed on the button.
   * @param menu The menu that is revealed when the body is clicked.
   * @param parent The parent widget.
   */
  MenuButton* make_menu_label_button(QString label, OverlayPanel& menu,
    QWidget* parent = nullptr);
}

#endif
