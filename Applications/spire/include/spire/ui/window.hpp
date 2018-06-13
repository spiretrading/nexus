#ifndef SPIRE_WINDOW_HPP
#define SPIRE_WINDOW_HPP
#include <vector>
#include <boost/optional.hpp>
#include <QAbstractNativeEventFilter>
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace spire {

  //! A customized window container for top-level windows.
  class window : public QWidget, QAbstractNativeEventFilter {
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

#ifdef Q_OS_WIN
      bool nativeEventFilter(const QByteArray& event_type, void* message,
        long* result) override;
#endif

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      enum class active_resize_rect {
        TOP_LEFT,
        TOP,
        TOP_RIGHT,
        RIGHT,
        BOTTOM_RIGHT,
        BOTTOM,
        BOTTOM_LEFT,
        LEFT,
        NONE
      };
      active_resize_rect m_current_active_rect;
      struct resize_boxes {
        QRect m_top_left;
        QRect m_top;
        QRect m_top_right;
        QRect m_right;
        QRect m_bottom_right;
        QRect m_bottom;
        QRect m_bottom_left;
        QRect m_left;
      };
      std::unique_ptr<drop_shadow> m_shadow;
      QWidget* m_border;
      QWidget* m_body;
      title_bar* m_title_bar;
      boost::optional<resize_boxes> m_resize_boxes;
      QPoint m_last_mouse_pos;
      bool m_is_resizing;
      bool m_hovered;

      void handle_resize();
      void set_border_stylesheet(const QColor& color);
      void update_resize_boxes();
      void update_resize_cursor();
  };
}

#endif
