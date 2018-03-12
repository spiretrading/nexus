#ifndef SPIRE_WINDOW_HPP
#define SPIRE_WINDOW_HPP
#include <QString>
#include <QWidget>

namespace spire {

  //! Base class for Spire windows that need a drop shadow and border.
  class window : public QWidget {
    public:

      //! Constructs a spire_window.
      /*!
        \param widget The widget that will have the window style applied
                      to it.
        \param parent The parent widget to the window.
      */
      window(QWidget* widget, QWidget* parent = nullptr);

      //! Constructs a window with a specific border color.
      /*!
        param widget The widget that will have the window style applied
                      to it.
        \param color_hex The color of the border as a hex value (e.g., #D3A2F8).
        \param parent The parent widget to the window.
      */
      window(QWidget* widget, const QString& color_hex,
        QWidget* parent = nullptr);

      //! Returns the frame's interior window.
      QWidget* get_window();

    private:
      QWidget* m_window;
      QWidget* m_border_widget;
      void set_stylesheet(const QString& color_hex);
  };
}

#endif
