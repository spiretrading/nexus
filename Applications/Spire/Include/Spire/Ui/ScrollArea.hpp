#ifndef SPIRE_SCROLL_AREA_HPP
#define SPIRE_SCROLL_AREA_HPP
#include <QScrollArea>
#include <QTimer>

namespace Spire {

  class ScrollArea : public QScrollArea {
    public:

      ScrollArea(QWidget* parent = nullptr);

      void setWidget(QWidget* widget);

    protected:
      bool eventFilter(QObject* watched, QEvent* event);
      void wheelEvent(QWheelEvent* event) override;

    private:
      QTimer m_h_scroll_bar_timer;
      QTimer m_v_scroll_bar_timer;

      void fade_out_horizontal_scroll_bar();
      void fade_out_vertical_scroll_bar();
      bool is_within_horizontal_scroll_bar(int pos_y);
      bool is_within_vertical_scroll_bar(int pos_x);
      void set_scroll_bar_style(int handle_size);
  };
}

#endif
