#ifndef SPIRE_WINDOW_HPP
#define SPIRE_WINDOW_HPP
#include <QColor>
#include <QWidget>

namespace spire {

  //! Base class for Spire windows that need a drop shadow and border.
  class window : public QWidget {
    public:

      //! Constructs a spire_window.
      /*!
        \param w The window that will have the window style applied to it.
        \param parent The parent widget to the window.
      */
      window(QWidget* w, QWidget* parent = nullptr);

      //! Constructs a window with a specific border color.
      /*!
        param w The window that will have the window style applied to it.
        \param border_color The color of the border.
        \param parent The parent widget to the window.
      */
      window(QWidget* w, const QColor& border_color,
        QWidget* parent = nullptr);

      //! Returns the frame's interior window.
      QWidget* get_window();

    private:
      QWidget* m_window;
      QWidget* m_border_widget;
      void set_border_color(const QColor& color);
  };
}

#endif
