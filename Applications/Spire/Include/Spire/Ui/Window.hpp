#ifndef SPIRE_WINDOW_HPP
#define SPIRE_WINDOW_HPP
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! A customized window container for top-level windows.
  class Window : public QWidget {
    public:

      //! Signals that the window closed.
      using ClosedSignal = Signal<void ()>;

      //! Constructs a Window.
      /*!
        \param parent The window's parent widget.
      */
      explicit Window(QWidget* parent = nullptr);

      //! Sets the icon to display.
      /*!
        \param icon The icon to display when the window has focus.
      */
      void set_icon(const QImage& icon);

      //! Sets the icon to display using an SVG file with default size.
      /*!
        \param path Path to the SVG file to display.
      */
      void set_svg_icon(const QString& icon_path);

      //! Sets the Window's minimum size.
      /*!
        \param size The Window's minimum size.
      */
      void set_minimum_size(const QSize& size);

    protected:

      //! Resizes the Window's body iff the body is not null.
      /*!
        \param size The size of the Window's body.
      */
      void resize_body(const QSize& size);

      //! Sets the Window's body widget iff there is no existing body.
      /*!
        \param body The body widget.
      */
      void set_body(QWidget* body);

      //! Sets the Window's body widget with an initial size iff there is no
      //! existing body.
      /*!
        \param body The body widget.
        \param size The body's initial size.
      */
      void set_body(QWidget* body, const QSize& size);

      //! Sets the Window to have a fixed size body iff there is no existing
      //! body. This removes the maximize button and resize cursors.
      //! The Window's layout automatically adjusts to include the size of the
      //! body.
      /*!
        \param body The window's body widget.
        \param size The fixed size of the Window's body.
      */
      void set_fixed_body(QWidget* body, const QSize& size);

      void changeEvent(QEvent* event) override;
      void closeEvent(QCloseEvent* event) override;
      bool event(QEvent* event) override;
      bool nativeEvent(const QByteArray& eventType, void* message,
        long* result) override;

    private:
      mutable ClosedSignal m_closed_signal;
      TitleBar* m_title_bar;
      QWidget* m_body;
      bool m_is_resizable;
      boost::optional<QSize> m_frame_size;

      void on_screen_changed(QScreen* screen);
      void set_window_attributes(bool is_resizeable);
  };
}

#endif
