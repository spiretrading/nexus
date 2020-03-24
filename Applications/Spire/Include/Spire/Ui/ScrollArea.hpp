#ifndef SPIRE_SCROLL_AREA_HPP
#define SPIRE_SCROLL_AREA_HPP
#include <QScrollArea>
#include <QTimer>

namespace Spire {

  //! Displays a scroll area with styled scroll bars.
  class ScrollArea : public QScrollArea {
    public:

      //! Constructs an empty ScrollArea.
      /*
        \param parent The parent widget.
      */
      ScrollArea(QWidget* parent = nullptr);

      //! Sets the widget displayed in the scroll area.
      /*
        \param widget The widget to display.
      */
      void setWidget(QWidget* widget);

    protected:
      bool eventFilter(QObject* watched, QEvent* event);
      void leaveEvent(QEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;

    private:
      QTimer m_h_scroll_bar_timer;
      QTimer m_v_scroll_bar_timer;

      void hide_horizontal_scroll_bar();
      void hide_vertical_scroll_bar();
      bool is_within_horizontal_scroll_bar(int pos_y);
      bool is_within_vertical_scroll_bar(int pos_x);
      void set_scroll_bar_style(int handle_size);
  };
}

#endif
