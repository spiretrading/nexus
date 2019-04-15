#ifndef SPIRE_WINDOW_HPP
#define SPIRE_WINDOW_HPP
#include <QMainWindow>
#include "spire/ui/ui.hpp"

namespace Spire {

  //! A customized window container for top-level windows.
  class Window : public QMainWindow {
    public:

      //! Signals that the window closed.
      using ClosedSignal = Signal<void ()>;

      //! Constructs a Window.
      /*
        \param body The widget displayed within the window.
        \param parent The window's parent widget.
      */
      explicit Window(QWidget* m_body, QWidget* parent = nullptr);

      //! Sets the icon to display.
      /*
        \param icon The icon to display when the window has focus.
      */
      void set_icon(const QImage& icon);

      //! Sets the icons to display.
      /*
        \param icon The icon to display when the window has focus.
        \param unfocused_icon The icon to display when the window lacks focus.
      */
      void set_icon(const QImage& icon, const QImage& unfocused_icon);

      //! Sets the icon to display using an SVG file with default size.
      /*
        \param path Path to the SVG file to display.
      */
      void set_svg_icon(const QString& icon_path);

      //! Sets the icons to display using an SVG file with default sizes.
      /*
        \param icon_path Path to the SVG file to display when the window has
                          focus.
        \param unfocused_icon_path Path to the SVG file to display when the
                                   window lacks focus.
      */
      void set_svg_icon(const QString& icon_path,
        const QString& unfocused_icon_path);

      void setFixedSize(int width, int height);

      void setFixedSize(const QSize& size);

      void setWindowFlag(Qt::WindowType flag, bool on = true);

      void setWindowFlags(Qt::WindowFlags type);

      //! Connects a slot to the window closed signal.
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    protected:
      void changeEvent(QEvent* event) override;
      void closeEvent(QCloseEvent* event) override;
      bool nativeEvent(const QByteArray &eventType, void *message,
        long *result) override;

    private:
      mutable ClosedSignal m_closed_signal;
      QWidget* m_container;
      TitleBar* m_title_bar;
      int m_resize_area_width;
      bool m_is_resizeable;

      void set_resizeable(bool resizeable);
  };
}

#endif
