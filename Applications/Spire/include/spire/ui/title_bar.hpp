#ifndef SPIRE_TITLE_BAR_HPP
#define SPIRE_TITLE_BAR_HPP
#include <QLabel>
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays a window's title bar.
  class title_bar : public QWidget {
    public:

      //! Constructs a title bar.
      /*!
        \param icon The window icon to display at the top-left corner.
        \param parent The parent widget.
      */
      title_bar(const QImage& icon, QWidget* parent = nullptr);

      //! Constructs a title bar.
      /*!
        \param icon The window icon to display at the top-left corner.
        \param unfocused_icon The icon to display when the window lacks focus.
        \param parent The parent widget.
      */
      title_bar(const QImage& icon, const QImage& unfocused_icon,
        QWidget* parent = nullptr);

    protected:
      void mouseDoubleClickEvent(QMouseEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      icon_button* m_icon;
      QLabel* m_title_label;
      icon_button* m_minimize_button;
      icon_button* m_maximize_button;
      icon_button* m_restore_button;
      icon_button* m_close_button;
      bool m_is_dragging;
      QPoint m_last_mouse_pos;

      void on_window_title_change(const QString& title);
      void on_minimize_button_press();
      void on_maximize_button_press();
      void on_restore_button_press();
      void on_close_button_press();
  };
}

#endif
