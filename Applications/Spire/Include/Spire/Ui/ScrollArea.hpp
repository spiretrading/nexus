#ifndef SPIRE_SCROLL_AREA_HPP
#define SPIRE_SCROLL_AREA_HPP
#include <QScrollArea>
#include <QTimer>

namespace Spire {

  //! Displays a scroll area with styled scroll bars.
  class ScrollArea : public QScrollArea {
    public:

      //! Constructs an empty ScrollArea with static scroll bars.
      /*
        \param parent The parent widget.
      */
      explicit ScrollArea(QWidget* parent = nullptr);
      
      //! Constructs an empty ScrollArea.
      /*
        \param is_dynamic True if the scroll area should support dynamic-sized,
                          auto-hiding scroll bars. False for static scroll
                          bars.
        \param parent The parent widget.
      */
      explicit ScrollArea(bool is_dynamic, QWidget* parent = nullptr);

      //! Sets the scroll area's border style.
      /*
        \param width The border width.
        \param color The border color.
      */
      void set_border_style(int width, const QColor& color);

      //! Sets the widget displayed in the scroll area.
      /*
        \param widget The widget to display.
      */
      void setWidget(QWidget* widget);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void showEvent(QShowEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;

    private:
      bool m_is_dynamic;
      bool m_is_wheel_disabled;
      QTimer m_horizontal_scroll_bar_timer;
      QTimer m_vertical_scroll_bar_timer;
      double m_horizontal_scrolling_error;
      double m_vertical_scrolling_error;
      QColor m_border_color;
      int m_border_width;

      void hide_horizontal_scroll_bar();
      void hide_vertical_scroll_bar();
      bool is_within_opposite_scroll_bar(QScrollBar* scroll_bar, int pos,
        int scroll_size, int widget_size) const;
      void update_scrollbar_position(QScrollBar* scroll_bar, int delta,
        double& scrolling_error);
      void set_scroll_bar_style(int handle_size);
  };
}

#endif
