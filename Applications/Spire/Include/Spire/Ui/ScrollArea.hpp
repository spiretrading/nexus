#ifndef SPIRE_SCROLL_AREA_HPP
#define SPIRE_SCROLL_AREA_HPP
#include <QScrollArea>
#include <QTimer>

namespace Spire {

  class ScrollArea : public QScrollArea {
    public:

      ScrollArea(QWidget* parent = nullptr);

    protected:
      void leaveEvent(QEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;

    private:
      QTimer m_h_scroll_bar_timer;
      QTimer m_v_scroll_bar_timer;

      void fade_out_horizontal_scroll_bar();
      void fade_out_vertical_scroll_bar();
      bool is_within_horizontal_scroll_bar(const QPoint& pos);
      bool is_within_vertical_scroll_bar(const QPoint& pos);
      void set_scroll_bar_style(int handle_size);
  };
}

#endif
