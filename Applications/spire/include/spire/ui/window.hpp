#ifndef SPIRE_WINDOW_HPP
#define SPIRE_WINDOW_HPP
#include <QColor>
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace spire {

  //! A customized window container for top-level windows.
  class window : public QWidget {
    public:

      //! Constructs a window.
      /*!
        \param body The widget displayed within the window.
        \param parent The parent widget to the window.
      */
      window(QWidget* body, QWidget* parent = nullptr);

      //! Sets the icon to display.
      /*!
        \param icon The icon to display when the window has focus.
      */
      void set_icon(const QImage& icon);

      //! Sets the icon to display.
      /*!
        \param icon The icon to display when the window has focus.
        \param icon The icon to display when the window lacks focus.
      */
      void set_icon(const QImage& icon, const QImage& unfocused_icon);

      //! Returns the body.
      QWidget* get_body();

    protected:
      void changeEvent(QEvent* event);

    private:
      QWidget* m_frame;
      QWidget* m_body;
      title_bar* m_title_bar;

      void on_maximize();
  };
}

#endif
