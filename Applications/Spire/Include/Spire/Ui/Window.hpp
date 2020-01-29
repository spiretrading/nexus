#ifndef SPIRE_WINDOW_HPP
#define SPIRE_WINDOW_HPP
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

      //! Sets the icons to display.
      /*!
        \param icon The icon to display when the window has focus.
        \param unfocused_icon The icon to display when the window lacks focus.
      */
      void set_icon(const QImage& icon, const QImage& unfocused_icon);

      //! Sets the icon to display using an SVG file with default size.
      /*!
        \param path Path to the SVG file to display.
      */
      void set_svg_icon(const QString& icon_path);

      //! Sets the icons to display using an SVG file with default sizes.
      /*!
        \param icon_path Path to the SVG file to display when the window has
               focus.
        \param unfocused_icon_path Path to the SVG file to display when the
               window lacks focus.
      */
      void set_svg_icon(const QString& icon_path,
        const QString& unfocused_icon_path);

    protected:

      //! Resizes the Window, adding the TitleBar's height to provided size.
      /*!
        \param size The size of the Window's body.
      */
      void resize_body(const QSize& size);

      //! Sets the Window to have a fixed size, removing the maximize button
      //! and resize cursors. Adds the TitleBar's height to the provided size.
      /*!
        \param size The fixed size of the Window's body.
      */
      void set_fixed_body_size(const QSize& size);

      void changeEvent(QEvent* event) override;
      void closeEvent(QCloseEvent* event) override;
      bool event(QEvent* event) override;
      bool nativeEvent(const QByteArray& eventType, void* message,
        long* result) override;

    private:
      mutable ClosedSignal m_closed_signal;
      TitleBar* m_title_bar;
      int m_resize_area_width;
      bool m_is_resizeable;

      void on_screen_changed(QScreen* screen);
      void set_window_attributes(bool is_resizeable);
  };
}

#endif
