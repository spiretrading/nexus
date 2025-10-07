#ifndef SPIRE_WINDOW_HPP
#define SPIRE_WINDOW_HPP
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects a highlighted widget. */
  using Highlighted = StateSelector<void, struct HighlightedTag>;
}

  /** A customized window container for top-level windows. */
  class Window : public QWidget {
    public:

      /** Signals that the window closed. */
      using ClosedSignal = Signal<void ()>;

      /**
       * Constructs a Window.
       * @param parent The window's parent widget.
       */
      explicit Window(QWidget* parent = nullptr);

      /**
       * Sets the icon to display.
       * @param icon The icon to display when the window has focus.
       */
      void set_icon(const QImage& icon);

      /**
       * Sets the icon to display using an SVG file with default size.
       * @param path Path to the SVG file to display.
       */
      void set_svg_icon(const QString& icon_path);

    protected:

      /**
       * Sets the Window's body widget iff there is no existing body.
       * @param body The body widget.
       */
      void set_body(QWidget* body);

      void closeEvent(QCloseEvent* event) override;
      bool event(QEvent* event) override;
      bool nativeEvent(
        const QByteArray& eventType, void* message, long* result) override;

    private:
      mutable ClosedSignal m_closed_signal;
      TitleBar* m_title_bar;
      QWidget* m_body;
      bool m_is_resizable;
      boost::optional<bool> m_is_bottom_border_mismatched;
      boost::optional<bool> m_has_maximize_attribute;
      boost::optional<QSize> m_frame_size;

      void show_system_menu(const QPoint& position);
      void on_highlighted(bool is_match);
      void on_screen_changed(QScreen* screen);
      void on_logical_dots_per_inch_changed();
      void set_window_attributes(bool is_resizeable);
  };
}

#endif
