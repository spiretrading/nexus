#ifndef SPIRE_MENU_BUTTON_HPP
#define SPIRE_MENU_BUTTON_HPP
#include <QTimer>
#include <QWidget>
#include "Spire/Ui/ContextMenu.hpp"
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

      /**
       * Sets the message of the empty state.
       * @param message The message displayed when the menu is empty.
       */
      void set_empty_message(const QString& message);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      QWidget* m_body;
      QString m_empty_message;
      ContextMenu* m_menu;
      EmptyState* m_empty_state;
      QTimer m_timer;
      bool m_is_mouse_down_on_button;
      int m_menu_border_size;

      int get_panel_minimum_width() const;
      EmptyState* make_empty_state();
      void show_panel();
      void hide_panel();
      void update_panel_minimum_width();
      void on_menu_window_style();
  };

  /**
   * Returns a newly constructed MenuButton displaying an Icon and no tooltip.
   * @param icon The icon used within the button.
   * @param parent The parent widget.
   */
  MenuButton* make_menu_icon_button(QImage icon, QWidget* parent = nullptr);

  /**
   * Returns a newly constructed MenuButton displaying an Icon and a tooltip.
   * @param icon The icon used within the button.
   * @param tooltip The text of the Tooltip to display.
   * @param parent The parent widget.
   */
  MenuButton* make_menu_icon_button(QImage icon, QString tooltip,
    QWidget* parent = nullptr);

  /**
   * Returns a newly constructed MenuButton displaying a text label.
   * @param label The button's text label.
   * @param parent The parent widget.
   */
  MenuButton* make_menu_label_button(QString label, QWidget* parent = nullptr);
}

#endif
