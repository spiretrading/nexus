#ifndef SPIRE_WINDOW_HPP
#define SPIRE_WINDOW_HPP
#include <vector>
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

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      std::unique_ptr<drop_shadow> m_shadow;
      QWidget* m_border;
      QWidget* m_body;
      title_bar* m_title_bar;
      QRect m_top_left_rect;
      QRect m_top_rect;
      QRect m_top_right_rect;
      QRect m_right_rect;
      QRect m_bottom_right_rect;
      QRect m_bottom_rect;
      QRect m_bottom_left_rect;
      QRect m_left_rect;
      bool m_first_show;

      void calculate_resize_rects();
      void set_border_stylesheet(const QColor& color);
  };
}

#endif
